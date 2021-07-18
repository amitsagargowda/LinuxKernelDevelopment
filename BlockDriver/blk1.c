#include <linux/version.h> 	/* LINUX_VERSION_CODE  */
#include <linux/blk-mq.h>	
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>

MODULE_LICENSE("GPL");

static int blk1_major = 0;
module_param(blk1_major, int, 0);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0);
static int nsectors = 1024;	/* How big the drive is */
module_param(nsectors, int, 0);

/*
 * The different "request modes" we can use.
 */
enum {
	RM_SIMPLE  = 0,	/* The extra-simple request function */
	RM_FULL    = 1,	/* The full-blown version */
	RM_NOQUEUE = 2,	/* Use make_request */
};
static int request_mode = RM_SIMPLE;
module_param(request_mode, int, 0);

/*
 * Minor number and partition management.
 */
#define SBULL_MINORS	16
#define MINOR_SHIFT	4
#define DEVNUM(kdevnum)	(MINOR(kdev_t_to_nr(kdevnum)) >> MINOR_SHIFT

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE	512

/*
 * After this much idle time, the driver will simulate a media change.
 */
#define INVALIDATE_DELAY	30*HZ

/*
 * The internal representation of our device.
 */
struct blk1_dev {
        int size;                       /* Device size in sectors */
        u8 *data;                       /* The data array */
        short users;                    /* How many users */
        short media_change;             /* Flag a media change? */
        spinlock_t lock;                /* For mutual exclusion */
	struct blk_mq_tag_set tag_set;	/* tag_set added */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
        struct timer_list timer;        /* For simulated media changes */
};

static struct blk1_dev *Devices = NULL;

static inline struct request_queue *
blk_generic_alloc_queue(make_request_fn make_request, int node_id)
{
	return (blk_alloc_queue(make_request, node_id));
}

/*
 * Handle an I/O request.
 */
static void blk1_transfer(struct blk1_dev *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector*KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if (write)
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

/*
 * The simple form of the request function.
 */
static blk_status_t blk1_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data* bd)   /* For blk-mq */
{
	struct request *req = bd->rq;
	struct blk1_dev *dev = req->rq_disk->private_data;
        struct bio_vec bvec;
        struct req_iterator iter;
        sector_t pos_sector = blk_rq_pos(req);
	void	*buffer;
	blk_status_t  ret;

	blk_mq_start_request (req);

	if (blk_rq_is_passthrough(req)) {
		printk (KERN_NOTICE "Skip non-fs request\n");
                ret = BLK_STS_IOERR;  //-EIO
			goto done;
	}
	rq_for_each_segment(bvec, req, iter)
	{
		size_t num_sector = blk_rq_cur_sectors(req);
		printk (KERN_NOTICE "Req dev %u dir %d sec %lld, nr %ld\n",
                        (unsigned)(dev - Devices), rq_data_dir(req),
                        pos_sector, num_sector);
		buffer = page_address(bvec.bv_page) + bvec.bv_offset;
		blk1_transfer(dev, pos_sector, num_sector,
				buffer, rq_data_dir(req) == WRITE);
		pos_sector += num_sector;
	}
	ret = BLK_STS_OK;
done:
	blk_mq_end_request (req, ret);
	return ret;
}


/*
 * Transfer a single BIO.
 */
static int blk1_xfer_bio(struct blk1_dev *dev, struct bio *bio)
{
	struct bio_vec bvec;
	struct bvec_iter iter;
	sector_t sector = bio->bi_iter.bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, iter) {
		char *buffer = kmap_atomic(bvec.bv_page) + bvec.bv_offset;
		blk1_transfer(dev, sector, (bio_cur_bytes(bio) / KERNEL_SECTOR_SIZE),
				buffer, bio_data_dir(bio) == WRITE);
		sector += (bio_cur_bytes(bio) / KERNEL_SECTOR_SIZE);
		kunmap_atomic(buffer);
	}
	return 0; /* Always "succeed" */
}

/*
 * Transfer a full request.
 */
static int blk1_xfer_request(struct blk1_dev *dev, struct request *req)
{
	struct bio *bio;
	int nsect = 0;
    
	__rq_for_each_bio(bio, req) {
		blk1_xfer_bio(dev, bio);
		nsect += bio->bi_iter.bi_size/KERNEL_SECTOR_SIZE;
	}
	return nsect;
}



/*
 * Smarter request function that "handles clustering".
 */
static blk_status_t blk1_full_request(struct blk_mq_hw_ctx * hctx, const struct blk_mq_queue_data * bd)
{
	struct request *req = bd->rq;
	int sectors_xferred;
	struct blk1_dev *dev = req->q->queuedata;
	blk_status_t  ret;

	blk_mq_start_request (req);
	if (blk_rq_is_passthrough(req)) {
		printk (KERN_NOTICE "Skip non-fs request\n");
		ret = BLK_STS_IOERR; //-EIO;
		goto done;
	}
	sectors_xferred = blk1_xfer_request(dev, req);
	ret = BLK_STS_OK; 
done:
	blk_mq_end_request (req, ret);

	return ret;
}



/*
 * The direct make request version.
 */
static blk_qc_t blk1_make_request(struct request_queue *q, struct bio *bio)
{
	struct blk1_dev *dev = bio->bi_disk->private_data;
	int status;

	status = blk1_xfer_bio(dev, bio);
	bio->bi_status = status;
	bio_endio(bio);
	return BLK_QC_T_NONE;
}


/*
 * Open and close.
 */

static int blk1_open(struct block_device *bdev, fmode_t mode)
{
	struct blk1_dev *dev = bdev->bd_disk->private_data;

	del_timer_sync(&dev->timer);
	spin_lock(&dev->lock);
	if (! dev->users) 
	{
		check_disk_change(bdev);
	}
	dev->users++;
	spin_unlock(&dev->lock);
	return 0;
}

static void blk1_release(struct gendisk *disk, fmode_t mode)
{
	struct blk1_dev *dev = disk->private_data;

	spin_lock(&dev->lock);
	dev->users--;

	if (!dev->users) {
		dev->timer.expires = jiffies + INVALIDATE_DELAY;
		add_timer(&dev->timer);
	}
	spin_unlock(&dev->lock);
}

/*
 * Look for a (simulated) media change.
 */
int blk1_media_changed(struct gendisk *gd)
{
	struct blk1_dev *dev = gd->private_data;
	
	return dev->media_change;
}

/*
 * Revalidate.  WE DO NOT TAKE THE LOCK HERE, for fear of deadlocking
 * with open.  That needs to be reevaluated.
 */
int blk1_revalidate(struct gendisk *gd)
{
	struct blk1_dev *dev = gd->private_data;
	
	if (dev->media_change) {
		dev->media_change = 0;
		memset (dev->data, 0, dev->size);
	}
	return 0;
}

/*
 * The "invalidate" function runs out of the device timer; it sets
 * a flag to simulate the removal of the media.
 */
void blk1_invalidate(struct timer_list * ldev)
{
	struct blk1_dev *dev = from_timer(dev, ldev, timer);

	spin_lock(&dev->lock);
	if (dev->users || !dev->data) 
		printk (KERN_WARNING "blk1: timer sanity check failed\n");
	else
		dev->media_change = 1;
	spin_unlock(&dev->lock);
}

/*
 * The ioctl() implementation
 */

int blk1_ioctl (struct block_device *bdev, fmode_t mode,
                 unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	struct blk1_dev *dev = bdev->bd_disk->private_data;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a virtual device, we have to make
		 * up something plausible.  So we claim 16 sectors, four heads,
		 * and calculate the corresponding number of cylinders.  We set the
		 * start of data at sector four.
		 */
		size = dev->size*(hardsect_size/KERNEL_SECTOR_SIZE);
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; /* unknown command */
}



/*
 * The device operations structure.
 */
static struct block_device_operations blk1_ops = {
	.owner           = THIS_MODULE,
	.open 	         = blk1_open,
	.release 	 = blk1_release,
	.media_changed   = blk1_media_changed, 
	.revalidate_disk = blk1_revalidate,
	.ioctl	         = blk1_ioctl
};

static struct blk_mq_ops mq_ops_simple = {
    .queue_rq = blk1_request,
};

static struct blk_mq_ops mq_ops_full = {
    .queue_rq = blk1_full_request,
};


/*
 * Set up our internal device.
 */
static void setup_device(struct blk1_dev *dev)
{
	/*
	 * Get some memory.
	 */
	memset (dev, 0, sizeof (struct blk1_dev));
	dev->size = nsectors*hardsect_size;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL) {
		printk (KERN_NOTICE "vmalloc failure.\n");
		return;
	}
	spin_lock_init(&dev->lock);
	
	/*
	 * The timer which "invalidates" the device.
	 */
        timer_setup(&dev->timer, blk1_invalidate, 0);

	
	/*
	 * The I/O queue, depending on whether we are using our own
	 * make_request function or not.
	 */
	switch (request_mode) {
	    case RM_NOQUEUE:
		dev->queue =  blk_generic_alloc_queue(blk1_make_request, NUMA_NO_NODE);
		if (dev->queue == NULL)
			goto out_vfree;
		break;

	    case RM_FULL:
		dev->queue = blk_mq_init_sq_queue(&dev->tag_set, &mq_ops_full, 128, BLK_MQ_F_SHOULD_MERGE);
		if (dev->queue == NULL)
			goto out_vfree;
		break;

	    case RM_SIMPLE:
		dev->queue = blk_mq_init_sq_queue(&dev->tag_set, &mq_ops_simple, 128, BLK_MQ_F_SHOULD_MERGE);
		if (dev->queue == NULL)
			goto out_vfree;
		break;
            default:
                printk(KERN_NOTICE "Bad request mode %d, using simple\n", request_mode);
	}
	blk_queue_logical_block_size(dev->queue, hardsect_size);
	dev->queue->queuedata = dev;
	/*
	 * And the gendisk structure.
	 */
	dev->gd = alloc_disk(SBULL_MINORS);
	if (! dev->gd) {
		printk (KERN_NOTICE "alloc_disk failure\n");
		goto out_vfree;
	}
	dev->gd->major = blk1_major;
	dev->gd->first_minor = SBULL_MINORS;
	dev->gd->fops = &blk1_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf (dev->gd->disk_name, 32, "blk1a");
	set_capacity(dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	add_disk(dev->gd);
	return;

  out_vfree:
	if (dev->data)
		vfree(dev->data);
}



static int __init blk1_init(void)
{
	/*
	 * Get registered.
	 */
	blk1_major = register_blkdev(blk1_major, "blk1");
	if (blk1_major <= 0) {
		printk(KERN_WARNING "blk1: unable to get major number\n");
		return -EBUSY;
	}
	/*
	 * Allocate the device array, and initialize each one.
	 */
	Devices = kmalloc(sizeof (struct blk1_dev), GFP_KERNEL);
	if (Devices == NULL)
		goto out_unregister;
	
	setup_device(Devices);
    
	return 0;

  out_unregister:
	unregister_blkdev(blk1_major, "blk1");
	return -ENOMEM;
}

static void blk1_exit(void)
{
	struct blk1_dev *dev = Devices;

	del_timer_sync(&dev->timer);
	if (dev->gd) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}

	if (dev->queue) {
		if (request_mode == RM_NOQUEUE)
			blk_put_queue(dev->queue);
		else
			blk_cleanup_queue(dev->queue);
	}

	if (dev->data)
		vfree(dev->data);

	unregister_blkdev(blk1_major, "blk1");
	kfree(Devices);
}
	
module_init(blk1_init);
module_exit(blk1_exit);

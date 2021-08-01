/* Address map of LCD controller registers */
#define LCD_CONTROLLER_BASE   0x01000D00
#define SIZE_REG     (*(volatile u32 *)(LCD_CONTROLLER_BASE))
#define HSYNC_REG    (*(volatile u32 *)(LCD_CONTROLLER_BASE + 4))
#define VSYNC_REG    (*(volatile u32 *)(LCD_CONTROLLER_BASE + 8))
#define CONF_REG     (*(volatile u32 *)(LCD_CONTROLLER_BASE + 12))
#define CTRL_REG     (*(volatile u32 *)(LCD_CONTROLLER_BASE + 16))
#define DMA_REG      (*(volatile u32 *)(LCD_CONTROLLER_BASE + 20))
#define STATUS_REG   (*(volatile u32 *)(LCD_CONTROLLER_BASE + 24))
#define CONTRAST_REG (*(volatile u32 *)(LCD_CONTROLLER_BASE + 28))
#define LCD_CONTROLLER_SIZE   32

/* Resources for the LCD controller platform device */
static struct resource myfb_resources[] = {
  [0] = {
    .start      = LCD_CONTROLLER_BASE,
    .end        = LCD_CONTROLLER_SIZE,
    .flags      = IORESOURCE_MEM,
  },
};

/* Platform device definition */
static struct platform_device myfb_device = {
  .name      = "myfb",
  .id        = 0,
  .dev       = {
    .coherent_dma_mask = 0xffffffff,
  },
  .num_resources = ARRAY_SIZE(myfb_resources),
  .resource      = myfb_resources,
};

/* Set LCD controller parameters */
static int
myfb_set_par(struct fb_info *info)
{
  unsigned long adjusted_fb_start;
  struct fb_var_screeninfo *var = &info->var;
  struct fb_fix_screeninfo *fix = &info->fix;

  /* Top 16 bits of HSYNC_REG hold HSYNC duration, next 8 contain
     the left margin, while the bottom 8 house the right margin */
  HSYNC_REG = (var->hsync_len << 16) |
              (var->left_margin << 8)|
              (var->right_margin);
  /* Top 16 bits of VSYNC_REG hold VSYNC duration, next 8 contain
     the upper margin, while the bottom 8 house the lower margin */
  VSYNC_REG = (var->vsync_len << 16)  |
              (var->upper_margin << 8)|
              (var->lower_margin);

  /* Top 16 bits of SIZE_REG hold xres, bottom 16 hold yres */
  SIZE_REG  = (var->xres << 16) | (var->yres);

  /* Set bits per pixel, pixel polarity, clock dividers for
     the pixclock, and color/monochrome mode in CONF_REG */
  /* ... */

  /* Fill DMA_REG with the start address of the frame buffer
     coherently allocated from myfb_probe(). Adjust this address
     to account for any offset to the start of screen area */
  adjusted_fb_start = fix->smem_start +
          (var->yoffset * var->xres_virtual + var->xoffset) *
          (var->bits_per_pixel) / 8;
  __raw_writel(adjusted_fb_start, (unsigned long *)DMA_REG);

  /*  Set the DMA burst length and watermark sizes in DMA_REG */
  /* ... */

  /* Set fixed information */
  fix->accel  = FB_ACCEL_NONE;       /* No hardware acceleration */
  fix->visual = FB_VISUAL_TRUECOLOR; /* True color mode */
  fix->line_length = var->xres_virtual * var->bits_per_pixel/8;

  return 0;
}

/* Enable LCD controller */
static void
myfb_enable_controller(struct fb_info *info)
{
  /* Enable LCD controller, start DMA, enable clocks and power
     by writing to CTRL_REG */
  /* ... */
}
/* Disable LCD controller */
static void
myfb_disable_controller(struct fb_info *info)
{
  /* Disable LCD controller, stop DMA, disable clocks and power
     by writing to CTRL_REG */
  /* ... */
}

/* Sanity check and adjustment of variables */
static int
myfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
  /* Round up to the minimum resolution supported by
     the LCD controller */
  if (var->xres < 64) var->xres = 64;
  if (var->yres < 64) var->yres = 64;

  /* ... */
  /* This hardware supports the RGB565 color format.
     See the section "Color Modes" for more details */
  if (var->bits_per_pixel == 16) {
    /* Encoding Red */
    var->red.length = 5;
    var->red.offset = 11;
    /* Encoding Green */
    var->green.length = 6;
    var->green.offset = 5;
    /* Encoding Blue */
    var->blue.length = 5;
    var->blue.offset = 0;
    /* No hardware support for alpha blending */
    var->transp.length = 0;
    var->transp.offset = 0;
  }
  return 0;
}

/* Blank/unblank screen */
static int
myfb_blank(int blank_mode, struct fb_info *info)
{
  switch (blank_mode) {
  case FB_BLANK_POWERDOWN:
  case FB_BLANK_VSYNC_SUSPEND:
  case FB_BLANK_HSYNC_SUSPEND:
  case FB_BLANK_NORMAL:
    myfb_disable_controller(info);
    break;
  case FB_BLANK_UNBLANK:
    myfb_enable_controller(info);
    break;
  }
  return 0;
}

/* Configure pseudo color palette map */
static int
myfb_setcolreg(u_int color_index, u_int red, u_int green,
               u_int blue, u_int transp, struct fb_info *info)
{
  if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
    /* Do any required translations to convert red, blue, green and
       transp, to values that can be directly fed to the hardware */
    /* ... */

    ((u32 *)(info->pseudo_palette))[color_index] =
           (red << info->var.red.offset)     |
           (green << info->var.green.offset) |
           (blue << info->var.blue.offset)   |
           (transp << info->var.transp.offset);
  }
  return 0;
}

/* Device-specific ioctl definition */
#define MYFB_SET_BRIGHTNESS _IOW('M', 3, int8_t)

/* Device-specific ioctl */
static int
myfb_ioctl(struct fb_info *info, unsigned int cmd,
           unsigned long arg)
{
  u32 blevel ;
  switch (cmd) {
    case MYFB_SET_BRIGHTNESS :
      copy_from_user((void *)&blevel, (void *)arg,
                     sizeof(blevel)) ;
      /* Write blevel to CONTRAST_REG */
      /* ... */
      break;
    default:
      return 鈥揈INVAL;
  }
  return 0;
}

/* The fb_ops structure */
static struct fb_ops myfb_ops = {
  .owner        = THIS_MODULE,
  .fb_check_var = myfb_check_var,/* Sanity check */
  .fb_set_par   = myfb_set_par,  /* Program controller registers */
  .fb_setcolreg = myfb_setcolreg,/* Set color map */
  .fb_blank     = myfb_blank,    /* Blank/unblank display */
  .fb_fillrect  = cfb_fillrect,  /* Generic function to fill
                                    rectangle */
  .fb_copyarea  = cfb_copyarea,  /* Generic function to copy area */
  .fb_imageblit = cfb_imageblit, /* Generic function to draw */
  .fb_ioctl     = myfb_ioctl,    /* Device-specific ioctl */
};

/* Platform driver's probe() routine */
static int __init
myfb_probe(struct platform_device *pdev)
{
  struct fb_info *info;
  struct resource *res;

  info = framebuffer_alloc(0, &pdev->dev);
  /* ... */
  /* Obtain the associated resource defined while registering the
     corresponding platform_device */
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  /* Get the kernel's sanction for using the I/O memory chunk
     starting from LCD_CONTROLLER_BASE and having a size of
     LCD_CONTROLLER_SIZE bytes */
  res = request_mem_region(res->start, res->end - res->start + 1,
                           pdev->name);

  /* Fill the fb_info structure with fixed (info->fix) and variable
     (info->var) values such as frame buffer length, xres, yres,
     bits_per_pixel, fbops, cmap, etc */
  initialize_fb_info(info, pdev);  /* Not expanded */
  info->fbops = &myfb_ops;
  fb_alloc_cmap(&info->cmap, 16, 0);

  /* DMA-map the frame buffer memory coherently. info->screen_base
     holds the CPU address of the mapped buffer,
     info->fix.smem_start carries the associated hardware address */
  info->screen_base = dma_alloc_coherent(0, info->fix.smem_len,
                                  (dma_addr_t *)&info->fix.smem_start,
                                   GFP_DMA | GFP_KERNEL);
  /* Set the information in info->var to the appropriate
     LCD controller registers */
  myfb_set_par(info);

  /* Register with the frame buffer core */
  register_framebuffer(info);
  return 0;
}

/* Platform driver's remove() routine */
static int
myfb_remove(struct platform_device *pdev)
{
  struct fb_info *info = platform_get_drvdata(pdev);
  struct resource *res;

  /* Disable screen refresh, turn off DMA,.. */
  myfb_disable_controller(info);

  /* Unregister frame buffer driver */
  unregister_framebuffer(info);
  /* Deallocate color map */
  fb_dealloc_cmap(&info->cmap);
  kfree(info->pseudo_palette);

  /* Reverse of framebuffer_alloc() */
  framebuffer_release(info);
  /* Release memory region */
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  release_mem_region(res->start, res->end - res->start + 1);
  platform_set_drvdata(pdev, NULL);

  return 0;
}

/* The platform driver structure */
static struct platform_driver myfb_driver = {
  .probe     = myfb_probe,
  .remove    = myfb_remove,
  .driver    = {
    .name    = "myfb",
  },
};

/* Module Initialization */
int __init
myfb_init(void)
{
  platform_device_add(&myfb_device);
  return platform_driver_register(&myfb_driver);
}

/* Module Exit */
void __exit
myfb_exit(void)
{
  platform_driver_unregister(&myfb_driver);
  platform_device_unregister(&myfb_device);
}

module_init(myfb_init);
module_exit(myfb_exit);
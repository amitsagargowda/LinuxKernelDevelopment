#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>

static int hello_proc_show(struct seq_file *m, void *v) {
  seq_printf(m, "jiffies = %lu\n", jiffies );
  return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, hello_proc_show, NULL);
}


static const struct proc_ops hello_proc_fops={
    .proc_open = hello_proc_open,
    .proc_release = single_release,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek
};

static int __init hello_proc_init(void) {
  proc_create("jiffies_test", 0, NULL, &hello_proc_fops);
  return 0;
}

static void __exit hello_proc_exit(void) {
  remove_proc_entry("jiffies_test", NULL);
}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);

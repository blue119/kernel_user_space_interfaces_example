#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

u8 myvalue;
int file_value;
struct dentry *tmp, *dir, *file;
char mybuf[200];

static ssize_t my_read_file(struct file *file, char __user *userbuf,
                                size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(userbuf, count, ppos, mybuf, 200);
} 

static ssize_t my_write_file(struct file *file, const char __user *buf,
                                size_t count, loff_t *ppos)
{
	if(count > 200)
		return -EINVAL;
	copy_from_user(mybuf, buf, count);
	return count;
}

static const struct file_operations my_fops = {
        .read = my_read_file,
        .write = my_write_file,
};
static int __init debugexample_module_init(void)
{
	/* the simplest interface */
	tmp = debugfs_create_u8("myfile", 0644, NULL, &myvalue);
	if (!tmp) {
		printk("error creating file");
		return -ENODEV;
	}

	/* custom read and write functions */
	dir = debugfs_create_dir("mydirectory", NULL);
	file = debugfs_create_file("myfile", 0644, dir, &file_value, &my_fops);

	return 0;
}

static void __exit debugexample_module_exit(void)
{
	debugfs_remove(tmp);
	debugfs_remove(file);
	debugfs_remove(dir);
}

module_init(debugexample_module_init);
module_exit(debugexample_module_exit);
MODULE_LICENSE("GPL");

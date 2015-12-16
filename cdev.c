#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
 
static int major; 
static char msg[200];

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
  	return simple_read_from_buffer(buffer, length, offset, msg, 200);
}


static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	if (len > 199)
		return -EINVAL;
	copy_from_user(msg, buff, len);
	msg[len] = '\0';
	return len;
}

static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
};

static int __init cdevexample_module_init(void)
{
	major = register_chrdev(0, "my_device", &fops);
	if (major < 0) {
     		printk ("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("cdev example: assigned major: %d\n", major);
	printk("create node with mknod /dev/cdev_example c %d 0\n", major);
 	return 0;
}

static void __exit cdevexample_module_exit(void)
{
	unregister_chrdev(major, "my_device");
}  

module_init(cdevexample_module_init);
module_exit(cdevexample_module_exit);
MODULE_LICENSE("GPL");

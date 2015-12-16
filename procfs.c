#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

static struct proc_dir_entry *example_dir, *first_file;


static int write_first(struct file *file,
                             const char __user *buffer,
                             unsigned long count, 
                             void *data)
{
	char * kernel_buf;
	if (count < 0 || count > 1024)
		return -EFAULT;

	kernel_buf = kmalloc(count + 1, GFP_KERNEL);
        if(copy_from_user(kernel_buf, buffer, count)) {
		kfree(kernel_buf);
		return -EFAULT;
        }
	kernel_buf[count] = '\0';
	printk("write_first received data: %s\n", kernel_buf);
	printk("first filename %s\n", file->f_dentry->d_iname);
	printk("write_first data %s\n", (char *)data);

	return count; 
}

static int read_first(char *page, char **start,
                            off_t off, int count, 
                            int *eof, void *data)
{
	int offset = 0;
	char * message = "hello world: ";
	strcpy(page + offset, message);
	offset += strlen(message);
	memcpy(page + offset, data, strlen(data));
	offset += strlen(data);
	page[offset] = '\n';
	offset += 1;
	return offset;
}

static int __init procexample_module_init(void)
{
	/* create a directory */
	example_dir = proc_mkdir("example", NULL);
        if(example_dir == NULL)
                return -ENOMEM;
        example_dir->owner = THIS_MODULE;
        
        /* create a file */
        first_file = create_proc_entry("first", 0644, example_dir);
        if(first_file == NULL) {
		remove_proc_entry("example", NULL);
		return -ENOMEM;
	}
	first_file->data = kmalloc(strlen( "first file private data"), GFP_KERNEL);
	strcpy(first_file->data, "first file private data");
	first_file->read_proc = read_first;
	first_file->write_proc = write_first;
        first_file->owner = THIS_MODULE;
        return 0;
}

static void __exit procexample_module_exit(void)
{
	kfree(first_file->data);	
	remove_proc_entry("first", example_dir);
	remove_proc_entry("example", NULL);
}

module_init(procexample_module_init);
module_exit(procexample_module_exit);
MODULE_LICENSE("GPL");

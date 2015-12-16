#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mm.h>		/* everything */
#include <linux/errno.h>	/* error codes */
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
 
static int major; 
static char msg[200];


struct scullp_dev {
	char *data;
	int vmas;                 /* active mappings */
	int size;
	struct semaphore sem;     /* Mutual exclusion */
};

int my_open_file (struct inode *inode, struct file *filp)
{
	printk("file %p myfileopen %p, private %p \n", filp, filp->private_data, inode->i_private);
	if(inode->i_private != NULL) {
		filp->private_data = inode->i_private;
		return 0;
	}

	struct scullp_dev *dev; /* device information */
	dev = kmalloc(sizeof(struct scullp_dev), GFP_KERNEL);
    	dev->data = NULL;
	/* and use filp->private_data to point to the device data */
	inode->i_private = dev;
	filp->private_data = inode->i_private;
	printk("open dev %p\n", filp->private_data);

	return 0;          /* success */
}


static ssize_t my_read_file(struct file *file, char __user *userbuf,
                                size_t count, loff_t *ppos)
{
	printk("myfileread\n");
	struct scullp_dev * dev = file->private_data;
	printk("read dev %p\n", file->private_data);
	if(!dev)
		return -1;
	printk("read: %s\n", dev->data);
	return simple_read_from_buffer(userbuf, count, ppos, dev->data, 200);
} 

static ssize_t my_write_file(struct file *file, const char __user *buf,
                                size_t count, loff_t *ppos)
{
	printk("mywritefile\n");
	struct scullp_dev * dev = file->private_data;
	printk("write dev %p\n", file->private_data);

	if(!dev) {
		printk("write: no dev\n");
		return -1;
	}
	if (!dev->data) {
		dev->data = vmalloc(PAGE_SIZE);
		memset(dev->data, 0, PAGE_SIZE);
		memcpy(dev->data, "hello from kernel", 18);
		printk("dev->data %s\n", dev->data);
	}

	if(count > 200)
		return -EINVAL;
	copy_from_user(dev->data, buf, count);
	return count;
}



/*
 * open and close: just keep track of how many times the device is
 * mapped, to avoid releasing it.
 */

void scullp_vma_open(struct vm_area_struct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;

	dev->vmas++;
}

void scullp_vma_close(struct vm_area_struct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;

	dev->vmas--;
}

/*
 * The nopage method: the core of the file. It retrieves the
 * page required from the scullp device and returns it to the
 * user. The count for the page must be incremented, because
 * it is automatically decremented at page unmap.
 *
 * For this reason, "order" must be zero. Otherwise, only the first
 * page has its count incremented, and the allocating module must
 * release it as a whole block. Therefore, it isn't possible to map
 * pages from a multipage block: when they are unmapped, their count
 * is individually decreased, and would drop to 0.
 */

struct page *scullp_vma_nopage(struct vm_area_struct *vma,
                                unsigned long address, int *type)
{
	unsigned long offset;
	struct scullp_dev *dev = vma->vm_private_data;
	struct page *page = NOPAGE_SIGBUS;
	void *pageptr = NULL; /* default to "missing" */

	down(&dev->sem);
	offset = (address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);
	if (offset >= dev->size) 
		goto out; /* out of range */

	/*
	 * Now retrieve the scullv device from the list,then the page.
	 * If the device has holes, the process receives a SIGBUS when
	 * accessing the hole.
	 */
	offset >>= PAGE_SHIFT; /* offset is a number of pages */
	if (dev && dev->data) 
		pageptr = dev->data;
	if (!pageptr) 
		goto out; /* hole or end-of-file */

	/*
	 * After scullv lookup, "page" is now the address of the page
	 * needed by the current process. Since it's a vmalloc address,
	 * turn it into a struct page.
	 */
	page = vmalloc_to_page(pageptr);

	/* got it, now increment the count */
	get_page(page);
	if (type)
		*type = VM_FAULT_MINOR;
  out:
	up(&dev->sem);
	return page;


}



struct vm_operations_struct scullp_vm_ops = {
	.open =     scullp_vma_open,
	.close =    scullp_vma_close,
	.nopage =   scullp_vma_nopage,
};

int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* don't do anything here: "nopage" will set up page table entries */
	vma->vm_ops = &scullp_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;
	scullp_vma_open(vma);
	return 0;
}


static const struct file_operations my_fops = {
	.open = my_open_file,
        .read = my_read_file,
        .write = my_write_file,
	.mmap = my_mmap,
};


static int __init cdevexample_module_init(void)
{
	major = register_chrdev(0, "my_device", &my_fops);
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

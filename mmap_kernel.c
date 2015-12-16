#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>

#include <linux/mm.h>  /* mmap related stuff */

struct dentry  *file;

struct mmap_info {
	char *data;	/* the data */
	int vmas;       /* how many times it is mmapped */  	
};


/* keep track of how many times it is mmapped */

void mmap_open(struct vm_area_struct *vma)
{
	struct mmap_info *info = vma->vm_private_data;
	info->vmas++;
}

void mmap_close(struct vm_area_struct *vma)
{
	struct mmap_info *info = vma->vm_private_data;
	info->vmas--;
}

/* nopage is called the first time a memory area is accessed which is not in memory,
 * it does the actual mapping between kernel and user space memory
 */
struct page *mmap_nopage(struct vm_area_struct *vma, unsigned long address, int *type)
{
	struct mmap_info *info = vma->vm_private_data;
	struct page *page;
	void *pageptr = NULL;

	/* is the address valid? */
	if (address > vma->vm_end) {
		printk("invalid address\n");
		return NOPAGE_SIGBUS;
	}
	/* the data belonging to this file is in dev->data */
	pageptr = info->data;
	if (!pageptr) {
		printk("no data\n");
		return NULL;	
	}

	/* get the page */
	page = virt_to_page(pageptr);
	
	/* increment the reference count of this page */
	get_page(page);
	/* type is the page fault type */
	if (type)
		*type = VM_FAULT_MINOR;

	return page;
}

struct vm_operations_struct mmap_vm_ops = {
	.open =     mmap_open,
	.close =    mmap_close,
	.nopage =   mmap_nopage,
};

int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;
	mmap_open(vma);
	return 0;
}

int my_open (struct inode *inode, struct file *filp)
{
	struct mmap_info *info;	/* info about the mmaped area */
	/* it will be saved in inode->i_private */
	if (!inode->i_private) {
		info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
		/* obtain new memory */
    		info->data = (char *)get_zeroed_page(GFP_KERNEL);
		memset(info->data, 0, PAGE_SIZE);
		memcpy(info->data, "hello from kernel", 18);
		inode->i_private = info;
	}
	/* all further references by mmap, read, write goes with this file pointer */
	filp->private_data = inode->i_private;
	return 0;
}

static const struct file_operations my_fops = {
	.open = my_open,
	.mmap = my_mmap,
};

static int __init mmapexample_module_init(void)
{
	printk("Init mmap example\n");
	file = debugfs_create_file("mmap_example", 0644, NULL, NULL, &my_fops);
	return 0;
}

static void __exit mmapexample_module_exit(void)
{
	debugfs_remove(file);
}

module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");

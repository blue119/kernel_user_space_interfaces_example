/********************************
mmap example code
found on http://people.ee.ethz.ch/~arkeller/linux/multi/kernel_user_space_howto-8.html
changed by bta on 27 feb 2014 based on http://lkml.iu.edu//hypermail/linux/kernel/0802.0/0672.html  mmap_nopage -> mmap_fault
********************************/ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>

#include <linux/mm.h>  /* mmap related stuff */

#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif


struct dentry  *file1;

struct mmap_info {
	char *data;	/* the data */
	int reference;       /* how many times it is mmapped */  	
};


/* keep track of how many times it is mmapped */

void mmap_open(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference++;
}

void mmap_close(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference--;
}

/* nopage is called the first time a memory area is accessed which is not in memory,
 * it does the actual mapping between kernel and user space memory
 */
//struct page *mmap_nopage(struct vm_area_struct *vma, unsigned long address, int *type)	--changed
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;
	/* is the address valid? */			//--changed
	/*if (address > vma->vm_end) {
		printk("invalid address\n");
		//return NOPAGE_SIGBUS;
		return VM_FAULT_SIGBUS;
	}
	/* the data is in vma->vm_private_data */
	info = (struct mmap_info *)vma->vm_private_data;
	if (!info->data) {
		printk("no data\n");
		return NULL;	
	}

	/* get the page */
	page = virt_to_page(info->data);
	
	/* increment the reference count of this page */
	get_page(page);
	vmf->page = page;					//--changed
	/* type is the page fault type */
	/*if (type)
		*type = VM_FAULT_MINOR;
	*/
	return 0;
}

struct vm_operations_struct mmap_vm_ops = {
	.open =     mmap_open,
	.close =    mmap_close,
	.fault =    mmap_fault,
	//.nopage =   mmap_nopage,				//--changed
};

int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	/* assign the file private data to the vm private data */
	vma->vm_private_data = filp->private_data;
	mmap_open(vma);
	return 0;
}

int my_close(struct inode *inode, struct file *filp)
{
	struct mmap_info *info = filp->private_data;
	/* obtain new memory */
	free_page((unsigned long)info->data);
    	kfree(info);
	filp->private_data = NULL;
	return 0;
}

int my_open(struct inode *inode, struct file *filp)
{
	struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	/* obtain new memory */
    	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "hello from kernel this is file: ", 32);
    /* memcpy(info->data + 32, filp->f_->d_name.name, strlen(filp->f_dentry->d_name.name)); */
	memcpy(info->data + 32, filp->f_path.dentry->d_name.name, strlen(filp->f_path.dentry->d_name.name));
	/* assign this info struct to the file */
	filp->private_data = info;
	return 0;
}

static const struct file_operations my_fops = {
	.open = my_open,
	.release = my_close,
	.mmap = my_mmap,
};

static int __init mmapexample_module_init(void)
{
	file1 = debugfs_create_file("mmap_example", 0644, NULL, NULL, &my_fops);
	return 0;
}

static void __exit mmapexample_module_exit(void)
{
	debugfs_remove(file1);

}

module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


static int __init usermodehelper_example_init(void)
{
	int ret = 0;
	char *argv[] = {"/home/arkeller/eth/paper/code/callee", "2", NULL };
	char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	printk("usermodehelper: init\n");
	/* last parameter: 1 -> wait until execution has finished, 0 go ahead without waiting*/
	/* returns 0 if usermode process was started successfully, errorvalue otherwise*/
	/* no possiblity to get return value of usermode process*/
	ret = call_usermodehelper("/home/arkeller/eth/paper/code/callee", argv, envp, UMH_WAIT_EXEC);
	if (ret != 0)
		printk("error in call to usermodehelper: %i\n", ret);
	else
		printk("everything all right\n");
        return 0;
}

static void __exit usermodehelper_example_exit(void)
{
	printk("usermodehelper: exit\n");
}

module_init(usermodehelper_example_init);
module_exit(usermodehelper_example_exit);
MODULE_LICENSE("GPL");

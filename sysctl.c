#include <linux/kernel.h>
#include <linux/module.h>
 
#include <linux/sysctl.h>


static struct ctl_table_header * test_sysctl_header;

int value1 = 0;
int value2 = 1;

int min = 10;
int max = 20;

/*only values between min and max can be written to value1 and value2 respectively. */
static ctl_table test_table[] = {
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "value1",
		.data		= &value1,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_minmax,
		.strategy	= &sysctl_intvec,
		.extra1         = &min,
		.extra2         = &max
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "value2",
		.data		= &value2,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_minmax,
		.strategy	= &sysctl_intvec,
		.extra1         = &min,
		.extra2         = &max
	},
	{ .ctl_name = 0 }
};

static ctl_table test_net_table[] = {
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "test",
		.mode		= 0555,
		.child		= test_table
	},
	{ .ctl_name = 0 }
};

static ctl_table test_root_table[] = {
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "net",
		.mode		= 0555,
		.child		= test_net_table
	},
	{ .ctl_name = 0 }
};




static int __init sysctl_module_init(void)
{
	test_sysctl_header = register_sysctl_table(test_root_table);
	return 0;
}

static void __exit sysctl_module_exit(void)
{
	unregister_sysctl_table(test_sysctl_header);

}  

module_init(sysctl_module_init);
module_exit(sysctl_module_exit);
MODULE_LICENSE("GPL");

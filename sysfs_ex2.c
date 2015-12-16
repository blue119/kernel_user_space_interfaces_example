#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>

/* the parameter value shows up in /sys/module/sysfs/parameters/ 
 * it can be read an changed, however it is not possible to do 
 * some "additional action" when it is read/written
 */
static int example_value;
module_param_named(value, example_value, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(value, "a test value");

/* a file called busval will show up under /sys/bus/my_pseudo_bus/
 * bus_show and bus_store is executed when it is read and written
 * respectively
 */
static int bus_value = 1;

static struct bus_type my_pseudo_bus = {
	.name = "my_pseudo_bus",
};


static ssize_t bus_show(struct bus_type *bus, char *buf) {
	return scnprintf(buf, PAGE_SIZE, "%d\n", bus_value);
}

static ssize_t bus_store(struct bus_type *bus, const char *buf, size_t count) {
	sscanf(buf, "%d", &bus_value);
        return sizeof(int);
}
BUS_ATTR(busval, S_IRUGO | S_IWUSR, bus_show, bus_store);

static int __init sysfsexample_module_init(void)
{
	int ret = -1;
	
	ret = bus_register(&my_pseudo_bus);
	if (ret < 0) {
             	printk(KERN_WARNING "sysfs: error register bus: %d\n", ret);
		return ret;
     	}

	ret = bus_create_file(&my_pseudo_bus, &bus_attr_busval);
	if (ret < 0) {
		printk(KERN_WARNING "sysfs: error creating busfile\n");
		bus_unregister(&my_pseudo_bus);	
	}

	return 0;
}

static void __exit sysfsexample_module_exit(void)
{
	bus_remove_file(&my_pseudo_bus, &bus_attr_busval);
	bus_unregister(&my_pseudo_bus);
}

module_init(sysfsexample_module_init);
module_exit(sysfsexample_module_exit);
MODULE_LICENSE("GPL");

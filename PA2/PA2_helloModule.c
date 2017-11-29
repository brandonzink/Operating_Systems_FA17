

#include<linux/init.h>
#include<linux/module.h>

int _init hello_init(void)
{
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	return 0;
}

static void _exit hello_exit(void)
{
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
}

module_init(hello_init);
module_exit(hello_exit);

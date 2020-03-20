#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab3");

static int __init my_module_init(void)
{
   printk(KERN_INFO "Start!\n");
   return 0;
}

static void __exit my_module_exit(void)
{
   printk(KERN_INFO "End!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

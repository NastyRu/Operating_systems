#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab3");

static int __init my_module_init(void)
{
  printk(KERN_INFO "MODULE2: loaded\n");
  printk(KERN_INFO "MODULE2: Число экспортированное из md1 : %d\n", md1_int_data);
  printk(KERN_INFO "MODULE2: Строка экспортированная из md1 : %s\n", md1_str_data);
  printk(KERN_INFO "MODULE2: Результат работы функции md1_get_str(10) : %s\n", md1_get_str(10));
  printk(KERN_INFO "MODULE2: Результат работы функции md1_get_str(1) : %s\n", md1_get_str(1));
  printk(KERN_INFO "MODULE2: Результат работы функции md1_get_str(2) : %s\n", md1_get_str(2));
  printk(KERN_INFO "MODULE2: Результат работы функции md1_factorial(10) : %d\n", md1_factorial(10));
  return 0;
}

static void __exit my_module_exit(void)
{
  printk(KERN_INFO "MODULE2: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

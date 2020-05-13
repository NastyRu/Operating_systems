#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab3");

static int __init my_module_init(void)
{
  printk(KERN_INFO "MODULE3: loaded\n");
  printk(KERN_INFO "MODULE3: Число экспортированное из md1 : %d\n", md1_int_data);
  printk(KERN_INFO "MODULE3: Строка экспортированная из md1 : %s\n", md1_str_data);
  printk(KERN_INFO "MODULE3: Результат работы функции md1_get_str(2) : %s\n", md1_get_str(10));
  printk(KERN_INFO "MODULE3: Результат работы функции md1_get_str(0) : %s\n", md1_get_str(1));
  printk(KERN_INFO "MODULE3: Результат работы функции md1_get_str(1) : %s\n", md1_get_str(2));
  printk(KERN_INFO "MODULE3: Результат работы функции md1_factorial(4) : %d\n", md1_factorial(10));
  return -1;
}

module_init(my_module_init);

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab3");

// Инициализация модуля
static int __init my_module_init(void)
{
   printk(KERN_INFO "MODULE1: loaded!\n");

   struct task_struct *task = &init_task;
   do
   {
     printk(KERN_INFO "MODULE1: process: %s - %d, parent: %s - %d\n",
     task->comm, task->pid, task->parent->comm, task->parent->pid);
   }
   while ((task = next_task(task)) != &init_task);

   printk(KERN_INFO "MODULE1: current: %s — %d, parent: %s — %d\n",
   current->comm, current->pid, current->parent->comm, current->parent->pid);
   return 0;
}

// Удаление модуля
static void __exit my_module_exit(void)
{
   printk(KERN_INFO "MODULE1: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

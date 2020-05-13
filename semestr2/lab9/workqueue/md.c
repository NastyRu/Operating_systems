#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab9");

#define MOUSEIRQ 12

char my_workqueue_data[] = "MOUSE IRQ";
// Очередь работ
static struct workqueue_struct *my_wq;
struct work_struct *my_work;

// Bottom Half Function
void my_workqueue_function(struct work_struct *my_work)
{
  struct timeval t;
  struct tm broken;
  do_gettimeofday(&t);
  time_to_tm(t.tv_sec, 0, &broken);
  printk("%s in: %dd.%dm.%dy %dh:%dm:%ds\n", (char *)my_workqueue_data,
			broken.tm_mday, broken.tm_mon + 1,
			broken.tm_year + 1900, broken.tm_hour + 3,
			broken.tm_min, broken.tm_sec);
  kfree(my_work);
}

irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
  // Проверка, что произошло именно нужное 12-е прерывание
  if(irq == MOUSEIRQ)
  {
    my_work = (struct work_struct*)kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (my_work)
    {
       INIT_WORK(my_work, my_workqueue_function);
       queue_work(my_wq, my_work);
    }

    return IRQ_HANDLED; // прерывание обработано
  }
  else
    return IRQ_NONE; // прерывание не обработано
}

// Инициализация модуля
static int __init my_module_init(void)
{
  printk(KERN_DEBUG "MODULE loaded!\n");
  // Разделение(совместное использование) линии IRQ с другими устройствами
  int ret = request_irq(MOUSEIRQ, (irq_handler_t)irq_handler, IRQF_SHARED,
				"tmy_irq_handler", (void *)(irq_handler));
  if (ret != 0)
  {
    printk(KERN_ERR "Mouse IRQ handler wasn't registered");
    return -ENOMEM;
  }

  // Cоздание очереди работ
  my_wq = create_workqueue("my_queue");
  if (my_wq)
    printk(KERN_INFO "Workqueue was allocated successfully");
  else
  {
    free_irq(MOUSEIRQ, (void *)(irq_handler));
    printk(KERN_ERR "Workqueue wasn't allocated");
    return -ENOMEM;
  }

  printk(KERN_INFO "Mouse IRQ handler was registered successfully");
  return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии прерывания
  free_irq(MOUSEIRQ, (void *)(irq_handler));
  // Удаление очереди работ
  flush_workqueue(my_wq);
  destroy_workqueue(my_wq);
  printk(KERN_DEBUG "MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

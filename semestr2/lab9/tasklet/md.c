#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab9");

#define MOUSEIRQ 12

char my_tasklet_data[] = "MOUSE IRQ";

// Bottom Half Function
void my_tasklet_function(unsigned long data)
{
  struct timeval t;
  struct tm broken;
  do_gettimeofday(&t);
  time_to_tm(t.tv_sec, 0, &broken);
  printk("%s in: %dd.%dm.%dy %dh:%dm:%ds\n", (char *)data,
			broken.tm_mday, broken.tm_mon + 1,
			broken.tm_year + 1900, broken.tm_hour + 3,
			broken.tm_min, broken.tm_sec);
}

// Регистрация тасклета
DECLARE_TASKLET(my_tasklet, my_tasklet_function, (unsigned long)
						&my_tasklet_data);

// Обработчик прерывания
irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
  // Проверка, что произошло именно нужное 12-е прерывание
  if(irq == MOUSEIRQ)
  {
    // Постановка тасклета в очередь на выполнение
    tasklet_schedule(&my_tasklet);
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
    printk(KERN_ERR "MOUSE IRQ handler wasn't registered");
    return ret;
  }
  printk(KERN_INFO "MOUSE IRQ handler was registered successfully");
  return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии прерывания
  free_irq(MOUSEIRQ, (void *)(irq_handler));
  // Удаление тасклета
  tasklet_disable(&my_tasklet);
  tasklet_kill(&my_tasklet);
  printk(KERN_DEBUG "MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

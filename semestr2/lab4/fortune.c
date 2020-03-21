#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab4");

static struct proc_dir_entry *proc_entry, *dir, *symlink;
static char* buffer; // Хранилище 'фортунок'
int buffer_index, next_fortune; // Индексы для записи и вывода

ssize_t fortune_write(struct file *filp, const char __user *buf, size_t len, loff_t *offp)
{
  int space_available = (PAGE_SIZE - buffer_index) + 1;

  // Хватит ли места для размещения
  if (len > space_available)
    return -ENOSPC;

  // Копирование строки
  if (copy_from_user(&buffer[buffer_index], buf, len))
    return -EFAULT;

  buffer_index += len;
  buffer[buffer_index - 1] = '\n';

  return len;
}

ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
  int len;
  if (*offp > 0)
    return 0;

  // Перевод индекса на первый элемент
  if (next_fortune >= buffer_index)
    next_fortune = 0;

  len = copy_to_user(buf, &buffer[next_fortune], count);
  next_fortune += len;
  *offp += len;

  return len;
}

struct file_operations fileops =
{
  .read = fortune_read,
  .write = fortune_write
};

int fortune_module_init(void)
{
  buffer = (char *)vmalloc(PAGE_SIZE);
  if (!buffer)
  {
    printk(KERN_INFO "fortune: No memory for create buffer\n");
    return -ENOMEM;
  }
  memset(buffer, 0, PAGE_SIZE);

  proc_entry = proc_create("fortune", 0666, NULL, &fileops);
  if (proc_entry == NULL)
  {
    vfree(buffer);
    printk(KERN_INFO "fortune: Couldn't create proc entry\n");
    return -ENOMEM;
  }

  dir = proc_mkdir("fortune_dir", NULL);
  symlink = proc_symlink("fortune_symlink", NULL, "/proc/fortune_dir");
  if ((dir == NULL) || (symlink == NULL))
  {
    vfree(buffer);
    printk(KERN_INFO "fortune: Couldn't create proc dir, symlink\n");
    return -ENOMEM;
  }

  buffer_index = 0;
  next_fortune = 0;

  printk(KERN_INFO "fortune: Module loaded.\n");
  return 0;
}

void fortune_module_exit(void)
{
  remove_proc_entry("fortune", NULL);
  remove_proc_entry("fortune_symlink", NULL);
  remove_proc_entry("fortune_dir", NULL);
  vfree(buffer);
  printk(KERN_INFO "fortune: Module unloaded.\n");
}

module_init(fortune_module_init);
module_exit(fortune_module_exit);

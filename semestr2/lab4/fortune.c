#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Usage /proc file system");
MODULE_AUTHOR("Sofia Kirillova");

static struct proc_dir_entry *proc_entry, *direc;
static char* strings;
static int scursor, temp, num_of_entries = 0;

ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp )
{
  if (count > temp)
    count = temp;

  temp -= count;

  copy_to_user(buf, strings, count);
  if (count == 0)
    temp=scursor;
  scursor = 0;
  memset(strings, 0, PAGE_SIZE);
  num_of_entries = 0;
  return count;
}


ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
  int free_space = (PAGE_SIZE - scursor) + 2;

  if (count > free_space || num_of_entries == 8)
  {
    printk(KERN_ERR "test_proc: the list is full!\n");
    return -ENOSPC;
  }

  num_of_entries++;
  strings[scursor++] = num_of_entries + '0';
  strings[scursor++] = '.';
  copy_from_user(&strings[scursor], buf, count);
  scursor += count + 3;
  temp = scursor;
  return count;
}


struct file_operations fileops =
{
	read: read_proc,
	write: write_proc
};

int init_fortune_module( void )
{
  int ret = 0;
  strings = (char *)vmalloc(PAGE_SIZE);
  if (!strings)
  {
    ret = -ENOMEM;
  }
  else
  {
    memset(strings, 0, PAGE_SIZE);
    proc_entry = proc_create("test_proc", 0666, NULL, &fileops);
    //struct proc_dir_entry *direc;
    direc = proc_mkdir("test_proc", NULL);
    proc_symlink("test_proc_symlink", direc, "/proc/test_proc");
    if (proc_entry == NULL)
    {
      ret = -EFAULT;
      vfree(strings);
      printk(KERN_INFO "test_proc: Couldn't create proc entry\n");
    }

    else
    {
      printk(KERN_INFO "test_proc: Module loaded.\n");
    }
  }
  return ret;
}

void cleanup_fortune_module( void )
{
  //remove_proc_symlink("todolist_symlink", direc, "/proc/todolist");
  remove_proc_entry("test_proc", NULL);
  remove_proc_entry("test_proc", NULL);
  vfree(strings);
  printk(KERN_INFO "test_proc: Module unloaded.\n");
}


module_init( init_fortune_module );
module_exit( cleanup_fortune_module );

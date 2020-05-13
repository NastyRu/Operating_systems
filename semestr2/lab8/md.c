#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pagemap.h> 	/* PAGE_CACHE_SIZE */
#include <linux/fs.h>     	/* This is where libfs stuff is declared */
#include <asm/atomic.h>
#include <asm/uaccess.h>	/* copy_to_user */
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");
MODULE_DESCRIPTION("Lab8");

#define VFS_MAGIC_NUMBER 0x13131313
#define SLABNAME "vfs_cache"

// Размер элементов кэша
static int size = 7;
// Значение параметра из командной строки, иначе значение по умолчанию
module_param(size, int, 0);
static int number = 31;
// Значение параметра из командной строки, иначе значение по умолчанию
module_param(number, int, 0);

static void* obj = NULL;

// Конструктор вызывается при размещении каждого элемента
static void co(void* p)
{
  *(int*)p = (int)p;
}
struct kmem_cache *cache = NULL;

// Структура нужна для собственного inode
static struct vfs_inode
{
  int i_mode;
  unsigned long i_ino;
} vfs_inode;

// Создание inode
static struct inode * vfs_make_inode(struct super_block *sb, int mode)
{
  // Размещает новую структуру inode
  struct inode *ret = new_inode(sb);
  if (ret)
  {
    // mode определяет не только права доступа, но и тип
    inode_init_owner(ret, NULL, mode);
    // Заполнение значениями
    ret->i_size = PAGE_SIZE;
    ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
    ret->i_private = &vfs_inode;
  }
  return ret;
}

// Печать строки, будет вызвана перед уничтожением суперблока
static void vfs_put_super(struct super_block *sb)
{
  printk(KERN_DEBUG "VFS super block destroyed!\n");
}

// Операции структуры суперблок
static struct super_operations const vfs_super_ops = {
	.put_super = vfs_put_super, // деструктор суперблока
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
};

// Функция инициализации суперблока
// Создание корневого каталога ФС
static int vfs_fill_sb(struct super_block *sb, void *data, int silent)
{
  struct inode* root = NULL;

  // Заполнение структуры
  sb->s_blocksize = PAGE_SIZE;
  sb->s_blocksize_bits = PAGE_SHIFT;
  sb->s_magic = VFS_MAGIC_NUMBER;
  sb->s_op = &vfs_super_ops;

  // Создание inode каталога ФС (указывает на это S_IFDIR)
  root = vfs_make_inode(sb, S_IFDIR | 0755);
  if (!root)
  {
    printk (KERN_ERR "VFS inode allocation failed !\n");
    return -ENOMEM;
  }

  // Файловые и inode-операции, которые мы назначаем новому inode
  root->i_op = &simple_dir_inode_operations;
  root->i_fop = &simple_dir_operations;

  // Создаем dentry для представления каталога в ядре
  sb->s_root = d_make_root(root);
  if (!sb->s_root)
  {
    printk(KERN_ERR "VFS root creation failed!\n");
    iput(root);
    return -ENOMEM;
  }
  return 0;
}

// Монтирование ФС
static struct dentry* vfs_mount(struct file_system_type *type, int flags, const char *dev, void *data)
{
  // Так как создается виртуальная ФС, несвязанная с носителем, используем
  struct dentry* const entry = mount_nodev(type, flags, data, vfs_fill_sb);

  if (IS_ERR(entry))
    printk(KERN_ERR  "VFS mounting failed!\n");
  else
    printk(KERN_DEBUG "VFS mounted!\n");

  // Корневой каталог нашей ФС
  return entry;
}

// Описание создаваемой ФС
static struct file_system_type vfs_type  =  {
  .owner  =  THIS_MODULE, // счетчик ссылок на модуль
  .name  =  "vfs", // название ФС
  .mount  =  vfs_mount, // функция, вызываемая при монтировании ФС
  .kill_sb  =  kill_litter_super, // функция, вызываемая при размонтировании ФС
};

// Инициализация модуля
static int __init vfs_module_init(void)
{
  if (size < 0)
  {
    printk(KERN_ERR "VFS_MODULE invalid sizeof objects\n");
    return -EINVAL;
  }

  // Выделение области, является непрерывной в физической памяти
  obj = kmalloc(sizeof(void*), GFP_KERNEL);
  if (!obj)
  {
    printk(KERN_ERR "VFS_MODULE kmalloc error\n");
    kfree(obj);
    return -ENOMEM;
  }

  // Создание слаба
  // Расположение каждого элемента в слабе выравнивается по строкам
  cache = kmem_cache_create(SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, co);

  if (!cache)
  {
    printk(KERN_ERR "VFS_MODULE cannot create cache\n");
    // Уничтожени слаба
    kmem_cache_destroy(cache);
    return -ENOMEM;
  }

  // Выделение объекта
  if (NULL == (obj = kmem_cache_alloc(cache, GFP_KERNEL)))
  {
    printk(KERN_ERR "VFS_MODULE cannot alloc object\n");
    // Уничтожени слаба
    kmem_cache_destroy(cache);
    return -ENOMEM;
  }

  printk(KERN_INFO "VFS_MODULE allocate %d objects into slab: %s\n", number, SLABNAME);
  printk(KERN_INFO "VFS_MODULE object size %d bytes, full size %ld bytes\n", size, (long)size *number);

  // Регистрация файловой ситемы
  int ret = register_filesystem(&vfs_type);
  if (ret != 0)
  {
    printk(KERN_ERR "VFS_MODULE cannot register filesystem!\n");
    return ret;
  }

  printk(KERN_DEBUG "VFS_MODULE loaded!\n");
  return 0;
}

// Выход загружаемого модуля
static void __exit vfs_module_exit(void)
{
  // Возвращение объекта
  kmem_cache_free(cache, obj);
  

  // Уничтожени слаба
  kmem_cache_destroy(cache);
  kfree(obj);

  // Дерегистрация
  if (unregister_filesystem(&vfs_type) != 0)
  {
    printk(KERN_ERR "VFS_MODULE cannot unregister filesystem!\n");
  }

  printk(KERN_DEBUG "VFS_MODULE unloaded!\n");
}

module_init(vfs_module_init);
module_exit(vfs_module_exit);

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main()
{
  // Создает файловый дескриптор для открытого файла
  int fd = open("alphabet.txt", O_RDONLY);
  if (fd == -1)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Создает 2 буферизованных потока ввода-вывода,
  // используя файловый дескриптор
  FILE *fs1 = fdopen(fd, "r");
  if (fs1 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }
  char buff1[20];
  setvbuf(fs1, buff1, _IOFBF, 20);

  FILE *fs2 = fdopen(fd, "r");
  if (fs2 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }
  char buff2[20];
  setvbuf(fs2, buff2, _IOFBF, 20);

  // Чтение char и вывод на экран попеременно из fs1 и fs2
  int flag1 = 1, flag2 = 2;
  while (flag1 == 1 || flag2 == 1)
  {
    char c;
    flag1 = fscanf(fs1, "%c", &c);
    if (flag1 == 1)
    {
      fprintf(stdout, "%c", c);
    }

    flag2 = fscanf(fs2, "%c", &c);
    if (flag2 == 1)
    {
      fprintf(stdout, "%c", c);
    }
  }

  close(fd);
  return 0;
}

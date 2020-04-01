#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main()
{
  // Создает файловый дескриптор для открытого файла 2 раза
  int fd1 = open("alphabet.txt", O_RDONLY);
  if (fd1 == -1)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  int fd2 = open("alphabet.txt", O_RDONLY);
  if (fd2 == -1)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Чтение char и вывод на экран попеременно
  char c1;
  char c2;
  // read возвращает количество действительно считанных байт.
  // Соответственно, когда кол-во считанных байтов не будет равно 1,
  // значит или конец файла или ошибка.
  while ((read(fd1, &c1, 1) == 1) && (read(fd2, &c2, 1) == 1))
  {
    write(1, &c1, 1);
    write(1, &c2, 1);
  }

  close(fd1);
  close(fd2);

  return 0;
}

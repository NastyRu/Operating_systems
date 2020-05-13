#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main()
{
  // Открывает файл и связывает с ним буферизованный поток
  FILE *fd1 = fopen("test3.txt", "w");
  fd1->aa();
  if (fd1 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  FILE *fd2 = fopen("test3.txt", "a");
  if (fd1 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Цикл по всем буквам алфавита (нечетные - fd1, четные - fd2)
  for(char c = 'a'; c <= 'z'; c++)
  {
    if (c % 2)
    {
      fprintf(fd1, "%c", c);
      fflush(fd1);
    }
    else
    {
      fprintf(fd2, "%c", c);
      fflush(fd2);
    }
  }

  fclose(fd1);
  fclose(fd2);
  return 0;
}

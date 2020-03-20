#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUF_SIZE 0x100

// Вывод содержимого файла
void readFile(char* path)
{
  char buf[BUF_SIZE];
  int len, i;
  FILE *f;

  f = fopen(path, "r");
  if (f == NULL)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
  {
    for (i = 0; i < len; i++)
      if (buf[i] == 0)
        buf[i] = 10;
    buf[len] = 0;
    printf("%s", buf);
  }

  fclose(f);
}

// Вывод содержимого директории
void contentDir(char* path)
{
  struct dirent	*dirp;
	DIR	*dp;

  if ((dp = opendir(path)) == NULL)
	{
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((dirp = readdir(dp)) != NULL)
  {
    printf("%s\n", dirp->d_name);
	}

	if (closedir(dp) < 0)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }
}

void checkParams(char* path)
{
  struct stat	statbuf;

  if (lstat(path, &statbuf) < 0)
	{
    printf("%s", strerror(errno));
    exit(errno);
  }

	if (S_ISDIR(statbuf.st_mode) == 0)
	{ // Если не каталог
    readFile(path);
  }
  else
  { // Если каталог
    contentDir(path);
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Использование: ./output.exe <каталог/файл>\n");
    return 1;
  }

  checkParams(argv[1]);

  return 0;
}

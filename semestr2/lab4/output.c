#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUF_SIZE 0x1000

static char* param_stat[] = {"pid", "comm", "state", "ppid", "pgrp", "session",
"tty_nr", "tpgid", "flags", "minflt", "cminflt", "majflt", "cmajflt", "utime",
"stime", "cutime", "cstime", "priority", "nice", "num_threads", "itrealvalue",
"starttime", "vsize", "rss", "rsslim", "startcode", "endcode", "startstack",
"kstkesp", "kstkeip", "signal", "blocked", "sigignore","sigcatch", "wchan",
"nswap", "сnswap", "exit_signal", "processor", "rt_priority", "policy",
"delayacct_blkio_ticks","guest_time", "cguest_time", "start_data", "end_data",
"start_brk", "arg_start", "arg_end", "env_start", "env_end", "exit_code"};

// Вывод содержимого файла ENVIRON
void readEnviron()
{
  char buf[BUF_SIZE];
  int len, i;
  FILE *f = fopen("/proc/self/environ", "r");
  if (f == NULL)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
  {
    for (i = 0; i < len; i++)
      if (buf[i] == 0)
        buf[i] = 10; // код 10, перевод строки
    buf[len] = 0;
    printf("%s", buf);
  }

  fclose(f);
}

// Вывод содержимого файла STAT, CMDLINE
void readFile(char *path)
{
  char buf[BUF_SIZE];
  FILE *f = fopen(path, "r");
  if (f == NULL)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }

  fread(buf, 1, BUF_SIZE, f);
  char *pch = strtok(buf, " "); // поиск разделителей в файле

  int i = 0;
  while (pch != NULL)
  {
    if (strcmp("/proc/self/stat", path) == 0)
      printf("%s=", param_stat[i]);
    i++;
    printf("%s\n", pch);
    pch = strtok(NULL, " "); // продолжеени сканирование с того места,
                             // где был остановлен предыдущий успешный вызов функции
  }

  fclose(f);
}

// Вывод содержимого директории FD
void contentDir()
{
  struct dirent *dirp;
  DIR *dp;
  char str[BUF_SIZE];
  char path[BUF_SIZE];

  if ((dp = opendir("/proc/self/fd/")) == NULL)
	{
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((dirp = readdir(dp)) != NULL)
  {
    // Пропуск каталогов . и ..
    if ((strcmp(dirp->d_name, ".") != 0) && (strcmp(dirp->d_name, "..") != 0))
    {
      sprintf(path, "%s%s", "/proc/self/fd/", dirp->d_name);
      readlink(path, str, BUF_SIZE); // Считывает значение символьной ссылки
      printf("%s -> %s\n", dirp->d_name, str);
    }
  }

  if (closedir(dp) < 0)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Использование: ./output.exe <stat/environ/fd/cmdline>\n");
    return 1;
  }

  if (strcmp("environ", argv[1]) == 0)
    readEnviron();
  else if (strcmp("stat", argv[1]) == 0)
    readFile("/proc/self/stat");
  else if (strcmp("fd", argv[1]) == 0)
    contentDir("/proc/self/fd/");
  else if (strcmp("cmdline", argv[1]) == 0)
    readFile("/proc/self/cmdline");
  else
    printf("Использование: ./output.exe <stat/environ/fd/cmdline>\n");

  return 0;
}

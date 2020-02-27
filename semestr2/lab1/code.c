#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int already_running(void)
{
  int fd;
  char buf[16];

  fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
  if (fd < 0)
  {
    syslog(LOG_ERR, "не возможно открыть %s: %s", LOCKFILE, strerror(errno));
    exit(1);
  }

  // если файл уже заблокирован завершится неудачей
  if (flock(fd, LOCK_EX|LOCK_NB) < 0)
  {
    if (errno == EWOULDBLOCK)
    {
      close(fd);
      return 1;
    }
    syslog(LOG_ERR, "невозможно установить блокировку %s: %s", LOCKFILE, strerror(errno));
    exit(1);
  }

  // усекает размер файла до нуля, вдруг предыдущий id демона был длиннее
  ftruncate(fd, 0);
  sprintf(buf, "%ld", (long)getpid());
  write(fd, buf, strlen(buf) + 1);

  return(0);
}

void err_quit(const char *error_message, const char *command) {
  syslog(LOG_ERR, "%s %s", error_message, command);
  exit(1);
}

void daemonize(const char *cmd)
{
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;
  // Сбросить маску режима создания файла
  // 1 правило, для возможности создания файлов с любыми правами доступа
  umask(0);

  // Получить максимально возможный номер дескриптора файла
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    err_quit("%s: невозможно получить максимальный номер дескриптора ", cmd);

  // Создаем дочерний процесс и завершаем предок
  // 2 правило
  if ((pid = fork()) < 0)
    err_quit("%s: ошибка вызова функции fork", cmd);
  else if (pid != 0) { // родительский процесс
    exit(0);
  }
  // Создание новой сессии
  // Процесс становится лидером новой сессии, лидером новой группы процессов, теруяет управляющий терминал
  // 3 правило
  setsid();

  // Обеспечить невозможность обретения управляющего терминала в будущем
  // Игнорирование сообщения о потере сигнала с управляющим терминалом
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0)
    err_quit("%s: невозможно игнорировать сигнал SIGHUP ", cmd);

  // Назначить корневой каталог текущим рабочим каталогом,
  // если вдруг он будет запушен с подмонтированной ФС
  // 4 правило
  if (chdir("/") < 0)
    err_quit("%s: невозможно сделать текущим рабочим каталогом /", cmd);

  // Закрыть все открытые фaйловые дескрипторы
  // 5 правило
  if (rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;
  for (i = 0; i < rl.rlim_max; i++)
    close(i);

  // Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null
  // Чтобы стандартные функции не влияли на работу демона
  // 6 правило
  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);

  // Инициализировать файл журнала
  openlog(cmd, LOG_CONS, LOG_DAEMON);
  if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
    syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
    exit(1);
  }
}

int main()
{
  char *cmd = "daemon";
  daemonize(cmd);
  time_t timer = 0;

  if (already_running())
  {
     syslog(LOG_ERR, "Демон уже существует");    
     exit(1);
  }

  while (1)
  {
     timer = time(NULL);
     syslog(LOG_INFO, "Время: %s", ctime(&timer));
     sleep(10);
  }
}

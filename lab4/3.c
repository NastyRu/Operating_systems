/* Написать программу, в которой процесс-потомок вызывает системный вызов exec(),
а процесс-предок ждет завершения процесса-потомка. */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  pid_t childpid1;
  pid_t childpid2;
  if ((childpid1 = fork()) == -1) /* если fork завершился успешно, pid > 0 в родительском процессе */
  {
    perror("Can't fork"); /* fork потерпел неудачу (например, память или какая-либо */
    exit(1);                                        /* таблица заполнена) */
  }
  if (childpid1 == 0)
  { /* здесь располагается дочерний код */
    printf("Child1 forked \n");
    printf("Child1 id = %d, parent id = %d, group id = %d \n", getpid(), getppid(), getpgrp());

    char msg[] = "exec() called from child1";

    /* обращаемся к программе echo, параметрами передаём её имя и текст для печати */
    /* NULL - завершает список параметров */
		if (execlp("echo", "echo", msg, NULL) == -1) /* при успешном вызове exec() заменяет адресное пространство процесса */
		{
			perror("Can't exec"); /* - эти вызовы произойдут только в том случае, если exec() не выполнится*/
      exit(1);
		}
    return 0;
  }

  if ((childpid2 = fork()) == -1) /* если fork завершился успешно, pid > 0 в родительском процессе */
  {
    perror("Can't fork"); /* fork потерпел неудачу (например, память или какая-либо */
    exit(1);                                        /* таблица заполнена) */
  }
  if (childpid2 == 0)
  { /* здесь располагается дочерний код */
    printf("Child2 forked \n");
    printf("Child2 id = %d, parent id = %d, group id = %d \n", getpid(), getppid(), getpgrp());

    char msg[40] = "exec() called from child2";

    /* обращаемся к программе echo, параметрами передаём её имя и текст для печати */
    /* NULL - завершает список параметров */
    if (execlp( "echo", "echo", msg, NULL ) == -1) /* при успешном вызове exec() заменяет адресное пространство процесса */
		{
			perror("Can't exec"); /* - эти вызовы произойдут только в том случае, если exec() не выполнится*/
			exit(1);
		}
  }

  if (childpid1 != 0 && childpid2 != 0)
  { /* здесь располагается родительский код */
    printf("Parent id = %d, child1 id = %d, child2 id = %d, group id = %d \n", getpid(), childpid1, childpid2, getpgrp());
    int status1;
    childpid1 = wait(&status1);
    printf("Child has finished: PID = %d\n", childpid1);

    int status2;
    childpid2 = wait(&status2);
    printf("Child has finished: PID = %d\n", childpid2);

    if (WIFEXITED(status1) && WIFEXITED(status2))
      printf("Children exited with code %d and %d\n", WEXITSTATUS(status1), WEXITSTATUS(status2));
    else
      printf("Child terminated abnormally\n");
  }
  return 0;
}

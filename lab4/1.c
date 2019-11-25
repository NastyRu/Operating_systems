/* Написать программу, запускающую новый процесс системным вызовом fork().
В предке вывести собственный идентификатор ( функция getpid()),
идентификатор группы ( функция getpgrp())  и идентификатор потомка.
В процессе-потомке вывести собственный идентификатор,
идентификатор предка ( функция getppid()) и идентификатор группы.
Убедиться, что при завершении процесса-предка потомок,
который продолжает выполняться, получает идентификатор предка (PPID),
равный 1 или идентификатор процесса-посредника. */

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
    getchar();
    printf("Child1 id = %d, parent id = %d, group id = %d \n", getpid(), getppid(), getpgrp());
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
    getchar();
    printf("Child2 id = %d, parent id = %d, group id = %d \n", getpid(), getppid(), getpgrp());
  }

  if (childpid1 != 0 && childpid2 != 0)
  { /* здесь располагается родительский код */
    printf("Parent id = %d, child1 id = %d, child2 id = %d, group id = %d \n", getpid(), childpid1, childpid2, getpgrp());
    getchar();
    printf("Parent exited \n");
  }

  return 0;
}

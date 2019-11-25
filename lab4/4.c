/* Написать программу, в которой предок и потомок обмениваются сообщением через программный канал. */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  pid_t childpid1;
  pid_t childpid2;
  /* дескрипторы программных каналов */
  int my_pipe1[2];
  int my_pipe2[2];
  /* [0] - выход для чтения, [1] - выход для записи */

	/* потомок унаследует открытый программный канал предка */
  if (pipe(my_pipe1) == -1)
  {
    perror("Can't pipe");
    exit(1);
  }
  if (pipe(my_pipe2) == -1)
  {
    perror("Can't pipe");
    exit(1);
  }

  if ((childpid1 = fork()) == -1) /* если fork завершился успешно, pid > 0 в родительском процессе */
  {
    perror("Can't fork"); /* fork потерпел неудачу (например, память или какая-либо */
    exit(1);                                        /* таблица заполнена) */
  }
  if (childpid1 == 0)
  { /* здесь располагается дочерний код */
    printf("Child1 forked \n");
    printf("Child1 id = %d, parent id = %d, group id = %d \n", getpid(), getppid(), getpgrp());

    char buf[40];
    close(my_pipe1[1]); /* потомок ничего не запишет в канал */
    read(my_pipe1[0], buf, sizeof(buf));
    printf("Child1 catch message = %s \n", buf);

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

    char buf[40];
		close(my_pipe2[1]); /* потомок ничего не запишет в канал */
    read(my_pipe2[0], buf, sizeof(buf));
    printf("Child2 catch message = %s \n", buf);
  }

  if (childpid1 != 0 && childpid2 != 0)
  { /* здесь располагается родительский код */
    printf("Parent id = %d, child1 id = %d, child2 id = %d, group id = %d \n", getpid(), childpid1, childpid2, getpgrp());

    char msg1[] = "Hello, child1!";
    close(my_pipe1[0]); /* предок ничего не считает из канала */
    write(my_pipe1[1], msg1, sizeof(msg1));

    char msg2[] = "Hello, child2!";
    close(my_pipe2[0]); /* предок ничего не считает из канала */
    write(my_pipe2[1], msg2, sizeof(msg2));

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

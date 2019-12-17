#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/shm.h>

#define COUNT 3
#define PRODUCER 0
#define CONSUMER 1

#define PERMS S_IRWXU | S_IRWXG | S_IRWXO

#define EMPTYCOUNT 0
#define FULLCOUNT 1
#define BIN 2

int shared_memory;
char **addr_shared_memory;

// Массив структур
struct sembuf producer_grab[2] = { {EMPTYCOUNT, -1, SEM_UNDO}, {BIN, -1, SEM_UNDO} };
struct sembuf producer_free[2] = { {BIN, 1, SEM_UNDO}, {FULLCOUNT, 1, SEM_UNDO} };
struct sembuf consumer_grab[2] = { {FULLCOUNT, -1, SEM_UNDO}, {BIN, -1, SEM_UNDO} };
struct sembuf consumer_free[2] = { {BIN, 1, SEM_UNDO}, {EMPTYCOUNT, 1, SEM_UNDO} };

// Потребитель
void consumer(int semaphore, int value)
{
  while(1)
  {
    sleep(1);
    int sem_op_p = semop(semaphore, consumer_grab, 2);
  	if (sem_op_p == -1)
  	{
  		perror("Can't semop \n");
  		exit(1);
  	}

    printf("Consumer%d get %d\n", value, **(addr_shared_memory + sizeof(int *)));
  	(*(addr_shared_memory + sizeof(int *)))++;

  	int sem_op_v = semop(semaphore, consumer_free, 2);
  	if (sem_op_v == -1)
  	{
  		perror("Can't semop \n");
  		exit(1);
    }
  }
}

// Производитель
void producer(int semaphore, int value)
{
  while(1)
  {
    sleep(2);
    int sem_op_p = semop(semaphore, producer_grab, 2);
    if (sem_op_p == -1)
  	{
  		perror("Can't semop \n");
  		exit(1);
  	}

    *(*addr_shared_memory) = value;
  	printf("Producer%d put %d\n", value, *(*addr_shared_memory));
    (*addr_shared_memory)++;

  	int sem_op_v = semop(semaphore, producer_free, 2);
    if (sem_op_v == -1)
  	{
  		perror("Can't semop \n");
  		exit(1);
    }
  }
}

int getsem()
{
  int sem = semget(IPC_PRIVATE, 3, IPC_CREAT | PERMS);
  if (sem == -1)
  {
    perror("Can't semget \n");
    exit(1);
  }
  int se = semctl(sem, EMPTYCOUNT, SETVAL, COUNT);
	int sf = semctl(sem, FULLCOUNT, SETVAL, 0);
  int sb = semctl(sem, BIN, SETVAL, 1);

  return sem;
}

int* getshared_memory()
{
  shared_memory = shmget(IPC_PRIVATE, 2 * sizeof(int *) + 5 * sizeof(int), IPC_CREAT | PERMS);
  if (shared_memory == -1)
  {
    perror("Can't shmget \n");
    exit(1);
  }

  char *addr = shmat(shared_memory, 0, 0);
	if ((*addr) == -1)
	{
		perror("Can't shmat \n");
		exit(1);
	}
  return addr;
}

int main()
{
  int process;
  int consumers[3];
  int producers[3];
  // Создание семафора
  int semaphore = getsem();
  // Объявление разделяемого сегмента
  addr_shared_memory = getshared_memory();
  *(addr_shared_memory) = addr_shared_memory + 2 * sizeof(int *);
  *(addr_shared_memory + sizeof(int *)) = addr_shared_memory + 2 * sizeof(int *);

  // Создание процессов
  for (int i = 0; i < COUNT; i++) {
    if (-1 == (producers[i] = fork()))
    {
      perror("Can’t fork \n");
      return 1;
    }
    else if (0 == producers[i])
    {
      producer(semaphore, i);
      exit(0);
    }

    if (-1 == (consumers[i] = fork()))
    {
      perror("Can’t fork \n");
      return 1;
    }
    else if (0 == consumers[i])
    {
      consumer(semaphore, i);
      exit(0);
    }
  }

  int status;
  wait(&status);

  // Очистка памяти
  shmctl(shared_memory, IPC_RMID, NULL);
  semctl(semaphore, 0, IPC_RMID, 0);
  return 0;
}

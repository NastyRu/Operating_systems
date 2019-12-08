#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/shm.h>

#define COUNT 5
#define PRODUCER 0
#define CONSUMER 1

#define PERMS S_IRWXU | S_IRWXG | S_IRWXO

#define EMPTYCOUNT 0
#define FULLCOUNT 1
#define BIN 2

int shared_memory;
int *addr_shared_memory;

int pos_consumer;
int pos_producer;

// Массив структур
struct sembuf producer_grab[2] = { {EMPTYCOUNT, -1, SEM_UNDO}, {BIN, -1, SEM_UNDO} };
struct sembuf producer_free[2] = { {BIN, 1, SEM_UNDO}, {FULLCOUNT, 1, SEM_UNDO} };
struct sembuf consumer_grab[2] = { {FULLCOUNT, -1, SEM_UNDO}, {BIN, -1, SEM_UNDO} };
struct sembuf consumer_free[2] = { {BIN, 1, SEM_UNDO}, {EMPTYCOUNT, 1, SEM_UNDO} };

// Потребитель
void consumer(int semaphore, int value)
{
  sleep(2);
  int sem_op_p = semop(semaphore, consumer_grab, 2);
	if (sem_op_p == -1)
	{
		perror("Can't semop \n");
		exit(1);
	}

  printf("Consumer%d get %d\n", value, addr_shared_memory[pos_consumer]);
	pos_consumer++;

	int sem_op_v = semop(semaphore, consumer_free, 2);
	if (sem_op_v == -1)
	{
		perror("Can't semop \n");
		exit(1);
	}
}

// Производитель
void producer(int semaphore, int value)
{
  sleep(1);
  int sem_op_p = semop(semaphore, producer_grab, 2);
  if (sem_op_p == -1)
	{
		perror("Can't semop \n");
		exit(1);
	}

  addr_shared_memory[pos_producer] = pos_producer;
	printf("Producer%d put %d\n", value, addr_shared_memory[pos_producer]);
  pos_producer++;

	int sem_op_v = semop(semaphore, producer_free, 2);
  if (sem_op_v == -1)
	{
		perror("Can't semop \n");
		exit(1);
	}
}

int getsem()
{
  int sem = semget(IPC_PRIVATE, COUNT, IPC_CREAT | PERMS);
  if (sem == -1)
  {
    perror("Can't semget \n");
    exit(1);
  }
  int se = semctl(sem, EMPTYCOUNT, SETVAL, 5);
	int sf = semctl(sem, FULLCOUNT, SETVAL, 0);
  int sb = semctl(sem, BIN, SETVAL, 1);

  return sem;
}

int* getshared_memory()
{
  shared_memory = shmget(IPC_PRIVATE, COUNT * sizeof(int), IPC_CREAT | PERMS);
  if (shared_memory == -1)
  {
    perror("Can't shmget \n");
    exit(1);
  }

  int *addr = shmat(shared_memory, 0, 0);
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
  // Создание семафора
  int semaphore = getsem();
  // Объявление разделяемого сегмента
  addr_shared_memory = getshared_memory();
  pos_producer = 0;
	pos_consumer = 0;

  // Создание процесса
  process = fork();
  if (-1 == process)
  {
    perror("Can’t fork \n");
    return 1;
  }

  // Потребитель -- ребенок
  if (process == 0)
  {
    int valuec = 0;
    while (valuec < COUNT)
    {
      consumer(semaphore, valuec);
      valuec++;
    }
  }
  // Производитель -- родитель
  if (process != 0)
  {
    int valuep = 0;
    while (valuep < COUNT)
    {
      producer(semaphore, valuep);
      valuep++;
    }

    int status;
    wait(&status);
  }

  // Очистка памяти
  shmctl(shared_memory, IPC_RMID, NULL);
  semctl(semaphore, IPC_RMID, NULL);
  return 0;
}

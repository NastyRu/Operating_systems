#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/shm.h>
#include <signal.h>

#define WRITER 3
#define READER 5

#define PERMS S_IRWXU | S_IRWXG | S_IRWXO

// количество читателей
#define READERCOUNT 0
// активные писатели
#define ACTIVEWRITER 1
// очередь писатей
#define WRITERCOUNT 2

int semaphore;
int shared_memory;
int *addr_shared_memory;

// Массив структур
struct sembuf start_read[] = { {ACTIVEWRITER, 0, SEM_UNDO}, {WRITERCOUNT, 0, SEM_UNDO}, {READERCOUNT, 1, SEM_UNDO} };
struct sembuf stop_read[] = { {READERCOUNT, -1, SEM_UNDO} };
struct sembuf start_write[] = { {WRITERCOUNT, 1, SEM_UNDO}, {READERCOUNT, 0, SEM_UNDO}, {ACTIVEWRITER, -1, SEM_UNDO}, {WRITERCOUNT, -1, SEM_UNDO}, };
struct sembuf stop_write[] = { {ACTIVEWRITER, 1, SEM_UNDO} };

// собственный обработчик сигнала ctrl-c
void catch_sigp(int sig_numb)
{
    signal(sig_numb, catch_sigp);
    shmctl(shared_memory, IPC_RMID, NULL);
    semctl(semaphore, 0, IPC_RMID, 0);
}

void StartRead()
{
    semop(semaphore, start_read, 3);
}

void StopRead()
{
    semop(semaphore, stop_read, 1);
}

void StartWrite()
{
    semop(semaphore, start_write, 4);
}

void StopWrite()
{
    semop(semaphore, stop_write, 1);
}

// Читатель
void Reader(int value)
{
    while (1)
  	{
    		StartRead();
    		printf("Reader%d get = %d\n", value, *addr_shared_memory);
    		StopRead();
    		sleep(1);
  	}
}

// Писатель
void Writer(int value)
{
    while (1)
    {
        StartWrite();
        (*addr_shared_memory)++;
        printf("Writer%d put = %d\n", value, *addr_shared_memory);
        StopWrite();
        sleep(2);
    }
}

int getsem()
{
  int sem = semget(IPC_PRIVATE, 4, IPC_CREAT | PERMS);
  if (sem == -1)
  {
    perror("Can't semget \n");
    exit(1);
  }
  int sr = semctl(sem, READERCOUNT, SETVAL, 0);
	int sa = semctl(sem, ACTIVEWRITER, SETVAL, 1);
  int sw = semctl(sem, WRITERCOUNT, SETVAL, 0);

  return sem;
}

int* getshared_memory()
{
  shared_memory = (int*)shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | PERMS);
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
  // Создание семафора
  semaphore = getsem();
  // Объявление разделяемого сегмента
  addr_shared_memory = getshared_memory();

  // Создание процессов
  int processes[READER + WRITER];
  int parent = getpid();

  for (int i = 0; i < WRITER + READER; i++)
	{
		processes[i] = fork();
		if (getpid() != parent)
		{
  			for (int j = 0; j < i; j++)
  				  processes[j] = -1;
  			break;
		}
	}

	for (int i = 0; i < WRITER + READER; i++)
	{
		if (processes[i] != 0)
			continue;

		if (i < WRITER)
		{
      Writer(i);
			return 0;
		}
		else
		{
			Reader(i - WRITER);
			return 0;
		}
	}

  signal(SIGINT, catch_sigp);
  int status;
  wait(&status);

  // Очистка памяти
  shmctl(shared_memory, IPC_RMID, NULL);
  semctl(semaphore, 0, IPC_RMID, 0);
  return 0;
}

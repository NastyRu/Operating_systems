#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MSG_SIZE 256
#define LISTENQ 1024

int maxi, maxfd;

// Соединение с новым клиентом
int newClient(int listensock, int client[FD_SETSIZE], fd_set *allset, fd_set *reset)
{
  // Индекс
  int i;
  int connfd;

  if (FD_ISSET(listensock, reset))
  {
    // Установка соединения в ответ на запрос клиента
    // Так как нас не интересуют данные об адресе клиента
    // передаем значения NULL в предпоследнем и последнем параметрах
    int connfd = accept(listensock, NULL, NULL);
    if (connfd < 0)
    {
      printf("%s", strerror(errno));
      return errno;
    }
    // Функция accept() возвращает новый сокет, открытый
    // для обмена данными с клиентом, запросившим соединение

    // Сохраняем дескриптор в первый свободный
    for (i = 0; i < FD_SETSIZE; i++)
    {
      if (client[i] < 0)
      {
        client[i] = connfd;
        break;
      }
    }

    if (i == FD_SETSIZE)
    {
      printf("Достигнуто максимальное число клиентов");
      return errno;
    }

    // Добавление нового дескриптора
    FD_SET(connfd, allset);

    // Максимальный для функции select
    if (connfd > maxfd)
      maxfd = connfd;

    // Максимальный индекс в массиве клиентов
    if (i > maxi)
      maxi = i;
    printf("Клиент %d подключился\n", i);
  }
  return errno;
}

int readMsg(int client[FD_SETSIZE], fd_set *allset, fd_set *reset)
{
  int n, i;
  int sockfd;
  char msg[MSG_SIZE];
  // Проверяем все клиенты на наличие данных, пока не дошли до конца
  // или не закончились дескрипторы готовые для чтения
  for (i = 0; i <= maxi; i++)
  {
    // Если не пустой
    if ((sockfd = client[i]) > 0)
    {
      // Установлен ли бит?
      if (FD_ISSET(sockfd, reset))
      {
        // Соединение закрыто клиентом
        if ((n = read(sockfd, msg, MSG_SIZE)) == 0)
        {
          // Закрываем сокет
          close(sockfd);
          // Сброс бита
          FD_CLR(sockfd, allset);
          // Освобождаем ячейку в массиве клиентов
          client[i] = -1;
          printf("Клиент %d отключился\n", i);
        }
        else
        {
          // Сообщение клиенту о доставке сообщения
          write(sockfd, "OK", 2);
          // Установка символа конца строки и вывод сообщения на экран
          msg[n] = 0;
          printf("Сообщение от клиента %d: %s", i, msg);
        }
      }
    }
  }
  return errno;
}

int main(int argc, char ** argv)
{
  int listensock;
  int client[FD_SETSIZE];
  fd_set reset, allset;
  // Структура предназначен для хранения адресов в формате Интернета
  struct sockaddr_in server;

  if (argc < 2)
  {
    fprintf(stderr, "Использование: %s <port_number>\n", argv[0]);
    return 1;
  }

  // Создание сетевого сокета (домен AF_INET)
  // Тип сокета -- SOCK_STREAM, сокет должен быть потоковым
  // Протокол -- 0, протокол выбирается по умолчанию
  listensock = socket(AF_INET, SOCK_STREAM, 0);
  if (listensock < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Укажем семейство адресов, которыми мы будем пользоваться
  server.sin_family = AF_INET;
  // Укажем адрес (наша программа-сервер зарегистрируется на всех адресах
  // машины, на которой она выполняется)
  server.sin_addr.s_addr = INADDR_ANY;
  // Укажем значение порта. Функция htons() переписывает двухбайтовое значение
  // порта так, чтобы порядок байтов соответствовал принятому в Интернете
  server.sin_port = htons(atoi(argv[1]));

  // Связывание сокета с заданным адресом
  // bind(дескриптор сокета, указатель на структуру, длина структуры)
  if (bind(listensock, (struct sockaddr *) &server, sizeof(server)) < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Переводим сервер в режим ожидания запроса на соединение
  // Второй параметр - максимальное число обрабатываемых одновременно соединений
  listen(listensock, LISTENQ);

  // Инициализация значения
  maxfd = listensock;
  // Индекс в массиве клиентов (наибольший используемый)
  maxi = -1;
  // Массив дескрипторов присоединенного сокета для каждого клиента
  for (int i = 0; i < FD_SETSIZE; i++)
    client[i] = -1; // -1 означает, что элемент свободен

  // Сбрасываем все биты в allset
  FD_ZERO(&allset);
  // Устанавливаем бит для listensock в allset
  FD_SET(listensock, &allset);

  while(1)
  {
    // Присваивание значения структуре
    reset = allset;
    // select() ждет пока не будет установлено новое клиентское соединение
    // или на существующем не прибудут данные
    // select(количество проверяемых дескрипторов, 2-4 наборы дескрипторов,
    // которые следует проверять, интервал времени, по прошествии которого
    // она вернет управление в любом случае)
    select(maxfd + 1, &reset, NULL, NULL, NULL);

    if (newClient(listensock, client, &allset, &reset) ||
                                              readMsg(client, &allset, &reset))
      return errno;
  }

  // Закрываем сокет
  close(listensock);

  return errno;
}

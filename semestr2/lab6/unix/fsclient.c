#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_NAME "socket.soc"
#define MSG_SIZE 256

int main(int argc, char ** argv)
{
  // Создание сокета в файловом пространстве имен (домен AF_UNIX)
  // Тип сокета -- SOCK_DGRAM означает датаграммный сокет
  // Протокол -- 0, протокол выбирается по умолчанию
  char msg[MSG_SIZE];
  struct sockaddr_un server;
  char id[10];
  sprintf(id, "%d", getpid());
  id[strlen(id)] = 0;

  int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Укажем семейство адресов, которыми мы будем пользоваться
  server.sun_family = AF_UNIX;
  // Укажем имя файла сокета
  strcpy(server.sun_path, SOCK_NAME);

  // Приглашение и ввод сообщения для сервера
  printf("Введите сообщение:\n");
  scanf("%s", msg);

  // Передаем сообщение серверу
  // sendto(дескриптор сокета, адрес буфера для передачи данных, его длина,
  // дополнительные флаги, адрес сервера, его длине)
  sendto(sock, id, strlen(id), 0, (struct sockaddr *) &server, sizeof(server));
  sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &server, sizeof(server));

  return errno;
}

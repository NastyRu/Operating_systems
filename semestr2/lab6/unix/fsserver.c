#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_NAME "socket.soc"
#define MSG_SIZE 256

int main()
{
  // Для сокетов Unix (сокетов в файловом пространстве имен)
  // есть специализированная структура sockaddr_un
  struct sockaddr_un server;
  char msg[MSG_SIZE];
  int bytes;

  // Создание сокета в файловом пространстве имен (домен AF_UNIX)
  // Тип сокета -- SOCK_DGRAM означает датаграммный сокет
  // Протокол -- 0, протокол выбирается по умолчанию
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

  // Связывание сокета с заданным адресом
  // bind(дескриптор сокета, указатель на структуру, длина структуры)
  if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Наша программа-сервер становится доступна для соединения
  // по заданному адресу (имени файла)

  // Пока клиент не отправит сообщение "break"
  while (strcmp(msg, "break"))
  {
    // Для чтения данных из датаграммного сокета - recvfrom,
    // которая блокирует программу до тех пор, пока на входе не появятся новые данные
    // Так как нас не интересуют данные об адресе клиента
    // передаем значения NULL в предпоследнем и последнем параметрах
    bytes = recvfrom(sock, msg, MSG_SIZE, 0, NULL, NULL);
    if (bytes < 0)
    {
      printf("%s", strerror(errno));
      return errno;
    }
    // Символ окончания строки
    msg[bytes] = 0;
    printf("Сообщение от клиента: %s\n", msg);
  }

  // Закрываем сокет
  close(sock);
  // Удаляем файл сокета
  unlink(SOCK_NAME);

  return errno;
}

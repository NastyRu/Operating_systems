#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* тип фугкции, которая будет вызываться для каждого встреченного файла */
typedef	int	AnyType(const char *, int, int);

static AnyType myFunc;
static int myFtw(char *, AnyType *);
static int doPath(AnyType *, char*, int);

int pal[10] = {0};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Использование: ./code.out <начальный_каталог>\n");
		return 1;
	}

	return myFtw(argv[1], myFunc); /* выполняет всю работу */
}

/*
 * Обойти дерево каталогов, начиная с каталога "pathname".
 * Пользовательская функция func() вызывается для каждого встреченного файла.
 */
#define	FTW_F	1 /* файл, не являющийся каталогом */
#define	FTW_D	2	/* каталог */
#define	FTW_DNR	3	/* каталог, недоступный для чтения */
#define	FTW_NS 4	/* файл, информацию о котором */
                  /* невозможно получить с помощью stat */

/* Первый вызов для введенного каталога */
static int myFtw(char *pathname, AnyType *func)
{
	/* Изменение текущей директории для использования коротких имен */
	if (chdir(pathname) == -1)
	{
		printf("Ошибка вызова функции chdir %s\n", pathname);
		return 1;
	}

	return doPath(func, ".", 0);
}

/*
 * Обход дерева каталогов, начиная с "fullpath". Если "fullpath" не является
 * каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
 * Для директорий производится рекурсивный вызов функции.
 */
static int doPath(AnyType* func, char *fullpath, int len)
{
	struct stat	statbuf;
	struct dirent	*dirp;
	DIR	*dp;

	if (lstat(fullpath, &statbuf) < 0)	/* Ошибка вызова stat */
	{
    return 1;
  }

	if (S_ISDIR(statbuf.st_mode) == 0)	/* Если не каталог */
	{
    return 1;
  }

	/*
	 * Это каталог. Сначала вызовем func(),
	 * а затем обработаем все файлы в каталоге.
	 */
	func(fullpath, FTW_D, len);

  if ((dp = opendir(fullpath)) == NULL) /* Каталог не доступен */
	{
    return 1;
  }

	/* Изменение текущей директории для использования коротких имен */
	if (chdir(fullpath) < 0)
	{
		printf("Ошибка вызова функции chdir %s\n", fullpath);
		return 1;
	}

  len += 5;

  while ((dirp = readdir(dp)) != NULL)
  {
		/* Пропуск каталогов . и .. */
    if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".git") != 0)
      doPath(func, dirp->d_name, len);
	}

	if (chdir("..") == -1)
		printf("Ошибка вызова функции chdir ..\n");

	if (closedir(dp) < 0)
		printf("Невозможно закрыть каталог %s\n", fullpath);

	return 0;
}

static int myFunc(const char *pathname, int type, int len)
{
	if (type == FTW_D)
  {
		for (int i = 0; i < len; i++)
		{
			if (i % 5 == 1 && i / 5 != 0)
				printf("|");
			else
				printf(" ");
		}
		printf(" |— %s\n", pathname);
  }

	return 0;
}

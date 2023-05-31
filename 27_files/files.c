/* Программа записывает строку из буфера * buf в файл,
затем считывает эту строку из файла в массив * buf_read
в обратном порядке.
Елизавета Кириллова. Версия 02. Дата релиза 11.05.2023 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	int descriptor;
	const char* buf = "Hello, World!"; // Исходная строка для записи в файл
	int str_size = strlen(buf);
	char buf_read[str_size]; // Для записи строки в обратном порядке
	ssize_t nr;// Для записи ошибки ф-ции write
	int pos; // Установка позиции каретки

	descriptor = open("./newfile", O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (descriptor == -1)
		printf("Ошибка открытия файла!");

	nr = write(descriptor, buf, strlen(buf));
	if (nr == -1)
		printf("Ошибка! Проверить переменную errno");

	for(int i = 0; i <= str_size; i++){
		pos = lseek (descriptor, - i, SEEK_END); // Перевод каретки в конец файла
		read(descriptor, buf_read + i, 1); // Вывод с конца массива по одному символу
		//buf_read++;
	}
	
	for(int i = 0; i <= str_size; i++) // Вывод содержимого buf_read на экран побайтно
	{
		printf("%c", buf_read[i]);
	}

	close(descriptor);

	return 0;
}

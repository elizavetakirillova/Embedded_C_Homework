/*Кириллова Е.С.
Программа заполняет матрицу числами от 1 до N2 улиткой */

#include <stdio.h>
#define ROWS 5
#define COLUMNS 5
#define T 8

int main()
{
	int cnt = 0; 
	int arr[T][T];

	// Заполняем массив по итерациям
	for(int step = 0; step < (int)(T/2) + 1; step++) 
	{
		for(int i = step ; i < T-step; i++)
    		{
       			arr[step][i]= cnt;
       			cnt++;
    		}
    		cnt--;
	
		for(int i = step; i < T-step; i++)
    		{
       			arr[i][T-1-step]= cnt;
       			cnt++;
    		}
    		cnt--;
	
		for(int i = T-1-step; i >= step; i--)
    		{
       			arr[T-1-step][i] = cnt;
       			cnt++;
    		}
    		cnt--;

		for(int i = T-1-step; i > step; i--)
    		{
       			arr[i][step]= cnt;
       			cnt++;
    		}
		//cnt--;
	
	}

	// 
	for(int i = 0; i < T; i++)
	{
		for(int j = 0; j < T; j++) 
		{
			(arr[i][j] < 10) && (arr[i][j] >= 0) ? 
			printf("  %d", arr[i][j]) : printf(" %d", arr[i][j]);
       		}
		printf("\n");
	}
	return 0;
}


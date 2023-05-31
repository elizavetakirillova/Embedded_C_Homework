/* Программа имитирует реальную жизнь. Главный поток создает глобальный массив, каждый из элементов которого - это магазин, заполняет его значениями по 1000. Главный поток создает 3 потока покупателя и 1 поток погрузчик. Покупателю назначает потребность - 10 000. В каждом магазине должен быть только один покупатель в один момент. Он покупает весть товар, который там имеется, уходит из магазина,  изменяется его потребность (10 000 - 1000) и покупатель засыпает на 2 сек. При пробуждении идет в свободный магазин и покупает товар до тех пор пока его потребность не будет равна 0 или меньше 0. Есть поток погрузчик - заходит в любой магазин, в котором нет покупателей, закидывает 1000 и засыпает на 1 сек. работает по все три покупателя не насытятся. Главный поток видит, что покупатели завершились, убивает поток погрузчик, поскольку тот бесконечен, и красиво завершает программу. */ 

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#define SHOPS 5
#define BUYERS 3
#define NEED 10000
#define GOODS 1000
#define PTHREADS_SUM 4

int satisfaction[3] = {0, 0, 0};

struct shop 
{
    pthread_mutex_t door;
    int load;
} shops[SHOPS];

void* buyer (void* args)
{
    int id = (int)args;
    printf("Buyer created. ID = %d\n", id);

    int bag = 0; // Состояние потока
    
    while(1)
    {
    	for(int i = 0; i < SHOPS; i++)
    	{
    	    // Входим в критическую секцию
    	    if(pthread_mutex_trylock(&shops[i].door) == 0)
    	    {
    	    	if(shops[i].load > 0)
    	    	{
    	    	    bag = bag + shops[i].load;
    	    	    printf("Buyer %d get %d from %d shop. Bag = %d\n", id, shops[i].load, i, bag);
    	    	    shops[i].load = 0;
    	    	    sleep(1);
    	    	}
    	    	pthread_mutex_unlock(&shops[i].door);
    	    	sleep(3);
    	    }
    	    if(bag >= NEED)
    	    {
    	        satisfaction[id] = 1;
    	        printf("Buyer %d satisfied\n", id);
                return NULL;
    	    }
    	}
    }  
}

void* loader (void* args)
{
    int all_satisfied;
    
    while(1)
    {
    	for(int i = 0; i < SHOPS; i++)
    	{
    	    // Входим в критическую секцию
    	    if(pthread_mutex_trylock(&shops[i].door) == 0)
    	    {
    	    	shops[i].load = shops[i].load + 1000;
    	    	pthread_mutex_unlock(&shops[i].door);
    	    	sleep(2);
    	    }
    	}
    	
    	all_satisfied = 1;
    	printf("Satisfaction: %d %d %d\n",satisfaction[0],satisfaction[1],satisfaction[2]);
    	for(int i = 0; i < BUYERS; i++)
    	{ 	    
    	    if(satisfaction[i] == 0)
    	    	all_satisfied = 0;
    	}
    	
    	if(all_satisfied)
    	{
    	    printf("All buyers satisfied!\n");
    	    return NULL;
    	}
    }
    return NULL;
}

int main()
{
    for(int i = 0; i < SHOPS; i++)
    	shops[i].load = 1000;
    	
    pthread_t thread[PTHREADS_SUM];

    int ids[] = {0, 1, 2};

    for(int i = 0; i < BUYERS; i++)
        // Главный поток создает потоки 3х покупателей
        pthread_create(&thread[i], NULL, buyer, (void*) ids[i]);

    // Погрузчик дозаполняет массив
    pthread_create(&thread[3], NULL, loader, NULL); 

    printf("Я главный поток. Я создал %d покупателей\n", BUYERS);
    printf("и 1 погрузчик.\n");
    printf("Магазинов создал %d. \n", SHOPS);

    /* Главной поток должен дожидаться завершения остальных через pthread_join. Иначе он сразу завершается и в месте с ним все наши не успевшие запуститься потоки */
    pthread_join(thread[3], NULL); 

    return 0;
}

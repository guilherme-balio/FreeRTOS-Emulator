#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "queue.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_FreeRTOS_Utils.h"
#include "TUM_Print.h"

#include "AsyncIO.h"

#include "defines.h"
#include "demo_tasks.h"
#include "async_sockets.h"
#include "async_message_queues.h"
#include "draw.h"
#include "buttons.h"


#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;

SemaphoreHandle_t DrawSignal = NULL;

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    while (1) {
        tumDrawUpdateScreen();
        tumEventFetchEvents(FETCH_EVENT_BLOCK);
        xSemaphoreGive(DrawSignal);
        vTaskDelayUntil(&xLastWakeTime,
                        pdMS_TO_TICKS(frameratePeriod));
    }
}

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define NUM_OF_PHILOSOPHERS (5)

SemaphoreHandle_t forks[NUM_OF_PHILOSOPHERS];
SemaphoreHandle_t entry_sem;
TaskHandle_t philosophers[NUM_OF_PHILOSOPHERS];
int comeu = 0;

#define left(i) (i)
#define right(i) ((i + 1) % NUM_OF_PHILOSOPHERS)

void take_fork(int i) {
	xSemaphoreTake(forks[left(i)], portMAX_DELAY);
	xSemaphoreTake(forks[right(i)], portMAX_DELAY);
	//printf("Philosopher %d got the fork %d and %d\n", i, left(i), right(i));

}

void put_fork(int i) {
    //printf("Philosopher %d is eating\n", i);
    vTaskDelay(10000);
	xSemaphoreGive(forks[left(i)]);
	xSemaphoreGive(forks[right(i)]);
    comeu++;
	//printf("Philosopher %d Gave up the fork %d and %d\n", i, left(i), right(i));
}

int gen_random(int min, int max){
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void philosophers_task(void *param) {

	int i = *(int *)param;
    printf("Iniciou a task %d \n", i);
    int pensar;

	while (1) {
        
        pensar = gen_random(0, 100);
        //printf("Contador de pensar: %d\n", pensar);

        vTaskDelay(pensar);

        // pega garfo
		take_fork(i);

        // coloca garfo na mesa
        put_fork(i);
	}
}

void task_measure(void *param){

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 20000;
    xLastWakeTime = xTaskGetTickCount ();
    printf("Tempo (s), Trabalho completo\n");

    for(;;){

        printf("%d,", xLastWakeTime/10000);
        printf("%d \n", comeu);
        // alterando o número de filósofos

        // Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

int main(void) {

	int i;
	int param[NUM_OF_PHILOSOPHERS];
    srand(time(NULL));


	// Create Five Semaphores for the five shared resources. 
	// Which is the fork in this case.

	for (i = 0; i < NUM_OF_PHILOSOPHERS; i++) {
		forks[i] = xSemaphoreCreateMutex();
	}

	// This is the critical piece to avoid deadlock.
	// If one less philosopher is allowed to act then there will no deadlock.
	// As one philosopher will always get two forks and so it will go on.

    xTaskCreate(task_measure, "medição", 30, NULL, 1, NULL);

	for (i = 0; i < NUM_OF_PHILOSOPHERS; i++) {
		// Ofcourse, you can just pass i as every thread needs it's own
		// address to store the parameter.
		param[i] = i;
		xTaskCreate(philosophers_task, "filósofos", 30, &(param[i]), 2, NULL);
	}
	vTaskStartScheduler();
    return 0;
}

//int main(int argc, char *argv[])
//{
    

//}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}

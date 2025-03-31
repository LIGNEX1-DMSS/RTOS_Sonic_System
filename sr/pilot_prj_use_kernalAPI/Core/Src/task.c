/*
 * task.c
 *
 *  Created on: Dec 22, 2020
 *      Author: YUNGKI HONG (guileschool@gmail.com)
 *      Copyright © 2015 guileschool
 */

/* FreeRTOS.org includes. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

/* task's priority */
#define MAIN_PRIO	    20
#define SYNC_PRIO		5
#define GEN1_PRIO		8
#define GEN2_PRIO		8
#define RX_PRIO		    9

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */


typedef struct _SensorData {	/* struct for parameter passing to task */
       int 	      info;
       TickType_t time;
       int        data;
} SensorData;

#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof( SensorData )


/* The task functions. */
void mainTask( void *pvParameters )		  ;
void syncTask( void *pvParameters )		  ;
void gen1Task( void *pvParameters )		  ;
void gen2Task( void *pvParameters )		  ;
void rxTask  ( void *pvParameters )		  ;

TaskHandle_t xHandleMain, xHandleSync, xHandleGen1, xHandleGen2, xHandleRx;

/* ...........................................................................
 *
 * 이벤트 컨트롤 & 사용자 정의 블럭 정의
 * ===================
 */
#define SYNC_SIGNAL_OK (1<<0)

EventGroupHandle_t sync_id;
EventBits_t 	   uxBits;
QueueHandle_t 	   sdata_qid;

/*-----------------------------------------------------------*/

void USER_THREADS( void )
{
	/* Setup the hardware for use with the Beagleboard. */
	//prvSetupHardware();
#ifdef CMSIS_OS
	osThreadDef(defaultTask, TaskMain, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
#else
	/* Create one of the two tasks. */
	xTaskCreate(	(TaskFunction_t)mainTask,		/* Pointer to the function that implements the task. */
					"mainTask",	/* Text name for the task.  This is to facilitate debugging only. */
					256,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					MAIN_PRIO,	/* This task will run at this priority */
					&xHandleMain );		/* We are not using the task handle. */
#endif
}
/*-----------------------------------------------------------*/

void mainTask( void *pvParameters )
{
	/* TODO #1:
		create a event flag
	    use group_id */
	srand(time(NULL));
#if 1
	sync_id= xEventGroupCreate();
	if (sync_id == NULL) {
		printf("xEventGroupCreate error found\n");
	}
	sdata_qid = xQueueCreate( QUEUE_LENGTH, QUEUE_ITEM_SIZE );
	if (sdata_qid == NULL) printf("xQueueCreate error found\n");
#endif // TODO #1

	/* Create the other task in exactly the same way. */
	xTaskCreate(	(TaskFunction_t)syncTask,		/* Pointer to the function that implements the task. */
					"syncTask",	/* Text name for the task.  This is to facilitate debugging only. */
					256,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					SYNC_PRIO,	/* This task will run at this priority */
					&xHandleSync );		/* We are not using the task handle. */
	xTaskCreate(	(TaskFunction_t)gen1Task,		/* Pointer to the function that implements the task. */
					"gen1Task",	/* Text name for the task.  This is to facilitate debugging only. */
					256,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					GEN1_PRIO,	/* This task will run at this priority */
					&xHandleGen1 );		/* We are not using the task handle. */
	xTaskCreate(	(TaskFunction_t)gen2Task,		/* Pointer to the function that implements the task. */
					"gen2Task",	/* Text name for the task.  This is to facilitate debugging only. */
					256,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					GEN2_PRIO,	/* This task will run at this priority */
					&xHandleGen2 );		/* We are not using the task handle. */

	xTaskCreate(	(TaskFunction_t)rxTask,		/* Pointer to the function that implements the task. */
					"rxTask",	/* Text name for the task.  This is to facilitate debugging only. */
					256,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					RX_PRIO,	/* This task will run at this priority */
					&xHandleRx );		/* We are not using the task handle. */

	vTaskDelete (xHandleMain);
}
/*-----------------------------------------------------------*/


void syncTask( void *pvParameters )
{
	for(;;) {
#if 1
	printf("S "); fflush(stdout);
	uxBits = xEventGroupSetBits(
			sync_id, /* The event group being updated. */
			SYNC_SIGNAL_OK);/* The bits being set. */

#endif // TODO #2
	vTaskDelay (pdMS_TO_TICKS (100));
	}
}



void gen1Task( void *pvParameters )
{
	//EventBits_t uxBits;

	/* Print out the name of this task. */
	//TickType_t = uint32_t
	SensorData sensor1_data;
	sensor1_data.info = 1;

	for(;;) {
#if 1
	uxBits = xEventGroupWaitBits(sync_id	  ,
								SYNC_SIGNAL_OK,
								pdTRUE	      ,
								pdFALSE	      ,
								portMAX_DELAY);
	if((uxBits & SYNC_SIGNAL_OK) == SYNC_SIGNAL_OK) {
		printf("* ");fflush(stdout);
		//send Queue, info(1 or 2), time, distance
		sensor1_data.time = xTaskGetTickCount();
		sensor1_data.data = rand()%101;
		if(xQueueSendToBack(sdata_qid, &sensor1_data,10) != pdPASS )
		{
			printf("1sF\n");fflush(stdout);
		}
	}


#endif
	}
}


void gen2Task( void *pvParameters )
{
	SensorData sensor2_data;
	sensor2_data.info = 2;
	for(;;) {
#if 1
	uxBits = xEventGroupWaitBits(sync_id	  ,
								SYNC_SIGNAL_OK,
								pdTRUE	      ,
								pdFALSE	      ,
								portMAX_DELAY);

		if((uxBits & SYNC_SIGNAL_OK) == SYNC_SIGNAL_OK) {
			//			vTaskDelay(pdMS_TO_TICKS(500));
			// send Queue
			printf(". ");fflush(stdout);
			sensor2_data.time = xTaskGetTickCount();
			sensor2_data.data = rand()%101;
			if(xQueueSendToBack(sdata_qid, &sensor2_data,10) != pdPASS)
			{
				printf("2sF\n");fflush(stdout);
			}
		}

#endif
	}
}

void rxTask(void *pvParameters){

	SensorData* ps1;
	SensorData* ps2;
	SensorData s1, s2;
	char str[100];
	int		   time_diff;
	int 	   min_dis_info;

	for(;;){
		if(uxQueueMessagesWaiting(sdata_qid) >= 2){
			if(xQueueReceive(sdata_qid, &s1, portMAX_DELAY)!=pdPASS){
				printf("frF\n");fflush(stdout);
			}
			if(xQueueReceive(sdata_qid, &s2, portMAX_DELAY)!=pdPASS){
				printf("srF\n");fflush(stdout);
			}
		// sensor matching
		if(s1.info == 1){
			ps1 = &s1;
			ps2 = &s2;
		}else{
			ps1 = &s2;
			ps2 = &s1;
		}
		// smaller distance value sensor info
		if(ps1->data - ps2->data >= 0){
			if(ps1->data == ps2->data) min_dis_info = 0;
			else 					   min_dis_info = 2;
		}else{
			min_dis_info = 1;
		}
		// time diff calculate(tickcount)
		time_diff = (ps1->time >= ps2->time) ? (ps1->time - ps2->time) : (ps2->time - ps1->time);
		sprintf(str,"%d / %d / %d -> %d\n\r",time_diff, ps1->data, ps2->data, min_dis_info);
		printf("%s",str);
		}
		else{
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}


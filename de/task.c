/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Task Priorities ----------------------------------------------------------*/
#define TASK_GEN_PRIO   6
#define TASK_COMP_PRIO  6
#define TASK_PCTX_PRIO  8
#define TASK_PCCMD_PRIO 10

/* RTOS Handles -------------------------------------------------------------*/
osThreadId_t tidGen1, tidGen2, tidComp, tidPcTx, tidPcCmd;
osMutexId_t uartMutex;
osMessageQueueId_t sensorQueue;
osMessageQueueId_t compInputQueue;
osSemaphoreId_t notifySem;

/* External UART Handle -----------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/* Data Structure -----------------------------------------------------------*/
typedef struct {
    int sensor_id;
    unsigned long timestamp;
    int data;
} SensorData;

/* Global Variables ---------------------------------------------------------*/
static SensorData latestSensor1 = {1, 0, 0};
static SensorData latestSensor2 = {2, 0, 0};
static uint8_t rxByte;
static uint8_t cmdBuffer[16];
static uint8_t cmdIndex = 0;

/* Function Prototypes ------------------------------------------------------*/
static void TaskGen(void *pvParameters);
static void TaskComp(void *pvParameters);
static void TaskPcTx(void *pvParameters);
static void TaskPcCmd(void *pvParameters);

/* RTOS Thread Initialization -----------------------------------------------*/
void USER_THREADS(void) {
    sensorQueue = osMessageQueueNew(10, sizeof(SensorData), NULL);
    compInputQueue = osMessageQueueNew(2, sizeof(SensorData), NULL);
    uartMutex = osMutexNew(NULL);
    notifySem = osSemaphoreNew(1, 0, NULL);

    tidGen1 = osThreadNew(TaskGen, (void*)1, NULL);
    tidGen2 = osThreadNew(TaskGen, (void*)2, NULL);
    tidComp = osThreadNew(TaskComp, NULL, NULL);
    tidPcCmd  = osThreadNew(TaskPcCmd, NULL, NULL);
    tidPcTx   = osThreadNew(TaskPcTx, NULL, NULL);
}

/* Sensor Data Generation Task ----------------------------------------------*/
static void TaskGen(void *pvParameters) {
    int id = (int)(uintptr_t)pvParameters;
    SensorData s = { .sensor_id = id };

    for (;;) {
        osDelay(1000);
        s.timestamp = osKernelGetTickCount();
        s.data = rand() % 100;
        osMessageQueuePut(sensorQueue, &s, 0, osWaitForever);
    }
}

/* Sensor Data Comparison Task ----------------------------------------------*/
static void TaskComp(void *pvParameters) {
    SensorData s;

    for (;;) {
        if (osMessageQueueGet(sensorQueue, &s, NULL, osWaitForever) == osOK) {
            if (s.sensor_id == 1) latestSensor1 = s;
            else if (s.sensor_id == 2) latestSensor2 = s;

            osMessageQueueReset(compInputQueue);
            osMessageQueuePut(compInputQueue, &latestSensor1, 0, 0);
            osMessageQueuePut(compInputQueue, &latestSensor2, 0, 0);
        }
    }
}

/* UART Transmission Task ---------------------------------------------------*/
static void TaskPcTx(void *pvParameters) {
    SensorData s1, s2;
    char msg[64];

    for (;;) {
        osDelay(2000);

        if (osMessageQueueGetCount(compInputQueue) >= 2) {
            osMessageQueueGet(compInputQueue, &s1, NULL, 0);
            osMessageQueueGet(compInputQueue, &s2, NULL, 0);

            int diff = abs(s1.data - s2.data);
            snprintf(msg, sizeof(msg), "DIFF:%d (S1:%d S2:%d)\r\n", diff, s1.data, s2.data);

            osMutexAcquire(uartMutex, osWaitForever);
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            osMutexRelease(uartMutex);
        }
    }
}

/* UART Command Handling Task -----------------------------------------------*/
static void TaskPcCmd(void *pvParameters) {
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);

    for (;;) {
        osSemaphoreAcquire(notifySem, osWaitForever);

        char buffer[64];
        if (cmdBuffer[0] == 'A') strcpy(buffer, "ACK_FIRE:A\r\n");
        else if (cmdBuffer[0] == 'B') strcpy(buffer, "ACK_FIRE:B\r\n");
        else strcpy(buffer, "NACK_FIRE\r\n");

        osMutexAcquire(uartMutex, osWaitForever);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
        osMutexRelease(uartMutex);

        memset(cmdBuffer, 0, sizeof(cmdBuffer));
        cmdIndex = 0;
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

/* UART RX Callback ---------------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (rxByte == ';') {
            cmdBuffer[cmdIndex] = '\0';
            osSemaphoreRelease(notifySem);
        } else {
            if (cmdIndex < sizeof(cmdBuffer) - 1)
                cmdBuffer[cmdIndex++] = rxByte;
            HAL_UART_Receive_IT(&huart1, &rxByte, 1);
        }
    }
}

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

/* Priorities */
#define TASK_GEN1_PRIO   6
#define TASK_GEN2_PRIO   6
#define TASK_COMP_PRIO   6
#define TASK_PCTX_PRIO   8
#define TASK_PCCMD_PRIO 10

/* Handles */
TaskHandle_t xHandleGen1, xHandleGen2, xHandleComp, xHandlePcCmd, xHandlePcTx;
QueueHandle_t sensorQueue;
QueueHandle_t compInputQueue;
SemaphoreHandle_t uartMutex;

extern UART_HandleTypeDef huart1;

/* Data structure */
typedef struct {
    int sensor_id;
    unsigned long timestamp;
    int data;
} SensorData;

/* Function Prototypes */
static void TaskGen1(void *pvParameters);
static void TaskGen2(void *pvParameters);
static void TaskComp(void *pvParameters);
static void TaskpcCmd(void *pvParameters);
static void TaskpcTx(void *pvParameters);

/* Globals */
SensorData latestSensor1 = {1, 0, 0};
SensorData latestSensor2 = {2, 0, 0};
uint8_t rxByte;
uint8_t cmdBuffer[16];
uint8_t cmdIndex = 0;

void USER_THREADS(void) {
    sensorQueue = xQueueCreate(10, sizeof(SensorData));
    compInputQueue = xQueueCreate(2, sizeof(SensorData));
    uartMutex = xSemaphoreCreateMutex();

    xTaskCreate(TaskGen1, "TaskGen1", 256, NULL, TASK_GEN1_PRIO, &xHandleGen1);
    xTaskCreate(TaskGen2, "TaskGen2", 256, NULL, TASK_GEN2_PRIO, &xHandleGen2);
    xTaskCreate(TaskComp,  "TaskComp", 256, NULL, TASK_COMP_PRIO, &xHandleComp);
    xTaskCreate(TaskpcCmd, "TaskpcCmd", 256, NULL, TASK_PCCMD_PRIO, &xHandlePcCmd);
    xTaskCreate(TaskpcTx,  "TaskpcTx", 256, NULL, TASK_PCTX_PRIO, &xHandlePcTx);
}

/* Generate random sensor data */
static void TaskGen1(void *pvParameters) {
    SensorData s;
    s.sensor_id = 1;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        s.timestamp = xTaskGetTickCount();
        s.data = rand() % 100;
        xQueueSend(sensorQueue, &s, portMAX_DELAY);
    }
}

static void TaskGen2(void *pvParameters) {
    SensorData s;
    s.sensor_id = 2;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        s.timestamp = xTaskGetTickCount();
        s.data = rand() % 100;
        xQueueSend(sensorQueue, &s, portMAX_DELAY);
    }
}

static void TaskComp(void *pvParameters) {
    SensorData s;
    while (1) {
        if (xQueueReceive(sensorQueue, &s, portMAX_DELAY) == pdPASS) {
            if (s.sensor_id == 1) {
                latestSensor1 = s;
            } else if (s.sensor_id == 2) {
                latestSensor2 = s;
            }

            xQueueReset(compInputQueue);
            xQueueSend(compInputQueue, &latestSensor1, 0);
            xQueueSend(compInputQueue, &latestSensor2, 0);
        }
    }
}

static void TaskpcTx(void *pvParameters) {
    SensorData s1, s2;
    char msg[64];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));

        if (uxQueueMessagesWaiting(compInputQueue) >= 2) {
            xQueueReceive(compInputQueue, &s1, 0);
            xQueueReceive(compInputQueue, &s2, 0);

            int diff = abs(s1.data - s2.data);
            snprintf(msg, sizeof(msg), "DIFF:%d (S1:%d S2:%d)\r\n", diff, s1.data, s2.data);

            xSemaphoreTake(uartMutex, portMAX_DELAY);
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            xSemaphoreGive(uartMutex);
        }
    }
}

static void TaskpcCmd(void *pvParameters) {
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        char buffer[64];
        if (cmdBuffer[0] == 'A')
            strcpy(buffer, "ACK_FIRE:A\r\n");
        else if (cmdBuffer[0] == 'B')
            strcpy(buffer, "ACK_FIRE:B\r\n");
        else
            strcpy(buffer, "NACK_FIRE\r\n");

        xSemaphoreTake(uartMutex, portMAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
        xSemaphoreGive(uartMutex);

        memset(cmdBuffer, 0, sizeof(cmdBuffer));
        cmdIndex = 0;
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (rxByte == ';') {
            cmdBuffer[cmdIndex] = '\0';
            vTaskNotifyGiveFromISR(xHandlePcCmd, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        } else {
            if (cmdIndex < sizeof(cmdBuffer) - 1)
                cmdBuffer[cmdIndex++] = rxByte;
            HAL_UART_Receive_IT(&huart1, &rxByte, 1);
        }
    }
}

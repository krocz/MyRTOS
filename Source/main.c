#include "tinyOS.h"
#include "tConfig.h"
/************************************** 全局变量 ***************************************/
Task_t xTask1, xTask2, xIdleTask;
TaskStack_t xTask1Env[1024], xTask2Env[1024], xTaskIdleEnv[1024];


/************************************** 静态函数声明 ***************************************/
static void vTask1Entry (void * param);
static void vTask2Entry (void * param);
static void vTaskIdleEntry (void * param);


int main ()
{
	vHardwareInit();
	
	vTaskInit(&xTask1, vTask1Entry, (void *)0x11111111, 0, &xTask1Env[1024]);
	vTaskInit(&xTask2, vTask2Entry, (void *)0x22222222, 1, &xTask2Env[1024]);
	vTaskInit(&xIdleTask, vTaskIdleEntry, (void *)0x33333333, TINYOS_PRO_COUNT - 1, &xTaskIdleEnv[1024]);
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	nextTask = taskTable[0];
	idleTask = &tIdleTask;
	
	vTaskStartScheduler();
}





/**********************************************************************************************************
** 应用示例
**********************************************************************************************************/


int iTask1Flag;
int iShareCount;
void vTask1Entry (void * pvParam) 
{
    for (;;) 
    {
		int iVar;
		
		vTaskSchedDisable();
		iVar = iShareCount;
		iVar ++;
		iShareCount = iVar;
		vTaskSchedEnable();
		
        iTask1Flag = 1;
        vTaskDelay(10);
        iTask1Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int iTask2Flag;
void vTask2Entry (void * pvParam) 
{
    for (;;) 
    {
		vTaskSchedDisable();
		iShareCount++;
		vTaskSchedEnable();
		
        iTask2Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask2Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void taskIdleEntry(void *pvParam)
{
	for(;;)
		;
}


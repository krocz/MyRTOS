#include "tinyOS.h"
#include "tConfig.h"
/************************************** 全局变量 ***************************************/
Task_t xTask1, xTask2, xTask3, xIdleTask;
TaskStack_t xTask1Env[1024], xTask2Env[1024], xTask3Env[1024], xTaskIdleEnv[1024];


/************************************** 静态函数声明 ***************************************/
static void prvTask1Entry (void * param);
static void prvTask2Entry (void * param);
static void prvTask3Entry (void * param);
static void prvTaskIdleEntry (void * param);


int main ()
{
	vHardwareInit();
	vTaskSchedInit();
	vTaskDelayedInit();
	
	vTaskInit(&xTask1, prvTask1Entry, (void *)0x11111111, 0, &xTask1Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xTask2, prvTask2Entry, (void *)0x22222222, 1, &xTask2Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xTask3, prvTask3Entry, (void *)0x33333333, 1, &xTask3Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xIdleTask, prvTaskIdleEntry, (void *)0x4444444, TINYOS_PRO_COUNT - 1, &xTaskIdleEnv[TINYOS_STACK_SIZE]);
	
	vTaskSetNext(pxTaskHightestReady());
	vTaskStartScheduler();
}





/**********************************************************************************************************
** 应用示例
**********************************************************************************************************/

void delay(void)
{
	int i;
	for(i = 0; i < 0xff; i++)
	;
}


int iTask1Flag;
int iShareCount;
static void prvTask1Entry (void * pvParam) 
{
    for (;;) 
    {
		int iVar;
		
//		vTaskSchedDisable();
//		iVar = iShareCount;
//		iVar ++;
//		iShareCount = iVar;
//		vTaskSchedEnable();
		
        iTask1Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask1Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int iTask2Flag;
static void prvTask2Entry (void * pvParam) 
{
    for (;;) 
    {
//		vTaskSchedDisable();
//		iShareCount++;
//		vTaskSchedEnable();
		
        iTask2Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(20));
		//vTaskWakeUp(&xTask3);
		//delay();
        iTask2Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(20));
		//vTaskWakeUp(&xTask3);
		//delay();
    }
}

int iTask3Flag;
static void prvTask3Entry (void * pvParam) 
{
    for (;;) 
    {
//		vTaskSchedDisable();
//		iShareCount++;
//		vTaskSchedEnable();
		
        iTask3Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
		vTaskSuspend(&xTask3);
		//delay();
        iTask3Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
		vTaskSuspend(&xTask3);
		//delay();
    }
}

static void prvTaskIdleEntry(void *pvParam)
{
	for(;;)
		;
}


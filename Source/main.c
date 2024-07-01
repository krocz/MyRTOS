#include "tinyOS.h"
/************************************** 全局变量 ***************************************/
Task_t xTask1, xTask2, xTask3, xTask4, xIdleTask;
TaskStack_t xTask1Env[1024], xTask2Env[1024], xTask3Env[1024], xTask4Env[1024], xTaskIdleEnv[1024];

/************************************** 静态函数声明 ***************************************/
static void prvTask1Entry (void * param);
static void prvTask2Entry (void * param);
static void prvTask3Entry (void * param);
static void prvTask4Entry (void * param);
static void prvTaskIdleEntry (void * param);

Timer_t xTime1, xTime2, xTime3;
uint32_t uiBit1, uiBit2, uiBit3;

int main ()
{
	vHardwareInit();
	vTaskSchedInit();
	vTaskDelayedInit();
	vTimerModuleInit();
	
	vTaskInit(&xTask1, prvTask1Entry, (void *)0x11111111, 0, &xTask1Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xTask2, prvTask2Entry, (void *)0x22222222, 1, &xTask2Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xTask3, prvTask3Entry, (void *)0x33333333, 1, &xTask3Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xTask4, prvTask4Entry, (void *)0x44444444, 1, &xTask4Env[TINYOS_STACK_SIZE]);
	vTaskInit(&xIdleTask, prvTaskIdleEntry, (void *)0xffffffff, TINYOS_PRO_COUNT - 1, &xTaskIdleEnv[TINYOS_STACK_SIZE]);
	
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

void vTimerFunc (void * param) 
{
    uint32_t * puiBit = (uint32_t *) param;
	*puiBit = (*puiBit) ^ 0x1;
}

int iFlag = 0;
static void prvTask1Entry (void * pvParam) 
{	
	uint32_t uiStopped = 0;
	
	vTimerInit(&xTime1, 100, 30, vTimerFunc, (void *)&uiBit1, TIMER_CONFIG_TYPE_HARD);
	vTimerStart(&xTime1);
	
	vTimerInit(&xTime2, 200, 40, vTimerFunc, (void *)&uiBit2, TIMER_CONFIG_TYPE_SOFT);
	vTimerStart(&xTime2);
	
	vTimerInit(&xTime3, 300, 0, vTimerFunc, (void *)&uiBit3, TIMER_CONFIG_TYPE_HARD);
	vTimerStart(&xTime3);
	
	for (;;) 
    {
        iTask1Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask1Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
		
		if(uiStopped == 0)
		{
			vTaskDelay(200);
			vTimerStop(&xTime1);
			uiStopped = 1;
		}
    }
}

int iTask2Flag;
static void prvTask2Entry (void * pvParam) 
{
	uint32_t uiResultFlags = 0;
	
    for (;;) 
    {
        iTask2Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask2Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int iTask3Flag;
static void prvTask3Entry (void * pvParam) 
{
    for (;;) 
    {
        iTask3Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask3Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int iTask4Flag;
static void prvTask4Entry (void * pvParam) 
{
    for (;;) 
    {
        iTask4Flag = 1;
        vTaskDelay(pdMS_TO_TICKS(10));
        iTask4Flag = 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void prvTaskIdleEntry(void *pvParam)
{
	for(;;)
		;
}


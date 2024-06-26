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

uint8_t cMem[20][100];
MemBlock_t xMemBlock;

int main ()
{
	vHardwareInit();
	vTaskSchedInit();
	vTaskDelayedInit();
	
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

void task1DestroyFunc (void * param) 
{
    iTask1Flag = 1;
}

int iFlag = 0;
static void prvTask1Entry (void * pvParam) 
{	
	uint8_t i;
	uint8_t (*pListBlock[20])[100];
	vMemBlockInit(&xMemBlock, (uint8_t *)cMem, 100, 20);
	for(i = 0; i < 20; i++)
		uiMemBlockWait(&xMemBlock, (uint8_t **)&pListBlock[i], 0);
	vTaskDelay(pdMS_TO_TICKS(2));
	
	for(i = 0; i < 20; i++)
	{
		memset(pListBlock[i], i, 100);
		vMemBlockNotify(&xMemBlock, (uint8_t *)pListBlock[i]);
		vTaskDelay(pdMS_TO_TICKS(2));
	}
	
	for (;;) 
    {
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
		uint8_t (*pListBlock)[100];
		uiMemBlockWait(&xMemBlock, (uint8_t **)&pListBlock, 0);
		iTask2Flag = *(uint8_t *)pListBlock;
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


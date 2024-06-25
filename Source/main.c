#include "tinyOS.h"
#include "tConfig.h"
/************************************** 全局变量 ***************************************/
Task_t xTask1, xTask2, xTask3, xTask4, xIdleTask;
TaskStack_t xTask1Env[1024], xTask2Env[1024], xTask3Env[1024], xTask4Env[1024], xTaskIdleEnv[1024];

/************************************** 静态函数声明 ***************************************/
static void prvTask1Entry (void * param);
static void prvTask2Entry (void * param);
static void prvTask3Entry (void * param);
static void prvTask4Entry (void * param);
static void prvTaskIdleEntry (void * param);
Mbox_t xMbox1, xMbox2;
void * pvListMbox1MsgBuf[20];
void * pvListMobx2MsgBuf[20];
uint32_t listMsg[20];

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
	vTaskSetCleanCallFunc(pxCurrentTask, task1DestroyFunc, (void *)0);
	
	vMboxInit(&xMbox1, pvListMbox1MsgBuf, 20);
	for (;;) 
    {
		uint32_t i = 0;
		for(i = 0; i < 20; i++)
		{
			listMsg[i] = i;
			uiMboxNotify(&xMbox1, &listMsg[i], MBOXSENDNORMAL);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
		for(i = 0; i < 20; i++)
		{
			listMsg[i] = i;
			uiMboxNotify(&xMbox1, &listMsg[i], MBOXSENDFRONT);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
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
		void * pvMsg;
		uint32_t uiErr = uiMboxWait(&xMbox1, &pvMsg, 10);
		if(uiErr == eErrorNoError)
		{
			iTask2Flag = *(uint32_t *)pvMsg;
			vTaskDelay(pdMS_TO_TICKS(1));
		}
    }
}

int iTask3Flag;
static void prvTask3Entry (void * pvParam) 
{
	vMboxInit(&xMbox2, pvListMobx2MsgBuf, 20);
    for (;;) 
    {
		void * pvMsg;
		uiMboxWait(&xMbox2, &pvMsg, 100);
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


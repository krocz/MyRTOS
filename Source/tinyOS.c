#include "tinyOS.h"
#include "tConfig.h"

void SysTick_Handler(void);
/***************** 静态全局函数声明 ************************/
static void prvTaskSched(void); 
static void prvTaskSystemTickHandler(void);
static void prvSetSysTickPeriod(uint32_t ms);
static Task_t * prxTaskHightestReady (void);
/****************** 全局变量 *************************/

// 当前任务：记录当前是哪个任务正在运行
static Task_t * g_pxCurrentTask;

// 下一个即将运行的任务：在任务切换之前，需要先设置好该值
static Task_t * g_pxNextTask;

static Task_t * g_pxIdleTask;

static Task_t * g_pxTaskTable[2];

// 调度锁计数器
static uint8_t g_cSchedLockCount;

// 位图
static Bitmap_t g_xTaskPrioBitmap;

// 延时队列
static List_t g_xTaskDelayedList;



/**********************************************************************************************************
** Function name        :   vTaskInit
** Descriptions         :   初始化任务
** parameters           :   pxTask           要初始化的任务结构
** parameters           :   pxTaskCode       任务的入口函数
** parameters           :   uiPrio           传递给任务的运行参数
** parameters           :   uiPrio             任务的优先级
** parameters           :   pxStack          任务堆栈栈顶地址
** Returned value       :   无
***********************************************************************************************************/
void vTaskInit(Task_t * pxTask, TaskFunction_pt pxTaskCode, void *pvParam, uint32_t uiPrio, TaskStack_t *pxStack)
{
    *(--pxStack) = (unsigned long)(1<<24);                // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
    *(--pxStack) = (unsigned long)pxTaskCode;                  // 程序的入口地址
    *(--pxStack) = (unsigned long)0x14;                   // R14(LR), 任务不会通过return xxx结束自己，所以未用
    *(--pxStack) = (unsigned long)0x12;                   // R12, 未用
    *(--pxStack) = (unsigned long)0x3;                    // R3, 未用
    *(--pxStack) = (unsigned long)0x2;                    // R2, 未用
    *(--pxStack) = (unsigned long)0x1;                    // R1, 未用
    *(--pxStack) = (unsigned long)pvParam;                  // R0 = param, 传给任务的入口函数
    *(--pxStack) = (unsigned long)0x11;                   // R11, 未用
    *(--pxStack) = (unsigned long)0x10;                   // R10, 未用
    *(--pxStack) = (unsigned long)0x9;                    // R9, 未用
    *(--pxStack) = (unsigned long)0x8;                    // R8, 未用
    *(--pxStack) = (unsigned long)0x7;                    // R7, 未用
    *(--pxStack) = (unsigned long)0x6;                    // R6, 未用
    *(--pxStack) = (unsigned long)0x5;                    // R5, 未用
    *(--pxStack) = (unsigned long)0x4;                    // R4, 未用

    pxTask->pxStack = pxStack;                                // 保存最终的值
	pxTask->uiDelayTicks = 0;
	pxTask->uiPrio = uiPrio;
	
	g_pxTaskTable[uiPrio] = pxTask;
	vBitmapSet(&g_xTaskPrioBitmap, uiPrio);
}


/**********************************************************************************************************
** Function name        :   uiTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入临界区之前的中断状态值
***********************************************************************************************************/
uint32_t uiTaskEnterCritical (void)
{
	uint32_t uiPrimask = __get_PRIMASK();
	__disable_irq();
	return uiPrimask;
}

/**********************************************************************************************************
** Function name        :   vTaskExitCritical
** Descriptions         :   退出临界区
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskExitCritical (uint32_t uiStatus)
{
	__set_PRIMASK(uiStatus);
}



/**********************************************************************************************************
** Function name        :   vTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay(uint32_t uiDelay)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	g_pxCurrentTask->uiDelayTicks = uiDelay;
	vTaskExitCritical(uiStatus);
	prvTaskSched();
}

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void)
{
    g_cSchedLockCount = 0;
}


/**********************************************************************************************************
** Function name        :   vTaskSchedEnable
** Descriptions         :   使能任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedEnable(void)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(g_cSchedLockCount > 0)
	{
		if(--g_cSchedLockCount == 0)
		{
			prvTaskSched();
		}		
	}
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   vTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedDisable(void)
{
	uint32_t status = uiTaskEnterCritical();
	
	if(g_cSchedLockCount < 255)
	{
		g_cSchedLockCount++;
	}
	
	vTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   vHardwareInit
** Descriptions         :   硬件初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHardwareInit(void)
{
	prvSetSysTickPeriod(TINYOS_ONE_TICK_TO_MS);
}





/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler(void)
{
	prvTaskSystemTickHandler();
}


/**********************************************************************************************************
** Function name        :   vTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayedInit (void) 
{
    vListInit(&g_xTaskDelayedList);
}

/**********************************************************************************************************
** Function name        :   tTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWait (Task_t * pxTask, uint32_t uiTicks)
{
	Node_t *pxCur = pxListFirst(&g_xTaskDelayedList);
	int sum = 0;
    
	
	for(int i = 0; i < g_xTaskDelayedList.uiNodeCnt; i++)
	{
		Task_t *pxTemp = pxNodeParent(pxCur, Task_t, uiDelayTicks);
		sum += pxTemp->uiDelayTicks;

		if(sum > uiTicks)
		{
			pxTask->uiDelayTicks = uiTicks - (sum - pxTemp->uiDelayTicks);
			vListInsertForward(&g_xTaskDelayedList, pxCur, &pxTask->xDelayNode);
			break;
		}
		pxCur = pxListNext(&g_xTaskDelayedList, pxCur);
	}
	if(pxCur == &g_xTaskDelayedList.xHeadNode)
	{
		pxTask->uiDelayTicks = uiTicks - sum;
		vListAddLast(&g_xTaskDelayedList, &pxTask->xDelayNode);
	}
	else if(pxTask->uiDelayTicks)
	{
		while(pxCur != &g_xTaskDelayedList.xHeadNode)
		{
			Task_t *pxTemp = pxNodeParent(pxCur, Task_t, uiDelayTicks);
			pxTemp->uiDelayTicks -= pxTask->uiDelayTicks;
			pxCur = pxListNext(&g_xTaskDelayedList, pxCur);
		}
	}
    pxTask->uiState |= TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   vTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   pxTask  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimeTaskWakeUp (Task_t * pxTask)
{
    vListRemove(&g_xTaskDelayedList, &(pxTask->xDelayNode));
    pxTask->uiState &= ~TINYOS_TASK_STATE_DELAYED;
}



/*------------------------------------------------------- 静态函数 --------------------------------------------------------*/


/**********************************************************************************************************
** Function name        :   prvTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTaskSystemTickHandler(void)
{
	int i;
	uint32_t status = uiTaskEnterCritical();
	Node_t *pxNode;
	
	
	
	for(i = 0; i < 2; i++)
	{
		if(g_pxTaskTable[i]->uiDelayTicks > 0)
			g_pxTaskTable[i]->uiDelayTicks--;
	}
	// 防止中断嵌套调用 
	vTaskExitCritical(status);
	prvTaskSched();
}

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   设置定时器中断触发的间隔
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvSetSysTickPeriod(uint32_t ms)
{
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL   = 0;
	/* 使用内部时钟源，使能中断，使能定时器 */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
					 SysTick_CTRL_TICKINT_Msk   |
					 SysTick_CTRL_ENABLE_Msk;
}


/**********************************************************************************************************
** Function name        :   prvTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTaskSched ( void ) 
{    
	Task_t * pxTempTask;
	uint32_t status = uiTaskEnterCritical();
	
	if(g_cSchedLockCount > 0)
	{
		vTaskExitCritical(status);
		return;
	}
	
	pxTempTask = prxTaskHightestReady();
	if(pxTempTask != g_pxCurrentTask)
	{
		g_pxNextTask = pxTempTask;
		vTaskSwitch();
	}

	vTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   prxTaskHightestReady
** Descriptions         :   获取优先级最高的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static Task_t * prxTaskHightestReady (void)
{
	uint32_t uiPrio = uiBitmapGetFirstSet(&g_xTaskPrioBitmap);
	return g_pxTaskTable[uiPrio];
	
}



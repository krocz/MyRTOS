#include "tinyOS.h"
#include "tConfig.h"
#include "tLib.h"

void SysTick_Handler(void);
/***************** 静态全局函数声明 ************************/
static void prvTaskSched(void); 
static void prvTaskSchedUnRdy (Task_t * pxTask);
static void prvTaskSchedRdy (Task_t * pxTask);
static void prvTimeTaskWakeUp (Task_t * pxTask);
static void prvTimeTaskWait (Task_t * pxTask, uint32_t uiTicks);
/****************** 全局变量 *************************/

// 当前任务：记录当前是哪个任务正在运行
Task_t * pxCurrentTask;

// 下一个即将运行的任务：在任务切换之前，需要先设置好该值
Task_t * pxNextTask;

static List_t g_xTaskTable[TINYOS_PRO_COUNT];

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
	pxTask->uiState = TINYOS_TASK_STATE_RDY;
	pxTask->slice = TINYOS_SLICE_MAX;
	pxTask->uiSuspendCount = 0;
	
	vNodeInit(&pxTask->xDelayNode);
	vNodeInit(&pxTask->xLinkNode);                       // 初始化链接结点
	prvTaskSchedRdy(pxTask);
}

/**********************************************************************************************************
** Function name        :   vHardwareInit
** Descriptions         :   硬件初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHardwareInit(void)
{
	vSetSysTickPeriod(TINYOS_ONE_TICK_TO_MS);
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
** Function name        :   vTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay(uint32_t uiDelay)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	// 设置延时值，插入延时队列
	prvTimeTaskWait(pxCurrentTask, uiDelay);
	// 将任务从就绪表中移除
	prvTaskSchedUnRdy(pxCurrentTask);
	vTaskExitCritical(uiStatus);
	prvTaskSched();
}

/**********************************************************************************************************
** Function name        :   vTaskSchedInit
** Descriptions         :   初始化调度器
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedInit (void)
{
	int i = 0;
    g_cSchedLockCount = 0;
	vBitmapInit(&g_xTaskPrioBitmap);
	for(i = 0; i < TINYOS_PRO_COUNT; i++)
	{
		vListInit(&g_xTaskTable[i]);
	}
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
** Function name        :   tTaskSuspend
** Descriptions         :   挂起指定的任务
** parameters           :   task        待挂起的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskSuspend(Task_t * pxTask)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(!(pxTask->uiState & TINYOS_TASK_STATE_DELAYED))
	{
        // 增加挂起计数，仅当该任务被执行第一次挂起操作时，才考虑是否
        // 要执行任务切换操作
        if (++pxTask->uiSuspendCount <= 1)
		{
			pxTask->uiState |= TINYOS_TASK_STATE_SUSPEND;
			prvTaskSchedUnRdy(pxTask);   // 从就绪队列中移除该任务
			
			if(pxTask == pxCurrentTask)  // 如果该任务为当前任务，则进行任务切换
				prvTaskSched();
		}
	}
	
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   vTaskWakeUp
** Descriptions         :   唤醒被挂起的任务
** parameters           :   task        待唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskWakeUp (Task_t * pxTask)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if((pxTask->uiState & TINYOS_TASK_STATE_SUSPEND))
	{
        // 递减挂起计数，如果为0了，则清除挂起标志，同时设置进入就绪状态
        if (--pxTask->uiSuspendCount == 0) 
		{
			pxTask->uiState &= ~TINYOS_TASK_STATE_SUSPEND;
			prvTaskSchedRdy(pxTask);
			
			prvTaskSched();  // 因为被唤醒任务可能优先级更高，所以需要进行任务调度
		}
	}
	
	vTaskExitCritical(uiStatus);
}


/**********************************************************************************************************
** Function name        :   pxTaskHightestReady
** Descriptions         :   获取优先级最高的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
Task_t * pxTaskHightestReady (void)
{
	uint32_t uiPrio = uiBitmapGetFirstSet(&g_xTaskPrioBitmap);
	Node_t *pxFirstNode = pxListFirst(&g_xTaskTable[uiPrio]);
	return pxNodeParent(pxFirstNode, Task_t, xLinkNode);
}

/**********************************************************************************************************
** Function name        :   vTaskSetNext
** Descriptions         :   设置下一个任务
** parameters           :   pxTask 
** Returned value       :   无
***********************************************************************************************************/
void vTaskSetNext(Task_t *pxTask)
{
	pxNextTask = pxTask;
}

/**********************************************************************************************************
** Function name        :   vTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSystemTickHandler(void)
{
	uint32_t status = uiTaskEnterCritical();
	Node_t *pxNode = pxListFirst(&g_xTaskDelayedList);
	
	if(pxNode)
	{
		Task_t *pxTask = pxNodeParent(pxNode, Task_t, xDelayNode);     
		pxTask->uiDelayTicks--;
		while(!pxTask->uiDelayTicks)
		{
			prvTimeTaskWakeUp(pxTask);   // 将任务从延时队列中删除
			prvTaskSchedRdy(pxTask);     // 根据优先级将任务加入就绪优先级数组
			pxNode = pxListNext(&g_xTaskDelayedList, pxNode);
			if(!pxNode) break;
			pxTask = pxNodeParent(pxNode, Task_t, xDelayNode);
		}
	}
	
	/* 如果时间片用完的话，则将当前任务移动到链表的最后一项，从而在调度函数中完成任务切换 */
	if(--pxCurrentTask->slice == 0)
	{
		/* 需要进行链表节点数量判断，因为可能调用延时函数，将任务从就绪链表中清除 */
		if(uiListCount(&g_xTaskTable[pxCurrentTask->uiPrio]) > 0)
		{
			vListRemove(&g_xTaskTable[pxCurrentTask->uiPrio], &pxCurrentTask->xLinkNode);
			pxCurrentTask->slice = TINYOS_SLICE_MAX;
			vListAddLast(&g_xTaskTable[pxCurrentTask->uiPrio], &pxCurrentTask->xLinkNode);			
		}
	}
	
	// 防止中断嵌套调用 
	vTaskExitCritical(status);
	prvTaskSched();
}


/*------------------------------------------------------- 静态函数 --------------------------------------------------------*/


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
	
	pxTempTask = pxTaskHightestReady();
	if(pxTempTask != pxCurrentTask)
	{
		pxNextTask = pxTempTask;
		vTaskSwitch();
	}

	vTaskExitCritical(status);
}


/**********************************************************************************************************
** Function name        :   prvTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTimeTaskWait (Task_t * pxTask, uint32_t uiTicks)
{
	int sum = 0, i;
	Node_t *pxCur = g_xTaskDelayedList.xHeadNode.pxNextNode;
	
	for(i = 0; i < g_xTaskDelayedList.uiNodeCnt; i++)
	{
		Task_t *pxTemp = pxNodeParent(pxCur, Task_t, xDelayNode);
		sum += pxTemp->uiDelayTicks;

		if(sum > uiTicks)
		{
			pxTask->uiDelayTicks = uiTicks - (sum - pxTemp->uiDelayTicks);
			vListInsertForward(&g_xTaskDelayedList, pxCur, &pxTask->xDelayNode);
			break;
		}
		pxCur = pxListNext(&g_xTaskDelayedList, pxCur);
	}
	if(sum <= uiTicks)           // 如果延时队列中所有节点的时延之和小于等于当前任务，则将任务加入队列末尾
	{
		pxTask->uiDelayTicks = uiTicks - sum;
		vListAddLast(&g_xTaskDelayedList, &pxTask->xDelayNode);
	}
	else if(pxTask->uiDelayTicks)  // 否则，任务已经插入到队列中，则将任务后面的节点的时延进行更新
	{
		while(pxCur != &g_xTaskDelayedList.xHeadNode)
		{
			Task_t *pxTemp = pxNodeParent(pxCur, Task_t, xDelayNode);
			pxTemp->uiDelayTicks -= pxTask->uiDelayTicks;
			pxCur = pxListNext(&g_xTaskDelayedList, pxCur);
		}
	}
    pxTask->uiState |= TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   prvTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   pxTask  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTimeTaskWakeUp (Task_t * pxTask)
{
    vListRemove(&g_xTaskDelayedList, &(pxTask->xDelayNode));
    pxTask->uiState &= ~TINYOS_TASK_STATE_DELAYED;
}



/**********************************************************************************************************
** Function name        :   prvTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTaskSchedRdy (Task_t * pxTask)
{
	vListAddLast(&g_xTaskTable[pxTask->uiPrio], &pxTask->xLinkNode);
    vBitmapSet(&g_xTaskPrioBitmap, pxTask->uiPrio);
}

/************************************************************************************************************
** Descriptions         :   prvTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待从就绪列表中移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
static void prvTaskSchedUnRdy (Task_t * pxTask)
{
    vListRemove(&g_xTaskTable[pxTask->uiPrio], &pxTask->xLinkNode);
	if(!g_xTaskTable[pxTask->uiPrio].uiNodeCnt)
		vBitmapClear(&g_xTaskPrioBitmap, pxTask->uiPrio);
}







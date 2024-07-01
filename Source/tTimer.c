#include "tinyOS.h"

// "硬"定时器列表
static List_t xTimerHardList;

// "软"定时器列表
static List_t xTimerSoftList;

// 用于访问软定时器列表的信号量
static Sem_t xTimerProtectSem;

// 用于软定时器任务与中断同步的计数信号量
static Sem_t xTimerTickSem;

static Task_t xTimeTask;
static TaskStack_t xTimerTaskStack[TINYOS_STACK_SIZE];


static void prvTimerSoftTask(void * pvParam);
static void prvTimerCallFuncList (List_t * pxTimerList);

/**********************************************************************************************************
** Function name        :   vTimerInit
** Descriptions         :   初始化定时器
** parameters           :   pxTimer 等待初始化的定时器
** parameters           :   uiDelayTicks 定时器初始启动的延时ticks数。
** parameters           :   uiDurationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   pxTimerFunc 定时器回调函数
** parameters           :   pvArg 传递给定时器回调函数的参数
** parameters           :   uiConfig 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void vTimerInit (Timer_t * pxTimer, uint32_t uiDelayTicks, uint32_t uiDurationTicks,
                 void (*pxTimerFunc) (void * arg), void * pvArg, uint32_t uiConfig)
{
	vNodeInit(&pxTimer->xLinkNode);
	pxTimer->uiStartDelayTicks = uiDelayTicks;
	pxTimer->uiDurationTicks = uiDurationTicks;
	pxTimer->pvTimerFunc = pxTimerFunc;
	pxTimer->pvArg = pvArg;
	pxTimer->uiConfig = uiConfig;
	
	if(uiDelayTicks)
	{
		pxTimer->uiDelayTicks = pxTimer->uiStartDelayTicks;
	}
	else
	{
		pxTimer->uiDelayTicks = uiDurationTicks;
	}
	pxTimer->eState = eTimerCreated;
}

/**********************************************************************************************************
** Function name        :   vTimerStart
** Descriptions         :   启动定时器
** parameters           :   pxTimer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStart (Timer_t * pxTimer)
{
	switch(pxTimer->eState)
	{
		case eTimerCreated:
		case eTimerStopped:
			pxTimer->uiDelayTicks = pxTimer->uiStartDelayTicks ? pxTimer->uiStartDelayTicks : pxTimer->uiDurationTicks;
			pxTimer->eState = eTimerStarted;
			
			// 根据定时器类型加入相应的定时器列表
			if(pxTimer->uiConfig & TIMER_CONFIG_TYPE_HARD)
			{
				// 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
				uint32_t uiStatus = uiTaskEnterCritical();
				// 加入硬定时器列表
				vListAddLast(&xTimerHardList, &pxTimer->xLinkNode);
				vTaskExitCritical(uiStatus);
			}
			else
			{
				// 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
				uiSemWait(&xTimerProtectSem, 0);
				vListAddLast(&xTimerSoftList, &pxTimer->xLinkNode);
				vSemNotify(&xTimerProtectSem);
			}
			break;
		default:
			break;
	}
}

/**********************************************************************************************************
** Function name        :   vTimerStop
** Descriptions         :   终止定时器
** parameters           :   pxTimer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStop (Timer_t * pxTimer)
{
	switch(pxTimer->eState)
	{
		case eTimerStarted:
		case eTimerRunning:
			// 如果已经启动，判断定时器类型，然后从相应的延时列表中移除
			if(pxTimer->uiConfig & TIMER_CONFIG_TYPE_HARD)
			{
				// 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
				uint32_t uiStatus = uiTaskEnterCritical();
				vListRemove(&xTimerHardList, &pxTimer->xLinkNode);
				vTaskExitCritical(uiStatus);
			}
			else
			{
				// 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
				uiSemWait(&xTimerProtectSem, 0);
				vListRemove(&xTimerSoftList, &pxTimer->xLinkNode);
				vSemNotify(&xTimerProtectSem);
			}
			pxTimer->eState = eTimerStopped;
			break;
		default:
			break;
	}
}

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleTickNotify (void)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	// 处理硬定时器列表
	prvTimerCallFuncList(&xTimerHardList);
	vTaskExitCritical(uiStatus);
	
	vSemNotify(&xTimerTickSem);
}

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleInit (void)
{
	vListInit(&xTimerHardList);
	vListInit(&xTimerSoftList);
	vSemInit(&xTimerProtectSem, 1, 1);
	vSemInit(&xTimerTickSem, 0, 0);
	
#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
    #error "The proprity of timer task must be greater then (TINYOS_PRO_COUNT - 1)"
#endif
    vTaskInit(&xTimeTask, prvTimerSoftTask, (void *)0,
        TINYOS_TIMERTASK_PRIO, &xTimerTaskStack[TINYOS_STACK_SIZE]);
}


/**********************************************************************************************************
** Function name        :   vTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   pxTimer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerDestroy (Timer_t * pxTimer)
{
	vTimerStop(pxTimer);
	pxTimer->eState = eTimerDestroyed;
}

/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   pxTimer 查询的定时器
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tTimerGetInfo (Timer_t * pxTimer, TimerInfo_t * pxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	pxInfo->uiStartDelayTicks = pxTimer->uiStartDelayTicks;
	pxInfo->uiDurationTicks   = pxTimer->uiDurationTicks;
	pxInfo->pvTimerFunc       = pxTimer->pvTimerFunc;
	pxInfo->pvArg             = pxTimer->pvArg;
	pxInfo->uiConfig          = pxTimer->uiConfig;
	pxInfo->eSstate           = pxTimer->eState;
	vTaskExitCritical(uiStatus);
}



/**********************************************************************************************************
** Function name        :   prvTimerSoftTask
** Descriptions         :   处理软定时器列表的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTimerSoftTask(void * pvParam)
{
	for(;;)
	{
        // 等待系统节拍发送的中断事件信号
        uiSemWait(&xTimerTickSem, 0);

        // 获取软定时器列表的访问权限
        uiSemWait(&xTimerProtectSem, 0);

        // 处理软定时器列表
        prvTimerCallFuncList(&xTimerSoftList);

        // 释放定时器列表访问权限
        vSemNotify(&xTimerProtectSem);
	}
}

/**********************************************************************************************************
** Function name        :   prvTimerCallFuncList
** Descriptions         :   遍历指定的定时器列表，调用各个定时器处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvTimerCallFuncList (List_t * pxTimerList)
{
	Node_t * pxNode = pxListFirst(pxTimerList);
	
	while(pxNode)
	{
		Timer_t * pxTimer = pxNodeParent(pxNode, Timer_t, xLinkNode);
		
		// 如果延时已到，则调用定时器处理函数
		if((!pxTimer->uiDelayTicks) || (--pxTimer->uiDelayTicks == 0))
		{
			pxTimer->eState = eTimerRunning;
			pxTimer->pvTimerFunc(pxTimer->pvArg);
			pxTimer->eState = eTimerStarted;
			
			if(pxTimer->uiDurationTicks > 0)
				pxTimer->uiDelayTicks = pxTimer->uiDurationTicks;
			else
			{
				vListRemove(pxTimerList, &pxTimer->xLinkNode);
				pxTimer->eState = eTimerStopped;
			}
		}
		pxNode = pxListNext(pxTimerList, pxNode);
	}
}

#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   pxEvent 事件控制块
** parameters           :   eType 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void vEventInit (Event_t * pxEvent, EventType_e eType)
{
	pxEvent->eType = eType;
	vListInit(&pxEvent->xWaitList);
}


/**********************************************************************************************************
** Function name        :   vEventWait
** Descriptions         :   让指定在事件控制块上等待事件发生
** parameters           :   pxEvent 事件控制块
** parameters           :   pxTask 等待事件发生的任务
** parameters           :   pvMsg 事件消息存储的具体位置
** parameters           :   uiState 消息类型
** parameters           :   uiTImeout 等待多长时间
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
void vEventWait(Event_t * pxEvent, Task_t * pxTask, void * pvMsg, uint32_t uiState, uint32_t uiTImeout)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	pxTask->uiState |= uiState;     // 标记任务处于等待某种事件的状态
	pxTask->pxWaitEvent = pxEvent;  // 设置任务等待的事件结构
	pxTask->pvEventMsg = pvMsg;     // 设置任务等待事件的消息存储位置
	pxTask->uiWaitEventResult = eErrorNoError;   //// 清空事件的等待结果

	// 将任务从就绪链表中移除
	vTaskSchedUnRdy(pxTask);
	
	// 将任务插入到等待队列中
	vListAddLast(&pxEvent->xWaitList, &pxTask->xEventNode);

	// 如果发现有设置超时，在同时插入到延时队列中
	// 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
	if(uiTImeout)
	{
		vTimeTaskWait(pxTask, uiTImeout);
	}
	
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   pxEventWakeUp
** Descriptions         :   从事件控制块中唤醒首个等待的任务
** parameters           :   pxEvent 事件控制块
** parameters           :   pvMsg 事件消息
** parameters           :   uiResult 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
Task_t * pxEventWakeUp (Event_t * pxEvent, void * pvMsg, uint32_t uiResult)
{
	Node_t * pxNode;
	Task_t * pxTask;
	
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if((pxNode = pxListRemoveFirst(&pxEvent->xWaitList)) != (Node_t *)0)
	{
		pxTask = (Task_t *)pxNodeParent(pxNode, Task_t, xEventNode);
		
		// 设置收到的消息、结构，清除相应的等待标志位
		pxTask->pxWaitEvent = (Event_t *)0;
		pxTask->pvEventMsg = pvMsg;
		pxTask->uiWaitEventResult = uiResult;
		pxTask->uiState &= ~TINYOS_TASK_WAIT_MASK;
		
		// 任务申请了超时等待，这里检查下，将其从延时队列中移除
		if(pxTask->uiDelayTicks)
			vTimeTaskWakeUp(pxTask);
		
		vTaskSchedRdy(pxTask);
	}
	
	vTaskExitCritical(uiStatus);
	
	return pxTask;
}

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   pxTask 待移除的任务
** parameters           :   pvMsg 事件消息
** parameters           :   uiResult 告知事件的等待结果
** Returned value       :   无
***********************************************************************************************************/
void vEventRemoveTask (Task_t * pxTask, void * pvMsg, uint32_t uiResult)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	// 将任务从所在的等待队列中移除
	// 注意，这里没有检查waitEvent是否为空。既然是从事件中移除，那么认为就不可能为空
	vListRemove(&pxTask->pxWaitEvent->xWaitList, &pxTask->xEventNode);

	// 设置收到的消息、结构，清除相应的等待标志位
	pxTask->pxWaitEvent = (Event_t *)0;
	pxTask->pvEventMsg = pvMsg;
	pxTask->uiWaitEventResult = uiResult;
	pxTask->uiState &= ~TINYOS_TASK_WAIT_MASK;
	
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   uiEventRemoveAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t uiEventRemoveAll (Event_t *pxEvent, void * pvMsg, uint32_t uiResult)
{
    Node_t  * pxNode;
    uint32_t uiCnt;
    
    // 进入临界区
    uint32_t uiStatus = uiTaskEnterCritical();

    // 获取等待中的任务数量
    uiCnt = uiListCount(&pxEvent->xWaitList);

    // 遍历所有等待中的任务
    while ((pxNode = pxListRemoveFirst(&pxEvent->xWaitList)) != (Node_t *)0)
    {                                                                   
        // 转换为相应的任务结构                                          
        Task_t * pxTask = (Task_t *)pxNodeParent(pxNode, Task_t, xEventNode);
        
		// 设置收到的消息、结构，清除相应的等待标志位
		pxTask->pxWaitEvent = (Event_t *)0;
		pxTask->pvEventMsg = pvMsg;
		pxTask->uiWaitEventResult = uiResult;
		pxTask->uiState &= ~TINYOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (pxTask->uiDelayTicks != 0)
        { 
            vTimeTaskWakeUp(pxTask);
        }

        // 将任务加入就绪队列
        vTaskSchedRdy(pxTask);        
    }  

    // 退出临界区
    vTaskExitCritical(uiStatus); 

    return  uiCnt;	
}

/**********************************************************************************************************
** Function name        :   uiEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t uiEventWaitCount (Event_t * pxEvent)
{  
    uint32_t uiCount = 0;

    // 进入临界区
    uint32_t uiStatus = uiTaskEnterCritical();

    uiCount = uiListCount(&pxEvent->xWaitList);  

    // 退出临界区
    vTaskExitCritical(uiStatus);     

    return uiCount;
}  

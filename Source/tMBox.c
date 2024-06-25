#include "tMbox.h"
#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   vMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   pxMbox 等待初始化的邮箱
** parameters           :   pdvMsgBuf 消息存储缓冲区
** parameters           :   uiMaxCnt 最大计数
** Returned value       :   无
***********************************************************************************************************/
void vMboxInit(Mbox_t * pxMbox, void **pdvMsgBuf, uint32_t uiMaxCnt)
{
	vEventInit(&pxMbox->xEvent, eEventTypeSem);
	
	pxMbox->pdvMsgBuf = pdvMsgBuf;
	pxMbox->uiMaxCnt = uiMaxCnt;
	pxMbox->uiRead = 0;
	pxMbox->uiWrite = 0;
	pxMbox->uiCnt = 0;
}

/**********************************************************************************************************
** Function name        :   uiMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   pxMbox 等待的邮箱
** parameters           :   pdvMsg 消息存储缓存区
** parameters           :   uiWaitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,eErrorResourceUnavaliable.eErrorNoError,eErrorTimeout
***********************************************************************************************************/
uint32_t uiMboxWait (Mbox_t * pxMbox, void **pdvMsg, uint32_t uiWaitTicks)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(pxMbox->uiCnt)   // 如果当前邮箱中有消息
	{
		pxMbox->uiCnt --;
		*pdvMsg = pxMbox->pdvMsgBuf[pxMbox->uiRead++];
		if(pxMbox->uiRead >= pxMbox->uiMaxCnt)
			pxMbox->uiRead = 0;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else               // 如果当前邮箱中没有消息  
	{
		vEventWait(&pxMbox->xEvent, pxCurrentTask, (void *)0, eEventTypeMbox, uiWaitTicks);
		vTaskExitCritical(uiStatus);
		// 执行任务调度
		vTaskSched();
		// 当切换回来时，取出相关信息
		*pdvMsg = pxCurrentTask->pvEventMsg;
		return pxCurrentTask->uiWaitEventResult;
	}
}


/**********************************************************************************************************
** Function name        :   uiMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   pxMbox 获取消息的邮箱
** parameters           :   pdvMsg 消息存储缓存区
** Returned value       :   获取结果, eErrorResourceUnavaliable.eErrorNoError
***********************************************************************************************************/
uint32_t uiMboxNoWaitGet (Mbox_t * pxMbox, void **pdvMsg)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(pxMbox->uiCnt)   // 如果当前邮箱中有消息
	{
		pxMbox->uiCnt --;
		*pdvMsg = pxMbox->pdvMsgBuf[pxMbox->uiRead++];
		if(pxMbox->uiRead >= pxMbox->uiMaxCnt)
			pxMbox->uiRead = 0;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else               // 如果当前邮箱中没有消息  
	{
		vTaskExitCritical(uiStatus);
		return eErrorResourceUnavaliable;
	}
}

/**********************************************************************************************************
** Function name        :   uiMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   pxMbox 操作的信号量
** parameters           :   pvMsg 发送的消息
** parameters           :   uiNotifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t uiMboxNotify (Mbox_t * pxMbox, void * pvMsg, uint32_t uiNotifyOption)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(uiEventWaitCount(&pxMbox->xEvent) > 0)  // 如果有任务正在等待
	{
		Task_t * pxTask = pxEventWakeUp(&pxMbox->xEvent, pvMsg, eErrorNoError);
		if(pxTask->uiPrio > pxCurrentTask->uiPrio)
			vTaskSched();
	}
	else                                      // 如果没有，则将消息插入到缓冲区中
	{
		if(pxMbox->uiCnt >= pxMbox->uiMaxCnt)
		{
			vTaskExitCritical(uiStatus);
			return eErrorResourceFull;
		}
		
		if(uiNotifyOption & MBOXSENDFRONT)   // 将消息插入到缓冲区最前面
		{
			
			if(!pxMbox->uiRead)
				pxMbox->uiRead = pxMbox->uiMaxCnt - 1;
			else
				pxMbox->uiRead--;
			pxMbox->pdvMsgBuf[pxMbox->uiRead] = pvMsg;
		}
		else
		{
			pxMbox->pdvMsgBuf[pxMbox->uiWrite++] = pvMsg;
			if(pxMbox->uiWrite >= pxMbox->uiMaxCnt)
				pxMbox->uiWrite = 0;
		}
		pxMbox->uiCnt++;
	}
	vTaskExitCritical(uiStatus);
	return eErrorNoError;
}


/**********************************************************************************************************
** Function name        :   vMboxFlush
** Descriptions         :   清空邮箱中所有消息
** parameters           :   pxMbox 等待清空的邮箱
** Returned value       :   无
***********************************************************************************************************/
void vMboxFlush(Mbox_t * pxMbox)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	// 仅在没有等待任务时，才说明可能存在消息
	if(uiEventWaitCount(&pxMbox->xEvent) == 0)  
	{
		pxMbox->uiCnt = 0;
		pxMbox->uiRead = 0;
		pxMbox->uiWrite = 0;
	}
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   tMboxDestroy
** Descriptions         :   销毁邮箱
** parameters           :   mbox 需要销毁的邮箱
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiMboxDestroy(Mbox_t * pxMbox)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	uint32_t uiCnt = uiEventRemoveAll(&pxMbox->xEvent, (void *)0, eErrorDel);
	vTaskExitCritical(uiStatus);
	if(uiCnt)           // 清空过程中可能有任务就绪，执行一次调度
		vTaskSched();
	
	return uiCnt;
}

/**********************************************************************************************************
** Function name        :   vMboxGetInfo
** Descriptions         :   查询状态信息
** parameters           :   pxMbox 查询的邮箱
** parameters           :   pxMboxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMboxGetInfo(Mbox_t * pxMbox, MboxInfo_t *pxMboxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	pxMboxInfo->uiCnt     = pxMbox->uiCnt;
	pxMboxInfo->uiTaskCnt = uiEventWaitCount(&pxMbox->xEvent);
	pxMboxInfo->uiMaxCnt  = pxMbox->uiMaxCnt;
	vTaskExitCritical(uiStatus);
}

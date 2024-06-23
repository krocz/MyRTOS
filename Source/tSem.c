#include "tSem.h"
#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   初始化信号量
** parameters           :   uiStartCount 初始的计数
** parameters           :   uiMaxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void vSemInit (Sem_t * pxSem, uint32_t uiStartCount, uint32_t uiMaxCount)
{
	vEventInit(&pxSem->xEvent, eEventTypeSem);
	pxSem->uiMaxCnt = uiMaxCount;
	if(uiMaxCount)
		pxSem->uiCnt = (uiStartCount > uiMaxCount)? uiMaxCount : uiStartCount;
	else
		pxSem->uiCnt = uiStartCount;
}


/**********************************************************************************************************
** Function name        :   uiSemWait
** Descriptions         :   等待信号量
** parameters           :   pxSem 等待的信号量
** parameters           :   uiWaitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiSemWait (Sem_t * pxSem, uint32_t uiWaitTicks)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(pxSem->uiCnt > 0)
	{
		--pxSem->uiCnt;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else
	{
		// 然后将任务插入事件队列中
		vEventWait(&pxSem->xEvent, pxCurrentTask, (void *)0, eEventTypeSem, uiWaitTicks);
		vTaskExitCritical(uiStatus);
		// 最后再执行一次事件调度，以便于切换到其它任务
		vTaskSched();
		// 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结构
		return pxCurrentTask->uiWaitEventResult;
	}
}

/**********************************************************************************************************
** Function name        :   uiSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiSemNoWaitGet (Sem_t * pxSem)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(pxSem->uiCnt > 0)
	{
		--pxSem->uiCnt;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else
	{
		vTaskExitCritical(uiStatus);
		return eErrorResourceUnavaliable;
	}
}


/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vSemNotify (Sem_t * pxSem)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if (uiEventWaitCount(&pxSem->xEvent))
	{
		Task_t *pxTask = pxEventWakeUp(&pxSem->xEvent, (void *)0, eErrorNoError);
		if(pxTask->uiPrio > pxCurrentTask->uiPrio)
			vTaskSched();
	}
	else
	{
		++pxSem->uiCnt;
		if(pxSem->uiMaxCnt != 0 && pxSem->uiCnt > pxSem->uiMaxCnt)
			pxSem->uiCnt = pxSem->uiMaxCnt;
	}
	
	vTaskExitCritical(uiStatus);
}


/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   查询信号量的状态信息
** parameters           :   pxSem 查询的信号量
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vSemGetInfo (Sem_t * pxSem, SemInfo_t * pxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	pxInfo->uiCnt      = pxSem->uiCnt;
	pxInfo->uiMaxCnt   = pxSem->uiMaxCnt;
	pxInfo->uiTaskCnt  = uiEventWaitCount(&pxSem->xEvent);
	
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   uiSemDestroy
** Descriptions         :   销毁信号量
** parameters           :   pxSem 需要销毁的信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiSemDestroy(Sem_t * pxSem)
{
	uint32_t uiStatus = uiTaskEnterCritical();	
	uint32_t uiCnt = uiEventRemoveAll(&pxSem->xEvent, (void *)0, eErrorDel);
	pxSem->uiCnt = 0;
	vTaskExitCritical(uiStatus);
	
	// 清空过程中可能有任务就绪，执行一次调度
	if(uiCnt > 0)
		vTaskSched();
	
	return uiCnt;
}



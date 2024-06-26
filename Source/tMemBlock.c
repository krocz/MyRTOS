#include "tinyOS.h"


/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   pxMemBlock 等待初始化的存储控制块
** parameters           :   pcMemStart 存储区的起始地址
** parameters           :   uiBlockSize 每个块的大小
** parameters           :   uiBlockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void vMemBlockInit (MemBlock_t * pxMemBlock, uint8_t * pcMemStart, uint32_t uiBlockSize, uint32_t uiBlockCnt)
{
	
	uint8_t * pcMemBlockStart = pcMemStart;
	uint8_t * pcMemBlockEnd   = pcMemBlockStart + uiBlockSize * uiBlockCnt;
	
	// 每个存储块需要来放置链接指针，所以空间至少要比tNode大
	// 即便如此，实际用户可用的空间并没有少，用户将节点从链表中取出，根据首地址来使用存储块
	if(uiBlockSize < sizeof(Node_t))
		return;
	
	vEventInit(&pxMemBlock->xEvent, eEventTypeMemBlock);
	pxMemBlock->pvMemStart = (void *)pcMemStart;
	pxMemBlock->uiBlockSize = uiBlockSize;
	pxMemBlock->uiMaxCnt = uiBlockCnt;
	
	vListInit(&pxMemBlock->xBlockList);
	while(pcMemBlockStart < pcMemBlockEnd)
	{
		vNodeInit((Node_t *)pcMemBlockStart);
		vListAddLast(&pxMemBlock->xBlockList, (Node_t *)pcMemBlockStart);
		
		pcMemBlockStart += uiBlockSize;
	}
}

/**********************************************************************************************************
** Function name        :   uiMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   pxMemBlock 等待的存储块
** parameters			:   pdcMem 存储块存储的地址
** parameters           :   uiWaitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiMemBlockWait (MemBlock_t * pxMemBlock, uint8_t ** pdcMem, uint32_t uiWaitTicks)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(uiListCount(&pxMemBlock->xBlockList))   // 如果还有存储块
	{
		*pdcMem = (uint8_t *)pxListRemoveFirst(&pxMemBlock->xBlockList);
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else                                      // 如果没有存储块
	{
		vEventWait(&pxMemBlock->xEvent, pxCurrentTask, (void *)0, eEventTypeMemBlock, uiWaitTicks);
		vTaskExitCritical(uiStatus);
		vTaskSched();    // 调度到其他任务
		*pdcMem = (uint8_t *)pxCurrentTask->pvEventMsg;
		return pxCurrentTask->uiWaitEventResult;
	}
}

/**********************************************************************************************************
** Function name        :   uiMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   pxMemBlock 等待的存储块
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiMemBlockNoWaitGet (MemBlock_t * pxMemBlock, void ** pdcMem)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(uiListCount(&pxMemBlock->xBlockList))   // 如果还有存储块
	{
		*pdcMem = (uint8_t *)pxListRemoveFirst(&pxMemBlock->xBlockList);
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else                                      // 如果没有存储块
	{
		vTaskExitCritical(uiStatus);
		return eErrorResourceUnavaliable;	
	}
}

/**********************************************************************************************************
** Function name        :   vMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   pxMemBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockNotify (MemBlock_t * pxMemBlock, uint8_t * pcMem)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(uiEventWaitCount(&pxMemBlock->xEvent))   // 如果有等待存储块的任务，则直接分配给第一个
	{
		Task_t *pxTask = pxEventWakeUp(&pxMemBlock->xEvent, (void *)pcMem, eErrorNoError);
		if(pxTask->uiPrio < pxCurrentTask->uiPrio)
			vTaskSched();
	}
	else                                       // 如果没有则考虑是否能够插入到存储链表中
	{
		if(uiListCount(&pxMemBlock->xBlockList) < pxMemBlock->uiMaxCnt)
		{
			vNodeInit((Node_t *)pcMem);
			vListAddLast(&pxMemBlock->xBlockList, (Node_t *)pcMem);
		}
	}
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   pxMemBlock 存储控制块
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockGetInfo (MemBlock_t * pxMemBlock, MemBlockInfo_t * pxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	pxInfo->uiBlockSize = pxMemBlock->uiBlockSize;
	pxInfo->uiCnt       = uiListCount(&pxMemBlock->xBlockList);
	pxInfo->uiMaxCnt    = pxMemBlock->uiMaxCnt;
	pxInfo->uiTaskCnt   = uiEventWaitCount(&pxMemBlock->xEvent);
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   uiMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   pxMemBlock 需要销毁的存储控制块
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiMemBlockDestroy (MemBlock_t * pxMemBlock)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	uint32_t uiCnt = uiEventRemoveAll(&pxMemBlock->xEvent, (void *)0, eErrorDel);
	vTaskExitCritical(uiStatus);
	if(uiCnt > 0)
		vTaskSched();
	
	return uiCnt;
}

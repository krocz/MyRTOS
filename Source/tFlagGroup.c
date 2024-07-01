#include "tinyOS.h"

static uint32_t pruiFlagGroupCheckAndCosume(FlagGroup_t * pxFlagGroup, uint32_t uiType, uint32_t * puiFlags);

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   pxFlagGroup 等待初始化的事件标志组
** parameters           :   uiFlags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupInit (FlagGroup_t * pxFlagGroup, uint32_t uiFlags)
{
	vEventInit(&pxFlagGroup->xEvent, eEventTypeFlagGroup);
	pxFlagGroup->uiFlags = uiFlags;
}


/**********************************************************************************************************
** Function name        :   uiFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   pxFlagGroup 等待的事件标志组
** parameters           :   uiWaitType 等待的事件类型
** parameters           :   uiRequestFlag 请求的事件标志
** parameters           :   puiResultFlag 等待标志结果
** parameters           :   uiWaitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiFlagGroupWait (FlagGroup_t * pxFlagGroup, uint32_t uiWaitType, uint32_t uiRequestFlag,
						uint32_t * puiResultFlag, uint32_t uiWaitTicks)
{
	uint32_t uiResult;
	uint32_t uiFlags = uiRequestFlag;
	uint32_t uiStatus = uiTaskEnterCritical();
	uiResult = pruiFlagGroupCheckAndCosume(pxFlagGroup, uiWaitType, &uiFlags);
	
	if(uiResult == eErrorNoError)   // 如果资源充足，则成功获取资源并返回
	{
		*puiResultFlag = uiResult;
		vTaskExitCritical(uiStatus);
	}
	else                            // 资源不足，则放入等待队列
	{
		pxCurrentTask->uiWaitFlagsCheckType = uiWaitType;
		pxCurrentTask->uiWaitEventFlags    = uiRequestFlag;
		vEventWait(&pxFlagGroup->xEvent, pxCurrentTask, (void *)0, eEventTypeFlagGroup, uiWaitTicks);
		vTaskExitCritical(uiStatus);
		
		vTaskSched();
		
		*puiResultFlag = pxCurrentTask->uiWaitEventFlags;
		uiResult = pxCurrentTask->uiWaitEventResult;
	}
	
	return uiResult;
}

/**********************************************************************************************************
** Function name        :   uiFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   pxFlagGroup 获取的事件标志组
** parameters           :   uiWaitType 获取的事件类型
** parameters           :   uiRequstFlag 请求的事件标志
** parameters           :   puiResultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiFlagGroupNoWaitGet (FlagGroup_t * pxFlagGroup, uint32_t uiWaitType, uint32_t uiRequstFlag, uint32_t * puiResultFlag)
{
	uint32_t uiFlags = uiRequstFlag;
	uint32_t uiStatus = uiTaskEnterCritical();
	uint32_t uiResult = pruiFlagGroupCheckAndCosume(pxFlagGroup, uiWaitType, &uiRequstFlag);
	vTaskExitCritical(uiStatus);
	*puiResultFlag = uiFlags;
	return uiResult;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   向事件标志组中发送特定的标志
** parameters           :   pxFlagGroup 操作事件标志组
** parameters           :   cIsSet 设置方式
** parameters           :   uiFlags 标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupNotify (FlagGroup_t * pxFlagGroup, uint8_t cIsSet, uint32_t uiFlags)
{
	List_t *pxWaitList;
	Node_t *pxNode;
	uint32_t uiResult;
	uint8_t cSched = 0;
	uint32_t uiStatus = uiTaskEnterCritical();
	if(cIsSet)
		pxFlagGroup->uiFlags |= uiFlags;    // 置1事件
	else 
		pxFlagGroup->uiFlags &= ~uiFlags;   // 清0事件

	
	pxWaitList = &pxFlagGroup->xEvent.xWaitList;
	pxNode = pxListFirst(pxWaitList);
	// 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
	while(pxNode)
	{
		Task_t * pxTask = pxNodeParent(pxNode, Task_t, xEventNode);
		uint32_t uiFlags = pxTask->uiWaitEventFlags;
		uiResult = pruiFlagGroupCheckAndCosume(pxFlagGroup, pxTask->uiWaitFlagsCheckType, &uiFlags);
		if(uiResult == eErrorNoError)
		{
			pxTask->uiWaitEventFlags = uiFlags;
			vEventWakeUpTask(&pxFlagGroup->xEvent, pxTask, (void *)0, eErrorNoError);
			cSched = 1;
		}
		pxNode = pxListNext(pxWaitList, pxNode);
	}
	
	// 如果有任务就绪，则执行一次调度
	if(cSched)
	{
		vTaskSched();
	}
	vTaskExitCritical(uiStatus);
}


/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   pxFlagGroup 事件标志组
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupGetInfo(FlagGroup_t * pxFlagGroup, FlagGroupInfo * pxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	pxInfo->uiFlags = pxFlagGroup->uiFlags;
	pxInfo->uiTaskCnt = uiEventWaitCount(&pxFlagGroup->xEvent);
	
	vTaskExitCritical(uiStatus);
}

/**********************************************************************************************************
** Function name        :   uiFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiFlagGroupDestroy(FlagGroup_t * pxFlagGroup)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	uint32_t uiCnt = uiEventRemoveAll(&pxFlagGroup->xEvent, (void *)0, eErrorDel);
	
	vTaskExitCritical(uiStatus);
	
	if(uiCnt > 0)
		vTaskSched();
	
	return uiCnt;
}


/**********************************************************************************************************
** Function name        :   pruiFlagGroupCheckAndCosume
** Descriptions         :   辅助函数。检查并消耗掉事件标志
** parameters           :   pxFlagGroup 等待初始化的事件标志组
** parameters           :   uiType 事件标志检查类型
** parameters           :   puiFlags 待检查事件标志存储地址和检查结果存储位置
** Returned value       :   tErrorNoError 事件匹配；tErrorResourceUnavaliable 事件未匹配
***********************************************************************************************************/
static uint32_t pruiFlagGroupCheckAndCosume(FlagGroup_t * pxFlagGroup, uint32_t uiType, uint32_t * puiFlags)
{
	uint32_t uiSrcFlags = *puiFlags;
	uint32_t uiIsSet = uiType & TFLAGGROUP_SET;
	uint32_t uiIsAll = uiType & TFLAGGROUP_ALL;
	uint32_t uiIsConsume = uiType & TFLAGGROUP_CONSUME;
	
	uint32_t uiCalFlag = uiIsSet? (pxFlagGroup->uiFlags & uiSrcFlags) : (~pxFlagGroup->uiFlags & uiSrcFlags);
	
	if((uiIsAll && uiSrcFlags == uiCalFlag) || (!uiIsAll && uiCalFlag))
	{
		if(uiIsConsume)
		{
			if(uiIsSet)
				pxFlagGroup->uiFlags &= ~uiSrcFlags;
			else
				pxFlagGroup->uiFlags |= uiSrcFlags;
		}
		*puiFlags = uiCalFlag;
		return eErrorNoError;
	}
	
	*puiFlags = uiCalFlag;
	return eErrorResourceUnavaliable;
}

#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void vMutexInit(Mutex_t * pxMutex)
{
	vEventInit(&pxMutex->xEvent, eEventTYpeMutex);
	
	pxMutex->uiLockedCnt = 0;
	pxMutex->pxOwner = (Task_t *)0;
	pxMutex->uiOwnerOriginalPrio = TINYOS_PRO_COUNT;
}


/**********************************************************************************************************
** Function name        :   uiMutexWait
** Descriptions         :   等待信号量
** parameters           :   pxMutex 等待的信号量
** parameters           :   uiWaitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiMutexWait(Mutex_t * pxMutex, uint32_t uiWaitTicks)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	if(pxMutex->uiLockedCnt <= 0)
	{
		// 如果没有锁定，则使用当前任务进行锁定
		pxMutex->pxOwner = pxCurrentTask;
		pxMutex->uiOwnerOriginalPrio = pxCurrentTask->uiPrio;
		pxMutex->uiLockedCnt++;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else
	{
		// 信号量已经被锁定
		if(pxMutex->pxOwner == pxCurrentTask)
		{
			// 如果是信号量的拥有者再次wait，简单增加计数
			pxMutex->uiLockedCnt++;
			vTaskExitCritical(uiStatus);
			return eErrorNoError;
		}
		else
		{
			// 如果是信号量拥有者之外的任务wait，则要检查下是否需要使用
			// 优先级继承方式处理
			if(pxCurrentTask->uiPrio < pxMutex->pxOwner->uiPrio)
			{
				Task_t * pxOwnerTask = pxMutex->pxOwner;
				
                // 如果当前任务的优先级比拥有者优先级更高，则使用优先级继承
                // 提升原拥有者的优先
				if(pxOwnerTask->uiState == TINYOS_TASK_STATE_RDY)
				{
					// 任务处于就绪状态时，更改任务在就绪表中的位置
					vTaskSchedUnRdy(pxOwnerTask);
					pxOwnerTask->uiPrio = pxCurrentTask->uiPrio;
					vTaskSchedRdy(pxOwnerTask);
				}
				else
				{
					// 其它任务状态，只需要修改优先级
					pxOwnerTask->uiPrio = pxCurrentTask->uiPrio;
				}
			}
			
			// 当前任务进入等待队列中
			vEventWait(&pxMutex->xEvent, pxCurrentTask, (void *)0, eEventTYpeMutex, uiWaitTicks);
			vTaskExitCritical(uiStatus);
			// 执行调度， 切换至其它任务
			vTaskSched();
			return pxCurrentTask->uiWaitEventResult;
		}
	}
}

/**********************************************************************************************************
** Function name        :   uiMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   pxMutex 获取的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiMutexNoWaitGet(Mutex_t * pxMutex)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(pxMutex->uiLockedCnt <= 0)
	{
		// 如果没有锁定，则使用当前任务进行锁定
		pxMutex->pxOwner = pxCurrentTask;
		pxMutex->uiOwnerOriginalPrio = pxCurrentTask->uiPrio;
		pxMutex->uiLockedCnt++;
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	else
	{
		// 信号量已经被锁定
		if(pxMutex->pxOwner == pxCurrentTask)
		{
			// 如果是信号量的拥有者再次wait，简单增加计数
			pxMutex->uiLockedCnt++;
			vTaskExitCritical(uiStatus);
			return eErrorNoError;
		}
		
		vTaskExitCritical(uiStatus);
		return eErrorResourceUnavaliable;
	}
}

/**********************************************************************************************************
** Function name        :   uiMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   pxMutex 操作的信号量
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t uiMutexNotify(Mutex_t * pxMutex)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	
	if(pxMutex->uiLockedCnt <= 0)
	{
		// 锁定计数为0，信号量未被锁定，直接退出
		vTaskExitCritical(uiStatus);
		return eErrorNoError;
	}
	
	if(pxMutex->pxOwner != pxCurrentTask)
	{
		// 不是拥有者释放，认为是非法
		vTaskExitCritical(uiStatus);
		return eErrorOwner;
	}
	
	if(--pxMutex->uiLockedCnt > 0)
	{
		// 减1后计数仍不为0, 直接退出，不需要唤醒等待的任务
		vTaskExitCritical(uiStatus);
		return eErrorOwner;
	}
	
	// 是否有发生优先级继承
	if (pxMutex->uiOwnerOriginalPrio != pxMutex->pxOwner->uiPrio)
	{
		// 有发生优先级继承，恢复拥有者的优先级
		if (pxMutex->pxOwner->uiState == TINYOS_TASK_STATE_RDY)
		{
			// 任务处于就绪状态时，更改任务在就绪表中的位置
			vTaskSchedUnRdy(pxMutex->pxOwner);
			pxCurrentTask->uiPrio = pxMutex->uiOwnerOriginalPrio;
			vTaskSchedRdy(pxMutex->pxOwner);
		}
		else
		{
			// 其它状态，只需要修改优先级
			pxCurrentTask->uiPrio = pxMutex->uiOwnerOriginalPrio;
		}
	}
	
	if(uiEventWaitCount(&pxMutex->xEvent) > 0)
	{
		// 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
		Task_t * pxTask = pxEventWakeUp(&pxMutex->xEvent, (void *)0, eErrorNoError);
		
		pxMutex->pxOwner = pxTask;
		pxMutex->uiOwnerOriginalPrio = pxTask->uiPrio;
		pxMutex->uiLockedCnt++;
		
		// 如果这个任务的优先级更高，就执行调度，切换过去
		if(pxTask->uiPrio < pxCurrentTask->uiPrio)
		{
			vTaskSched();
		}
	}
	vTaskExitCritical(uiStatus);
	return eErrorNoError;
}

/**********************************************************************************************************
** Function name        :   uiMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   pxMutex 销毁互斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiMutexDestroy(Mutex_t * pxMutex)
{
	uint32_t uiCnt = 0;
	uint32_t uiStatus = uiTaskEnterCritical();
	
	// 信号量是否已经被锁定，未锁定时没有任务等待，不必处理
	if(pxMutex->uiLockedCnt > 0)
	{
		// 是否有发生优先级继承
		if (pxMutex->uiOwnerOriginalPrio != pxMutex->pxOwner->uiPrio)
		{
			// 有发生优先级继承，恢复拥有者的优先级
			if (pxMutex->pxOwner->uiState == TINYOS_TASK_STATE_RDY)
			{
				// 任务处于就绪状态时，更改任务在就绪表中的位置
				vTaskSchedUnRdy(pxMutex->pxOwner);
				pxMutex->pxOwner->uiPrio = pxMutex->uiOwnerOriginalPrio;
				vTaskSchedRdy(pxMutex->pxOwner);
			}
			else
			{
				// 其它状态，只需要修改优先级
				pxMutex->pxOwner->uiPrio = pxMutex->uiOwnerOriginalPrio;
			}
		}
		
		// 清空事件控制块中的任务
		uiCnt = uiEventRemoveAll(&pxMutex->xEvent, (void *)0, eErrorDel);
			
		// 清空过程中可能有任务就绪，执行一次调度
		if(uiCnt)
		{
			vTaskSched();
		}
	}
	vTaskExitCritical(uiStatus);
	return uiCnt;
}

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   pxMutex 查询的互斥信号量
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMutexGetInfo (Mutex_t * pxMutex, MutexInfo_t * pxInfo)
{
	uint32_t uiStatus = uiTaskEnterCritical();
	pxInfo->uiTaskCnt = uiEventWaitCount(&pxMutex->xEvent);
	pxInfo->uiOwnerPrio = pxMutex->uiOwnerOriginalPrio;
	
	if(pxMutex->pxOwner != (Task_t *)0)
		pxInfo->uiInheritedPrio = pxMutex->pxOwner->uiPrio;
	else
		pxInfo->uiInheritedPrio = TINYOS_PRO_COUNT;
	pxInfo->pxOwner = pxMutex->pxOwner;
	pxInfo->uiLockedCnt = pxMutex->uiLockedCnt;
	
	vTaskExitCritical(uiStatus);	
}

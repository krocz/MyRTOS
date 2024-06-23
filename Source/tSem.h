#ifndef _TSEM_H
#define _TSEM_H

#include "tEvent.h"

typedef struct _tSem{
	Event_t xEvent;
	uint32_t uiCnt;
	uint32_t uiMaxCnt;
}Sem_t;


typedef struct _tSemInfo{
	uint32_t uiCnt;         // 当前信号量的计数
	uint32_t uiMaxCnt;      // 信号量允许的最大计数
	uint32_t uiTaskCnt;     // 当前等待的任务计数
}SemInfo_t;

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   初始化信号量
** parameters           :   uiStartCount 初始的计数
** parameters           :   uiMaxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void vSemInit (Sem_t * pxSem, uint32_t uiStartCount, uint32_t uiMaxCount);

/**********************************************************************************************************
** Function name        :   uiSemWait
** Descriptions         :   等待信号量
** parameters           :   pxSem 等待的信号量
** parameters           :   uiWaitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiSemWait (Sem_t * pxSem, uint32_t uiWaitTicks);

/**********************************************************************************************************
** Function name        :   uiSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiSemNoWaitGet (Sem_t * pxSem);

/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vSemNotify (Sem_t * pxSem);

/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   查询信号量的状态信息
** parameters           :   pxSem 查询的信号量
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vSemGetInfo (Sem_t * pxSem, SemInfo_t * pxInfo);

/**********************************************************************************************************
** Function name        :   uiSemDestroy
** Descriptions         :   销毁信号量
** parameters           :   pxSem 需要销毁的信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiSemDestroy(Sem_t * pxSem);
#endif 

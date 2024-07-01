#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct {
	// 事件控制块
	Event_t xEvent;
	// 已被锁定的次数
	uint32_t uiLockedCnt;
	// 拥有者
	Task_t * pxOwner;
	// 拥有者原始的优先级
	uint32_t uiOwnerOriginalPrio;
}Mutex_t;

// 互斥信号量查询结构
typedef struct  {
    // 等待的任务数量
    uint32_t uiTaskCnt;

    // 拥有者任务的优先级
    uint32_t uiOwnerPrio;

    // 继承优先级
    uint32_t uiInheritedPrio;

    // 当前信号量的拥有者
    Task_t * pxOwner;

    // 锁定次数
    uint32_t uiLockedCnt;
}MutexInfo_t;

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void vMutexInit(Mutex_t * pxMutex);

/**********************************************************************************************************
** Function name        :   uiMutexWait
** Descriptions         :   等待信号量
** parameters           :   pxMutex 等待的信号量
** parameters           :   uiWaitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiMutexWait(Mutex_t * pxMutex, uint32_t uiWaitTicks);

/**********************************************************************************************************
** Function name        :   uiMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   pxMutex 获取的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiMutexNoWaitGet(Mutex_t * pxMutex);

/**********************************************************************************************************
** Function name        :   uiMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   pxMutex 操作的信号量
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t uiMutexNotify(Mutex_t * pxMutex);

/**********************************************************************************************************
** Function name        :   uiMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   pxMutex 销毁互斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiMutexDestroy(Mutex_t * pxMutex);

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   pxMutex 查询的互斥信号量
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMutexGetInfo (Mutex_t * pxMutex, MutexInfo_t * pxInfo);

#endif /* TMUTEX_H */

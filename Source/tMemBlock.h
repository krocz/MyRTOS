#ifndef _MemBlock_t_H
#define _MemBlock_t_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct {
	// 事件控制块
	Event_t xEvent;
	// 存储块的首地址
	void * pvMemStart;
	// 每个存储块的大小
	uint32_t uiBlockSize;
	// 总的存储块的个数
	uint32_t uiMaxCnt;
	// 存储块列表
	List_t xBlockList;
}MemBlock_t;

typedef struct _tMemBlockInfo
{
	// 当前存储块的计数
    uint32_t uiCnt;

    // 允许的最大计数
    uint32_t uiMaxCnt;

    // 每个存储块的大小
    uint32_t uiBlockSize;

    // 当前等待的任务计数
    uint32_t uiTaskCnt;
}MemBlockInfo_t;


/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   pxMemBlock 等待初始化的存储控制块
** parameters           :   pcMemStart 存储区的起始地址
** parameters           :   uiBlockSize 每个块的大小
** parameters           :   uiBlockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void vMemBlockInit (MemBlock_t * pxMemBlock, uint8_t * pcMemStart, uint32_t uiBlockSize, uint32_t uiBlockCnt);

/**********************************************************************************************************
** Function name        :   uiMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   pxMemBlock 等待的存储块
** parameters			:   pdcMem 存储块存储的地址
** parameters           :   uiWaitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiMemBlockWait (MemBlock_t * pxMemBlock, uint8_t ** pdcMem, uint32_t uiWaitTicks);

/**********************************************************************************************************
** Function name        :   uiMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   pxMemBlock 等待的存储块
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiMemBlockNoWaitGet (MemBlock_t * pxMemBlock, void ** pdcMem);

/**********************************************************************************************************
** Function name        :   vMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   pxMemBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockNotify (MemBlock_t * pxMemBlock, uint8_t * pcMem);

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   pxMemBlock 存储控制块
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockGetInfo (MemBlock_t * pxMemBlock, MemBlockInfo_t * pxInfo);

/**********************************************************************************************************
** Function name        :   uiMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   pxMemBlock 需要销毁的存储控制块
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiMemBlockDestroy (MemBlock_t * pxMemBlock);

#endif




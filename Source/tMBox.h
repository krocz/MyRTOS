#ifndef _Mbox_t_H
#define _Mbox_t_H

#include "tConfig.h"
#include "tEvent.h"

#define MBOXSENDNORMAL             0x00        // 正常发送发送至缓冲区
#define MBOXSENDFRONT              0x01        // 消息发送至缓冲区头部


typedef struct _Mbox_t{
	// 事件控制块
	Event_t xEvent;
	// 当前消息数量
	uint32_t uiCnt;
	// 读取消息的索引
	uint32_t uiRead;
	// 写消息的索引
	uint32_t uiWrite;
	// 最大允许容纳的消息数量
	uint32_t uiMaxCnt;
	// 消息存储缓冲区
	void **pdvMsgBuf;
}Mbox_t;

/**********************************************************************************************************
** Function name        :   vMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   pxMbox 等待初始化的邮箱
** parameters           :   pdvMsgBuf 消息存储缓冲区
** parameters           :   uiMaxCnt 最大计数
** Returned value       :   无
***********************************************************************************************************/
void vMboxInit (Mbox_t * pxMbox, void ** pdvMsgBuf, uint32_t uiMaxCnt);

/**********************************************************************************************************
** Function name        :   uiMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   pxMbox 等待的邮箱
** parameters           :   pdvMsg 消息存储缓存区
** parameters           :   uiWaitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t uiMboxWait (Mbox_t * pxMbox, void **pdvMsg, uint32_t uiWaitTicks);

/**********************************************************************************************************
** Function name        :   uiMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   pxMbox 获取消息的邮箱
** parameters           :   pdvMsg 消息存储缓存区
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiMboxNoWaitGet (Mbox_t * pxMbox, void **pdvMsg);

/**********************************************************************************************************
** Function name        :   uiMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   pxMbox 操作的信号量
** parameters           :   pvMsg 发送的消息
** parameters           :   uiNotifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t uiMboxNotify (Mbox_t * pxMbox, void * pvMsg, uint32_t uiNotifyOption);

#endif

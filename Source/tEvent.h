#ifndef _TEVENT_H
#define _TEVENT_H

#include "tLib.h"
#include "tTask.h"

typedef enum {
	eEventTypeUnknown = 0,     // 未知类型
	eEventTypeSem,             // 信号量类型
	eEventTypeMbox,            // 邮箱类型
	eEventTypeMemBlock,        // 存储块类型
	eEventTypeFlagGroup,
	eEventTYpeMutex,
}EventType_e;

typedef struct _Event{
	EventType_e eType;
	List_t xWaitList;
}Event_t;

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   pxEvent 事件控制块
** parameters           :   eType 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void vEventInit (Event_t * pxEvent, EventType_e eType);

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
void vEventWait(Event_t * pxEvent, Task_t * pxTask, void * pvMsg, uint32_t uiState, uint32_t uiTImeout);

/**********************************************************************************************************
** Function name        :   pxEventWakeUp
** Descriptions         :   从事件控制块中唤醒首个等待的任务
** parameters           :   pxEvent 事件控制块
** parameters           :   pvMsg 事件消息
** parameters           :   uiResult 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
Task_t * pxEventWakeUp (Event_t * pxEvent, void * pvMsg, uint32_t uiResult);

/**********************************************************************************************************
** Function name        :   vEventWakeUpTask
** Descriptions         :   从事件控制块中唤醒指定任务
** parameters           :   event 事件控制块
** parameters           :   task 等待唤醒的任务
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
void vEventWakeUpTask (Event_t * pxEvent, Task_t * pxTask, void * pvMsg, uint32_t uiResult);

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   pxTask 待移除的任务
** parameters           :   pvMsg 事件消息
** parameters           :   uiResult 告知事件的等待结果
** Returned value       :   无
***********************************************************************************************************/
void vEventRemoveTask (Task_t * pxTask, void * pvMsg, uint32_t uiResult);

/**********************************************************************************************************
** Function name        :   uiEventRemoveAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t uiEventRemoveAll (Event_t *pxEvent, void * pvMsg, uint32_t uiResult);

/**********************************************************************************************************
** Function name        :   uiEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t uiEventWaitCount (Event_t * pxEvent);

#endif /* TEVENT_H */


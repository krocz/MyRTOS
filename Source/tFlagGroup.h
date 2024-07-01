#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H
#include "tConfig.h"
#include "tEvent.h"

typedef struct {
	Event_t xEvent;
	uint32_t uiFlags;
}FlagGroup_t;

typedef struct {
	uint32_t uiFlags;    // 当前事件标志
	
	uint32_t uiTaskCnt;  // 当前等待的任务计数
}FlagGroupInfo;


#define TFLAGGROUP_SET_BIT     1
#define TFLAGGROUP_CLEAR_BIT   0

#define	TFLAGGROUP_CLEAR		(0x0 << 0)
#define	TFLAGGROUP_SET			(0x1 << 0)   
#define	TFLAGGROUP_ANY			(0x0 << 1)
#define	TFLAGGROUP_ALL			(0x1 << 1)

#define TFLAGGROUP_SET_ALL		(TFLAGGROUP_SET | TFLAGGROUP_ALL)
#define	TFLAGGROUP_SET_ANY		(TFLAGGROUP_SET | TFLAGGROUP_ANY)
#define TFLAGGROUP_CLEAR_ALL	(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
#define TFLAGGROUP_CLEAR_ANY	(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)

#define	TFLAGGROUP_CONSUME		(0x1 << 7)

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   pxFlagGroup 等待初始化的事件标志组
** parameters           :   uiFlags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupInit (FlagGroup_t * pxFlagGroup, uint32_t uiFlags);

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   向事件标志组中发送特定的标志
** parameters           :   pxFlagGroup 操作事件标志组
** parameters           :   cIsSet 设置方式
** parameters           :   uiFlags 标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupNotify (FlagGroup_t * pxFlagGroup, uint8_t cIsSet, uint32_t uiFlags);

/**********************************************************************************************************
** Function name        :   uiFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   pxFlagGroup 获取的事件标志组
** parameters           :   uiWaitType 获取的事件类型
** parameters           :   uiRequstFlag 请求的事件标志
** parameters           :   puiResultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t uiFlagGroupNoWaitGet (FlagGroup_t * pxFlagGroup, uint32_t uiWaitType, uint32_t uiRequstFlag, uint32_t * puiResultFlag);

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
						uint32_t * puiResultFlag, uint32_t uiWaitTicks);


/**********************************************************************************************************
** Function name        :   uiFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t uiFlagGroupDestroy(FlagGroup_t * pxFlagGroup);


/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   pxFlagGroup 事件标志组
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupGetInfo(FlagGroup_t * pxFlagGroup, FlagGroupInfo * pxInfo);

#endif


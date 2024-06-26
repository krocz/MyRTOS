#ifndef _TINYOS_H_
#define _TINYOS_H_

// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>

// tinyOS的内核库文件
#include "tLib.h"

// TinyOS的配置文件
#include "tConfig.h"

// 事件控制头文件
#include "tEvent.h"

#include "ARMCM3.h"

// 任务头文件
#include "tTask.h"

#include "tSem.h"

#include "tMbox.h"

#include "tMemBlock.h"

typedef enum {
	eErrorNoError = 0,                  // 没有错误
	eErrorTimeout,                      // 等待超时
	eErrorResourceUnavaliable,          // 资源不足
	eErrorDel,						    // 被删除
	eErrorResourceFull,                 // 资源缓冲区不足
}Error_e;

/**********************************************************************************************************
** Function name        :   vHardwareInit
** Descriptions         :   硬件初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHardwareInit(void);

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   设置定时器中断触发的间隔
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vSetSysTickPeriod(uint32_t ms);

/**********************************************************************************************************
** Function name        :   vTaskSetCleanCallFunc
** Descriptions         :   设置任务被删除时调用的清理函数
** parameters           :   pxTask  待设置的任务
** parameters           :   pvClean  清理函数入口地址
** parameters           :   pvParam  传递给清理函数的参数
** Returned value       :   无
***********************************************************************************************************/
void vTaskSetCleanCallFunc (Task_t * pxTask, void (*pvClean)(void *param), void * pvParam);


/**********************************************************************************************************
** Function name        :   vTaskForceDelete
** Descriptions         :   强制删除指定的任务
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskForceDelete(Task_t * pxTask);


/**********************************************************************************************************
** Function name        :   vTaskRequestDelete
** Descriptions         :   请求删除某个任务，由任务自己决定是否删除自己
** parameters           :   pxTask  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskRequestDelete(Task_t * pxTask);

/**********************************************************************************************************
** Function name        :   cTaskIsRequestedDelete
** Descriptions         :   是否已经被请求删除自己
** parameters           :   无
** Returned value       :   非0表示请求删除，0表示无请求
***********************************************************************************************************/
uint8_t cTaskIsRequestedDelete (void);

/**********************************************************************************************************
** Function name        :   vTaskDeleteSelf
** Descriptions         :   删除当前任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDeleteSelf (void);

/**********************************************************************************************************
** Function name        :   vTaskGetInfo
** Descriptions         :   获取任务相关信息
** parameters           :   task 需要查询的任务
** parameters           :   info 任务信息存储结构
** Returned value       :   无
***********************************************************************************************************/
void vTaskGetInfo (Task_t * pxTask, TaskInfo_t * pxInfo);

#endif /* _TINYOS_H_ */


#ifndef _TINYOS_H_
#define _TINYOS_H_

#include <stdint.h>
#include "ARMCM3.h"
#include "tLib.h"

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

typedef uint32_t TaskStack_t;

typedef struct {
	TaskStack_t *pxStack;
	uint32_t uiDelayTicks;
	uint32_t uiPrio;
	Node_t xDelayNode;
	Node_t xLinkNode;
	uint32_t uiSlice;
	uint32_t uiState;
	uint32_t uiSuspendCount;
	uint8_t cRequsetDeleteFlag;
	void (*vCleanResource) (void *param);
	void * pvCleanParam;
}Task_t;

typedef struct {
	uint32_t uiDelayTicks;
	uint32_t uiPrio;
	uint32_t uiState;
	uint32_t uiSlice;
	uint32_t uiSuspendCount;
}TaskInfo_t;


typedef void (*TaskFunction_pt)(void *);

// 当前任务：记录当前是哪个任务正在运行
extern Task_t * pxCurrentTask;

// 下一个即将运行的任务：在任务切换之前，需要先设置好该值
extern Task_t * pxNextTask;

/**********************************************************************************************************
** Function name        :   vTaskInit
** Descriptions         :   初始化任务
** parameters           :   pxTask           要初始化的任务结构
** parameters           :   pxTaskCode       任务的入口函数
** parameters           :   uiPrio           传递给任务的运行参数
** parameters           :   prio             任务的优先级
** parameters           :   pxStack          任务堆栈栈顶地址
** Returned value       :   无
***********************************************************************************************************/
void vTaskInit(Task_t * pxTask, TaskFunction_pt pxTaskCode, void *pvParam, uint32_t uiPrio, uint32_t *pxStack);

/**********************************************************************************************************
** Function name        :   vTaskSetNext
** Descriptions         :   设置下一个任务
** parameters           :   pxTask 
** Returned value       :   无
***********************************************************************************************************/
void vTaskSetNext(Task_t *pxTask);

/**********************************************************************************************************
** Function name        :   vTaskStartScheduler
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskStartScheduler(void);

/**********************************************************************************************************
** Function name        :   vHardwareInit
** Descriptions         :   硬件初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHardwareInit(void);

/**********************************************************************************************************
** Function name        :   vTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayedInit (void);

/**********************************************************************************************************
** Function name        :   vTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay(uint32_t uiDelay);


/**********************************************************************************************************
** Function name        :   uiTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入临界区之前的中断状态值
***********************************************************************************************************/
uint32_t uiTaskEnterCritical (void);


/**********************************************************************************************************
** Function name        :   vTaskExitCritical
** Descriptions         :   退出临界区
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskExitCritical (uint32_t uiStatus);


/**********************************************************************************************************
** Function name        :   vTaskSchedInit
** Descriptions         :   初始化调度器
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedInit (void);

/**********************************************************************************************************
** Function name        :   vTaskSchedEnable
** Descriptions         :   使能任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedEnable(void);

/**********************************************************************************************************
** Function name        :   vTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedDisable(void);


/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   进行一次任务切换，tinyOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSwitch(void);

/**********************************************************************************************************
** Function name        :   tTaskSuspend
** Descriptions         :   挂起指定的任务
** parameters           :   task        待挂起的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskSuspend(Task_t * pxTask);

/**********************************************************************************************************
** Function name        :   vTaskWakeUp
** Descriptions         :   唤醒被挂起的任务
** parameters           :   task        待唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskWakeUp (Task_t * pxTask);


/**********************************************************************************************************
** Function name        :   pxTaskHightestReady
** Descriptions         :   获取优先级最高的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
Task_t * pxTaskHightestReady (void);

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
** Function name        :   cTaskIsRequestedDelete
** Descriptions         :   是否已经被请求删除自己
** parameters           :   无
** Returned value       :   非0表示请求删除，0表示无请求
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


#ifndef _TTASK_H
#define _TTASK_H

#include <stdint.h>
#include "tLib.h"

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

#define TINYOS_TASK_WAIT_MASK                   (0xFF << 16)

typedef uint32_t TaskStack_t;

typedef struct {
	TaskStack_t *pxStack;
	uint32_t uiDelayTicks;
	uint32_t uiPrio;
	Node_t xDelayNode;
	Node_t xLinkNode;
	Node_t xEventNode;
	uint32_t uiSlice;
	uint32_t uiState;
	uint32_t uiSuspendCount;
	uint8_t cRequsetDeleteFlag;
	void (*vCleanResource) (void *param);
	void * pvCleanParam;
	
    // 任务正在等待的事件类型
    struct _Event * pxWaitEvent;

    // 等待事件的消息存储位置
    void * pvEventMsg;

    // 等待事件的结果
    uint32_t uiWaitEventResult;	
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
** Function name        :   vTaskStartScheduler
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskStartScheduler(void);

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
** Function name        :   vTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedRdy (Task_t * pxTask);

/************************************************************************************************************
** Descriptions         :   vTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待从就绪列表中移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void vTaskSchedUnRdy (Task_t * pxTask);

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
** Function name        :   vTaskSetNext
** Descriptions         :   设置下一个任务
** parameters           :   pxTask 
** Returned value       :   无
***********************************************************************************************************/
void vTaskSetNext(Task_t *pxTask);

/**********************************************************************************************************
** Function name        :   pxTaskHightestReady
** Descriptions         :   获取优先级最高的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
Task_t * pxTaskHightestReady (void);

/************************************************************************************************************
** Descriptions         :   vTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待从就绪列表中移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void vTaskSchedRemove (Task_t * pxTask);

/**********************************************************************************************************
** Function name        :   vTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSched ( void );

/**********************************************************************************************************
** Function name        :   vTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimeTaskWait (Task_t * pxTask, uint32_t uiTicks);

/**********************************************************************************************************
** Function name        :   vTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   pxTask  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimeTaskWakeUp (Task_t * pxTask);

/**********************************************************************************************************
** Function name        :   vTimeTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** input parameters     :   task  需要移除的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimeTaskRemove (Task_t * pxTask);

#endif

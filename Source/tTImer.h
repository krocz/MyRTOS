#ifndef Timer_t_H
#define Timer_t_H

#include "tConfig.h"
#include "tEvent.h"

// 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

typedef enum {
    eTimerCreated = 0,          // 定时器已经创建
    eTimerStarted,          // 定时器已经启动
    eTimerRunning,          // 定时器正在执行回调函数
    eTimerStopped,          // 定时器已经停止
    eTimerDestroyed         // 定时器已经销毁
}TimerState_e;

// 软定时器状态信息
typedef struct {
    // 初次启动延后的ticks数
    uint32_t uiStartDelayTicks;

    // 周期定时时的周期tick数
    uint32_t uiDurationTicks;

    // 定时回调函数
    void (*pvTimerFunc) (void * arg);

    // 传递给回调函数的参数
    void * pvArg;

    // 定时器配置参数
    uint32_t uiConfig;

    // 定时器状态
    TimerState_e eSstate;
}TimerInfo_t;



// 软定时器结构
typedef struct _Timer_t
{
    // 链表结点
    Node_t xLinkNode;

    // 初次启动延后的ticks数
    uint32_t uiStartDelayTicks;

    // 周期定时时的周期tick数
    uint32_t uiDurationTicks;

    // 当前定时递减计数值
    uint32_t uiDelayTicks;

    // 定时回调函数
    void (*pvTimerFunc) (void * arg);

    // 传递给回调函数的参数
    void * pvArg;

    // 定时器配置参数
    uint32_t uiConfig;

    // 定时器状态
    TimerState_e eState;
}Timer_t;

/**********************************************************************************************************
** Function name        :   vTimerInit
** Descriptions         :   初始化定时器
** parameters           :   pxTimer 等待初始化的定时器
** parameters           :   uiDelayTicks 定时器初始启动的延时ticks数。
** parameters           :   uiDurationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   pxTimerFunc 定时器回调函数
** parameters           :   pvArg 传递给定时器回调函数的参数
** parameters           :   uiConfig 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void vTimerInit (Timer_t * pxTimer, uint32_t uiDelayTicks, uint32_t uiDurationTicks,
                 void (*pxTimerFunc) (void * arg), void * pvArg, uint32_t uiConfig);

/**********************************************************************************************************
** Function name        :   vTimerStart
** Descriptions         :   启动定时器
** parameters           :   pxTimer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStart (Timer_t * pxTimer);

/**********************************************************************************************************
** Function name        :   vTimerStop
** Descriptions         :   终止定时器
** parameters           :   pxTimer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStop (Timer_t * pxTimer);

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleTickNotify (void);

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleInit (void);

/**********************************************************************************************************
** Function name        :   vTimerInitTask
** Descriptions         :   初始化软定时器任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerInitTask(void);

/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   pxTimer 查询的定时器
** parameters           :   pxInfo 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tTimerGetInfo (Timer_t * pxTimer, TimerInfo_t * pxInfo);
	
/**********************************************************************************************************
** Function name        :   vTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   pxTimer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerDestroy (Timer_t * pxTimer);

#endif 


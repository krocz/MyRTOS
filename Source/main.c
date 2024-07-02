#include "tinyOS.h"




int main ()
{
	// 优先初始化tinyOS的核心功能
	vTaskSchedInit();
	// 初始化延时队列
	vTaskDelayedInit();
	// 初始化定时器模块
	vTimerModuleInit();
	// 初始化时钟节拍
	vTimeTickInit();
	// 初始化cpu统计
	vInitCpuUsageStat();
	// 创建空闲任务
	vIdleTaskInit();
		
	vTaskSetNext(pxTaskHightestReady());
	vTaskStartScheduler();
}








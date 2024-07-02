#include "tinyOS.h"

extern void vTaskSystemTickHandler(void);
extern uint32_t uiTickCount; 

// 空闲任务计数与最大计数
uint32_t uiIdleCount;
uint32_t uiIdleMaxCount;
Task_t xIdleTask;
TaskStack_t xTaskIdleEnv[TINYOS_IDLETASK_STACK_SIZE];

static float fCpuUsage;                      // cpu使用率统计
static uint32_t uiEnableCpuUsageStat;         // 是否使能cpu统计
static void prvTaskIdleEntry (void * param);
static void prvCpuUsageSyncWithSysTick (void);
/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler(void)
{
	vTaskSystemTickHandler();
}


/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   设置定时器中断触发的间隔
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vSetSysTickPeriod(uint32_t ms)
{
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL   = 0;
	/* 使用内部时钟源，使能中断，使能定时器 */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
					 SysTick_CTRL_TICKINT_Msk   |
					 SysTick_CTRL_ENABLE_Msk;
}


/**********************************************************************************************************
** Function name        :   vInitCpuUsageStat
** Descriptions         :   初始化cpu统计
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vInitCpuUsageStat(void)
{
	uiIdleCount = 0;
	uiIdleMaxCount = 0;
	fCpuUsage = 0;
	uiEnableCpuUsageStat = 0;
}

/**********************************************************************************************************
** Function name        :   vIdleTaskInit
** Descriptions         :   初始化空闲任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vIdleTaskInit(void)
{
	vTaskInit(&xIdleTask, prvTaskIdleEntry, (void *)0xffffffff, TINYOS_PRO_COUNT - 1, xTaskIdleEnv, sizeof(xTaskIdleEnv));
}

/**********************************************************************************************************
** Function name        :   fGetCpuUsage
** Descriptions         :   获取CPU利用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float fGetCpuUsage(void)
{
	return fCpuUsage;
}


/**********************************************************************************************************
** Function name        :   vCheckCpuUsage
** Descriptions         :   检查cpu使用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vCheckCpuUsage(void)
{
	if(!uiEnableCpuUsageStat)
	{
		uiEnableCpuUsageStat = 1;
		uiTickCount = 0;
		return;
	}
	
	if(uiTickCount == TICKS_PER_SEC)
	{
		// 统计最初1s内的最大计数值
		uiIdleMaxCount = uiIdleCount;
		uiIdleCount = 0;
		
		// 计数完毕，开启调度器，允许切换到其它任务
		vTaskSchedEnable();
	}
	else if(uiTickCount % TICKS_PER_SEC == 0)
	{
		// 之后每隔1s统计一次，同时计算cpu利用率
		fCpuUsage = 100 - (uiIdleCount * 100.0 / uiIdleMaxCount);
		uiIdleCount = 0;
	}
}

/**********************************************************************************************************
** Function name        :   prvCpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void prvCpuUsageSyncWithSysTick (void)
{
    // 等待与时钟节拍同步
    while (uiEnableCpuUsageStat == 0)
    {
        ;;
    }
}

static void prvTaskIdleEntry(void *pvParam)
{
	// 禁止调度，防止后面在创建任务时切换到其它任务中去
    vTaskSchedDisable();
	
	// 初始化App相关配置
    vAppInit();

    // 初始化定时器任务
    vTimerInitTask();
	
	// 启动系统时钟节拍
	vSetSysTickPeriod(TINYOS_ONE_TICK_TO_MS);
	
	// 等待与时钟同步
    prvCpuUsageSyncWithSysTick();
	for(;;)
	{
		uint32_t uiStatus = uiTaskEnterCritical();
		uiIdleCount++;
		vTaskExitCritical(uiStatus);
	}
}





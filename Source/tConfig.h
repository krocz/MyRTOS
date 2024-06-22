#ifndef _TCONFIG_H
#define _TCONFIG_H

#define	TINYOS_PRO_COUNT				       32						// TinyOS任务的优先级序号
#define TINYOS_ONE_TICK_TO_MS                  1
#define TINYOS_SLICE_MAX				       5						// 每个任务最大运行的时间片计数
#define TINYOS_STACK_SIZE                      1024
#define pdMS_TO_TICKS(xTimeInMs) ( ( uint32_t ) ( ( uint32_t ) ( xTimeInMs ) / (TINYOS_ONE_TICK_TO_MS ))  )

#endif

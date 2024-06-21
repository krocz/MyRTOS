#include "tinyOS.h"

/****************** 宏/变量定义 ****************************/
#define NVIC_INT_CTRL     0xE000ED04    // 中断控制及状态寄存器
#define NVIC_PENDSVSET    0x10000000    // 挂起PendSV中断的值
#define NVIC_SYSPRI2      0xE000ED22    // PendSV优先级寄存器
#define NVIC_PENDSV_PRI   0x000000FF    // 优先级值最低

#define MEM32(addr)       *(volatile unsigned long*)(addr)
#define MEM8(addr)        *(volatile unsigned char*)(addr)

/**********************************************************************************************************
** Function name        :   PendSV_Handler
** Descriptions         :   PendSV异常处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
__asm void PendSV_Handler()
{
	IMPORT pxCurrentTask
	IMPORT pxNextTask
	
	MRS 	R0, PSP                    // 初始PSP为0   R0<-PSP
	CBZ 	R0, PendSV_Handler_nosave  /* 如果R0为0，说明是上电之后的第一个任务，
	                                      则不需要保存现场，直接切换到相应任务栈 */
	STMDB 	R0!, {R4-R11}              // 保存现场(注：XPSR/PC/LR等寄存器在PendSV异常触发时由硬件保存)
	LDR 	R1, =pxCurrentTask
	LDR 	R1, [R1]                 
	STR 	R0, [R1]                   // 更新当前任务栈的栈顶地址

PendSV_Handler_nosave
	
	LDR 	R0, =pxCurrentTask
	LDR		R1, =pxNextTask
	LDR 	R2, [R1]                   // 下一任务堆栈指针的地址
	STR 	R2, [R0]                   
	
	LDR 	R0, [R2]                   // 当前任务堆栈栈顶地址
	LDMIA	R0!, {R4-R11}              // 恢复现场，其他寄存器由硬件恢复
	
	MSR 	PSP, R0	                   // 将栈顶地址写入用户栈寄存器PSP  PSP<-R0  
	ORR 	LR, LR, #0x04		       // 标记退出处理函数时，切换到PSP堆栈
	BX		LR                         // LR->PC
}

/**********************************************************************************************************
** Function name        :   vTaskStartScheduler
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskStartScheduler(void)
{
	__set_PSP(0);                      // 初始化用户栈栈顶为0地址
	
	MEM8(NVIC_SYSPRI2)   = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;	
	
	// 可以看到，这个函数是没有返回
    // 这是因为，一旦触发PendSV后，将会在PendSV后立即进行任务切换，切换至第1个任务运行
    // 此后，tinyOS将负责管理所有任务的运行，永远不会返回到该函数运行
}

/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   调用后进行一次任务切换，调用前需要先设置好pxCurrentTask和pxNextTask
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSwitch(void)
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}






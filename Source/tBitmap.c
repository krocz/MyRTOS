#include "tLib.h"

/**********************************************************************************************************
** Function name        :   vBitmapInit
** Descriptions         :   初始化bitmap将所有的位全清0
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vBitmapInit (Bitmap_t * pxBitmap) 
{
	pxBitmap->uiBitmap = 0;
}

/**********************************************************************************************************
** Function name        :   uiBitmapPosCount
** Descriptions         :   返回最大支持的位置数量
** parameters           :   无
** Returned value       :   最大支持的位置数量
***********************************************************************************************************/
uint32_t uiBitmapPosCount (void) 
{
	return 32;
}

/**********************************************************************************************************
** Function name        :   tBitmapSet
** Descriptions         :   设置bitmap中的某个位
** parameters           :   pos 需要设置的位
** Returned value       :   无
***********************************************************************************************************/
void tBitmapSet (Bitmap_t * pxBitmap, uint32_t uiPos)
{
	pxBitmap->uiBitmap |= 1 << uiPos;
}

/**********************************************************************************************************
** Function name        :   vBitmapClear
** Descriptions         :   清除bitmap中的某个位
** parameters           :   pos 需要清除的位
** Returned value       :   无
***********************************************************************************************************/
void vBitmapClear (Bitmap_t * pxBitmap, uint32_t uiPos)
{
	pxBitmap->uiBitmap &= ~(1 << uiPos);
}

/**********************************************************************************************************
** Function name        :   uiBitmapGetFirstSet
** Descriptions         :   从位图中第0位开始查找，找到第1个被设置的位置序号
** parameters           :   无
** Returned value       :   第1个被设置的位序号
***********************************************************************************************************/
uint32_t uiBitmapGetFirstSet (Bitmap_t * pxBitmap) 
{
	static const uint8_t pcQuickFindTable[] =     
	{
	    /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};

	if (pxBitmap->uiBitmap & 0xff)
    {
        return pcQuickFindTable[pxBitmap->uiBitmap & 0xff];         
    }
    else if (pxBitmap->uiBitmap & 0xff00)
    {
        return pcQuickFindTable[(pxBitmap->uiBitmap >> 8) & 0xff] + 8;        
    }
    else if (pxBitmap->uiBitmap & 0xff0000)
    {
        return pcQuickFindTable[(pxBitmap->uiBitmap >> 16) & 0xff] + 16;        
    }
    else if (pxBitmap->uiBitmap & 0xFF000000)
    {
        return pcQuickFindTable[(pxBitmap->uiBitmap >> 24) & 0xFF] + 24;
    }
    else
    {
        return uiBitmapPosCount();
    }
}

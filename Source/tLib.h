#ifndef _TLIB_H_
#define _TLIB_H_

#include <stdint.h>

typedef struct {
	uint32_t uiBitmap;
}Bitmap_t, Bitmap_pt;

typedef void (*TaskFunction_t)(void *);

/**********************************************************************************************************
** Function name        :   vBitmapInit
** Descriptions         :   初始化bitmap将所有的位全清0
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vBitmapInit (Bitmap_t * pxBitmap);

/**********************************************************************************************************
** Function name        :   uiBitmapPosCount
** Descriptions         :   返回最大支持的位置数量
** parameters           :   无
** Returned value       :   最大支持的位置数量
***********************************************************************************************************/
uint32_t uiBitmapPosCount (void);

/**********************************************************************************************************
** Function name        :   vBitmapSet
** Descriptions         :   设置bitmap中的某个位
** parameters           :   pos 需要设置的位
** Returned value       :   无
***********************************************************************************************************/
void vBitmapSet (Bitmap_t * pxBitmap, uint32_t uiPos);

/**********************************************************************************************************
** Function name        :   vBitmapClear
** Descriptions         :   清除bitmap中的某个位
** parameters           :   pos 需要清除的位
** Returned value       :   无
***********************************************************************************************************/
void vBitmapClear (Bitmap_t * pxBitmap, uint32_t uiPos);

/**********************************************************************************************************
** Function name        :   uiBitmapGetFirstSet
** Descriptions         :   从位图中第0位开始查找，找到第1个被设置的位置序号
** parameters           :   无
** Returned value       :   第1个被设置的位序号
***********************************************************************************************************/
uint32_t uiBitmapGetFirstSet (Bitmap_t * pxBitmap);




typedef struct tNode{
	struct tNode *pxPreNode;
	struct tNode *pxNextNode;
}Node_t;


typedef struct tList{
	Node_t xHeadNode;
	uint32_t uiNodeCnt;
}List_t;

/**********************************************************************************************************
** Function name        :   pxNodeParent
** Descriptions         :   获取结点所在的父struct结构首地址
** parameters           :   无
** Returned value       :   父struct结构首地址
***********************************************************************************************************/
#define pxNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)

/**********************************************************************************************************
** Function name        :   vNodeInit
** Descriptions         :   初始化结点类型
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vNodeInit (Node_t* pxNode);

/**********************************************************************************************************
** Function name        :   pxNodeParent
** Descriptions         :   获取结点所在的父struct结构首地址
** parameters           :   无
** Returned value       :   父struct结构首地址
***********************************************************************************************************/
#define pxNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)

/**********************************************************************************************************
** Function name        :   vListInit
** Descriptions         :   链表初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vListInit (List_t * pxList);

/**********************************************************************************************************
** Function name        :   iListIsFirst
** Descriptions         :   判断给定节点是否为链表的第一个节点
** parameters           :   无
** Returned value       :   1：是，0：否
***********************************************************************************************************/
int iListIsFirst(List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   iListIsLast
** Descriptions         :   判断给定节点是否为链表的第一个节点
** parameters           :   无
** Returned value       :   1：是，0：否
***********************************************************************************************************/
int iListIsLast(List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   uiListCount
** Descriptions         :   返回链表中结点的数量
** parameters           :   无
** Returned value       :   结点数量
***********************************************************************************************************/
uint32_t uiListCount (List_t * pxList);

/**********************************************************************************************************
** Function name        :   pxListFirst
** Descriptions         :   返回链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
***********************************************************************************************************/
Node_t * pxListFirst (List_t * pxList);

/**********************************************************************************************************
** Function name        :   pxListLast
** Descriptions         :   返回链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
***********************************************************************************************************/
Node_t * pxListLast (List_t * pxList);

/**********************************************************************************************************
** Function name        :   pxListPre
** Descriptions         :   返回链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
Node_t * pxListPre (List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   pxListNext
** Descriptions         :   返回链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
Node_t * pxListNext (List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   vListRemoveAll
** Descriptions         :   移除链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
***********************************************************************************************************/
void vListRemoveAll (List_t * pxList);

/**********************************************************************************************************
** Function name        :   vListAddFirst
** Descriptions         :   将指定结点添加到链表的头部
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void vListAddFirst (List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   vListAddLast
** Descriptions         :   将指定结点添加到链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void vListAddLast (List_t * pxList, Node_t * pxNode);

/**********************************************************************************************************
** Function name        :   pxListRemoveFirst
** Descriptions         :   移除链表中的第1个结点
** parameters           :   list 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
***********************************************************************************************************/
Node_t * pxListRemoveFirst (List_t * pxList);

/**********************************************************************************************************
** Function name        :   vListInsertForward
** Descriptions         :   将指定的结点插入到某个结点前面
** parameters           :   pxList 			待插入的链表
** parameters           :   pxNodeAfter 		参考结点
** parameters           :   pxNodeToInsert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void vListInsertForward (List_t * pxList, Node_t* pxNodeForward, Node_t * pxNodeToInsert);

/**********************************************************************************************************
** Function name        :   vListInsertAfter
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   pxList 			待插入的链表
** parameters           :   pxNodeAfter 		参考结点
** parameters           :   pxNodeToInsert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void vListInsertAfter (List_t * pxList, Node_t* pxNodeAfter, Node_t * pxNodeToInsert);

/**********************************************************************************************************
** Function name        :   vListRemove
** Descriptions         :   将指定结点从链表中移除
** parameters           :   pxList 	待移除的链表
** parameters           :   pxNode 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void vListRemove (List_t * pxList, Node_t * pxNode);


#endif

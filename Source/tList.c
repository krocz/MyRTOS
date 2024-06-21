#include "tLib.h"


/**********************************************************************************************************
** Function name        :   vNodeInit
** Descriptions         :   初始化结点类型
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vNodeInit (Node_t* pxNode)
{
	pxNode->pxPreNode = pxNode->pxNextNode = pxNode;
}

/**********************************************************************************************************
** Function name        :   vListInit
** Descriptions         :   链表初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vListInit (List_t * pxList)
{
	pxList->xHeadNode.pxNextNode = &(pxList->xHeadNode);
	pxList->xHeadNode.pxPreNode = &(pxList->xHeadNode);
	pxList->uiNodeCnt = 0;
}

/**********************************************************************************************************
** Function name        :   iListIsFirst
** Descriptions         :   判断给定节点是否为链表的第一个节点
** parameters           :   无
** Returned value       :   1：是，0：否
***********************************************************************************************************/
int iListIsFirst(List_t * pxList, Node_t * pxNode)
{
	if(pxNode)
		return (pxNode->pxPreNode == &pxList->xHeadNode)? 1 : 0;
	else
		return 0;
}

/**********************************************************************************************************
** Function name        :   iListIsLast
** Descriptions         :   判断给定节点是否为链表的第一个节点
** parameters           :   无
** Returned value       :   1：是，0：否
***********************************************************************************************************/
int iListIsLast(List_t * pxList, Node_t * pxNode)
{
	if(pxNode)
		return (pxNode->pxNextNode == &pxList->xHeadNode)? 1 : 0;
	else
		return 0;
}

/**********************************************************************************************************
** Function name        :   uiListCount
** Descriptions         :   返回链表中结点的数量
** parameters           :   无
** Returned value       :   结点数量
***********************************************************************************************************/
uint32_t uiListCount (List_t * pxList)
{
	return pxList->uiNodeCnt;
}

/**********************************************************************************************************
** Function name        :   pxListFirst
** Descriptions         :   返回链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
***********************************************************************************************************/
Node_t * pxListFirst (List_t * pxList)
{
	if(pxList->uiNodeCnt)
		return pxList->xHeadNode.pxNextNode;
	else
		return (Node_t *)0;
}


/**********************************************************************************************************
** Function name        :   pxListLast
** Descriptions         :   返回链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
***********************************************************************************************************/
Node_t* pxListLast (List_t * pxList)
{
	if(pxList->uiNodeCnt)
		return pxList->xHeadNode.pxPreNode;
	else
		return (Node_t *)0;
}

/**********************************************************************************************************
** Function name        :   pxListPre
** Descriptions         :   返回链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
Node_t * pxListPre (List_t * pxList, Node_t * pxNode)
{
	if(pxNode->pxPreNode == &pxList->xHeadNode)
		return (Node_t *)0;
	else
		return pxNode->pxPreNode;	
}

/**********************************************************************************************************
** Function name        :   pxListNext
** Descriptions         :   返回链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
Node_t * pxListNext (List_t * pxList, Node_t * pxNode)
{
	if(pxNode->pxNextNode == &pxList->xHeadNode)
		return (Node_t *)0;
	else
		return pxNode->pxNextNode;	
}

/**********************************************************************************************************
** Function name        :   vListRemoveAll
** Descriptions         :   移除链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
***********************************************************************************************************/
void vListRemoveAll (List_t * pxList)
{
	int i;
	Node_t *pxCurNode = pxList->xHeadNode.pxNextNode, *pxNextNode;
	for(i = 0; i < pxList->uiNodeCnt; i++)
	{
		pxNextNode = pxCurNode->pxNextNode;
		pxCurNode->pxNextNode = pxCurNode->pxPreNode = pxCurNode;
		pxCurNode = pxNextNode;
	}
	pxList->xHeadNode.pxNextNode = pxList->xHeadNode.pxPreNode = &pxList->xHeadNode;
	pxList->uiNodeCnt = 0;
}


/**********************************************************************************************************
** Function name        :   vListAddFirst
** Descriptions         :   将指定结点添加到链表的头部
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void vListAddFirst (List_t * pxList, Node_t * pxNode)
{
	pxNode->pxPreNode = &pxList->xHeadNode;
	pxNode->pxNextNode = pxList->xHeadNode.pxNextNode;
	
	pxList->xHeadNode.pxNextNode->pxPreNode = pxNode;
	pxList->xHeadNode.pxNextNode = pxNode;
	pxList->uiNodeCnt++;
}

/**********************************************************************************************************
** Function name        :   vListAddLast
** Descriptions         :   将指定结点添加到链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void vListAddLast (List_t * pxList, Node_t * pxNode)
{
	pxNode->pxNextNode = &pxList->xHeadNode;
	pxNode->pxPreNode = pxList->xHeadNode.pxPreNode;
	
	pxList->xHeadNode.pxPreNode->pxNextNode = pxNode;
	pxList->xHeadNode.pxPreNode = pxNode;
	pxList->uiNodeCnt++;
}

/**********************************************************************************************************
** Function name        :   vListRemoveFirst
** Descriptions         :   移除链表中的第1个结点
** parameters           :   pxList 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
***********************************************************************************************************/
Node_t * vListRemoveFirst (List_t * pxList)
{
	Node_t *pxTemp = (Node_t *)0;
	if(pxList->uiNodeCnt)
	{
		pxTemp = pxList->xHeadNode.pxNextNode;
		pxList->xHeadNode.pxNextNode = pxTemp->pxNextNode;
		pxTemp->pxNextNode->pxPreNode = &pxList->xHeadNode;
		pxList->uiNodeCnt--;
	}
	return pxTemp;	
}

/**********************************************************************************************************
** Function name        :   vListInsertForward
** Descriptions         :   将指定的结点插入到某个结点前面
** parameters           :   pxList 			待插入的链表
** parameters           :   pxNodeAfter 		参考结点
** parameters           :   pxNodeToInsert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void vListInsertForward (List_t * pxList, Node_t* pxNodeForward, Node_t * pxNodeToInsert)
{
	pxNodeToInsert->pxNextNode = pxNodeForward;
	pxNodeToInsert->pxPreNode = pxNodeForward->pxPreNode;
	
	pxNodeForward->pxPreNode->pxNextNode = pxNodeToInsert;
	pxNodeForward->pxPreNode = pxNodeToInsert;
	pxList->uiNodeCnt++;
}


/**********************************************************************************************************
** Function name        :   vListInsertAfter
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   pxList 			待插入的链表
** parameters           :   pxNodeAfter 		参考结点
** parameters           :   pxNodeToInsert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void vListInsertAfter (List_t * pxList, Node_t* pxNodeAfter, Node_t * pxNodeToInsert)
{
	pxNodeToInsert->pxPreNode = pxNodeAfter;
	pxNodeToInsert->pxNextNode = pxNodeAfter->pxNextNode;
	
	pxNodeAfter->pxNextNode->pxPreNode = pxNodeToInsert;
	pxNodeAfter->pxNextNode = pxNodeToInsert;
	pxList->uiNodeCnt++;
}

/**********************************************************************************************************
** Function name        :   vListRemove
** Descriptions         :   将指定结点从链表中移除
** parameters           :   pxList 	待移除的链表
** parameters           :   pxNode 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void vListRemove (List_t * pxList, Node_t * pxNode)
{
	pxNode->pxPreNode->pxNextNode = pxNode->pxNextNode;
	pxNode->pxNextNode->pxPreNode = pxNode->pxPreNode;
	pxList->uiNodeCnt--;
}













/*
 *  modifier: Honrun
 *  date:     2021/12/21 20:42
 */
#include "stdint.h"
#include "DevicesMem.h"


MemType st_typeMemHead = {0};
uint8_t st_MemHeap[DEVICES_MEM_LENGTH + sizeof(MemType) * 2] = {0};



void vMemInit(void)
{
	MemType *ptypeMemNow = NULL;

	/* 地址 N 字节对齐 */
	st_typeMemHead.startAddr = memRoundUp((uint32_t)(&st_MemHeap), DEVICES_MEM_ROUNDUP_VALUE);
	st_typeMemHead.stopAddr  = st_typeMemHead.startAddr + DEVICES_MEM_LENGTH;
	st_typeMemHead.state     = DEVICES_MEM_DISABLE;

	/* 初始化第一个空闲空间 */
	ptypeMemNow  = (MemType *)st_typeMemHead.startAddr;
	*ptypeMemNow = st_typeMemHead;
}

void *pvMemMalloc(int32_t iSize)
{
	void *pvHandle = NULL;
	MemType *ptypeMemNow = NULL, *ptypeMemNext = NULL, *ptypeMemMin = NULL;
	int32_t iAddrNow = 0, iAddrStop = 0, iSizeNow = 0, iSizeMin = 0x7FFFFFFF;

	if(iSize < 0)
		return NULL;

	/* 长度 N 字节对齐 */
	iSize = memRoundUp(iSize, DEVICES_MEM_ROUNDUP_VALUE);

	/* 遍历mem空间 */
	for(iAddrNow = st_typeMemHead.startAddr; iAddrNow < st_typeMemHead.stopAddr; iAddrNow = ptypeMemNow->stopAddr)
	{
		ptypeMemNow = (MemType *)iAddrNow;

		if(ptypeMemNow->state != DEVICES_MEM_ENABLE)
		{
			iSizeNow = ptypeMemNow->stopAddr - ptypeMemNow->startAddr - sizeof(MemType);

			/* 最小空闲匹配 */
			if((iSizeNow >= iSize) && (iSizeMin > iSizeNow))
			{
				iSizeMin = iSizeNow;
				ptypeMemMin = ptypeMemNow;

				/* 找到刚好相同大小的空闲空间就直接退出遍历 */
				if(iSizeMin == iSize)
					break;
			}
		}
	}

	/* 没匹配到合适的最小空间，表示当前分配不了 */
	if(ptypeMemMin == NULL)
		return NULL;

	iAddrStop = ptypeMemMin->stopAddr;

	ptypeMemMin->state    = DEVICES_MEM_ENABLE;
	ptypeMemMin->stopAddr = ptypeMemMin->startAddr + sizeof(MemType) + iSize;

	/* 最小匹配空间被划分后还有剩余空间 */
	if(iSizeMin != iSize)
	{
		ptypeMemNext = (MemType *)ptypeMemMin->stopAddr;
		ptypeMemNext->startAddr = ptypeMemMin->stopAddr;
		ptypeMemNext->stopAddr  = iAddrStop;
		ptypeMemNext->state     = DEVICES_MEM_DISABLE;
	}

	/* 返回空间管理信息之后的空间地址 */
	pvHandle = (void *)(ptypeMemMin->startAddr + sizeof(MemType));

	return pvHandle;
}

void *pvMemCalloc(int32_t iNumber, int32_t iSize)
{
	void *pvHandle = NULL;
	int32_t iSizeTemp = iNumber * iSize;

	/* 长度 N 字节对齐 */
	iSizeTemp = memRoundUp(iSizeTemp, DEVICES_MEM_ROUNDUP_VALUE);

	pvHandle = pvMemMalloc(iSizeTemp);

	if(pvHandle != NULL)
	{
		cMemSet(pvHandle, 0, iSizeTemp);
	}

	return pvHandle;
}

void *pvMemRealloc(void *pvMemAddr, int32_t iSize)
{
	void *pvHandle = NULL;
	MemType *ptypeMemOld = NULL;
	int32_t iAddrStart = 0, iOldSize = 0;

	if(pvMemAddr == NULL)
		return NULL;

	iAddrStart = (int32_t)(pvMemAddr - sizeof(MemType));
	if((iAddrStart < st_typeMemHead.startAddr) || (iAddrStart > st_typeMemHead.stopAddr))
		return NULL;

	ptypeMemOld = (MemType *)(iAddrStart);

	/* 重新分配空间 */
	if((pvHandle = pvMemMalloc(iSize)) != NULL)
	{
		iOldSize = ptypeMemOld->stopAddr - ptypeMemOld->startAddr - sizeof(MemType);

		/* 拷贝数据 */
		pvMemCpy(pvHandle, pvMemAddr, iOldSize > iSize ? iSize : iOldSize);

		/* 释放旧空间 */
		vMemFree(pvMemAddr);

		return pvHandle;
	}
	else
	{
		return pvMemAddr;
	}
}

void vMemFree(void *pvMemAddr)
{
	MemType *ptypeMemNow = NULL, *ptypeMemPrev = NULL, *ptypeMemNext = NULL;
	int32_t iAddrStart = 0;

	if(pvMemAddr == NULL)
		return;

	/* 位移到当前待释放指针的管理信息位置 */
	iAddrStart = (int32_t)(pvMemAddr - sizeof(MemType));
	if((iAddrStart < st_typeMemHead.startAddr) || (iAddrStart > st_typeMemHead.stopAddr))
		return;

	ptypeMemNow = (MemType *)(iAddrStart);
	ptypeMemNow->state = DEVICES_MEM_DISABLE;

	/* 遍历，找到当前待删除空间的前面一个空间 */
	for(ptypeMemPrev = (MemType *)(st_typeMemHead.startAddr); ptypeMemPrev->stopAddr < ptypeMemNow->startAddr; ptypeMemPrev = (MemType *)ptypeMemPrev->stopAddr);

	/* 合并前面一个相连的空闲空间 */
	if(ptypeMemPrev->state == DEVICES_MEM_DISABLE)
	{
		ptypeMemPrev->stopAddr = ptypeMemNow->stopAddr;
		ptypeMemNow = ptypeMemPrev;
	}

	/* 合并后面一个相连的空闲空间 */
	if(ptypeMemNow->stopAddr < st_typeMemHead.stopAddr)
	{
		ptypeMemNext = (MemType *)ptypeMemNow->stopAddr;

		if(ptypeMemNext->state == DEVICES_MEM_DISABLE)
		{
			ptypeMemNow->stopAddr = ptypeMemNext->stopAddr;
		}
	}
}

int8_t cMemSet(void *pvMemAddr, int32_t iValue, int32_t iSize)
{
	uint8_t *pucData = pvMemAddr;

	if(pvMemAddr == NULL)
		return -1;

	if(iSize <= 0)
		return -2;

	while(iSize--)
	{
		*pucData++ = iValue;
	}

	return 0;
}

void *pvMemCpy(void *pvMemTargetAddr, void *pvMemSourceAddr, int32_t iSize)
{
	uint8_t *pucTargetData = pvMemTargetAddr, *pucSourceData = pvMemSourceAddr;

	if((pvMemTargetAddr == NULL) || (pvMemSourceAddr == NULL))
		return NULL;

	if(iSize <= 0)
		return NULL;

	while(iSize--)
	{
		*pucTargetData++ = *pucSourceData++;
	}

	return pvMemTargetAddr;
}

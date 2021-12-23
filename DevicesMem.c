/*
 *  modifier: Honrun
 *  date:     2021/12/21 20:42
 */
#include "stdint.h"
#include "DevicesMem.h"

MemType st_typeMemHead = {0};

/* ��Ҫ���� sizeof(MemType) * 2 �����ȣ���ʹ�ֽڶ��룬�Լ��洢������Ϣ */
uint8_t st_MemHeap[DEVICES_MEM_LENGTH + sizeof(MemType) * 2] = {0};



void vMemInit(void)
{
	MemType *ptypeMemNow = NULL;

	/* ��ַ N �ֽڶ��� */
	st_typeMemHead.startAddr = memRoundUp((uint32_t)(&st_MemHeap), DEVICES_MEM_ROUNDUP_VALUE);
	st_typeMemHead.stopAddr  = st_typeMemHead.startAddr + DEVICES_MEM_LENGTH;
	st_typeMemHead.state     = DEVICES_MEM_DISABLE;

	/* ��ʼ����һ�����пռ� */
	ptypeMemNow  = (MemType *)st_typeMemHead.startAddr;
	*ptypeMemNow = st_typeMemHead;

	/* ��ʼ���ڴ�ռ�ĩβ�Ĺ�����Ϣ */
	ptypeMemNow  = (MemType *)st_typeMemHead.stopAddr;
	*ptypeMemNow = st_typeMemHead;
	ptypeMemNow->state = DEVICES_MEM_ENABLE;
}

void *pvMemMalloc(int32_t iSize)
{
	void *pvHandle = NULL;
	MemType *ptypeMemNow = NULL, *ptypeMemNext = NULL, *ptypeMemMin = NULL;
	int32_t iAddrStop = 0, iSizeNow = 0, iSizeMin = 0x7FFFFFFF;

	if(iSize < 0)
		return NULL;

	/* ���� N �ֽڶ��� */
	iSize = memRoundUp(iSize, DEVICES_MEM_ROUNDUP_VALUE);

	/* ����mem�ռ� */
	for(ptypeMemNow = (MemType *)st_typeMemHead.startAddr; (int32_t)ptypeMemNow < st_typeMemHead.stopAddr; ptypeMemNow = (MemType *)ptypeMemNow->stopAddr)
	{
		if(ptypeMemNow->state != DEVICES_MEM_ENABLE)
		{
			iSizeNow = ptypeMemNow->stopAddr - ptypeMemNow->startAddr - sizeof(MemType);

			/* ��С����ƥ�� */
			if((iSizeNow >= iSize) && (iSizeMin > iSizeNow))
			{
				iSizeMin = iSizeNow;
				ptypeMemMin = ptypeMemNow;

				/* �ҵ��պ���ͬ��С�Ŀ��пռ��ֱ���˳����� */
				if(iSizeMin == iSize)
					break;
			}
		}
	}

	/* ûƥ�䵽���ʵ���С�ռ䣬��ʾ��ǰ���䲻�� */
	if(ptypeMemMin == NULL)
		return NULL;

	/* ��Сƥ��ռ䱻���ֺ���ʣ��ռ䣬���ܹ��洢��һ��ͷ����Ϣ */
    if((iSizeMin - iSize) >= sizeof(MemType))
	{
        iAddrStop = ptypeMemMin->startAddr + sizeof(MemType) + iSize;

		ptypeMemNext = (MemType *)iAddrStop;
		ptypeMemNext->startAddr = iAddrStop;
		ptypeMemNext->stopAddr  = ptypeMemMin->stopAddr;
		ptypeMemNext->state     = DEVICES_MEM_DISABLE;
	}
	/* ���ݣ��������ò��ϵ�С���пռ� */
	else
    {
        iSize = iSizeMin;
    }

    ptypeMemMin->state = DEVICES_MEM_ENABLE;
    ptypeMemMin->stopAddr = ptypeMemMin->startAddr + sizeof(MemType) + iSize;

	/* ���ؿռ������Ϣ֮��Ŀռ��ַ */
	pvHandle = (void *)(ptypeMemMin->startAddr + sizeof(MemType));

	return pvHandle;
}

void *pvMemCalloc(int32_t iNumber, int32_t iSize)
{
	void *pvHandle = NULL;
	int32_t iSizeTemp = iNumber * iSize;

	/* ���� N �ֽڶ��� */
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
	MemType *ptypeMemOld = NULL, *ptypeMemNow = NULL, *ptypeMemNext = NULL;
	int32_t iAddrStart = 0, iNewSize = 0;

	if(pvMemAddr == NULL)
		return NULL;

    /* λ�Ƶ�ͷ��������Ϣλ�� */
	iAddrStart = (int32_t)(pvMemAddr - sizeof(MemType));
	if((iAddrStart < st_typeMemHead.startAddr) || (iAddrStart > st_typeMemHead.stopAddr))
		return NULL;

	ptypeMemOld = (MemType *)(iAddrStart);

	/* ���� N �ֽڶ��� */
	iSize = memRoundUp(iSize, DEVICES_MEM_ROUNDUP_VALUE);

    /* ��������νӿ��пռ�Ĵ�С */
    for(ptypeMemNow = (MemType *)ptypeMemOld->stopAddr; ptypeMemNow->state == DEVICES_MEM_DISABLE; ptypeMemNow = (MemType *)(ptypeMemNow->stopAddr));

    iNewSize = (int32_t)ptypeMemNow - ptypeMemOld->startAddr - sizeof(MemType);

    /* ֱ����չ�νӵĿ��пռ� */
    if(iNewSize >= iSize)
    {
        /* ��Сƥ��ռ䱻���ֺ���ʣ��ռ䣬���ܹ��洢��һ��ͷ����Ϣ */
        if((iNewSize - iSize) >= sizeof(MemType))
        {
            ptypeMemOld->stopAddr = ptypeMemOld->startAddr + sizeof(MemType) + iSize;

            ptypeMemNext = (MemType *)ptypeMemOld->stopAddr;
            ptypeMemNext->startAddr = ptypeMemOld->stopAddr;
            ptypeMemNext->stopAddr  = (int32_t)ptypeMemNow;
            ptypeMemNext->state     = DEVICES_MEM_DISABLE;
        }
        /* ���ݣ��������ò��ϵ�С���пռ� */
        else
        {
            iSize = iNewSize;
        }

        ptypeMemOld->stopAddr = ptypeMemOld->startAddr + sizeof(MemType) + iSize;

        return pvMemAddr;
    }
    /* ���·���ռ� */
    else
    {
        /* ���䲻�����������С�Ŀռ� */
        if((pvHandle = pvMemMalloc(iSize)) == NULL)
            return NULL;

        /* ת�ƿ������� */
        pvMemCpy(pvHandle, pvMemAddr, iSize);

        /* �ͷžɿռ� */
        vMemFree(pvMemAddr);

        return pvHandle;
    }
}

void vMemFree(void *pvMemAddr)
{
	MemType *ptypeMemNow = NULL, *ptypeMemPrev = NULL, *ptypeMemNext = NULL;
	int32_t iAddrStart = 0;

	if(pvMemAddr == NULL)
		return;

    /* λ�Ƶ�ͷ��������Ϣλ�� */
	iAddrStart = (int32_t)(pvMemAddr - sizeof(MemType));
	if((iAddrStart < st_typeMemHead.startAddr) || (iAddrStart > st_typeMemHead.stopAddr))
		return;

	ptypeMemNow = (MemType *)(iAddrStart);
	ptypeMemNow->state = DEVICES_MEM_DISABLE;

	/* �������ҵ���ǰ��ɾ���ռ��ǰ��һ���ռ� */
	for(ptypeMemPrev = (MemType *)(st_typeMemHead.startAddr); ptypeMemPrev->stopAddr < ptypeMemNow->startAddr; ptypeMemPrev = (MemType *)ptypeMemPrev->stopAddr);

	/* �ϲ�ǰ��һ�������Ŀ��пռ� */
	if(ptypeMemPrev->state == DEVICES_MEM_DISABLE)
	{
		ptypeMemPrev->stopAddr = ptypeMemNow->stopAddr;
		ptypeMemNow = ptypeMemPrev;
	}

	/* �ϲ�����һ�������Ŀ��пռ� */
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

#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "string.h"
#include "DevicesMem.h"


uint8_t *pucHandleBuff[256] = {NULL};
uint8_t ucNumberBuff[256] = {0};


extern MemType st_typeMemHead;













int8_t cMemShowAll(MemType *ptypeHandle)
{
	MemType *ptypeMemNow = ptypeHandle;
	int32_t iAddrNow = 0;

	if(ptypeHandle == NULL)
		return -1;

	for(iAddrNow = ptypeHandle->startAddr; iAddrNow < ptypeHandle->stopAddr; iAddrNow = ptypeMemNow->stopAddr)
	{
		ptypeMemNow = (MemType *)iAddrNow;
		printf("ptypeMemNow %08X, %08X, %d, %d\r\n", ptypeMemNow->startAddr, ptypeMemNow->stopAddr, ptypeMemNow->state, ptypeMemNow->stopAddr - ptypeMemNow->startAddr - sizeof(MemType));
	}

	printf("\r\n\n");

	return 0;
}


int main(void)
{
	uint16_t x = 0, y = 0, z = 0;
	uint8_t *pucHandleRealloc = NULL;

    printf("Hello mem test!\n");

    vMemInit();

	while(1)
	{
		x = rand() % 32;
		y = rand() % 1024;
		z = rand() % 4;

		if(z == 0)
		{
			if(pucHandleBuff[x] == NULL)
			{
				pucHandleBuff[x] = pvMemMalloc(y);

				/* 数据写入测试 */
				if(pucHandleBuff[x] != NULL)
                    memset(pucHandleBuff[x], x, y);

				printf("mal: x: %d,\ty: %d,\taddr: %08X\r\n", x, y, pucHandleBuff[x] - 8);

                cMemShowAll(&st_typeMemHead);
			}
		}
		else if(z == 1)
		{
			if(pucHandleBuff[x] != NULL)
			{
				printf("rea: x: %d,\ty: %d,\taddr: %08X\r\n", x, y, pucHandleBuff[x] - 8);
				pucHandleRealloc = pvMemRealloc(pucHandleBuff[x], y);

				/* 数据写入测试 */
				if(pucHandleRealloc != NULL)
                {
                    pucHandleBuff[x] = pucHandleRealloc;
                    memset(pucHandleBuff[x], x, y);
                }
				printf("rea: x: %d,\ty: %d,\taddr: %08X\r\n", x, y, pucHandleBuff[x] - 8);

                cMemShowAll(&st_typeMemHead);
			}
		}
		else if(z == 2)
		{
			if(pucHandleBuff[x] == NULL)
			{
				pucHandleBuff[x] = pvMemCalloc(z, y);

				/* 数据写入测试 */
				if(pucHandleBuff[x] != NULL)
                    memset(pucHandleBuff[x], x, z * y);

				printf("cal: x: %d,\ty: %d,\taddr: %08X\r\n", x, z * y, pucHandleBuff[x] - 8);

                cMemShowAll(&st_typeMemHead);
			}
		}
		else
        {
			if(pucHandleBuff[x] != NULL)
			{
				printf("del: x: %d,\ty: %d,\taddr: %08X\r\n", x, y, pucHandleBuff[x] - 8);

				vMemFree(pucHandleBuff[x]);
				pucHandleBuff[x] = NULL;

                cMemShowAll(&st_typeMemHead);
			}
        }
	}

    return 0;
}

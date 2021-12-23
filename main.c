#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "string.h"
#include "DevicesMem.h"


uint8_t *pucHandleBuff[256] = {NULL};
uint8_t ucNumberBuff[256] = {0};



int main(void)
{
	uint16_t x = 0, y = 0;

    printf("Hello mem test!\n");

    vMemInit();

	while(1)
	{
		x = rand();
		y = rand() % 256;

		if(x & 1)
		{
			x = (x >> 1) % 256;
			if(pucHandleBuff[x] == NULL)
			{
				printf("add: x: %d,\ty: %d\r\n", x, y);

				pucHandleBuff[x] = pvMemMalloc(y);

				/* 数据写入测试 */
				if(pucHandleBuff[x] != NULL)
                    memset(pucHandleBuff[x], x, y);
			}
		}
		else
		{
			x = (x >> 1) % 256;
			if(pucHandleBuff[x] != NULL)
			{
				printf("del: x: %d,\ty: %d\r\n", x, y);

				vMemFree(pucHandleBuff[x]);
				pucHandleBuff[x] = NULL;
			}
		}
	}

    return 0;
}

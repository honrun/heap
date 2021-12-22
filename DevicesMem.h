#ifndef DEVICESMEM_H_INCLUDED
#define DEVICESMEM_H_INCLUDED


/* ģ�⶯̬�ڴ��С */
#define DEVICES_MEM_LENGTH        (1024 * 25)

#define DEVICES_MEM_DISABLE       0
#define DEVICES_MEM_ENABLE        1

/* 8 �ֽڶ��� */
#define DEVICES_MEM_ROUNDUP_VALUE 8

/* ʹx��n�ֽڶ��� */
#define memRoundUp(x, n) (((x) + (n) - 1) & (~((n) - 1)))



typedef struct{
	int32_t startAddr: 30;
	int32_t stopAddr: 30;
	uint32_t state: 2;
}MemType;





void vMemInit(void);
void *pvMemMalloc(int32_t iSize);
void *pvMemCalloc(int32_t iNumber, int32_t iSize);
void *pvMemRealloc(void *pvMemAddr, int32_t iSize);
void vMemFree(void *pvMemAddr);
int8_t cMemSet(void *pvMemAddr, int32_t iValue, int32_t iSize);
void *pvMemCpy(void *pvMemTargetAddr, void *pvMemSourceAddr, int32_t iSize);

#endif // DEVICESMEM_H_INCLUDED

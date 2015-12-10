/**
* @file JMemory.c
* @brief ����̬�����ڴ��ģ��
*
* @version V0.0.1
* @author joe
* @date 2012��3��27��
* @note
*		none
*
* @copy
*
* �˴���Ϊ���ںϽܵ������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2012 heroje</center></h1>
*/

#include "JMemory.h"
#include <stdlib.h>
#include <string.h>
#define ALLOCATE_ADDR_TABLE_SIZE  100

static int  p_allocate_addr[ALLOCATE_ADDR_TABLE_SIZE];		//ÿ�ζ�̬����ĵ�ַ���ᱣ���ڴ���������

void JMemory_init(void)
{
	MEMSET((void*)p_allocate_addr,0,ALLOCATE_ADDR_TABLE_SIZE*sizeof(int));
}

void * Jmalloc(int size)
{
	void *p;
	int		i;

	p = malloc(size);
	if(p)
	{
		for(i = 0;i < ALLOCATE_ADDR_TABLE_SIZE;i++)
		{
			if(p_allocate_addr[i] == 0)
			{
				p_allocate_addr[i] = (int)p;
				break;
			}
		}

		if(i == ALLOCATE_ADDR_TABLE_SIZE)
		{
			p_allocate_addr[0] = (int)p;
		}
	}

	return p;
}


void Jfree(void * p)
{
	int	i;
	for(i = 0;i < ALLOCATE_ADDR_TABLE_SIZE;i++)
	{
		if((int)p == p_allocate_addr[i])
		{
			free(p);
			p_allocate_addr[i] = 0;
			break;
		}
	}
}


void Jfree_all(void)
{
	int	i;
	for(i = 0;i < ALLOCATE_ADDR_TABLE_SIZE;i++)
	{
		if(p_allocate_addr[i])
		{
			free((void*)p_allocate_addr[i]);
		}
	}
}
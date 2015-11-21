/**
***************************************************************************
* @file record_mod.c
* @brief 
***************************************************************************
*
* @version V0.2.1
* @author jiangcx
* @date 2010��12��08��
* @note ����ECCУ�������ƣ�recordformatʱ�����recopt��������0x80����
*       ����ECCУ�������
*
***************************************************************************
*
* @version V0.2.0
* @author jiangcx
* @date 2010��12��06��
* @note ����length����,Լ��ÿ����¼�ĵ�һ���ֽ�Ϊ��־�ֽ�
*
***************************************************************************
*
* @version V0.1.2
* @author jiangcx
* @date 2010��12��06��
* @note �޸��ϵ�ָ�����bug���˰汾Ϊ����length���������հ汾��
*       ��һ���汾�Ž�ΪV0.2.0����length��������У��������
*
***************************************************************************
*
* @version V0.1.1
* @author jiangcx
* @date 2010��11��18��
* @note ����д��У�����
*       ֧��6����¼������һ���������������͵�����¼���ȱ���С��2K-1
*
***************************************************************************
*
* @copy
*
* �˴���Ϊ���ڽ������������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2010 netcom</center></h1>
*/


//note:�����Ҫ֧�ָ���ļ�¼������ֻ��Ҫ�޸ĺ궨��  RECTYPENUM	Ϊϣ��֧�ֵķ���������


#include "spi_flash.h"
#include "record_mod.h"
#include "rec_ecc.h"
#include <string.h>

#ifdef	RECORD_DEBUG
#include <stdio.h>
#endif

#define RECTYPENUM					0x01		//֧��1������ļ�¼����@note ���Ӧ����Ҫ֧�ָ���ĵ��ݣ���ô�˶���Ҳ��Ҫ��Ӧ���޸ģ�ע�ⲻ�ܳ���63
#define RITHD						0x80		//����ʱ�����������Flash������һ��ͷRIT
#define INDEX_MASK					0x3f		//��¼���͵ĵ�6λ��ʾ��¼����

typedef struct {
	unsigned char		RIT_OEMName[16];    		// 0  ASCII		���ڴ��OEM���̵����֣��磺"ShenZhenNetcom  "
	unsigned char		RIT_ManName[16];    		// 10 ASCII	   	�����������磺"ShenZhenNetcom  "
	unsigned char		RIT_ModuleName[16];			// 20 ASCII		ģ�������磺"POSRESOURCES    "
	unsigned short		RIT_ProdID;					// 30 WORD		��ƷID���û��Լ����塣�磺0x0001��1234
	unsigned short		RIT_Version;    			// 32 BCD��		�汾�ţ��û��Լ����塣�磺0x0001��1234
	unsigned short		RIT_Date;            		// 34 BCD��		����ʱ�䣺0x06,0x28 ��ʾ6��28��
	unsigned char		RIT_SN[10];            		// 36 ASCII		���кţ��û��Լ����塣�磺"1234567890"
	unsigned int		RIT_TotalSect;        		// 40 DWORD		�˷���Դ�������������������ͷ��512�ֽ�
	unsigned int		RIT_DataSec;        		// 44 DWORD	***	�˷���Դ���ݲ���ռ��������
	unsigned char		RIT_VerifyType;        		// 48 BYTE		�˷���Դ��У���㷨�������ݲ�ʹ��
	unsigned char		RIT_Endian;        			// 49 BYTE		�˷���Դ���õ�CPU��С��ģʽ��0x0B��ʾ��� 0x0C��ʾС��
	unsigned char		RIT_DataType;         		// 4A BYTE	***	�˷���Դ�������룺�����ֿ⣺0x10 ͼƬ���ݣ�0x11 �ֿ����ӣ�0x12 �������룺0x80 ������Դ���ݣ�0x81 ��������0x90 ��¼����0x91
	unsigned char		RIT_Reserved0;				// 4B BYTE		����
	unsigned int		RIT_StartAddr;        		// 4C DWORD		����������ram�д�ŵ���ʼ��ַ��������ڴ�����Ч
	unsigned int		RIT_RunAddr;        		// 50 DWORD		���д���ʱPC��ת�ĵ�ַ
	unsigned short   	RIT_DataID;					// 54 WORD		�˷���Դ��ID��
	unsigned char		RIT_Align;					// 56 BYTE		�˷���Դ����С���뵥λ��Ϊ2^n�ֽ�
	unsigned char		RIT_Reserved1;				// 57
	unsigned char		RIT_StrVer[13];				// 58           ��Դ�İ汾�ţ���ʽΪ20100610_0001
	unsigned char		RIT_Reserved2[0x18B];		// 65 
	unsigned int		RIT_DataCRC32;				// 1F0          ��������CRC32��������һ�������𣬵�RIT_CodeSecֹ���������ݵ�CRC32ֵ��CRC���������xml�ļ��е����������ɡ�
	unsigned int		RIT_Next;            		// 1F4 ***      ��һ��RIT�ڴ����еľ��Ե�ַ(����)
	unsigned int		RIT_CheckSum;        		// 1F8 �˷���Դ��RIT��CRC32У��ֵ���� 
	unsigned int		RIT_TrailSig;       		// 1FC ������־(0x00,0x00,0x55,0xAB)
}TResInfoTable;

#pragma pack(1)
typedef struct {
	unsigned int		RIT_DataSec;        		// 44 DWORD	***	�˷���Դ���ݲ���ռ��������
	unsigned int		RIT_Next;            		// 1F4 ***      ��һ��RIT�ڴ����еľ��Ե�ַ(����) 
	unsigned int		RIT_TrailSig;       		// 1FC ������־(0x00,0x00,0x55,0xAB)
	unsigned char		RIT_DataType;         		// 4A BYTE	***	�˷���Դ��������
}TSimpRIT;

typedef struct
{
	unsigned int	rec_start;		// ��¼��������ʼ��ַ			�洢��Flash
	unsigned int	rec_total;		// ��¼�����ݴ洢���ܴ�С		�洢��Flash
	unsigned int	rec_hdpt;		// Headָ�룬ָ���0����¼	�ڴ涯ֵ̬
	unsigned short	rec_size;		// һ����¼���ֽ���			�洢��Flash
	unsigned short	rec_space;		// ÿ����¼��ռ�ռ���			�洢��Flash
	unsigned short	rec_maxnum;		// �ɴ�����ļ�¼��			�洢��Flash
	unsigned short	rec_count;		// ��ǰ��Ч��¼��				�ڴ涯ֵ̬
	unsigned char	rec_eccflag;	// ECC��ʶ 0x80ʹ�� 0x00����	�洢��Flash
	unsigned char	rec_valid;		// ϵͳ��ʼ����־0x00|0xAA		�ڴ涯ֵ̬
}TRecFSCB;	// Record File System Control Block
#pragma pack()

#define BLKSIZE						0x1000		// 4K��
#define BLKMSK						0x00000FFF	// 4K������

#define	RITSIZE						512			// RIT��С
#define	SECSIZE						512			// ������С

//��¼״̬��־����
#define REC_STATEMSK				0x8F
#define REC_FREE					0xFF		// ����
#define REC_INVALID					0xEF		// ��Ч
#define REC_VALID					0xCF		// ��Ч
#define REC_OBSOLETE				0x8F		// ��ʱ

//����4K��ʱ�ķ����籣����־����
#define REC_FMTMSK					0xF8
#define REC_FMTNOOP					0xFF		// �޲�������
#define REC_FMTINOP					0xFE		// ���ڲ���	
#define REC_FMTEXOP					0xFC		// �����ݵ����ݽ�����
#define REC_FMTOVOP					0xF8		// �������


static TRecFSCB					recinfo[RECTYPENUM+1];
static TSimpRIT					simprit;

extern unsigned int				recmod_flasize;
//ECCУ�������㻺����
unsigned char		ecckey_read[3];
unsigned char		ecckey_cal[3];
unsigned char		eccdata_tmp[256];

static int			rec_getindex(unsigned char rectype);
static void			rec_readsimprit(unsigned int add);
static int			rec_writesimprit(unsigned int add);
static unsigned int	rec_getrecspc(unsigned int recsize, unsigned char eccflag);
static int			rec_copy4kblk(unsigned int desadd, unsigned int srcadd, unsigned int len);
static	unsigned char			tmpfgwi;		// ��ʱ�ֽ�write in
static	unsigned char			tmpfgrb;		// ��ʱ�ֽ�read back
static volatile int						test_var1;
static volatile int						test_var2;

/*-----------------  �ڲ�����  -----------------------*/
/**
* @brief  ��ȡ��¼�ļ�ϵͳ�����RIT��Ϣ
* @param   add: RIT���׵�ַ
* @retval None
*/
static void	rec_readsimprit(unsigned int add)
{
	spi_flash_raddr(add+0x44, 4, (unsigned char *)&simprit.RIT_DataSec);	//��ȡ��Դ���ݲ���ռ��������
	spi_flash_raddr(add+0x4A, 1, (unsigned char *)&simprit.RIT_DataType);	//��ȡ��Դ��������
	spi_flash_raddr(add+0x1F4, 4, (unsigned char *)&simprit.RIT_Next);		//��ȡ��һ��RIT�ڴ����е�������ַ
	spi_flash_raddr(add+0x1FC, 4, (unsigned char *)&simprit.RIT_TrailSig);	//��ȡ������־
}

/**
* @brief  ����һ����¼����Ŀռ�
* @param  recsize: һ����¼�ĳ���
* @param  eccflag: ECC�����Ƿ���
* @retval     2^n: ��¼����ռ�, 2^n 
*/
static unsigned int	rec_getrecspc(unsigned int recsize, unsigned char eccflag)
{
	unsigned int	recspace;
	int				i;
	// ����һ����¼����Ŀռ�, �ռ������2^nΪ��λ,
	// ÿ����¼��Ҫ���1byte�ϵ籣����־
	// ���ECC���ܿ�������Ҫ����N�ֽڵ�ECC����У����λ��
	// �������1�ֽڵ����У��λ
	if(eccflag)
	{
		recspace	= recsize + 1 + (recsize + 255) / 256 * 3;
	}
	else
	{
		recspace	= recsize + 2;
	}
	// ѭ����������˵�1
	for (i = 31; i > 0; i--)
	{
		if ((recspace & 0x80000000) == 0x80000000)
		{
			if (recspace == 0x80000000)
			{
				break;
			}
			else
			{
				i++;
				break;
			}
		}
		recspace <<= 1;
	}
	recspace = 0x1 << i;	//�õ�2^n��С
	return recspace;
}

/**
* @brief  д���¼�ļ�ϵͳ�����RIT��Ϣ
* @param   add: RIT���׵�ַ
* @retval    0: �ɹ�
*           -1: ʧ��
*/
static int	rec_writesimprit(unsigned int add)
{
	if (0 != spi_flash_waddr(add+0x44, 4, (unsigned char *)&simprit.RIT_DataSec))	//д����Դ���ݲ���ռ��������
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x4A, 1, (unsigned char *)&simprit.RIT_DataType))	//д����Դ��������
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x1F4, 4, (unsigned char *)&simprit.RIT_Next))		//д����һ��RIT�ڴ����е�������ַ
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x1FC, 4, (unsigned char *)&simprit.RIT_TrailSig))	//д�������־
	{
		return -1;
	}
	return 0;
}

/**
* @brief  ��һ��BLK�Ĳ������ݸ��Ƶ���һ��BLK
* @param  desadd: Ŀ���ַ
*         srcadd: Դ��ַ
*            len: ���Ƴ���
* @retval    0: ���Ƴɹ�
*           -1: дflash����
*/
static int rec_copy4kblk(unsigned int desadd, unsigned int srcadd, unsigned int len)
{
	unsigned char	tmpbuf[16];
	while(len / 16)	// ��16�ֽ�Ϊ��λ�ֶθ���
	{
		spi_flash_raddr(srcadd, 16, tmpbuf);
		if (0 != spi_flash_waddr(desadd, 16, tmpbuf))
		{
			return -1;
		}
		len -= 16;
		srcadd += 16;
		desadd += 16;
	}
	// ��ʣ����ֽڸ���
	memset(tmpbuf,0xFF,16);
	spi_flash_raddr(srcadd, len, tmpbuf);
	if (0 != spi_flash_waddr(desadd, 16, tmpbuf))
	{
		return -1;
	}
	return 0;
}

/**
* @brief  ��ȡ��¼��������
* @param  rectype: ����������
* @retval     ��¼����0-2
*         -1: ʧ�� 
*/
static int	rec_getindex(unsigned char rectype)
{
	if ((rectype & INDEX_MASK) < (RECTYPENUM+1))
	{
		return (rectype & INDEX_MASK);
	} 
	else
	{
		return -1;
	}
}


/*----------------------- �������� -----------------------*/
/**
 * @brief  ��ʽ��SPI Flash��ʹ֮���Դ��number����СΪrecordsize�ļ�¼��
 * @param      recopt: ��¼���ͱ����ECC������־ ��0x80����ECC
 *         recordsize: ÿ����¼�Ĵ�С���ر�ECCʱ���֧��2046������ECCʱ֧�����2023�ֽ�
 *             number: ��¼��ɴ�ż�¼�ĸ���
 * @retval    0: ��ʽ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: ��¼���ȴ���֧�ֳ���
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: Flash�ռ䲻��
 *           -6: дFlash����
 */
int record_format(unsigned char recopt, unsigned int recordsize, unsigned int number)
{
	int						recindex;		// ��¼��������
	unsigned int			ritadd;			// RIT��ַ
	unsigned int			ritold;			// ������һ��RIT��ַ��
	unsigned int			recordspace;	// һ����¼�Ŀռ�
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			tmpadd;			// ��ʱ�õ�ַ

	/********* ��ʽ������ **********/
	/* 1.����������� **************/
	recindex	= rec_getindex(recopt & 0x7F);
	if (-1 == recindex)
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ **************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.��������¼��С **************/
	if ((recopt & 0x80) == 0x80 && recordsize > 2023)
	{
		return -3;		// ��¼���ȴ���֧�ֳ���
	}
	else if (recordsize > 2046)
	{
		return -3;		// ��¼���ȴ���֧�ֳ���
	}

	/* 4.��ѯRIT��׼����ʽ�� *******/
	//���flash���µģ���δ��������
	//flash��0��ַ�ط�����һ���յ�RIT��
	//�γ�һ��RIT����ͷ��
loop:
	ritadd	= 0;
	rec_readsimprit(ritadd);
	if( simprit.RIT_TrailSig != 0xAB550000)
	{
		if (0 != spi_flash_erasesector(0))
		{
			return -6;		// дFlash����
		}
		simprit.RIT_TrailSig	= 0xAB550000;
		simprit.RIT_DataType	= RITHD;
		simprit.RIT_DataSec		= 0;
		simprit.RIT_Next		= 0xFFFFFFFF;
		if (0 != rec_writesimprit(ritadd))
		{
			return -6;		// дFlash����
		}
	}
	while(1)
	{
#ifdef	RECORD_DEBUG
		printf("simprit.RIT_DataType is 0x%X\r\n", simprit.RIT_DataType);
		printf("simprit.RIT_DataSec  is 0x%X\r\n", simprit.RIT_DataSec);
		printf("simprit.RIT_Next     is 0x%X\r\n", simprit.RIT_Next);
		printf("simprit.RIT_TrailSig is 0x%X\r\n\r\n", simprit.RIT_TrailSig);
#endif
		if( simprit.RIT_TrailSig != 0xAB550000 || simprit.RIT_Next == 0)
			// simprit.RIT_Next == 0����Ϊ��ǰ��RIT�ļ��ṹ��0��β��������RIT������
		{
			//������ִ˼�¼����RIT���ʽ������ô����һ��RIT���Nextָ���޸�Ϊ����������ʾ��Դ�����Ѿ�����
			rec_readsimprit(ritold);
			simprit.RIT_Next = 0xFFFFFFFF;
			ritadd = ritold;
		}

		if ( simprit.RIT_DataType == ((recopt & INDEX_MASK) | 0xC0) )		// ���1,������Ҫ��ʽ����¼���Ѿ�����
		{
			break;									// ֱ������ѭ�������¸�ʽ��
		}

		ritold	= ritadd;							// �����ϴ�RIT��ֵ

		if (simprit.RIT_Next == 0xFFFFFFFF)			// ���2,������RIT�����βҲû���ҵ���Ҫ��ʽ���ļ�¼��
		{
			ritadd	= ritadd + (simprit.RIT_DataSec + 1) * 512;
			if (0 != (ritadd & BLKMSK))				// �����ַ����4K���룬����4K����
			{
				ritadd	= (ritadd | BLKMSK) + 1;
			}
			break;									// �����¸�ʽ����¼������ʼλ�ú�������
		}
		ritadd	= simprit.RIT_Next * 512;			// ���µ�ǰRIT��ֵ
		rec_readsimprit(ritadd);
	}

#ifdef	RECORD_DEBUG
	printf("ritold is 0x%X\r\n", ritold);
	printf("ritadd is 0x%X\r\n\r\n", ritadd);
#endif

	/* 5.��ritaddΪ�������и�ʽ�� *******/
	// �����Ƿ���ECC�ֱ��ȡһ����¼����ռ�
	if ((recopt & 0x80) == 0x80)
	{
		recordspace	= rec_getrecspc(recordsize, 1);		
	} 
	else
	{
		recordspace	= rec_getrecspc(recordsize, 0);
	}
	recordtotal	= recordspace * number;
	if (0 != (recordtotal & BLKMSK))				// �������4K���룬����4K����
	{
		recordtotal	= (recordtotal | BLKMSK) + 1;
	}
	recordtotal	= recordtotal + 3 * BLKSIZE;		// �ܴ�С���ڼ�¼����Ŀռ����1��������1���ϵ籣����1��RIT���ڿ�
	if (ritadd + recordtotal > recmod_flasize)
	{
		return -5;	// Flash�ռ䲻��
	}
	// д����һ��RIT���ӵ���ǰRIT��Flash��
	//simprit.RIT_Next		= ritadd / 512;		//����һ��RIT���ӵ���ǰRIT
	//if (0 != spi_flash_waddr(ritold+0x1F4, 4, (unsigned char *)&simprit.RIT_Next))
	//{
	//	return -6;		// дFlash����
	//}
#ifdef	RECORD_DEBUG
	printf("start erase...\r\n");
	printf("Start address is 0x%X\r\n", ritadd);
	printf("End   address is 0x%X\r\n\r\n", ritadd + recordtotal);
#endif
	// ������Ҫ��ʽ����������
	//ʵ���ϸ�ʽ����ʱ����Բ���Ҫ��������������Ϊ�����Ӽ�¼ʱ����������4K���׵�ַд����ʱ���Ὣ4K�����һ��
	//�˲�����Joe.zhou��20120509���ε��ġ�
#if 1
	for(tmpadd = ritadd; tmpadd < ritadd + recordtotal; tmpadd += BLKSIZE)
	{
		if (0 != spi_flash_erasesector(tmpadd))
		{
			return -6;		// дFlash����
		}
	}
#endif

	// �ü�¼����д��RIT
	simprit.RIT_DataSec		= (recordtotal - BLKSIZE) / 512;
	simprit.RIT_DataType	= ((recopt & INDEX_MASK) | 0xC0);
	simprit.RIT_TrailSig	= 0xAB550000;
	simprit.RIT_Next		= 0xFFFFFFFF;
#ifdef	RECORD_DEBUG
	printf("simprit.RIT_DataType is 0x%X\r\n", simprit.RIT_DataType);
	printf("simprit.RIT_DataSec  is 0x%X\r\n", simprit.RIT_DataSec);
	printf("simprit.RIT_Next     is 0x%X\r\n", simprit.RIT_Next);
	printf("simprit.RIT_TrailSig is 0x%X\r\n\r\n", simprit.RIT_TrailSig);
#endif
	if (0 != rec_writesimprit(ritadd))
	{
		return -6;		// дFlash����
	}
	//simprit.RIT_Next		= ritadd / 512;		//����һ��RIT���ӵ���ǰRIT
	
	// ���ü�¼������Ϣͷд��Flash��
#ifdef	RECORD_DEBUG
	printf("TRecFSCB'size is %d\r\n\r\n",sizeof(TRecFSCB));
#endif
	recinfo[recindex].rec_start		= ritadd + BLKSIZE;
	recinfo[recindex].rec_total		= recordtotal - 2 * BLKSIZE;
	//С���ɣ���ʽ����Headָ��ָ���¼�������һ����¼������д����ÿ��
	//��Head����һ����¼�ռ���д��������д��һ����¼ʱ��Headָ���ָ���¼��
	//��λ���ˡ�
	recinfo[recindex].rec_hdpt		= ritadd + recordtotal - BLKSIZE - recordspace;
	recinfo[recindex].rec_space		= (unsigned short)recordspace;
	recinfo[recindex].rec_size		= (unsigned short)recordsize;
	recinfo[recindex].rec_maxnum	= (unsigned short)number;
	recinfo[recindex].rec_count		= 0;
	recinfo[recindex].rec_eccflag	= recopt & 0x80;
	recinfo[recindex].rec_valid		= 0x00;
#ifdef	RECORD_DEBUG
	printf("recinfo[%d].rec_start  is 0x%X\r\n", recindex, recinfo[recindex].rec_start);
	printf("recinfo[%d].rec_total  is 0x%X\r\n", recindex, recinfo[recindex].rec_total);
	printf("recinfo[%d].rec_hdpt   is 0x%X\r\n", recindex, recinfo[recindex].rec_hdpt);
	printf("recinfo[%d].rec_size   is %d\r\n", recindex, recinfo[recindex].rec_size);
	printf("recinfo[%d].rec_space  is %d\r\n", recindex, recinfo[recindex].rec_space);
	printf("recinfo[%d].rec_maxnum is %d\r\n", recindex, recinfo[recindex].rec_maxnum);
	printf("recinfo[%d].rec_count  is %d\r\n", recindex, recinfo[recindex].rec_count);
	printf("recinfo[%d].rec_valid  is 0x%X\r\n\r\n", recindex, recinfo[recindex].rec_valid);
#endif
	if(0 != spi_flash_waddr(ritadd + RITSIZE, sizeof(TRecFSCB), (unsigned char *)&recinfo[recindex]))
	{
		return -6;		// дFlash����
	}
	// д����һ��RIT���ӵ���ǰRIT��Flash��
	tmpadd = ritadd/512;
	if (0 != spi_flash_waddr(ritold+0x1F4, 4, (unsigned char *)&tmpadd))
	{
		return -6;		// дFlash����
	}
	recinfo[recindex].rec_valid		= 0xAA;
	return 0;
}



/**
 * @brief  ��ȡ��¼��������Ϣrecinfo[]����,��ִ�жϵ�ָ�����
 * @param  rectype: ����������
 *                 	recinfo[recindex].rec_size		= (unsigned short)recordsize;
 *                  recinfo[recindex].rec_maxnum	= (unsigned short)number;
 * @retval    0: ��ʼ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: û���ҵ���¼��
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: дFlash����
 *           -6: ��¼�ĳ�����������ԭ����ʽ���Ĳ�ƥ��
 */
int record_init(unsigned char rectype, unsigned int recordsize, unsigned int number)
{
	int						recindex;		// ��¼��������
	unsigned int			ritadd;			// RIT��ַ
	unsigned int			recordspace;	// һ����¼�Ŀռ�
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			scanpt;			// �ϵ�ָ�ɨ��ָ��
	unsigned int			tmpadd;			// ��ʱ�õ�ַ
	unsigned int			headpt;			// ��0����¼ָ��
	unsigned char			rcvflag;		// ��¼״̬��־
	unsigned char			tmpflag;		// ��ʱ�ñ�־

	/********* ��ʼ������ **********/
	/* 1.����������� **************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ **************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}


	/* 3.��ѯRIT��׼����ʼ�� *******/
	ritadd	= 0;
	while(1)
	{
		rec_readsimprit(ritadd);
		if( simprit.RIT_TrailSig != 0xAB550000  || simprit.RIT_Next == 0)
			// simprit.RIT_Next == 0����Ϊ��ǰ��RIT�ļ��ṹ��0��β��������RIT������
		{
			return -4;								// ��RIT�����ļ�ϵͳ����
		}
		if (simprit.RIT_DataType == ((rectype & INDEX_MASK) | 0xC0))		// �ҵ���¼��
		{
			break;
		}
		if (simprit.RIT_Next == 0xFFFFFFFF)			// û���ҵ���¼��
		{
			return -3;
		}
		ritadd	= simprit.RIT_Next * 512;			// ���µ�ǰRIT��ֵ
	}



#ifdef	RECORD_DEBUG
	printf("ritadd is 0x%X\r\n", ritadd);
#endif
	/* 5.��ʼ��ʼ�� ****************/
	// ��ȡ��¼���Ŀ��ƿ���Ϣ
	spi_flash_raddr(ritadd + RITSIZE, sizeof(TRecFSCB), (unsigned char *)&recinfo[recindex]);
	recordspace	= recinfo[recindex].rec_space;
	recordstart	= recinfo[recindex].rec_start;
	recordtotal	= recinfo[recindex].rec_total;

	/* 4.����¼���Ⱥʹ�С�Ƿ���ȷ **************/
	if (recinfo[recindex].rec_size != (unsigned short)recordsize || recinfo[recindex].rec_maxnum != (unsigned short)number)
	{
		return -6;		// ��¼�ĳ�����������ԭ����ʽ���Ĳ�ƥ��
	}

	// ִ�жϵ�ָ�����
	//  <1> ɨ��������¼�������Ƿ���ִ��ɾ��4K��ʱ���ֶϵ�������
	//      ����У�����ɾ��һ�Σ�����ɾ����־д���Ѿ�ɾ����
	//      ע��ɾ��4K��ϵ籣�����ƣ�ÿ��ɾ��4K��ʱ������һ���
	//          ��һ���ֽ���������ɾ���ı�ǡ�ɾ���ɹ����ٱ��Ϊ
	//          �ɹ�ɾ����
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_FMTMSK) == REC_FMTINOP)	// ����δɾ����ɵĿ飨Ŀ�괦��飩
		{
#ifdef	RECORD_DEBUG
			printf("Find 1 block unfinished format!\r\n");
#endif
			if (scanpt == recordstart)
			{
				tmpadd	= recordstart + recordtotal - BLKSIZE;
			}
			else
			{
				tmpadd	= scanpt - BLKSIZE;
			}
			// ����ɾ��δɾ����ɵĿ�
			if (0 != spi_flash_erasesector(tmpadd))
			{
				return -5;		// дFlash����
			}
			// ����־дΪ�Ѿ�ɾ�����
			rcvflag	= REC_FMTOVOP;
			if (0 != spi_flash_waddr(scanpt, 1, &rcvflag))
			{
				return -5;		// дFlash����
			}

			// �ϵ�ʱֻ���ܳ���һ����������������ɾ��4K��ʱ��
			// ��ָ�4K���������к���ɨ�貽�裬���ü�¼�����ƿ��
			// Headָ�룬��ǰ��¼��������ʼ����ɱ�־�󷵻�

			// ���ڷ�����ɾ��4K��ϵ���������ɸ���ɾ�������4K��
			// �ж�Headָ��Ӧ��ָ�����4K���ǰһ����¼λ��
			if (tmpadd == recordstart)
			{
				tmpadd	= recordstart + recordtotal - recordspace;
			}
			else
			{
				tmpadd	= tmpadd - recordspace;
			}
			recinfo[recindex].rec_hdpt	= tmpadd;
			recinfo[recindex].rec_valid	= 0xAA;
			recinfo[recindex].rec_count	= (unsigned short)record_count(rectype);
			return 0;
		}
		scanpt += BLKSIZE;
	}

	// ִ�жϵ�ָ�����
	//  <2> ɨ��������¼�������Ƿ���ִ�м�¼�滻����ʱɾ����¼���ڵ�4K��ʱ���ֶϵ�������
	//      ����У�����ɾ��һ�Σ�����������ļ�¼copy������
	//      ע��ɾ��4K��ϵ籣�����ƣ�ÿ��ɾ��4K��ʱ������һ���
	//          ��һ���ֽ���������ɾ���ı�ǡ�ɾ���ɹ����ٱ��Ϊ
	//          �ɹ�ɾ����
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_FMTMSK) == REC_FMTEXOP)	// ���ֱ��޸ĺ󱸷ݵ�������
		{
#ifdef	RECORD_DEBUG
			printf("Find 1 block unfinished format!\r\n");
#endif
			if (scanpt == recordstart)
			{
				tmpadd	= recordstart + recordtotal - BLKSIZE;
			}
			else
			{
				tmpadd	= scanpt - BLKSIZE;
			}
			// ����ɾ��δɾ����ɵĿ�
			if (0 != spi_flash_erasesector(tmpadd))
			{
				return -5;		// дFlash����
			}
			
			//��������ļ�¼copy����
			if (0 != rec_copy4kblk(tmpadd,recordstart + recordtotal,BLKSIZE))
			{
				return -5;		// дFlash����
			}
			
			// ����־дΪ�Ѿ�ɾ�����
			rcvflag	= REC_FMTOVOP;
			if (0 != spi_flash_waddr(scanpt, 1, &rcvflag))
			{
				return -5;		// дFlash����
			}

			// �ϵ�ʱֻ���ܳ���һ����������������ɾ��4K��ʱ��
			// ��ָ�4K���������к���ɨ�貽�裬���ü�¼�����ƿ��
			// Headָ�룬��ǰ��¼��������ʼ����ɱ�־�󷵻�

			// ���ڷ�����ɾ��4K��ϵ���������ɸ���ɾ�������4K��
			// �ж�Headָ��Ӧ��ָ�����4K���ǰһ����¼λ��
			if (tmpadd == recordstart)
			{
				tmpadd	= recordstart + recordtotal - recordspace;
			}
			else
			{
				tmpadd	= tmpadd - recordspace;
			}
			recinfo[recindex].rec_hdpt	= tmpadd;
			recinfo[recindex].rec_valid	= 0xAA;
			recinfo[recindex].rec_count	= (unsigned short)record_count(rectype);
			return 0;
		}
		scanpt += BLKSIZE;
	}

	// <3> ɨ��������¼�����ָ�д��¼ʱ�Ķϵ����Ϣ��ʵ�ʲ����ǣ�
	//     �������д��¼ʱ�ϵ�ļ�¼�������ü�¼�����Ķϵ�ָ�
	//     ����������û��д��ļ�¼����ɾ�������ָ�д��¼��ɺ�
	//     û��ɾ�����ϼ�¼�������
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_STATEMSK) == REC_INVALID)
		{
#ifdef	RECORD_DEBUG
			printf("Find 1 record unfinished write!\r\n");
#endif
			// ���ûд��ļ�¼���ڵ�ַ����4K���������ü�¼�����Ķϵ�ָ�
			// ����������û��д��ļ�¼����ɾ������
			if (0 != (scanpt & BLKMSK))	
			{
				tmpadd	= scanpt & (~BLKMSK);
				if (0 != spi_flash_erasesector(recordstart + recordtotal))	// ��նϵ�ָ���
				{
					return -5;		// дFlash����
				}
				// ����Ҫ������������ʱ��ŵ��ϵ�ָ���
				if (0 != rec_copy4kblk(recordstart + recordtotal, tmpadd, scanpt-tmpadd))
				{
					return -5;		// дFlash����
				}
				// ��ո�4K��
				if (0 != spi_flash_erasesector(tmpadd))
				{
					return -5;		// дFlash����
				}
				//����ʱ����ڶϵ�ָ��������ݴ����
				if (0 != rec_copy4kblk(tmpadd, recordstart + recordtotal, scanpt-tmpadd))
				{
					return -5;		// дFlash����
				}
			}
			// ���ûд��ļ�¼���ڵ�ַ��4K������
			// ֱ��ɾ����4K�顣
			else
			{
				if (0 != spi_flash_erasesector(scanpt))
				{
					return -5;		// дFlash����
				}
			}
			// �ָ��ɹ������öϵ�ļ�¼λ�������¼�����ƿ��
			// Headָ�룬��ǰ��¼��������ʼ����ɱ�־�󷵻�
			if (scanpt == recordstart)
			{
				tmpadd	= recordstart + recordtotal - recordspace;
			}
			else
			{
				tmpadd	= scanpt - recordspace;
			}
			// ����FSCB
			recinfo[recindex].rec_valid	= 0xAA;
			recinfo[recindex].rec_count	= (unsigned short)record_count(rectype);
			recinfo[recindex].rec_hdpt	= tmpadd;
			return 0;
		}
		// ����Ĵ����¼���һ����¼��λ�ã��������ܼ�
		// ¼��������ܼ�¼����������¼����˵
		// �����ϴγ��ֶϵ��¼���������д�����µļ�¼��
		// ɾ�����ϼ�¼֮ʱ�����ָ��ʼ��͡�
		if ((rcvflag | REC_STATEMSK) == REC_OBSOLETE)
		{
			if (scanpt == recordstart + recordtotal - recordspace)
			{
				tmpadd	= recordstart;
			}
			else
			{
				tmpadd	= scanpt + recordspace;
			}
#ifdef	RECORD_DEBUG
			//printf("tmpadd is 0x%X\r\n",tmpadd);
#endif
			spi_flash_raddr(tmpadd, 1, &tmpflag);

			if ((tmpflag | REC_STATEMSK) == REC_VALID)
			{
#ifdef	RECORD_DEBUG
				printf("Find last record!\r\n");
#endif
				recinfo[recindex].rec_valid	= 0xAA;
				if (recinfo[recindex].rec_maxnum < record_count(rectype))
				{
					tmpflag	= REC_OBSOLETE;
					if (0 != spi_flash_waddr(tmpadd, 1, &tmpflag))
					{
						recinfo[recindex].rec_valid	= 0x00;
						return -5;		// дFlash����
					}
				}
				recinfo[recindex].rec_valid	= 0x00;
			}
		}

		// ɨ���¼��0����¼ָ��
		if ((rcvflag | REC_STATEMSK) == REC_VALID)
		{
			if (scanpt == recordstart + recordtotal - recordspace)
			{
				tmpadd	= recordstart;
			}
			else
			{
				tmpadd	= scanpt + recordspace;
			}
			spi_flash_raddr(tmpadd, 1, &tmpflag);
			if ((tmpflag | REC_STATEMSK) == REC_FREE || (tmpflag | REC_STATEMSK) == REC_OBSOLETE)
			{
#ifdef	RECORD_DEBUG
				printf("Find head record!\r\n");
#endif
				headpt	= scanpt;
			}
		}
		scanpt += recordspace;
	}
	// ɨ��ָ���ɺ������¼�����ƿ��Headָ�룬
	// ��ǰ��¼��������ʼ����ɱ�־�󷵻�
	recinfo[recindex].rec_valid	= 0xAA;
	recinfo[recindex].rec_count = record_count(rectype);
	if (0 == recinfo[recindex].rec_count)
	{
		recinfo[recindex].rec_hdpt	= recordstart + recordtotal - recordspace;
	}
	else
	{
		recinfo[recindex].rec_hdpt	= headpt;
	}
#ifdef	RECORD_DEBUG
	printf("Finished init OP!\r\n");
	printf("recinfo[%d].rec_start  is 0x%X\r\n", recindex, recinfo[recindex].rec_start);
	printf("recinfo[%d].rec_total  is 0x%X\r\n", recindex, recinfo[recindex].rec_total);
	printf("recinfo[%d].rec_hdpt   is 0x%X\r\n", recindex, recinfo[recindex].rec_hdpt);
	printf("recinfo[%d].rec_size   is %d\r\n", recindex, recinfo[recindex].rec_size);
	printf("recinfo[%d].rec_space  is %d\r\n", recindex, recinfo[recindex].rec_space);
	printf("recinfo[%d].rec_maxnum is %d\r\n", recindex, recinfo[recindex].rec_maxnum);
	printf("recinfo[%d].rec_count  is %d\r\n", recindex, recinfo[recindex].rec_count);
	printf("recinfo[%d].rec_valid  is 0x%X\r\n\r\n", recindex, recinfo[recindex].rec_valid);
#endif
	return 0;
}

/**
 * @brief  ��ȡ��������Ч��¼����
 * @param    rectype: ��¼������
 * @retval  0..count: ��¼����
 *                -1: ��֧�ִ����ͼ�¼
 *                -2: Flash��δ��ʼ��
 *				  -3: �ļ�ϵͳ��δ��ʼ��
 */
int record_count(unsigned char rectype)
{
	int						recindex;		// ��¼��������
	int						count;			// ��Ч��¼����
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			scanpt;			// �ϵ�ָ�ɨ��ָ��
	unsigned char			rcvflag;		// ��¼״̬��־

	/********* ��ȡ��¼�������� **********/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.ɨ��������¼��������Ч��¼�� **/
	recordspace	= recinfo[recindex].rec_space;
	recordstart	= recinfo[recindex].rec_start;
	recordtotal	= recinfo[recindex].rec_total;
	count		= 0;
	for(scanpt = recordstart; scanpt < recordstart + recordtotal; scanpt += recordspace)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_STATEMSK) == REC_VALID)
		{
			count++;
		}
	}
	return count;
}

//ֱ�Ӵ��ڴ淵�ص�ǰ��¼���ļ�¼��
int record_count_ext(unsigned char rectype)
{
	int						recindex;		// ��¼��������

	/********* ��ȡ��¼�������� **********/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	return recinfo[recindex].rec_count;
}

/**
 * @brief  ��ȡָ�������ļ�¼
 * @param    index: Ҫ��ȡ�ļ�¼����
 * @param  *record: ��ŵĵ�ַ
 * @param  rectype: ��¼���� ��ֻ��Ҫ��ȡ��־�ֽ�ʱ����rectype������0x80���л�����
 * @param      len: Ҫ��ȡ�ļ�¼����
 * @retval    0: ��ȡ�ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: ����������ȴ��ڲ����������ɵĳ���
 *           -5: �޴˼�¼
 *           -6: ��������
 * @note  Լ����¼�ĵ�һ���ֽ���Ϊ��־�ֽڣ���
 *        �ֽڲ�����У��ֵ��
 */
int record_read(unsigned char rectype, int index, unsigned char *record, unsigned int len)
{
	int						i;			
	int						recindex;		// ��¼��������
	unsigned char			chkcode;		// ��¼��ŵ�У����
	unsigned char			tmpchk;			// �������У����
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordsize;		// һ����¼�Ĵ�С
	unsigned short			recordsizetmp;	// ��ʱ��recordsize
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned int			tmpadd;			// ��ʱ�õ�ַ


	/********* ����¼���� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.��λ����Ϊindex�ļ�¼λ�� ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;

	if (len > recordsize)
	{
		return -4;
	}
	else if (len != 0)
	{
		recordsize	= len;
	}

	if (index >= recordcount)	// �����Ŵ��ڻ���ڵ�ǰ�ܼ�¼��
	{
		return -5;		// �޴˼�¼
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}
	if ((rectype & 0xC0) == 0x80)	//ֻ���޸ı�־
	{
		spi_flash_raddr(tmpadd+1, 1, record);
		return 0;
	}
	/***** ����У������� *****/
	if (recinfo[recindex].rec_eccflag == 0x0)
	{
		// �����ļ�¼����
		spi_flash_raddr(tmpadd+1, recordsize, record);
		// ������У���ֽ�
		spi_flash_raddr(tmpadd+1+recordsize, 1, &chkcode);		//������Ƕ�ȡ������¼����ôУ���ֽ�û������
		// У�鲻�����¼�ĵ�һ���ֽ�
		tmpchk	= record[1];
		for (i = 2; i < recordsize; i++)
		{
			tmpchk ^= record[i];
		}
		if (tmpchk != chkcode)
		{
			return -6;
		}
		return 0;
	} 
	else
	{
		//У����������һ�ֽ�
		recordsizetmp	= recordsize;
		spi_flash_raddr(tmpadd+1, 1, record);
		recordsizetmp--;
		i = 0;
		while(recordsizetmp / 256)
		{
			spi_flash_raddr(tmpadd+2+256*i, 256, record+1+256*i);
			spi_flash_raddr(tmpadd+1+recordsize+3*i, 3, ecckey_read);
			calculate_ecc(record+1+256*i,256,ecckey_cal);
			if (0 != memcmp(ecckey_cal,ecckey_read,3))
			{
				correct_data(record+1+256*i, ecckey_read, ecckey_cal,256);
			}
			i++;
			recordsizetmp -= 256;
		}
		if (recordsizetmp != 0)
		{
			memset(eccdata_tmp, 0, 256);
			spi_flash_raddr(tmpadd+2+256*i, recordsizetmp, eccdata_tmp);
			spi_flash_raddr(tmpadd+1+recordsize+3*i, 3, ecckey_read);
			calculate_ecc(eccdata_tmp, 256, ecckey_cal);
			if (0 != memcmp(ecckey_cal,ecckey_read,3))
			{
				correct_data(eccdata_tmp, ecckey_read, ecckey_cal, 256);
			}
			memcpy(record+1+256*i, eccdata_tmp, recordsizetmp);
		}
		return 0;
	}
}


/**
 * @brief  �޸ı�־�ֽ�
 * @param  rectype: ��¼����
 * @param    index: Ҫ�޸ĵļ�¼���
 * @param  mrkflag: Ҫ�޸ĵı�־
 * @retval    0: �޸ĳɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: �޴˼�¼
 *           -5: дFlash����
 * @note  Լ����¼�ĵ�һ���ֽ���Ϊ��־�ֽڣ���
 *        �ֽڲ�����У��ֵ��
 */
int record_modify(unsigned char rectype, int index, unsigned char mrkflag)
{
	int						recindex;		// ��¼��������
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned int			tmpadd;			// ��ʱ�õ�ַ

	/********* ����¼���� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.��λ����Ϊindex�ļ�¼λ�� ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;

	if (index >= recordcount)	// �����Ŵ��ڻ���ڵ�ǰ�ܼ�¼��
	{
		return -4;				// �޴˼�¼
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}
	if (0 != spi_flash_waddr(tmpadd+1, 1, &mrkflag))	// ֻдrecord�ĵ�һ���ֽ�
	{
		return -5;		// дFlash����
	}
	
#ifdef	RECORD_DEBUG
	printf("Addr: 0x%08X\r\n",tmpadd);
#endif
	return 0;
}


/**
* @brief  �滻ĳһ����¼
* @param  rectype: ��¼����
* @param    index: Ҫ�޸ĵļ�¼���
* @param	*pNewData:	�滻ԭ����¼��������
* @retval    0: �޸ĳɹ�
*           -1: ��֧�ִ����ͼ�¼
*           -2: Flash��δ��ʼ��
*           -3: �ļ�ϵͳ��δ��ʼ��
*           -4: �޴˼�¼
*           -5: дFlash����
* @note  Լ����¼�ĵ�һ���ֽ���Ϊ��־�ֽڣ���
*        �ֽڲ�����У��ֵ��
*/
int record_replace(unsigned char rectype, int index, unsigned char *pNewData)
{
	int						recindex;		// ��¼��������
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordsize;		// һ����¼�Ĵ�С
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned int			tmpadd;			// ��ʱ�õ�ַ
	unsigned int			i,target_blk;		// Ҫ�޸ĵļ�¼���ڵ�blk
	unsigned short			recordsizetmp;	// ��¼��С��ʱ����
	unsigned int			swap_blk;		//������ĵ�ַ
//	unsigned int			before_cnt;		//��һ������������Ҫ�滻�ļ�¼֮ǰ�ļ�¼��

	/********* ����¼���� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.��λ����Ϊindex�ļ�¼λ�� ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;

	if (index >= recordcount)	// �����Ŵ��ڻ���ڵ�ǰ�ܼ�¼��
	{
		return -4;				// �޴˼�¼
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}

	//�Ƚ��ü�¼���ڵ�block copy����¼���Ľ�����
	target_blk	= tmpadd & (~BLKMSK);	//Ҫ�޸ĵļ�¼���ڵ�block
	swap_blk = recordstart + recordtotal;		//������ĵ�ַ
	if (0 != spi_flash_erasesector(swap_blk))	// ������ݽ�����
	{
		return -5;		// дFlash����
	}
	// ���ü�¼֮ǰ�ļ�¼��copy��������
	if (0 != rec_copy4kblk(swap_blk, target_blk, tmpadd-target_blk))
	{
		return -5;		// дFlash����
	}

	//���¼�¼���ӽ���
	/****** ��ECC�����У���������д��¼ ****************/
	if (recinfo[recindex].rec_eccflag == 0x80)	//ECC
	{
		// ******* дECC
		recordsizetmp	= recordsize-1;
		i = 0;
		while(recordsizetmp / 256)
		{
			calculate_ecc(pNewData+1+i*256, 256, ecckey_cal);
			if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk) + 1 + recordsize + i*3, 3, ecckey_cal))	// дУ����
			{
				return -5;		// дFlash����
			}
			spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_read);
			if (0 != memcmp(ecckey_read, ecckey_cal, 3))
			{
				spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_read);
				if (0 != memcmp(ecckey_read, ecckey_cal, 3))
				{
					return -5;
				}
			}
			i++;
			recordsizetmp -= 256;
		}
		if (recordsizetmp != 0)
		{
			memset(eccdata_tmp, 0, 256);
			memcpy(eccdata_tmp,pNewData+1+i*256,recordsizetmp);
			calculate_ecc(eccdata_tmp, 256, ecckey_cal);
			if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_cal))	// дУ����
			{
				return -5;		// дFlash����
			}
			spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_read);
			if (0 != memcmp(ecckey_read, ecckey_cal, 3))
			{
				spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_read);
				if (0 != memcmp(ecckey_read, ecckey_cal, 3))
				{
					return -5;
				}
			}
		}
		// *************  д��¼
		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1, recordsize,pNewData))	// д��¼
		{
			return -5;		// дFlash����
		}
		for (i = 0; i < recordsize; i++)
		{
			spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+i, 1, &tmpfgrb);
			if (pNewData[i] != tmpfgrb)
			{
				spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+i, 1, &tmpfgrb);
				if (pNewData[i] != tmpfgrb)
				{
					return -5;
				}
			}
		}
	} 
	else	// ���У��
	{
		// 1.����У���� ÿ����¼�ĵ�һ���ֽڲ�����У�飬���û�����־λ��
		tmpfgwi	= pNewData[1];
		for (i = 2; i < recordsize; i++)
		{
			tmpfgwi	^= pNewData[i];
		}
		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1+recordsize, 1, &tmpfgwi))	// дУ����
		{
			return -5;		// дFlash����
		}
		spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize, 1, &tmpfgrb);
		if (tmpfgwi != tmpfgrb)
		{
			spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+recordsize, 1, &tmpfgrb);
			if (tmpfgwi != tmpfgrb)
			{
				return -5;
			}
		}

		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1, recordsize, pNewData))	// д��¼
		{
			return -5;		// дFlash����
		}

		//����Ƚ��е���Ŷ������һ�ζ�ȡ�����Ƚ�Ӧ�û��ܶ�
		for (i = 0; i < recordsize; i++)
		{
			spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+i, 1, &tmpfgrb);
			if (pNewData[i] != tmpfgrb)
			{
				spi_flash_raddr(swap_blk + (tmpadd - target_blk)+1+i, 1, &tmpfgrb);
				if (pNewData[i] != tmpfgrb)
				{
					return -5;
				}
			}
		}
	}

	//д��¼��Ч���
	tmpfgwi	= REC_VALID;
	if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk), 1, &tmpfgwi))
	{
		return -5;		// дFlash����
	}
	spi_flash_raddr(swap_blk + (tmpadd - target_blk), 1, &tmpfgrb);
	if (tmpfgwi != tmpfgrb)
	{
		spi_flash_raddr(swap_blk + (tmpadd - target_blk), 1, &tmpfgrb);
		if (tmpfgwi != tmpfgrb)
		{
			return -5;
		}
	}

	//���ü�¼��֮��ļ�¼copy��������
	if (0 != rec_copy4kblk(swap_blk + (tmpadd - target_blk) + recordspace, tmpadd + recordspace, BLKSIZE - (tmpadd-target_blk) - recordspace))
	{
		return -5;		// дFlash����
	}

	//����Ϊֹ��Ҫ�滻�ļ�¼���ڵļ�¼������ļ�¼ȫ����copy��������������

	//�ڲ���Ҫ�滻�ļ�¼���ڵļ�¼��֮ǰ��Ҫ������һ����¼������ǣ���ʾ�˼�¼���Ѿ������ݵ��˽����飬���Զϵ�ָ�
	tmpfgwi	= REC_FMTEXOP;
	tmpadd	= target_blk + BLKSIZE;
	if (tmpadd == recordstart + recordtotal)
	{
		tmpadd	= recordstart;
	}
	if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
	{
		return -5;		// дFlash����
	}
	//����Ҫ�滻�ļ�¼���ڵļ�¼��
	if (0 != spi_flash_erasesector(target_blk))
	{
		return -5;		// дFlash����
	}

	//��������ļ�¼copy����
	if (0 != rec_copy4kblk(target_blk,swap_blk,BLKSIZE))
	{
		return -5;		// дFlash����
	}

	tmpfgwi	= REC_FMTOVOP;
	if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
	{
		return -5;		// дFlash����
	}
	return 0;
}


/**
 * @brief  д���¼
 * @param  *record: ��ŵĵ�ַ
 * @param  rectype: ��¼����
 * @param      len: Ҫ��ȡ�ļ�¼����
 * @retval    0: д��ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: ����������ȴ��ڲ����������ɵĳ���
 *           -5: д���¼ʧ��
 *           -6: дFlash����
 */
int record_write(unsigned char rectype, unsigned char *record, unsigned int len)
{
	int						i;	
	int						recindex;		// ��¼��������
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordsize;		// һ����¼�Ĵ�С
	unsigned short			recordsizetmp;	// ��¼��С��ʱ����
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned short			recordmaxnum;	// ��¼���ɴ������¼��
	unsigned int			tmpadd;			// ��ʱ�õ�ַ
	//unsigned char			tmpfgwi;		// ��ʱ�ֽ�write in				
	//unsigned char			tmpfgrb;		// ��ʱ�ֽ�read back

	/********* д��¼���� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}
	
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	recordmaxnum	= recinfo[recindex].rec_maxnum;

	/* 4.������볤�� ********************/
	if (len > recordsize)
	{
		return -4;
	}
	else if (len != 0)
	{
		recordsize	= len;
	}

	/* 5.д��¼ ***************************/
	recordheadpt	+= recordspace;

	if (0 == (recordheadpt & BLKMSK))	// �����д����¼�¼λ����4K���׵�ַ����Ҫɾ���ÿ���д���¼�¼
	{
		if (recordheadpt == recordstart + recordtotal)	// ����Ѿ����˼�¼��β����������ָ��ʼλ��
		{
			recordheadpt = recordstart;
		}
		// ɾ��һ��4K��
		tmpfgwi	= REC_FMTINOP;
		tmpadd	= recordheadpt + BLKSIZE;
		if (tmpadd == recordstart + recordtotal)
		{
			tmpadd	= recordstart;
		}
		if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
		{
			return -5;		// дFlash����
		}
		if (0 != spi_flash_erasesector(recordheadpt))
		{
			return -5;		// дFlash����
		}
		tmpfgwi	= REC_FMTOVOP;
		if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
		{
			return -5;		// дFlash����
		}
	}

	// д��¼����
	// ÿдһ���������϶��رȽϣ����д������ز�ͬ�����öϵ罻������
	// ɾ������д����������ݣ������س���
	// ��ΪECC�ͼ����У���������

	/********** д�ϵ��־����Ϊ��Ч *********************/
	tmpfgwi	= REC_INVALID;
	test_var1 = recordheadpt;
	if (0 != spi_flash_waddr(recordheadpt, 1, &tmpfgwi))
	{
		return -5;		// дFlash����
	}

	test_var2 = recordheadpt;
	spi_flash_raddr(recordheadpt, 1, &tmpfgrb);
	if (tmpfgwi != tmpfgrb)
	{
		spi_flash_raddr(recordheadpt, 1, &tmpfgrb);
		if (tmpfgwi != tmpfgrb)
		{
			goto Rollback;
		}
	}
	
	/****** ��ECC�����У���������д��¼ ****************/
	if (recinfo[recindex].rec_eccflag == 0x80)	//ECC
	{
		// ******* дECC
		recordsizetmp	= recordsize-1;
		i = 0;
		while(recordsizetmp / 256)
		{
			calculate_ecc(record+1+i*256, 256, ecckey_cal);
			if (0 != spi_flash_waddr(recordheadpt+1+recordsize+i*3, 3, ecckey_cal))	// дУ����
			{
				return -5;		// дFlash����
			}
			spi_flash_raddr(recordheadpt+1+recordsize+i*3, 3, ecckey_read);
			if (0 != memcmp(ecckey_read, ecckey_cal, 3))
			{
				spi_flash_raddr(recordheadpt+1+recordsize+i*3, 3, ecckey_read);
				if (0 != memcmp(ecckey_read, ecckey_cal, 3))
				{
					goto Rollback;
				}
			}
			i++;
			recordsizetmp -= 256;
		}
		if (recordsizetmp != 0)
		{
			memset(eccdata_tmp, 0, 256);
			memcpy(eccdata_tmp,record+1+i*256,recordsizetmp);
			calculate_ecc(eccdata_tmp, 256, ecckey_cal);
			if (0 != spi_flash_waddr(recordheadpt+1+recordsize+i*3, 3, ecckey_cal))	// дУ����
			{
				return -5;		// дFlash����
			}
			spi_flash_raddr(recordheadpt+1+recordsize+i*3, 3, ecckey_read);
			if (0 != memcmp(ecckey_read, ecckey_cal, 3))
			{
				spi_flash_raddr(recordheadpt+1+recordsize+i*3, 3, ecckey_read);
				if (0 != memcmp(ecckey_read, ecckey_cal, 3))
				{
					goto Rollback;
				}
			}
		}
		// *************  д��¼
		if (0 != spi_flash_waddr(recordheadpt+1, recordsize, record))	// д��¼
		{
			return -5;		// дFlash����
		}
		for (i = 0; i < recordsize; i++)
		{
			spi_flash_raddr(recordheadpt+1+i, 1, &tmpfgrb);
			if (record[i] != tmpfgrb)
			{
				spi_flash_raddr(recordheadpt+1+i, 1, &tmpfgrb);
				if (record[i] != tmpfgrb)
				{
					goto Rollback;
				}
			}
		}
	} 
	else	// ���У��
	{
		// 1.����У���� ÿ����¼�ĵ�һ���ֽڲ�����У�飬���û�����־λ��
		tmpfgwi	= record[1];
		for (i = 2; i < recordsize; i++)
		{
			tmpfgwi	^= record[i];
		}
		if (0 != spi_flash_waddr(recordheadpt+1+recordsize, 1, &tmpfgwi))	// дУ����
		{
			return -5;		// дFlash����
		}
		spi_flash_raddr(recordheadpt+1+recordsize, 1, &tmpfgrb);
		if (tmpfgwi != tmpfgrb)
		{
			spi_flash_raddr(recordheadpt+1+recordsize, 1, &tmpfgrb);
			if (tmpfgwi != tmpfgrb)
			{
				goto Rollback;
			}
		}

		if (0 != spi_flash_waddr(recordheadpt+1, recordsize, record))	// д��¼
		{
			return -5;		// дFlash����
		}

		//����Ƚ��е���Ŷ������һ�ζ�ȡ�����Ƚ�Ӧ�û��ܶ�
		for (i = 0; i < recordsize; i++)
		{
			spi_flash_raddr(recordheadpt+1+i, 1, &tmpfgrb);
			if (record[i] != tmpfgrb)
			{
				spi_flash_raddr(recordheadpt+1+i, 1, &tmpfgrb);
				if (record[i] != tmpfgrb)
				{
					goto Rollback;
				}
			}
		}
	}
	/*********** д�ϵ��־����Ϊ��Ч *********************/
	tmpfgwi	= REC_VALID;
	if (0 != spi_flash_waddr(recordheadpt, 1, &tmpfgwi))
	{
		return -5;		// дFlash����
	}
	spi_flash_raddr(recordheadpt, 1, &tmpfgrb);
	if (tmpfgwi != tmpfgrb)
	{
		spi_flash_raddr(recordheadpt, 1, &tmpfgrb);
		if (tmpfgwi != tmpfgrb)
		{
			goto Rollback;
		}
	}
	recinfo[recindex].rec_hdpt	= recordheadpt;		// ����Headָ��
	// �����ǰ��¼���ﵽ����¼��ֵ��ɾ�����ϼ�¼������ǰ��¼��1
	if (recordcount == recordmaxnum)
	{
		if (recordheadpt >= recordstart + recordspace * recordmaxnum)
		{
			tmpadd	= recordheadpt - recordspace * recordmaxnum;
		}
		else
		{
			tmpadd	= recordtotal + recordheadpt - recordspace * recordmaxnum;
		}
		tmpfgwi	= REC_OBSOLETE;
		if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
		{
			return -5;		// дFlash����
		}
	}
	else
	{
		recinfo[recindex].rec_count++;
	}
	return 0;

/********** д��ʧ��ʱ�Ļָ����� ************/
Rollback:
	tmpadd	= recordheadpt & (~BLKMSK);
	if (0 != spi_flash_erasesector(recordstart + recordtotal))	// ��նϵ�ָ���
	{
		return -5;		// дFlash����
	}
	// ����Ҫ������������ʱ��ŵ��ϵ�ָ���
	if (0 != rec_copy4kblk(recordstart + recordtotal, tmpadd, recordheadpt-tmpadd))
	{
		return -5;		// дFlash����
	}
	// ��ո�4K��
	if (0 != spi_flash_erasesector(tmpadd))
	{
		return -5;		// дFlash����
	}
	// ����ʱ����ڶϵ�ָ��������ݴ����
	if (0 != rec_copy4kblk(tmpadd, recordstart + recordtotal, recordheadpt-tmpadd))
	{
		return -5;		// дFlash����
	}
	return -6;			// д���¼ʧ��
}


/**
* @brief  ɾ��ָ����¼�������м�¼
* @param  rectype: ��¼������
* @retval    0: ɾ���ɹ�
*           -1: ��֧�ִ����ͼ�¼
*           -2: Flash��δ��ʼ��
*           -3: �ļ�ϵͳ��δ��ʼ��
*           -4: дFlash����
*/
int record_delall(unsigned char rectype)
{
	int						recindex;		// ��¼��������
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned char			rcvflag;		// ��¼״̬��־

	/******* ɾ�����м�¼���� ***********/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;

	/* 4.�����м�¼��־�ɹ�ʱ ***********/
	rcvflag			= REC_OBSOLETE;
	while (recordcount > 0)
	{
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -4;		// дFlashʧ��
		}
		if (recordheadpt == recordstart)
		{
			recordheadpt = recordstart + recordtotal;
		}
		recordheadpt -= recordspace;
		recordcount--;
	}
	recinfo[recindex].rec_count	= recordcount;
	return 0;
}


/**
* @brief  ��ȡ��¼������
* @param  rectype: ��¼����
* @param  recblkinfo: ��¼��Ϣ�ṹ��ָ��
* @retval    0:��ȡ�ɹ�
*           -1:��֧�ִ����ͼ�¼
*           -2:Flash��δ��ʼ��
*           -3:�ļ�ϵͳ��δ��ʼ��
*/
int get_recordinfo(unsigned char rectype, tRECBLKInfo *recblkinfo)
{
	int						recindex;		// ��¼��������

	/******* ��ȡ��¼������ **************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.��ȡ��Ϣ *************************/
	recblkinfo->recblk_startadd	= recinfo[recindex].rec_start - BLKSIZE;
	recblkinfo->recblk_totsize	= recinfo[recindex].rec_total + 2 * BLKSIZE;
	recblkinfo->recblk_datadd	= recinfo[recindex].rec_start;
	recblkinfo->recblk_datcnt	= recinfo[recindex].rec_count;
	recblkinfo->recblk_datotal	= recinfo[recindex].rec_total;
	recblkinfo->recblk_datsize	= recinfo[recindex].rec_size;
	recblkinfo->recblk_datspac	= recinfo[recindex].rec_space;
	recblkinfo->recblk_maxcap	= recinfo[recindex].rec_maxnum;
	recblkinfo->recblk_hdpt		= recinfo[recindex].rec_hdpt;
	recblkinfo->recblk_eccen	= recinfo[recindex].rec_eccflag;
	return 0;
}

/**
 * @brief  ��ʽ��һ����СΪparamsize�Ĳ�������
 * @param  recordsize: �������Ĵ�С
 * @retval    0: ��ʽ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �������ȴ���2K-2
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: Flash�ռ䲻��
 *           -6: дFlash����
 */
int param_format(unsigned int paramsize)
{
	return record_format(PARBLK, paramsize, 8);
}

/**
 * @brief  ��ȡ����
 * @param   *param: ��ŵĵ�ַ
 *             len: Ҫ��ȡ�Ĳ�������
 * @retval    0: ��ȡ�ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: �޴˼�¼
 *           -5: ����������ȴ��ڲ����������ɵĳ���
 *           -6: ����������
 */
int param_read(unsigned char *param, unsigned int len)
{
	int						i,index;			
	int						recindex;		// ��¼��������
	unsigned char			chkcode;		// ��¼��ŵ�У����
	unsigned char			tmpchk;			// �������У����
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordsize;		// һ����¼�Ĵ�С
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned int			tmpadd;			// ��ʱ�õ�ַ


	/********* ����¼���� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(PARBLK);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.��λ����Ϊindex�ļ�¼λ�� ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	
	if (recordcount == 0)
	{
		return -4;			// �޴˼�¼
	}

	if (len > recordsize)
	{
		return -5;			// ����������ȴ��ڲ����������ɵĳ���
	}
	else
	{
		recordsize	= len;
	}

	for (index = 0; index < 4; index++)
	{
		if (recordheadpt >= recordstart + index * recordspace)
		{
			tmpadd	= recordheadpt - index * recordspace;
		}
		else
		{
			tmpadd = recordtotal + recordheadpt - index * recordspace;
		}
		spi_flash_raddr(tmpadd+2, recordsize, param);

		// У������ļ�¼����
		spi_flash_raddr(tmpadd+1, 1, &chkcode);
		tmpchk	= param[0];
		for (i = 1; i < recordsize; i++)
		{
			tmpchk ^= param[i];
		}
		if (tmpchk == chkcode)
		{
			return 0;
		}
	}
	return -6;	// ����������
}

/**
 * @brief  д�����
 * @param  *param: ��ŵĵ�ַ
 * @param     len: д�볤��
 * @retval    0: д��ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: ����������ȴ��ڲ����������ɵĳ���
 *			 -5: дFlash����
 */
int param_write(unsigned char *param, unsigned int len)
{
	int						i, j;	
	int						recindex;		// ��¼��������
	unsigned int			recordtotal;	// ������¼������ռ�
	unsigned int			recordstart;	// ��¼����ʼ��ַ
	unsigned int			recordheadpt;	// ��0����¼ָ��
	unsigned short			recordspace;	// һ����¼�Ŀռ�
	unsigned short			recordsize;		// һ����¼�Ĵ�С
	unsigned short			recordcount;	// ��ǰ��¼����
	unsigned short			recordmaxnum;	// ��¼���ɴ������¼��
	unsigned int			tmpadd;			// ��ʱ�õ�ַ
	unsigned char			rcvflag;		// ��¼״̬��־
	unsigned char			chkcode;		// У����

	/********* д�������� ****************/
	/* 1.����������� ********************/
	recindex	= rec_getindex(PARBLK);
	if (-1 == recindex )
	{
		return -1;		// ��֧�ִ����ͼ�¼
	}

	/* 2.���Flash״̬ ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash��δ��ʼ��
	}

	/* 3.����ļ�ϵͳ�Ƿ��ʼ�� *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// �ļ�ϵͳ��δ��ʼ��
	}

	/* 4.д��¼ ***************************/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	recordmaxnum	= recinfo[recindex].rec_maxnum;

	if (len > recordsize)
	{
		return -4;			// ����������ȴ��ڲ����������ɵĳ���
	}
	else
	{
		recordsize	= len;
	}

	for(j = 0; j < 4; j++)	// д��4�β���
	{
		recordheadpt	+= recordspace;

		if (0 == (recordheadpt & BLKMSK))	// �����д����¼�¼λ����4K���׵�ַ����Ҫɾ���ÿ���д���¼�¼
		{
			if (recordheadpt == recordstart + recordtotal)	// ����Ѿ����˼�¼��β����������ָ��ʼλ��
			{
				recordheadpt = recordstart;
			}
			// ɾ��һ��4K��
			spi_flash_raddr(recordheadpt, 1, &rcvflag);
			if (REC_OBSOLETE == (rcvflag | REC_STATEMSK))
			{
				rcvflag	= REC_FMTINOP;
				tmpadd	= recordheadpt + BLKSIZE;
				if (tmpadd == recordstart + recordtotal)
				{
					tmpadd	= recordstart;
				}
				if (0 != spi_flash_waddr(tmpadd, 1, &rcvflag))
				{
					return -5;		// дFlash����
				}
				if (0 != spi_flash_erasesector(recordheadpt))
				{
					return -5;		// дFlash����
				}
				rcvflag	= REC_FMTOVOP;
				if (0 != spi_flash_waddr(tmpadd, 1, &rcvflag))
				{
					return -5;		// дFlash����
				}
			}
		}

		// д��������
		// 1.����У����
		chkcode	= param[0];
		for (i = 1; i < recordsize; i++)
		{
			chkcode	^= param[i];
		}
		// 2.д��¼
		rcvflag	= REC_INVALID;
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -5;		// дFlash����
		}
		if (0 != spi_flash_waddr(recordheadpt+2, recordsize, param))
		{
			return -5;		// дFlash����
		}
		rcvflag	= REC_VALID;
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -5;		// дFlash����
		}
		// 3.дУ����
		if (0 != spi_flash_waddr(recordheadpt+1, 1, &chkcode))
		{
			return -5;		// дFlash����
		}

		recinfo[recindex].rec_hdpt	= recordheadpt;		// ����Headָ��

		// �����ǰ��¼���ﵽ����¼��ֵ��ɾ�����ϼ�¼������ǰ��¼��1
		if (recordcount == recordmaxnum)
		{
			if (recordheadpt >= recordstart + recordspace * recordmaxnum)
			{
				tmpadd	= recordheadpt - recordspace * recordmaxnum;
			}
			else
			{
				tmpadd	= recordtotal + recordheadpt - recordspace * recordmaxnum;
			}
			rcvflag	= REC_OBSOLETE;
			if (0 != spi_flash_waddr(tmpadd, 1, &rcvflag))
			{
				return -5;		// дFlash����
			}
		}
		else
		{
			recinfo[recindex].rec_count++;
		}
	}
	return 0;
}

/**
 * @brief  ��ȡ������������Ϣrecinfo[]����,��ִ�жϵ�ָ�����
 * @param  None
 * @retval    0: ��ʼ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: û���ҵ�������
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: дFlash����
 *           -6: ��¼�ĳ�����������ԭ����ʽ���Ĳ�ƥ��
 */

int param_init(unsigned int paramsize)
{
	return record_init(PARBLK,paramsize,8);
}
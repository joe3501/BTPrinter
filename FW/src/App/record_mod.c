/**
***************************************************************************
* @file record_mod.c
* @brief 
***************************************************************************
*
* @version V0.2.1
* @author jiangcx
* @date 2010年12月08日
* @note 加入ECC校验纠错机制，recordformat时传入的recopt参数或上0x80即可
*       开启ECC校验纠错功能
*
***************************************************************************
*
* @version V0.2.0
* @author jiangcx
* @date 2010年12月06日
* @note 加入length参数,约定每条记录的第一个字节为标志字节
*
***************************************************************************
*
* @version V0.1.2
* @author jiangcx
* @date 2010年12月06日
* @note 修复断电恢复代码bug。此版本为不带length参数的最终版本。
*       下一个版本号将为V0.2.0加入length参数，与校验纠错机制
*
***************************************************************************
*
* @version V0.1.1
* @author jiangcx
* @date 2010年11月18日
* @note 加入写入校验机制
*       支持6个记录区，与一个参数区，参数和单条记录长度必须小于2K-1
*
***************************************************************************
*
* @copy
*
* 此代码为深圳江波龙电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2010 netcom</center></h1>
*/


//note:如果需要支持更多的记录分区，只需要修改宏定义  RECTYPENUM	为希望支持的分区类型数


#include "spi_flash.h"
#include "record_mod.h"
#include "rec_ecc.h"
#include <string.h>

#ifdef	RECORD_DEBUG
#include <stdio.h>
#endif

#define RECTYPENUM					0x01		//支持1个种类的记录区，@note 如果应用需要支持更多的单据，那么此定义也需要相应的修改，注意不能超过63
#define RITHD						0x80		//调试时，如果遇见新Flash，则建立一个头RIT
#define INDEX_MASK					0x3f		//记录类型的低6位表示记录索引

typedef struct {
	unsigned char		RIT_OEMName[16];    		// 0  ASCII		用于存放OEM厂商的名字，如："ShenZhenNetcom  "
	unsigned char		RIT_ManName[16];    		// 10 ASCII	   	制造商名，如："ShenZhenNetcom  "
	unsigned char		RIT_ModuleName[16];			// 20 ASCII		模块名，如："POSRESOURCES    "
	unsigned short		RIT_ProdID;					// 30 WORD		产品ID：用户自己定义。如：0x0001或1234
	unsigned short		RIT_Version;    			// 32 BCD码		版本号：用户自己定义。如：0x0001或1234
	unsigned short		RIT_Date;            		// 34 BCD码		更新时间：0x06,0x28 表示6月28日
	unsigned char		RIT_SN[10];            		// 36 ASCII		序列号，用户自己定义。如："1234567890"
	unsigned int		RIT_TotalSect;        		// 40 DWORD		此份资源的总扇区数，包括这份头的512字节
	unsigned int		RIT_DataSec;        		// 44 DWORD	***	此份资源数据部分占的扇区数
	unsigned char		RIT_VerifyType;        		// 48 BYTE		此份资源的校验算法，此域暂不使用
	unsigned char		RIT_Endian;        			// 49 BYTE		此份资源适用的CPU大小端模式。0x0B表示大端 0x0C表示小端
	unsigned char		RIT_DataType;         		// 4A BYTE	***	此份资源的类型码：点阵字库：0x10 图片数据：0x11 字库链接：0x12 升级代码：0x80 升级资源数据：0x81 参数区：0x90 记录区：0x91
	unsigned char		RIT_Reserved0;				// 4B BYTE		保留
	unsigned int		RIT_StartAddr;        		// 4C DWORD		启动代码在ram中存放的起始地址，此项对于代码有效
	unsigned int		RIT_RunAddr;        		// 50 DWORD		运行代码时PC跳转的地址
	unsigned short   	RIT_DataID;					// 54 WORD		此份资源的ID号
	unsigned char		RIT_Align;					// 56 BYTE		此份资源的最小对齐单位，为2^n字节
	unsigned char		RIT_Reserved1;				// 57
	unsigned char		RIT_StrVer[13];				// 58           资源的版本号，格式为20100610_0001
	unsigned char		RIT_Reserved2[0x18B];		// 65 
	unsigned int		RIT_DataCRC32;				// 1F0          数据区的CRC32，即从下一个扇区起，到RIT_CodeSec止的所有数据的CRC32值或CRC，此项根据xml文件中的配置来生成。
	unsigned int		RIT_Next;            		// 1F4 ***      下一份RIT在磁盘中的绝对地址(扇区)
	unsigned int		RIT_CheckSum;        		// 1F8 此份资源表RIT的CRC32校验值。， 
	unsigned int		RIT_TrailSig;       		// 1FC 结束标志(0x00,0x00,0x55,0xAB)
}TResInfoTable;

#pragma pack(1)
typedef struct {
	unsigned int		RIT_DataSec;        		// 44 DWORD	***	此份资源数据部分占的扇区数
	unsigned int		RIT_Next;            		// 1F4 ***      下一份RIT在磁盘中的绝对地址(扇区) 
	unsigned int		RIT_TrailSig;       		// 1FC 结束标志(0x00,0x00,0x55,0xAB)
	unsigned char		RIT_DataType;         		// 4A BYTE	***	此份资源的类型码
}TSimpRIT;

typedef struct
{
	unsigned int	rec_start;		// 记录区数据起始地址			存储到Flash
	unsigned int	rec_total;		// 记录区数据存储区总大小		存储到Flash
	unsigned int	rec_hdpt;		// Head指针，指向第0条记录	内存动态值
	unsigned short	rec_size;		// 一条记录的字节数			存储到Flash
	unsigned short	rec_space;		// 每条记录所占空间数			存储到Flash
	unsigned short	rec_maxnum;		// 可存放最大的记录数			存储到Flash
	unsigned short	rec_count;		// 当前有效记录数				内存动态值
	unsigned char	rec_eccflag;	// ECC标识 0x80使能 0x00除能	存储到Flash
	unsigned char	rec_valid;		// 系统初始化标志0x00|0xAA		内存动态值
}TRecFSCB;	// Record File System Control Block
#pragma pack()

#define BLKSIZE						0x1000		// 4K块
#define BLKMSK						0x00000FFF	// 4K块掩码

#define	RITSIZE						512			// RIT大小
#define	SECSIZE						512			// 扇区大小

//记录状态标志定义
#define REC_STATEMSK				0x8F
#define REC_FREE					0xFF		// 空闲
#define REC_INVALID					0xEF		// 无效
#define REC_VALID					0xCF		// 有效
#define REC_OBSOLETE				0x8F		// 过时

//擦除4K块时的防掉电保护标志定义
#define REC_FMTMSK					0xF8
#define REC_FMTNOOP					0xFF		// 无擦除操作
#define REC_FMTINOP					0xFE		// 正在擦除	
#define REC_FMTEXOP					0xFC		// 被备份到数据交换块
#define REC_FMTOVOP					0xF8		// 擦除完毕


static TRecFSCB					recinfo[RECTYPENUM+1];
static TSimpRIT					simprit;

extern unsigned int				recmod_flasize;
//ECC校验纠错计算缓冲区
unsigned char		ecckey_read[3];
unsigned char		ecckey_cal[3];
unsigned char		eccdata_tmp[256];

static int			rec_getindex(unsigned char rectype);
static void			rec_readsimprit(unsigned int add);
static int			rec_writesimprit(unsigned int add);
static unsigned int	rec_getrecspc(unsigned int recsize, unsigned char eccflag);
static int			rec_copy4kblk(unsigned int desadd, unsigned int srcadd, unsigned int len);
static	unsigned char			tmpfgwi;		// 临时字节write in
static	unsigned char			tmpfgrb;		// 临时字节read back
static volatile int						test_var1;
static volatile int						test_var2;

/*-----------------  内部函数  -----------------------*/
/**
* @brief  获取记录文件系统必须的RIT信息
* @param   add: RIT的首地址
* @retval None
*/
static void	rec_readsimprit(unsigned int add)
{
	spi_flash_raddr(add+0x44, 4, (unsigned char *)&simprit.RIT_DataSec);	//获取资源数据部分占的扇区数
	spi_flash_raddr(add+0x4A, 1, (unsigned char *)&simprit.RIT_DataType);	//获取资源的类型码
	spi_flash_raddr(add+0x1F4, 4, (unsigned char *)&simprit.RIT_Next);		//获取下一份RIT在磁盘中的扇区地址
	spi_flash_raddr(add+0x1FC, 4, (unsigned char *)&simprit.RIT_TrailSig);	//获取结束标志
}

/**
* @brief  计算一条记录所需的空间
* @param  recsize: 一条记录的长度
* @param  eccflag: ECC功能是否开启
* @retval     2^n: 记录所需空间, 2^n 
*/
static unsigned int	rec_getrecspc(unsigned int recsize, unsigned char eccflag)
{
	unsigned int	recspace;
	int				i;
	// 计算一条记录所需的空间, 空间分配以2^n为单位,
	// 每条记录需要添加1byte断电保护标志
	// 如果ECC功能开启则还需要加上N字节的ECC纠错校验码位，
	// 否则加上1字节的异或校验位
	if(eccflag)
	{
		recspace	= recsize + 1 + (recsize + 255) / 256 * 3;
	}
	else
	{
		recspace	= recsize + 2;
	}
	// 循环搜索最左端的1
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
	recspace = 0x1 << i;	//得到2^n大小
	return recspace;
}

/**
* @brief  写入记录文件系统必须的RIT信息
* @param   add: RIT的首地址
* @retval    0: 成功
*           -1: 失败
*/
static int	rec_writesimprit(unsigned int add)
{
	if (0 != spi_flash_waddr(add+0x44, 4, (unsigned char *)&simprit.RIT_DataSec))	//写入资源数据部分占的扇区数
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x4A, 1, (unsigned char *)&simprit.RIT_DataType))	//写入资源的类型码
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x1F4, 4, (unsigned char *)&simprit.RIT_Next))		//写入下一份RIT在磁盘中的扇区地址
	{
		return -1;
	}
	if (0 != spi_flash_waddr(add+0x1FC, 4, (unsigned char *)&simprit.RIT_TrailSig))	//写入结束标志
	{
		return -1;
	}
	return 0;
}

/**
* @brief  将一个BLK的部分内容复制到另一个BLK
* @param  desadd: 目标地址
*         srcadd: 源地址
*            len: 复制长度
* @retval    0: 复制成功
*           -1: 写flash出错
*/
static int rec_copy4kblk(unsigned int desadd, unsigned int srcadd, unsigned int len)
{
	unsigned char	tmpbuf[16];
	while(len / 16)	// 以16字节为单位分段复制
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
	// 将剩余的字节复制
	memset(tmpbuf,0xFF,16);
	spi_flash_raddr(srcadd, len, tmpbuf);
	if (0 != spi_flash_waddr(desadd, 16, tmpbuf))
	{
		return -1;
	}
	return 0;
}

/**
* @brief  获取记录类型索引
* @param  rectype: 参数区类型
* @retval     记录索引0-2
*         -1: 失败 
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


/*----------------------- 导出函数 -----------------------*/
/**
 * @brief  格式化SPI Flash，使之可以存放number条大小为recordsize的记录。
 * @param      recopt: 记录类型编号与ECC开启标志 或0x80开启ECC
 *         recordsize: 每条记录的大小，关闭ECC时最大支持2046，开启ECC时支持最大2023字节
 *             number: 记录块可存放记录的个数
 * @retval    0: 格式化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 记录长度大于支持长度
 *           -4: 读RIT出错，文件系统错误
 *           -5: Flash空间不够
 *           -6: 写Flash出错
 */
int record_format(unsigned char recopt, unsigned int recordsize, unsigned int number)
{
	int						recindex;		// 记录区号索引
	unsigned int			ritadd;			// RIT地址
	unsigned int			ritold;			// 保存上一个RIT地址。
	unsigned int			recordspace;	// 一条记录的空间
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			tmpadd;			// 临时用地址

	/********* 格式化步骤 **********/
	/* 1.检查输入类型 **************/
	recindex	= rec_getindex(recopt & 0x7F);
	if (-1 == recindex)
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 **************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查输入记录大小 **************/
	if ((recopt & 0x80) == 0x80 && recordsize > 2023)
	{
		return -3;		// 记录长度大于支持长度
	}
	else if (recordsize > 2046)
	{
		return -3;		// 记录长度大于支持长度
	}

	/* 4.查询RIT，准备格式化 *******/
	//如果flash是新的，这段代码可以在
	//flash的0地址地方建立一个空的RIT，
	//形成一个RIT链表头。
loop:
	ritadd	= 0;
	rec_readsimprit(ritadd);
	if( simprit.RIT_TrailSig != 0xAB550000)
	{
		if (0 != spi_flash_erasesector(0))
		{
			return -6;		// 写Flash出错
		}
		simprit.RIT_TrailSig	= 0xAB550000;
		simprit.RIT_DataType	= RITHD;
		simprit.RIT_DataSec		= 0;
		simprit.RIT_Next		= 0xFFFFFFFF;
		if (0 != rec_writesimprit(ritadd))
		{
			return -6;		// 写Flash出错
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
			// simprit.RIT_Next == 0是因为以前的RIT文件结构以0结尾，这里做RIT出错处理
		{
			//如果发现此记录区的RIT表格式错误，那么将上一份RIT表的Next指针修改为结束符，表示资源链表已经结束
			rec_readsimprit(ritold);
			simprit.RIT_Next = 0xFFFFFFFF;
			ritadd = ritold;
		}

		if ( simprit.RIT_DataType == ((recopt & INDEX_MASK) | 0xC0) )		// 情况1,发现需要格式化记录区已经存在
		{
			break;									// 直接跳出循环，重新格式化
		}

		ritold	= ritadd;							// 更新上次RIT的值

		if (simprit.RIT_Next == 0xFFFFFFFF)			// 情况2,搜索至RIT链表结尾也没有找到需要格式化的记录区
		{
			ritadd	= ritadd + (simprit.RIT_DataSec + 1) * 512;
			if (0 != (ritadd & BLKMSK))				// 如果地址不是4K对齐，则变成4K对齐
			{
				ritadd	= (ritadd | BLKMSK) + 1;
			}
			break;									// 计算新格式化记录区的起始位置后跳出。
		}
		ritadd	= simprit.RIT_Next * 512;			// 更新当前RIT的值
		rec_readsimprit(ritadd);
	}

#ifdef	RECORD_DEBUG
	printf("ritold is 0x%X\r\n", ritold);
	printf("ritadd is 0x%X\r\n\r\n", ritadd);
#endif

	/* 5.以ritadd为基础进行格式化 *******/
	// 根据是否开启ECC分别获取一条记录所需空间
	if ((recopt & 0x80) == 0x80)
	{
		recordspace	= rec_getrecspc(recordsize, 1);		
	} 
	else
	{
		recordspace	= rec_getrecspc(recordsize, 0);
	}
	recordtotal	= recordspace * number;
	if (0 != (recordtotal & BLKMSK))				// 如果不是4K对齐，则变成4K对齐
	{
		recordtotal	= (recordtotal | BLKMSK) + 1;
	}
	recordtotal	= recordtotal + 3 * BLKSIZE;		// 总大小等于记录所需的空间加上1个交换块1个断电保护块1个RIT所在块
	if (ritadd + recordtotal > recmod_flasize)
	{
		return -5;	// Flash空间不够
	}
	// 写入上一个RIT链接到当前RIT到Flash中
	//simprit.RIT_Next		= ritadd / 512;		//将上一个RIT链接到当前RIT
	//if (0 != spi_flash_waddr(ritold+0x1F4, 4, (unsigned char *)&simprit.RIT_Next))
	//{
	//	return -6;		// 写Flash出错
	//}
#ifdef	RECORD_DEBUG
	printf("start erase...\r\n");
	printf("Start address is 0x%X\r\n", ritadd);
	printf("End   address is 0x%X\r\n\r\n", ritadd + recordtotal);
#endif
	// 擦除需要格式化的数据区
	//实际上格式化的时候可以不需要擦除数据区，因为在增加记录时，发现是往4K块首地址写数据时都会将4K块擦除一遍
	//此部分是Joe.zhou于20120509屏蔽掉的。
#if 1
	for(tmpadd = ritadd; tmpadd < ritadd + recordtotal; tmpadd += BLKSIZE)
	{
		if (0 != spi_flash_erasesector(tmpadd))
		{
			return -6;		// 写Flash出错
		}
	}
#endif

	// 该记录区的写入RIT
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
		return -6;		// 写Flash出错
	}
	//simprit.RIT_Next		= ritadd / 512;		//将上一个RIT链接到当前RIT
	
	// 将该记录区的信息头写入Flash中
#ifdef	RECORD_DEBUG
	printf("TRecFSCB'size is %d\r\n\r\n",sizeof(TRecFSCB));
#endif
	recinfo[recindex].rec_start		= ritadd + BLKSIZE;
	recinfo[recindex].rec_total		= recordtotal - 2 * BLKSIZE;
	//小技巧，格式化后将Head指针指向记录区的最后一条记录，由于写操作每次
	//将Head加上一个记录空间再写，这样，写第一条记录时，Head指针就指向记录区
	//首位置了。
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
		return -6;		// 写Flash出错
	}
	// 写入上一个RIT链接到当前RIT到Flash中
	tmpadd = ritadd/512;
	if (0 != spi_flash_waddr(ritold+0x1F4, 4, (unsigned char *)&tmpadd))
	{
		return -6;		// 写Flash出错
	}
	recinfo[recindex].rec_valid		= 0xAA;
	return 0;
}



/**
 * @brief  获取记录区数据信息recinfo[]内容,并执行断电恢复操作
 * @param  rectype: 参数区类型
 *                 	recinfo[recindex].rec_size		= (unsigned short)recordsize;
 *                  recinfo[recindex].rec_maxnum	= (unsigned short)number;
 * @retval    0: 初始化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 没有找到记录区
 *           -4: 读RIT出错，文件系统错误
 *           -5: 写Flash出错
 *           -6: 记录的长度与条数与原来格式化的不匹配
 */
int record_init(unsigned char rectype, unsigned int recordsize, unsigned int number)
{
	int						recindex;		// 记录区号索引
	unsigned int			ritadd;			// RIT地址
	unsigned int			recordspace;	// 一条记录的空间
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			scanpt;			// 断电恢复扫描指针
	unsigned int			tmpadd;			// 临时用地址
	unsigned int			headpt;			// 第0条记录指针
	unsigned char			rcvflag;		// 记录状态标志
	unsigned char			tmpflag;		// 临时用标志

	/********* 初始化步骤 **********/
	/* 1.检查输入类型 **************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 **************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}


	/* 3.查询RIT，准备初始化 *******/
	ritadd	= 0;
	while(1)
	{
		rec_readsimprit(ritadd);
		if( simprit.RIT_TrailSig != 0xAB550000  || simprit.RIT_Next == 0)
			// simprit.RIT_Next == 0是因为以前的RIT文件结构以0结尾，这里做RIT出错处理
		{
			return -4;								// 读RIT出错，文件系统错误
		}
		if (simprit.RIT_DataType == ((rectype & INDEX_MASK) | 0xC0))		// 找到记录区
		{
			break;
		}
		if (simprit.RIT_Next == 0xFFFFFFFF)			// 没有找到记录区
		{
			return -3;
		}
		ritadd	= simprit.RIT_Next * 512;			// 更新当前RIT的值
	}



#ifdef	RECORD_DEBUG
	printf("ritadd is 0x%X\r\n", ritadd);
#endif
	/* 5.开始初始化 ****************/
	// 获取记录区的控制块信息
	spi_flash_raddr(ritadd + RITSIZE, sizeof(TRecFSCB), (unsigned char *)&recinfo[recindex]);
	recordspace	= recinfo[recindex].rec_space;
	recordstart	= recinfo[recindex].rec_start;
	recordtotal	= recinfo[recindex].rec_total;

	/* 4.检查记录长度和大小是否正确 **************/
	if (recinfo[recindex].rec_size != (unsigned short)recordsize || recinfo[recindex].rec_maxnum != (unsigned short)number)
	{
		return -6;		// 记录的长度与条数与原来格式化的不匹配
	}

	// 执行断电恢复操作
	//  <1> 扫描整个记录区查找是否有执行删除4K块时出现断电的情况，
	//      如果有，重新删除一次，并将删除标志写成已经删除。
	//      注：删除4K块断电保护机制，每次删除4K块时，在下一块的
	//          第一个字节做上正在删除的标记。删除成功后再标记为
	//          成功删除。
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_FMTMSK) == REC_FMTINOP)	// 发现未删除完成的块（目标处理块）
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
			// 重新删除未删除完成的块
			if (0 != spi_flash_erasesector(tmpadd))
			{
				return -5;		// 写Flash出错
			}
			// 将标志写为已经删除完成
			rcvflag	= REC_FMTOVOP;
			if (0 != spi_flash_waddr(scanpt, 1, &rcvflag))
			{
				return -5;		// 写Flash出错
			}

			// 断电时只可能出现一种情况，如果出现在删除4K块时，
			// 则恢复4K块后无需进行后续扫描步骤，设置记录区控制块的
			// Head指针，当前记录条数，初始化完成标志后返回

			// 由于发生了删除4K块断电的情况，则可根据删除的这个4K块
			// 判断Head指针应该指向这个4K块的前一条记录位置
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

	// 执行断电恢复操作
	//  <2> 扫描整个记录区查找是否有执行记录替换操作时删除记录所在的4K块时出现断电的情况，
	//      如果有，重新删除一次，并将交换块的记录copy回来。
	//      注：删除4K块断电保护机制，每次删除4K块时，在下一块的
	//          第一个字节做上正在删除的标记。删除成功后再标记为
	//          成功删除。
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_FMTMSK) == REC_FMTEXOP)	// 发现被修改后备份到交换块
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
			// 重新删除未删除完成的块
			if (0 != spi_flash_erasesector(tmpadd))
			{
				return -5;		// 写Flash出错
			}
			
			//将交换块的记录copy回来
			if (0 != rec_copy4kblk(tmpadd,recordstart + recordtotal,BLKSIZE))
			{
				return -5;		// 写Flash出错
			}
			
			// 将标志写为已经删除完成
			rcvflag	= REC_FMTOVOP;
			if (0 != spi_flash_waddr(scanpt, 1, &rcvflag))
			{
				return -5;		// 写Flash出错
			}

			// 断电时只可能出现一种情况，如果出现在删除4K块时，
			// 则恢复4K块后无需进行后续扫描步骤，设置记录区控制块的
			// Head指针，当前记录条数，初始化完成标志后返回

			// 由于发生了删除4K块断电的情况，则可根据删除的这个4K块
			// 判断Head指针应该指向这个4K块的前一条记录位置
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

	// <3> 扫描整个记录区，恢复写记录时的断电的信息，实际操作是：
	//     如果发现写记录时断电的记录，则利用记录区最后的断电恢复
	//     区，将这条没有写完的记录彻底删除掉。恢复写记录完成后
	//     没有删除最老记录的情况。
	scanpt	= recordstart;
	while(scanpt < recordstart + recordtotal)
	{
		spi_flash_raddr(scanpt, 1, &rcvflag);
		if ((rcvflag | REC_STATEMSK) == REC_INVALID)
		{
#ifdef	RECORD_DEBUG
			printf("Find 1 record unfinished write!\r\n");
#endif
			// 如果没写完的记录所在地址不是4K对齐则利用记录区最后的断电恢复
			// 区，将这条没有写完的记录彻底删除掉。
			if (0 != (scanpt & BLKMSK))	
			{
				tmpadd	= scanpt & (~BLKMSK);
				if (0 != spi_flash_erasesector(recordstart + recordtotal))	// 清空断电恢复区
				{
					return -5;		// 写Flash出错
				}
				// 将需要保留的数据临时存放到断电恢复区
				if (0 != rec_copy4kblk(recordstart + recordtotal, tmpadd, scanpt-tmpadd))
				{
					return -5;		// 写Flash出错
				}
				// 清空该4K块
				if (0 != spi_flash_erasesector(tmpadd))
				{
					return -5;		// 写Flash出错
				}
				//将临时存放在断电恢复区的数据存回来
				if (0 != rec_copy4kblk(tmpadd, recordstart + recordtotal, scanpt-tmpadd))
				{
					return -5;		// 写Flash出错
				}
			}
			// 如果没写完的记录所在地址是4K对齐则
			// 直接删除该4K块。
			else
			{
				if (0 != spi_flash_erasesector(scanpt))
				{
					return -5;		// 写Flash出错
				}
			}
			// 恢复成功后，利用断电的记录位置算出记录区控制块的
			// Head指针，当前记录条数，初始化完成标志后返回
			if (scanpt == recordstart)
			{
				tmpadd	= recordstart + recordtotal - recordspace;
			}
			else
			{
				tmpadd	= scanpt - recordspace;
			}
			// 更新FSCB
			recinfo[recindex].rec_valid	= 0xAA;
			recinfo[recindex].rec_count	= (unsigned short)record_count(rectype);
			recinfo[recindex].rec_hdpt	= tmpadd;
			return 0;
		}
		// 下面的代码记录最后一条记录的位置，并计算总记
		// 录数，如果总记录数大于最大记录数则说
		// 明，上次出现断电事件并发生在写入最新的记录后
		// 删除最老记录之时，这种概率极低。
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
						return -5;		// 写Flash出错
					}
				}
				recinfo[recindex].rec_valid	= 0x00;
			}
		}

		// 扫描记录第0条记录指针
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
	// 扫描恢复完成后，设这记录区控制块的Head指针，
	// 当前记录条数，初始化完成标志后返回
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
 * @brief  获取数据区有效记录条数
 * @param    rectype: 记录区类型
 * @retval  0..count: 记录条数
 *                -1: 不支持此类型记录
 *                -2: Flash尚未初始化
 *				  -3: 文件系统尚未初始化
 */
int record_count(unsigned char rectype)
{
	int						recindex;		// 记录区号索引
	int						count;			// 有效记录条数
	unsigned short			recordspace;	// 一条记录的空间
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			scanpt;			// 断电恢复扫描指针
	unsigned char			rcvflag;		// 记录状态标志

	/********* 获取记录条数步骤 **********/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.扫描整个记录区计算有效记录数 **/
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

//直接从内存返回当前记录区的记录数
int record_count_ext(unsigned char rectype)
{
	int						recindex;		// 记录区号索引

	/********* 获取记录条数步骤 **********/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	return recinfo[recindex].rec_count;
}

/**
 * @brief  读取指定索引的记录
 * @param    index: 要读取的记录索引
 * @param  *record: 存放的地址
 * @param  rectype: 记录类型 当只需要读取标志字节时，将rectype参数与0x80进行或运算
 * @param      len: 要读取的记录长度
 * @retval    0: 读取成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 输入参数长度大于参数所能容纳的长度
 *           -5: 无此记录
 *           -6: 读出错误
 * @note  约定记录的第一个字节做为标志字节，该
 *        字节不记入校验值中
 */
int record_read(unsigned char rectype, int index, unsigned char *record, unsigned int len)
{
	int						i;			
	int						recindex;		// 记录区号索引
	unsigned char			chkcode;		// 记录存放的校验码
	unsigned char			tmpchk;			// 计算出的校验码
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordsize;		// 一条记录的大小
	unsigned short			recordsizetmp;	// 临时用recordsize
	unsigned short			recordcount;	// 当前记录条数
	unsigned int			tmpadd;			// 临时用地址


	/********* 读记录步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.定位索引为index的记录位置 ******/
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

	if (index >= recordcount)	// 索引号大于或等于当前总记录数
	{
		return -5;		// 无此记录
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}
	if ((rectype & 0xC0) == 0x80)	//只读修改标志
	{
		spi_flash_raddr(tmpadd+1, 1, record);
		return 0;
	}
	/***** 读出校验与纠错 *****/
	if (recinfo[recindex].rec_eccflag == 0x0)
	{
		// 读出的记录内容
		spi_flash_raddr(tmpadd+1, recordsize, record);
		// 读出的校验字节
		spi_flash_raddr(tmpadd+1+recordsize, 1, &chkcode);		//如果不是读取整条记录，那么校验字节没有意义
		// 校验不计算记录的第一个字节
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
		//校验纠错不计算第一字节
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
 * @brief  修改标志字节
 * @param  rectype: 记录类型
 * @param    index: 要修改的记录编号
 * @param  mrkflag: 要修改的标志
 * @retval    0: 修改成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 无此记录
 *           -5: 写Flash出错
 * @note  约定记录的第一个字节做为标志字节，该
 *        字节不记入校验值中
 */
int record_modify(unsigned char rectype, int index, unsigned char mrkflag)
{
	int						recindex;		// 记录区号索引
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordcount;	// 当前记录条数
	unsigned int			tmpadd;			// 临时用地址

	/********* 读记录步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.定位索引为index的记录位置 ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;

	if (index >= recordcount)	// 索引号大于或等于当前总记录数
	{
		return -4;				// 无此记录
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}
	if (0 != spi_flash_waddr(tmpadd+1, 1, &mrkflag))	// 只写record的第一个字节
	{
		return -5;		// 写Flash出错
	}
	
#ifdef	RECORD_DEBUG
	printf("Addr: 0x%08X\r\n",tmpadd);
#endif
	return 0;
}


/**
* @brief  替换某一条记录
* @param  rectype: 记录类型
* @param    index: 要修改的记录编号
* @param	*pNewData:	替换原来记录的新数据
* @retval    0: 修改成功
*           -1: 不支持此类型记录
*           -2: Flash尚未初始化
*           -3: 文件系统尚未初始化
*           -4: 无此记录
*           -5: 写Flash出错
* @note  约定记录的第一个字节做为标志字节，该
*        字节不记入校验值中
*/
int record_replace(unsigned char rectype, int index, unsigned char *pNewData)
{
	int						recindex;		// 记录区号索引
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordsize;		// 一条记录的大小
	unsigned short			recordcount;	// 当前记录条数
	unsigned int			tmpadd;			// 临时用地址
	unsigned int			i,target_blk;		// 要修改的记录所在的blk
	unsigned short			recordsizetmp;	// 记录大小临时变量
	unsigned int			swap_blk;		//交换块的地址
//	unsigned int			before_cnt;		//在一个擦除块中在要替换的记录之前的记录数

	/********* 读记录步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.定位索引为index的记录位置 ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;

	if (index >= recordcount)	// 索引号大于或等于当前总记录数
	{
		return -4;				// 无此记录
	}
	if (recordheadpt >= recordstart + index * recordspace)
	{
		tmpadd	= recordheadpt - index * recordspace;
	}
	else
	{
		tmpadd = recordtotal + recordheadpt - index * recordspace;
	}

	//先将该记录所在的block copy到记录区的交换块
	target_blk	= tmpadd & (~BLKMSK);	//要修改的记录所在的block
	swap_blk = recordstart + recordtotal;		//交换块的地址
	if (0 != spi_flash_erasesector(swap_blk))	// 清空数据交换块
	{
		return -5;		// 写Flash出错
	}
	// 将该记录之前的记录先copy到交换块
	if (0 != rec_copy4kblk(swap_blk, target_blk, tmpadd-target_blk))
	{
		return -5;		// 写Flash出错
	}

	//将新记录增加进来
	/****** 分ECC和异或校验两种情况写记录 ****************/
	if (recinfo[recindex].rec_eccflag == 0x80)	//ECC
	{
		// ******* 写ECC
		recordsizetmp	= recordsize-1;
		i = 0;
		while(recordsizetmp / 256)
		{
			calculate_ecc(pNewData+1+i*256, 256, ecckey_cal);
			if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk) + 1 + recordsize + i*3, 3, ecckey_cal))	// 写校验码
			{
				return -5;		// 写Flash出错
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
			if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1+recordsize+i*3, 3, ecckey_cal))	// 写校验码
			{
				return -5;		// 写Flash出错
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
		// *************  写记录
		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1, recordsize,pNewData))	// 写记录
		{
			return -5;		// 写Flash出错
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
	else	// 异或校验
	{
		// 1.计算校验码 每条记录的第一个字节不计入校验，供用户做标志位用
		tmpfgwi	= pNewData[1];
		for (i = 2; i < recordsize; i++)
		{
			tmpfgwi	^= pNewData[i];
		}
		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1+recordsize, 1, &tmpfgwi))	// 写校验码
		{
			return -5;		// 写Flash出错
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

		if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk)+1, recordsize, pNewData))	// 写记录
		{
			return -5;		// 写Flash出错
		}

		//这个比较有点慢哦，可以一次读取出来比较应该会快很多
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

	//写记录有效标记
	tmpfgwi	= REC_VALID;
	if (0 != spi_flash_waddr(swap_blk + (tmpadd - target_blk), 1, &tmpfgwi))
	{
		return -5;		// 写Flash出错
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

	//将该记录区之后的记录copy到交换块
	if (0 != rec_copy4kblk(swap_blk + (tmpadd - target_blk) + recordspace, tmpadd + recordspace, BLKSIZE - (tmpadd-target_blk) - recordspace))
	{
		return -5;		// 写Flash出错
	}

	//到此为止，要替换的记录所在的记录块里面的记录全部被copy到交换块里面了

	//在擦除要替换的记录所在的记录块之前需要对其下一个记录块做标记，表示此记录块已经被备份到了交换块，可以断电恢复
	tmpfgwi	= REC_FMTEXOP;
	tmpadd	= target_blk + BLKSIZE;
	if (tmpadd == recordstart + recordtotal)
	{
		tmpadd	= recordstart;
	}
	if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
	{
		return -5;		// 写Flash出错
	}
	//擦除要替换的记录所在的记录块
	if (0 != spi_flash_erasesector(target_blk))
	{
		return -5;		// 写Flash出错
	}

	//将交换块的记录copy回来
	if (0 != rec_copy4kblk(target_blk,swap_blk,BLKSIZE))
	{
		return -5;		// 写Flash出错
	}

	tmpfgwi	= REC_FMTOVOP;
	if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
	{
		return -5;		// 写Flash出错
	}
	return 0;
}


/**
 * @brief  写入记录
 * @param  *record: 存放的地址
 * @param  rectype: 记录类型
 * @param      len: 要读取的记录长度
 * @retval    0: 写入成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 输入参数长度大于参数所能容纳的长度
 *           -5: 写入记录失败
 *           -6: 写Flash出错
 */
int record_write(unsigned char rectype, unsigned char *record, unsigned int len)
{
	int						i;	
	int						recindex;		// 记录区号索引
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordsize;		// 一条记录的大小
	unsigned short			recordsizetmp;	// 记录大小临时变量
	unsigned short			recordcount;	// 当前记录条数
	unsigned short			recordmaxnum;	// 记录区可存放最大记录数
	unsigned int			tmpadd;			// 临时用地址
	//unsigned char			tmpfgwi;		// 临时字节write in				
	//unsigned char			tmpfgrb;		// 临时字节read back

	/********* 写记录步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}
	
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	recordmaxnum	= recinfo[recindex].rec_maxnum;

	/* 4.检查输入长度 ********************/
	if (len > recordsize)
	{
		return -4;
	}
	else if (len != 0)
	{
		recordsize	= len;
	}

	/* 5.写记录 ***************************/
	recordheadpt	+= recordspace;

	if (0 == (recordheadpt & BLKMSK))	// 如果将写入的新记录位置是4K块首地址则需要删除该块以写入新记录
	{
		if (recordheadpt == recordstart + recordtotal)	// 如果已经到了记录区尾部，则重新指向开始位置
		{
			recordheadpt = recordstart;
		}
		// 删除一块4K块
		tmpfgwi	= REC_FMTINOP;
		tmpadd	= recordheadpt + BLKSIZE;
		if (tmpadd == recordstart + recordtotal)
		{
			tmpadd	= recordstart;
		}
		if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
		{
			return -5;		// 写Flash出错
		}
		if (0 != spi_flash_erasesector(recordheadpt))
		{
			return -5;		// 写Flash出错
		}
		tmpfgwi	= REC_FMTOVOP;
		if (0 != spi_flash_waddr(tmpadd, 1, &tmpfgwi))
		{
			return -5;		// 写Flash出错
		}
	}

	// 写记录步骤
	// 每写一段数据马上读回比较，如果写入与读回不同则利用断电交换区，
	// 删除掉刚写入的所有数据，并返回出错
	// 分为ECC和简单异或校验两种情况

	/********** 写断电标志设置为无效 *********************/
	tmpfgwi	= REC_INVALID;
	test_var1 = recordheadpt;
	if (0 != spi_flash_waddr(recordheadpt, 1, &tmpfgwi))
	{
		return -5;		// 写Flash出错
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
	
	/****** 分ECC和异或校验两种情况写记录 ****************/
	if (recinfo[recindex].rec_eccflag == 0x80)	//ECC
	{
		// ******* 写ECC
		recordsizetmp	= recordsize-1;
		i = 0;
		while(recordsizetmp / 256)
		{
			calculate_ecc(record+1+i*256, 256, ecckey_cal);
			if (0 != spi_flash_waddr(recordheadpt+1+recordsize+i*3, 3, ecckey_cal))	// 写校验码
			{
				return -5;		// 写Flash出错
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
			if (0 != spi_flash_waddr(recordheadpt+1+recordsize+i*3, 3, ecckey_cal))	// 写校验码
			{
				return -5;		// 写Flash出错
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
		// *************  写记录
		if (0 != spi_flash_waddr(recordheadpt+1, recordsize, record))	// 写记录
		{
			return -5;		// 写Flash出错
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
	else	// 异或校验
	{
		// 1.计算校验码 每条记录的第一个字节不计入校验，供用户做标志位用
		tmpfgwi	= record[1];
		for (i = 2; i < recordsize; i++)
		{
			tmpfgwi	^= record[i];
		}
		if (0 != spi_flash_waddr(recordheadpt+1+recordsize, 1, &tmpfgwi))	// 写校验码
		{
			return -5;		// 写Flash出错
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

		if (0 != spi_flash_waddr(recordheadpt+1, recordsize, record))	// 写记录
		{
			return -5;		// 写Flash出错
		}

		//这个比较有点慢哦，可以一次读取出来比较应该会快很多
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
	/*********** 写断电标志设置为有效 *********************/
	tmpfgwi	= REC_VALID;
	if (0 != spi_flash_waddr(recordheadpt, 1, &tmpfgwi))
	{
		return -5;		// 写Flash出错
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
	recinfo[recindex].rec_hdpt	= recordheadpt;		// 更新Head指针
	// 如果当前记录数达到最大记录数值则删除最老记录，否则当前记录加1
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
			return -5;		// 写Flash出错
		}
	}
	else
	{
		recinfo[recindex].rec_count++;
	}
	return 0;

/********** 写入失败时的恢复代码 ************/
Rollback:
	tmpadd	= recordheadpt & (~BLKMSK);
	if (0 != spi_flash_erasesector(recordstart + recordtotal))	// 清空断电恢复区
	{
		return -5;		// 写Flash出错
	}
	// 将需要保留的数据临时存放到断电恢复区
	if (0 != rec_copy4kblk(recordstart + recordtotal, tmpadd, recordheadpt-tmpadd))
	{
		return -5;		// 写Flash出错
	}
	// 清空该4K块
	if (0 != spi_flash_erasesector(tmpadd))
	{
		return -5;		// 写Flash出错
	}
	// 将临时存放在断电恢复区的数据存回来
	if (0 != rec_copy4kblk(tmpadd, recordstart + recordtotal, recordheadpt-tmpadd))
	{
		return -5;		// 写Flash出错
	}
	return -6;			// 写入记录失败
}


/**
* @brief  删除指定记录区的所有记录
* @param  rectype: 记录区类型
* @retval    0: 删除成功
*           -1: 不支持此类型记录
*           -2: Flash尚未初始化
*           -3: 文件系统尚未初始化
*           -4: 写Flash出错
*/
int record_delall(unsigned char rectype)
{
	int						recindex;		// 记录区号索引
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordcount;	// 当前记录条数
	unsigned char			rcvflag;		// 记录状态标志

	/******* 删除所有记录步骤 ***********/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;

	/* 4.将所有记录标志成过时 ***********/
	rcvflag			= REC_OBSOLETE;
	while (recordcount > 0)
	{
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -4;		// 写Flash失败
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
* @brief  获取记录区参数
* @param  rectype: 记录类型
* @param  recblkinfo: 记录信息结构体指针
* @retval    0:获取成功
*           -1:不支持此类型记录
*           -2:Flash尚未初始化
*           -3:文件系统尚未初始化
*/
int get_recordinfo(unsigned char rectype, tRECBLKInfo *recblkinfo)
{
	int						recindex;		// 记录区号索引

	/******* 获取记录区步骤 **************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(rectype);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.获取信息 *************************/
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
 * @brief  格式化一个大小为paramsize的参数区。
 * @param  recordsize: 参数区的大小
 * @retval    0: 格式化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 参数长度大于2K-2
 *           -4: 读RIT出错，文件系统错误
 *           -5: Flash空间不够
 *           -6: 写Flash出错
 */
int param_format(unsigned int paramsize)
{
	return record_format(PARBLK, paramsize, 8);
}

/**
 * @brief  读取参数
 * @param   *param: 存放的地址
 *             len: 要读取的参数长度
 * @retval    0: 读取成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 无此记录
 *           -5: 输入参数长度大于参数所能容纳的长度
 *           -6: 读参数出错
 */
int param_read(unsigned char *param, unsigned int len)
{
	int						i,index;			
	int						recindex;		// 记录区号索引
	unsigned char			chkcode;		// 记录存放的校验码
	unsigned char			tmpchk;			// 计算出的校验码
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordsize;		// 一条记录的大小
	unsigned short			recordcount;	// 当前记录条数
	unsigned int			tmpadd;			// 临时用地址


	/********* 读记录步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(PARBLK);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.定位索引为index的记录位置 ******/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	
	if (recordcount == 0)
	{
		return -4;			// 无此记录
	}

	if (len > recordsize)
	{
		return -5;			// 输入参数长度大于参数所能容纳的长度
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

		// 校验读出的记录内容
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
	return -6;	// 读参数出错
}

/**
 * @brief  写入参数
 * @param  *param: 存放的地址
 * @param     len: 写入长度
 * @retval    0: 写入成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 输入参数长度大于参数所能容纳的长度
 *			 -5: 写Flash出错
 */
int param_write(unsigned char *param, unsigned int len)
{
	int						i, j;	
	int						recindex;		// 记录区号索引
	unsigned int			recordtotal;	// 整个记录区所需空间
	unsigned int			recordstart;	// 记录区起始地址
	unsigned int			recordheadpt;	// 第0条记录指针
	unsigned short			recordspace;	// 一条记录的空间
	unsigned short			recordsize;		// 一条记录的大小
	unsigned short			recordcount;	// 当前记录条数
	unsigned short			recordmaxnum;	// 记录区可存放最大记录数
	unsigned int			tmpadd;			// 临时用地址
	unsigned char			rcvflag;		// 记录状态标志
	unsigned char			chkcode;		// 校验码

	/********* 写参数步骤 ****************/
	/* 1.检查输入类型 ********************/
	recindex	= rec_getindex(PARBLK);
	if (-1 == recindex )
	{
		return -1;		// 不支持此类型记录
	}

	/* 2.检查Flash状态 ********************/
	if (0 != spi_flash_valid())
	{
		return -2;		// Flash尚未初始化
	}

	/* 3.检查文件系统是否初始化 *********/
	if (0xAA != recinfo[recindex].rec_valid)
	{
		return -3;		// 文件系统尚未初始化
	}

	/* 4.写记录 ***************************/
	recordcount		= recinfo[recindex].rec_count;
	recordheadpt	= recinfo[recindex].rec_hdpt;
	recordstart		= recinfo[recindex].rec_start;
	recordspace		= recinfo[recindex].rec_space;
	recordtotal		= recinfo[recindex].rec_total;
	recordsize		= recinfo[recindex].rec_size;
	recordmaxnum	= recinfo[recindex].rec_maxnum;

	if (len > recordsize)
	{
		return -4;			// 输入参数长度大于参数所能容纳的长度
	}
	else
	{
		recordsize	= len;
	}

	for(j = 0; j < 4; j++)	// 写入4次参数
	{
		recordheadpt	+= recordspace;

		if (0 == (recordheadpt & BLKMSK))	// 如果将写入的新纪录位置是4K块首地址则需要删除该块以写入新记录
		{
			if (recordheadpt == recordstart + recordtotal)	// 如果已经到了记录区尾部，则重新指向开始位置
			{
				recordheadpt = recordstart;
			}
			// 删除一块4K块
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
					return -5;		// 写Flash出错
				}
				if (0 != spi_flash_erasesector(recordheadpt))
				{
					return -5;		// 写Flash出错
				}
				rcvflag	= REC_FMTOVOP;
				if (0 != spi_flash_waddr(tmpadd, 1, &rcvflag))
				{
					return -5;		// 写Flash出错
				}
			}
		}

		// 写参数步骤
		// 1.计算校验码
		chkcode	= param[0];
		for (i = 1; i < recordsize; i++)
		{
			chkcode	^= param[i];
		}
		// 2.写记录
		rcvflag	= REC_INVALID;
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -5;		// 写Flash出错
		}
		if (0 != spi_flash_waddr(recordheadpt+2, recordsize, param))
		{
			return -5;		// 写Flash出错
		}
		rcvflag	= REC_VALID;
		if (0 != spi_flash_waddr(recordheadpt, 1, &rcvflag))
		{
			return -5;		// 写Flash出错
		}
		// 3.写校验码
		if (0 != spi_flash_waddr(recordheadpt+1, 1, &chkcode))
		{
			return -5;		// 写Flash出错
		}

		recinfo[recindex].rec_hdpt	= recordheadpt;		// 更新Head指针

		// 如果当前记录数达到最大记录数值则删除最老记录，否则当前记录加1
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
				return -5;		// 写Flash出错
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
 * @brief  获取参数区数据信息recinfo[]内容,并执行断电恢复操作
 * @param  None
 * @retval    0: 初始化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 没有找到参数区
 *           -4: 读RIT出错，文件系统错误
 *           -5: 写Flash出错
 *           -6: 记录的长度与条数与原来格式化的不匹配
 */

int param_init(unsigned int paramsize)
{
	return record_init(PARBLK,paramsize,8);
}
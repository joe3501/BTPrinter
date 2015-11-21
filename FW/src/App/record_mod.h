/**
***************************************************************************
* @file  record_mod.h
* @brief spi-flash文件系统头文件
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

#ifndef _RECORD_MOD_H_
#define _RECORD_MOD_H_

/**
*	资源类型
*/
//#define	RECORD_DEBUG 
#define PARBLK						0			//参数区
#define REC1BLK						1		//记录区类型1
#define REC2BLK						2		//记录区类型2
#define REC3BLK						3		//记录区类型3
#define REC4BLK						4		//记录区类型4
#define REC5BLK						5		//记录区类型5
#define REC6BLK						6		//记录区类型6


typedef struct
{
	unsigned int	recblk_startadd;	// 记录区起始地址
	unsigned int	recblk_totsize;		// 记录区总大小
	unsigned int	recblk_datadd;		// 记录区数据起始地址
	unsigned int	recblk_datsize;		// 一条记录的字节数
	unsigned int	recblk_maxcap;		// 可存放最大的记录数
	unsigned int	recblk_datspac;		// 每条记录所占空间数
	unsigned int	recblk_datotal;		// 记录区数据存储区总大小
	unsigned int	recblk_datcnt;		// 当前有效记录数
	unsigned int	recblk_hdpt;		// 头指针
	unsigned char	recblk_eccen;		// ECC功能
}tRECBLKInfo;

/**
* @brief  获取记录区参数
* @param  rectype: 记录类型
* @param  recblkinfo: 记录信息结构体指针
* @retval    0:获取成功
*           -1:不支持此类型记录
*           -2:Flash尚未初始化
*           -3:文件系统尚未初始化
*/
int get_recordinfo(unsigned char rectype, tRECBLKInfo *recblkinfo);


/**
 * @brief  获取记录区数据信息recinfo[]内容,并执行断电恢复操作
 * @param  rectype: 参数区类型
 *         recordsize: 每条记录的大小，关闭ECC时最大支持2046，开启ECC时支持最大2023字节
 *         number: 记录块可存放记录的个数
 * @retval    0: 初始化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 没有找到记录区
 *           -4: 读RIT出错，文件系统错误
 *           -5: 写Flash出错
 *           -6: 记录的长度与条数与原来格式化的不匹配
 */
int record_init(unsigned char rectype, unsigned int recordsize, unsigned int number);

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
int record_format(unsigned char recopt, unsigned int recordsize, unsigned int number);

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
int record_read(unsigned char rectype, int index, unsigned char *record, unsigned int len);

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
int record_modify(unsigned char rectype, int index, unsigned char mrkflag);


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
int record_replace(unsigned char rectype, int index, unsigned char *pNewData);
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
int record_write(unsigned char rectype, unsigned char *record, unsigned int len);

/**
 * @brief  获取数据区有效记录条数
 * @param    rectype: 记录区类型
 * @retval  0..count: 记录条数
 *                -1: 不支持此类型记录
 *                -2: Flash尚未初始化
 *                -3: 文件系统尚未初始化
 */
int record_count(unsigned char rectype);

//直接从内存中返回记录数
int record_count_ext(unsigned char rectype);

/**
* @brief  删除指定记录区的所有记录
* @param  rectype: 记录区类型
* @retval    0: 删除成功
*           -1: 不支持此类型记录
*           -2: Flash尚未初始化
*           -3: 文件系统尚未初始化
*           -4: 写Flash出错
*/
int record_delall(unsigned char rectype);


/**
 * @brief  获取参数区数据信息recinfo[]内容,并执行断电恢复操作
 * @param  recordsize: 参数区的大小
 * @retval    0: 初始化成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 没有找到参数区
 *           -4: 读RIT出错，文件系统错误
 *           -5: 写Flash出错
 *           -6: 记录的长度与条数与原来格式化的不匹配
 */
int param_init(unsigned int paramsize);


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
int param_format(unsigned int paramsize);


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
int param_read(unsigned char *param, unsigned int len);


/**
 * @brief  写入参数
 * @param  *param: 存放的地址
 * @param     len: 写入长度
 * @retval    0: 写入成功
 *           -1: 不支持此类型记录
 *           -2: Flash尚未初始化
 *           -3: 文件系统尚未初始化
 *           -4: 输入参数长度大于参数所能容纳的长度
 *           -5: 写Flash出错
 */
int param_write(unsigned char *param, unsigned int len);

#endif /* _RECORD_MOD_H_ */
/**
 * @file  Terminal_Para.c
 * @brief T1Gen项目的终端参数管理模块
 * @version 1.0
 * @author joe
 * @date 2011年03月30日
 * @note
*/  
/* Private include -----------------------------------------------------------*/
#include "Terminal_Para.h"
#include <string.h>
#include "crc32.h"
#include "record_mod.h"



/* private Variable -----------------------------------------------*/
/* external Variable -----------------------------------------------*/

/* Global Variable -----------------------------------------------*/
//定义终端的系统参数
TTerminalPara		g_param;				//终端参数

static unsigned char param_mod_state = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private function  -----------------------------------------------*/

/**
* @brief	从保存终端系统参数的存储模块中读取参数保存到全局变量中
* @param[in]  none				
* @param[out] 存储终端参数的全局变量
* @return     unsigned char  0  :SUCCESS   else : 错误代码
* @note                  
*/
int ReadTerminalPara(void)
{
	unsigned long			checkvalue;
	int		ret;

	if (param_mod_state == 0)
	{
		ret = param_init(sizeof(TTerminalPara));
		if (ret)
		{
			if (ret == -3 || ret == -4 || ret == -6)
			{
				ret = param_format(sizeof(TTerminalPara));
				if (ret)
				{
					return ret;
				}
			}
			else
			{
				return ret;
			}
		}

		param_mod_state = 1;
	}

	ret = param_read((unsigned char*)&g_param,sizeof(TTerminalPara));
	if(ret)
	{
		return ret;
	}



#if 1
	//计算校验值是否正确
	checkvalue = crc32(0,(unsigned char*)&g_param.line_after_mark,sizeof(TTerminalPara) - 4);
	if (g_param.checkvalue != checkvalue)
	{
		//参数的校验值不对
		return 2;
	}

	// 检查参数是否正确
	if ((g_param.endtag[0] != 0x55)||(g_param.endtag[1] != 0xAA)||(g_param.endtag[2] != 0x5A)||(g_param.struct_ver != 1))
	{
		//参数的结束标记不对
		return 3;
	}


	//检查其余参数是否正确
	//@todo....
#endif
	return 0;	
}

/**
* @brief	将用户变更后的终端系统参数保存到存储模块
* @param[in]   none				
* @param[in] 存储终端参数的全局变量
* @return     none
* @note   此函数中实现的是将参数保存到PSAM卡中                
*/
int SaveTerminalPara(void)
{
	int					ret;

	// 重新计算校验        
	g_param.checkvalue = crc32(0,(unsigned char*)&g_param.line_after_mark, sizeof(TTerminalPara)-4);

	ret = param_write((unsigned char*)&g_param,sizeof(TTerminalPara));
	return ret;
}


/**
* @brief 建立默认的测试参数并保存
*/
int DefaultTerminalPara(void)
{
	unsigned char		*pSrc;
	//unsigned char		i,tmp[2];

	pSrc				= (unsigned char *)&g_param;
	//先清空所有参数
	memset(pSrc,0,sizeof(TTerminalPara));
	//设置结束标记
	g_param.endtag[0]		= 0x55;
	g_param.endtag[1]		= 0xAA;
	g_param.endtag[2]		= 0x5A;
	//设置参数结构版本号
	g_param.struct_ver		= 0x01;
	//构造默认的终端参数
	g_param.character_code_page = 0;
	g_param.max_mark_length	=	250;
	g_param.line_after_mark	=	32;   //32*0.0625=1mm
	return SaveTerminalPara();
}


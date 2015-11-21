#include "basic_fun.h"
#include "string.h"
#include "usb_app_config.h"

#if(USB_DEVICE_CONFIG &_USE_USB_KEYBOARD_DEVICE)
extern  unsigned int	keyboard_output_report_value;
typedef struct {
	unsigned char	speical_key;
	unsigned char	keyvalue;
	unsigned char	asciivalue;
	unsigned char   rfu;
} TUSBKeyValue_Ascii;

/**
*	@brief USB键值与ascii码对应表
*/
const TUSBKeyValue_Ascii UsbKeyValue_Ascii_Tbl[] = 
{
	{0,40,'\x0d',0},
	{0,4,'a',0},{2,4,'A',0},{0,5,'b',0},{2,5,'B',0},{0,6,'c',0},{2,6,'C',0},{0,7,'d',0},{2,7,'D',0},{0,8,'e',0},{2,8,'E',0},
	{0,9,'f',0},{2,9,'F',0},{0,10,'g',0},{2,10,'G',0},{0,11,'h',0},{2,11,'H',0},{0,12,'i',0},{2,12,'I',0},{0,13,'j',0},{2,13,'J',0},
	{0,14,'k',0},{2,14,'K',0},{0,15,'l',0},{2,15,'L',0},{0,16,'m',0},{2,16,'M',0},{0,17,'n',0},{2,17,'N',0},{0,18,'o',0},{2,18,'O',0},
	{0,19,'p',0},{2,19,'P',0},{0,20,'q',0},{2,20,'Q',0},{0,21,'r',0},{2,21,'R',0},{0,22,'s',0},{2,22,'S',0},{0,23,'t',0},{2,23,'T',0},
	{0,24,'u',0},{2,24,'U',0},{0,25,'v',0},{2,25,'V',0},{0,26,'w',0},{2,26,'W',0},{0,27,'x',0},{2,27,'X',0},{0,28,'y',0},{2,28,'Y',0},
	{0,29,'z',0},{2,29,'Z',0},{0,30,'1',0},{2,30,'!',0},{0,31,'2',0},{2,31,'@',0},{0,32,'3',0},{2,32,'#',0},{0,33,'4',0},{2,33,'$',0},
	{0,34,'5',0},{2,34,'%',0},{0,35,'6',0},{2,35,'^',0},{0,36,'7',0},{2,36,'&',0},{0,37,'8',0},{2,37,'*',0},{0,38,'9',0},{2,38,'(',0},
	{0,39,'0',0},{2,39,')',0},{0,44,' ',0},{0,45,'-',0},{2,45,'_',0},{0,46,'=',0},{2,46,'+',0},{0,47,'[',0},{2,47,'{',0},{0,48,']',0},
	{2,48,'}',0},{0,49,'\\',0},{2,49,'|',0},{0,51,';',0},{2,51,':',0},{0,52,'\'',0},{2,52,'"',0},{0,53,'`',0},{2,53,'~',0},{0,54,',',0},
	{2,54,'<',0},{0,55,'.',0},{2,55,'>',0},{0,56,'/',0},{2,56,'?',0}
};
#endif

/**
* @brief 将整型转成ascii
* @param[in] unsigned int int_value: 被转换的整数
* @param[out] unsigned char *pValue; 转换成的ascii字符串
* @param[out] unsigned int *plen: 整数对应的ascii字符串的有效长度
*/
void itoascii(unsigned int int_value, unsigned char *pValue, unsigned int *plen)
{
	unsigned int			i;
	unsigned int			temp_value = int_value;

	if (int_value == 0)
	{
		pValue[0] = '0';
		*plen = 1;
		return;
	}
	i = 0;
	while (int_value)
	{
		i++;
		int_value				/= 10;
	}
	*plen	= i;

	while(i)
	{
		i--;
		pValue[i]			= temp_value % 10 + '0';
		temp_value			/= 10;
	}
}

/**
* @brief 将整型转成 规定长度的 ascii, 如59转换成3字节的Ascii，就是"059"
* @param[in] unsigned int int_value: 被转换的整数
* @param[in] unsigned char type: 0 十进制; 1: 十六进制
* @param[out] unsigned char *pValue; 转换成的ascii字符串
* @param[out] unsigned int *plen: 整数对应的ascii字符串的有效长度
*/
#if 0
void int_to_ascii(unsigned int int_value, unsigned char *pValue, unsigned int len, unsigned char type)
{
	unsigned int			i,j;
	unsigned int			temp_value = int_value;

	i = 0;
	while (int_value)
	{
		i++;
		if (type == 0)
		{
			int_value				/= 10;
		}
		else
		{
			int_value				/= 16;
		}
		
	}
	
	j = 0;
	if (i < len)	//前面补0
	{
		for (j = 0; j < len-i; j++)
		{
			pValue[j] = '0';
		}
	}

	while(len > j)
	{
		len--;
		if (type == 0)
		{
			pValue[len]			= temp_value % 10 + '0';
			temp_value			/= 10;
		}
		else
		{
			pValue[len]			= temp_value % 16;
			if (pValue[len] < 0x0A)
			{
				pValue[len]		+= '0';
			}
			else
			{
				pValue[len]		= pValue[len] - 0x0A + 'A';
			}
			temp_value			/= 16;
		}
	}
}
#endif


/**
* @brief 将16进制数值转换为字符串
* @param[in]	DWORD	hex_val			需要转换的DWROD值
* @param[in]	BYTE	type			进制		eg:  10:10进制   0x10:16进制
* @param[in]	BYTE	outlen			需要转换的字符串长度	如果指定的长度超过了转换出来之后的长度，那么前面补‘0’，如果指定的长度小于转换之后的长度，那么按实际长度输出	
* @param[out]	BYTE *outstr			转换结果存放的地址
* @return BYTE	转换之后的长度
* @note 
*/
unsigned char hex_to_str(unsigned int hex_val, unsigned char type, unsigned char outlen, unsigned char *outstr)
{
	unsigned char	offset;
	unsigned char	i, j;
	unsigned int	q;			//因子

	if (0 == hex_val)
	{
		outstr[0] = '0';
		outstr[1] = 0;
		return 1;
	}

	offset = 0;
	while(hex_val > 0)
	{
		q = hex_val%type;
		hex_val /= type;
		outstr[offset++] = HexToAscii(q);
	}

	while(offset < outlen)
	{
		outstr[offset++] = 0x30;
	}
	outstr[offset] = 0;


	i = 0;
	j = offset-1;
	while(i < j)
	{
		q = outstr[i];
		outstr[i] = outstr[j];
		outstr[j] = q;
		i++;
		j--;
	}

	return offset;
}
/**
***************************************************************************
*@brief	int型数据转换成BCD码
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
int HexToBCD(unsigned int indata,unsigned char *outdata,unsigned char out_len)
{
	unsigned char i;
	unsigned char	tmp;
	if (out_len > 5)		//因为int值最多也只能转换为5个字节的BCD码
	{
		return -1;
	}
	for (i = 0;i < out_len;i++)
	{
		tmp = indata%100;
		outdata[out_len - 1 - i] = (tmp/10 << 4) + (tmp%10);
		indata /= 100;
	}
	return 0;

}

/**
***********************************************************************
* @brief 将输入的BCD码转换为十进制
* @param[in] unsigned char *indata  要转换的输入值
* @param[in] unsigned char len      要转换的输入值的长度
* @param[in] unsigned char *outdata  转换的结果存放的地址
* @return 0: 转换OK  -1:转换失败，输入的数据不是BCD码
* @note 高位在前,输入的数据不能太长，否则会造成int溢出
***********************************************************************
*/
int BCDToHex(unsigned char* indata,unsigned char len,unsigned int *outdata)
{
	unsigned int hex;
	unsigned char i;

	if (len > 5)
	{
		//溢出
		return -1;
	}
	hex = 0;
	for (i = 0;i < len;)
	{
		if (((indata[i]&0xf0) >> 4) > 0x09)
		{
			return -1;		//不是BCD码表示的数据
		}

		if ((indata[i]&0x0f) > 0x09)
		{
			return -1;	//不是BCD码表示的数据
		}

		hex += ((indata[i]&0xf0) >> 4)*10 + (indata[i]&0x0f);

		if (++i != len)
		{
			hex *= 100;
		}
	}
	*outdata = hex;
	return 0;
}

/**
* @brief BCD数据域转换为字符串
* @param[in]	unsigned char* bcd_field	需要处理的BCD数据域的指针
* @param[in]	unsigned int len			此数据域的规定长度		
* @param[out]	unsigned char *out			转换结果存放的地址
* @return		转换后的字符串长度
* @note 
*/
unsigned int bcd_field_to_str(unsigned char *bcd_field,unsigned int len,unsigned char *out)
{
	unsigned int i,j = 0;
	/*
	for (i = 0; i < len; i++)
	{
	if (bcd_field[i] != 0)
	{
	break;
	}
	}
	*/
	i = 0;
	for (;i < len;i++)
	{
		out[j++] = HexToAscii((bcd_field[i] & 0xf0) >> 4);
		out[j++] = HexToAscii(bcd_field[i] & 0x0f);
	}
	out[j] = 0;		//字符串结束符
	return j;
}

/**
* @brief 将字符串转换为前补0的BCD数据域
* @param[in]	unsigned char* str			需要处理的字符串的指针
* @param[in]	unsigned int len			此BCD field的规定长度		
* @param[out]	unsigned char *bcd_field	转换结果存放的BCD field
* @return		
* @note 
*/
void str_to_bcd_field(unsigned char *str,unsigned char *bcd_field,unsigned int len)
{
	unsigned int	i,j = 0;
	unsigned int	str_len;

	str_len = strlen((char const*)str);
	if (str_len < len*2)
	{
		i = len - ((str_len+1)/2);
		memset(bcd_field,0,i);
		if (str_len%2 != 0 )
		{
			//奇数长度
			bcd_field[i++] = Ascii_To_Hex(str[0]);
			j = 1;
		}
		for (;j < str_len;j += 2)
		{
			bcd_field[i++] = (Ascii_To_Hex(str[j]) << 4) + Ascii_To_Hex(str[j + 1]);
		}
	}
	else
	{
		for (i=0;i<len;i++)
		{
			bcd_field[i] = (Ascii_To_Hex(str[2*i]) << 4) + Ascii_To_Hex(str[2*i + 1]);
		}
	}

	return ;
}

/**
* @brief 将输入的Hex值转换为其ASCII值
* @param[in] BYTE hex  要转换的输入值
* @return ASCII
* @note 只对0，1，...，F 这16个Hex值进行转换,其中a,b,c,d,e,f这几个值转换为其大写字母的ASCII值
*/
unsigned char HexToAscii(unsigned char hex)
{
	if(hex<0x0a)
		return (0x30+hex);
	else
		return (0x37+hex);
}

/**
* @brief 将由ASCII码表示的数据转换为其对应的Hex值
* @author joe
* @param[in] BYTE  asciiValue
* @return 转换的Hex值
* @note 
*/
unsigned char Ascii_To_Hex(unsigned char  asciiValue)
{
	if((asciiValue>=0x30)&&(asciiValue<=0x39))
	{
		return (asciiValue-0x30);
	}
	else if((asciiValue>=0x41)&&(asciiValue<=0x46))
	{
		return (asciiValue-0x37);
	}
	else if((asciiValue>=0x61)&&(asciiValue<=0x66))
	{
		return (asciiValue-0x57);
	}
	return asciiValue;
}

/**
* @brief 将Ascii转换成int
* @note: 没有做溢出处理
*/
int ascii_to_int(unsigned char *pValue)
{
	int						i;
	int						value = 0;

	for(i=0; ;i++)
	{
		if( *pValue == 0x00 )
			return value;
		value				*= 10;
		value				+= (*pValue) - '0';
		pValue ++;
	}
	///return value;
}

/**
 * @brief 二分查找算法
 * @return -1:没找到； 其他: 数组下标
 */
#if 0
int binary_search(unsigned short *search_list, unsigned int len, unsigned short key)
{
	unsigned int low = 0, high = len-1, mid = 0;

	if (search_list[low] == key)
	{
		return 0;
	}

	while (low <= high)
	{
		mid = low + (high-low)/2; //使用(low+high)/2会有整数溢出的情况
		if (search_list[mid] == key)
		{
			return mid;
		}
		else if (search_list[mid] > key)
		{
			high = mid -1;
		}
		else
		{
			low = mid + 1;
		}
	}

	return -1;
}

/*
 *@brief: 将ascii转化为HID 按键编码
 *@note: ascii 0-9 -> 0x30-0x39; A-Z -> 0x41-0x5a; a-z -> 0x61->0x7a
 *		 keyboard code 1-9 -> 0x1E-0x26 0->0x27; a(A)->z(Z) 0x04-0x1D 
 */

void ascii_to_keycode(unsigned char *pAscii, unsigned char *pKeycode, unsigned int len)
{
	int i = 0;
	unsigned char cur_value;

	for (i = 0; i < len; i++)
	{
		cur_value	= pAscii[i];
		if (cur_value == '0')
		{
			pKeycode[i] = 0x27;
		}
		else if ('0' < cur_value && cur_value <= '9')
		{
			pKeycode[i] = cur_value - 19;
		}
		else if ('A' <= cur_value && cur_value <= 'Z')
		{
			pKeycode[i] = cur_value - 61;
		}
		else if ('a' <= cur_value && cur_value <= 'z')
		{
			pKeycode[i] = cur_value - 93;
		}
		else if (cur_value == ' ' || cur_value == ':')
		{
			pKeycode[i] = 0x2C;
		}
		else
		{
			pKeycode[i] = 0;
		}
	}
}
#endif

#if(USB_DEVICE_CONFIG &_USE_USB_KEYBOARD_DEVICE)
/*
*@brief: 将ascii字符转换为符合键盘输入报表描述符的数据格式的键值 
*/
void ascii_to_keyreport(unsigned char ascii, unsigned char*report)
{
	int i;

	report[0] = 0x00;
	report[1] = 0x00;
	report[2] = 0x00;

	for(i = 0; i < sizeof(UsbKeyValue_Ascii_Tbl)/sizeof(TUSBKeyValue_Ascii); i++)
	{
		if (((ascii >= 'a')&&(ascii <= 'z'))||((ascii >= 'A')&&(ascii <= 'Z')))
		{
			if (ascii == UsbKeyValue_Ascii_Tbl[i].asciivalue)
			{
				if(keyboard_output_report_value & 0x02)
				{
					//表示系统当前已经按下了CapsLcok键，所以发送小写字母时反而需要按住shift
					report[0] = ((UsbKeyValue_Ascii_Tbl[i].speical_key == 0)?0x02:0x00);
				}
				else
				{
					report[0] = UsbKeyValue_Ascii_Tbl[i].speical_key;
				}
				report[2] = UsbKeyValue_Ascii_Tbl[i].keyvalue;
				return;
			}
		}
		else
		{
			if (ascii == UsbKeyValue_Ascii_Tbl[i].asciivalue)
			{
				report[0] = UsbKeyValue_Ascii_Tbl[i].speical_key;
				report[2] = UsbKeyValue_Ascii_Tbl[i].keyvalue;
				return;
			}
		}

	}
	return;
}


/*
*@brief: 将ascii字符转换为符合键盘输入报表描述符的数据格式的键值
*@note: ascii 0-9 -> 0x30-0x39; A-Z -> 0x41-0x5a; a-z -> 0x61->0x7a
*		 keyboard code 1-9 -> 0x1E-0x26 0->0x27; a(A)->z(Z) 0x04-0x1D 
*/
void ascii_to_keyreport2(unsigned char ascii,unsigned char*report)
{
	int i;

	memset(report,0,9);

	for(i = 0; i < sizeof(UsbKeyValue_Ascii_Tbl)/sizeof(TUSBKeyValue_Ascii); i++)
	{
		if (ascii == UsbKeyValue_Ascii_Tbl[i].asciivalue)
		{
			report[1] = UsbKeyValue_Ascii_Tbl[i].speical_key;
			report[3] = UsbKeyValue_Ascii_Tbl[i].keyvalue;
			return;
		}
	}
	return;
}

/*
*@brief: 将ascii字符转换为符合键盘输入报表描述符的数据格式的键值,同时发送多个键值
*@note: ascii 0-9 -> 0x30-0x39; A-Z -> 0x41-0x5a; a-z -> 0x61->0x7a
*		 keyboard code 1-9 -> 0x1E-0x26 0->0x27; a(A)->z(Z) 0x04-0x1D 
*     同时发送多个键值时只能保证所有的键值要么都需要按抓shift键发送要么都不需要按住shift键发送
*     所以如果一次需要构造多个键值时,此函数碰到需要切换shift键时将会返回实际此报表中发送的键值数
*	  最多一次可以发送6个键值
*/
unsigned char ascii_to_keyreport2_ext(unsigned char *ascii,unsigned char len,unsigned char*report)
{
	int i,j;
	unsigned char special_key;
	unsigned char last_ascii = 0;

	memset(report,0,9);

	//if(len > 6) len = 6;		//一次传6个键值，发现手机操作系统会丢键值
	if(len > 1) len = 1;


	for(j = 0;j < len;j++)
	{
		if (last_ascii == ascii[j])
		{
			return j;
		}

		last_ascii = ascii[j];
		for(i = 0; i < sizeof(UsbKeyValue_Ascii_Tbl)/sizeof(TUSBKeyValue_Ascii); i++)
		{
			if (ascii[j] == UsbKeyValue_Ascii_Tbl[i].asciivalue)
			{
				if(j == 0)
				{
					special_key = UsbKeyValue_Ascii_Tbl[i].speical_key;
					report[0] = special_key;
				}
				else
				{
					if(UsbKeyValue_Ascii_Tbl[i].speical_key != special_key)
					{
						return j;
					}
				}
				report[2+j] = UsbKeyValue_Ascii_Tbl[i].keyvalue;
				break;
			}
		}
	}

	return j;
}
#endif

/*
*@brief: 构造时间显示字符串
*@note:  mode:1  构造成形如2010-03-05 11:12:30
*		 mode:0  构造成形如2010/03/05 11:12:30
*/
void build_time_dis_str(unsigned char *date_time_bcd,unsigned char *dis_str,unsigned char mode)
{
	unsigned char tmp[15];

	bcd_field_to_str(date_time_bcd,7,tmp);
	memcpy(dis_str,tmp,4);
	if (mode)
	{
		dis_str[4] ='-';
	}
	else
	{
		dis_str[4] ='/';
	}
	memcpy(dis_str+5,tmp+4,2);
	if (mode)
	{
		dis_str[7] ='-';
	}
	else
	{
		dis_str[7] ='/';
	}
	memcpy(dis_str+8,tmp+6,2);
	dis_str[10]= ' ';
	memcpy(dis_str+11,tmp+8,2);
	dis_str[13] = ':';
	memcpy(dis_str+14,tmp+10,2);
	dis_str[16] = ':';
	memcpy(dis_str+17,tmp+12,2);
	dis_str[19] = 0;
}


//判断最后一个字节是否是一个汉字的前半字节
int if_last_char_valid(unsigned char *buf,unsigned int len)
{
	unsigned char *p = buf;
	unsigned int	i = 0;

	while (i < len)
	{
		if (*p < 0x80)
		{
			p++;
			i++;
		}
		else
		{
			p+=2;
			i +=2;
		}
	}

	if (i == len)
	{
		return 0;	
	}

	return 1;		//最后一个字节是一个汉字的前半个字节
}
int	if_include_unicode_character(unsigned char *buf,unsigned int len)
{
	unsigned char *p = buf;
	unsigned int	i = 0;

	while (i < len)
	{
		if (*p > 0x80)
		{
			return 1;
		}
                i++;
	}
	return 0;
}
/*
* @brief: 浮点数转换为字符串
* @param[in] float d	
* @param[in] char* str
*/
char *F2S(float d, char* str)  
{  
	char str1[40];  
	int i,j=0;  
	int index=0;  
	int dotPos = 0;  
	unsigned long num;  
	//  正负号   
	if ( d < 0 )  
	{  
		str[index++] = '-';  
		d = 0 - d;  
	}  
	//  判断小数点位置   
	//  156.89      --> 0.15689      dotPos = 3   
	//  1.5689      --> 0.15689      dotPos = 1   
	//  0.0015689   --> 0.15689      dotPos = -2   
	//  0.0000000015689 -->  0.15689 dotPos = -8   
	//  0.0000000000015689  --> 0.015689 dotPos = -10   
	for (i=0; i<10; i++)  
	{  
		if (d >= 1)  
		{  
			d = d / 10;  
			dotPos += 1; 
		}  
		else if (d < 0.1)  
		{  
			d = d * 10;  
			dotPos -= 1;  
		}  
		else  
			break;  
	}  

	// 取七位有效数字，末位四舍五入   
	//  0.15689  --> 1568900   
	//  0.015689 --> 0156890   
	for (i=0; i<9; i++)  
		d *= 10;  
	num = (unsigned long)(d + 0.5);  

	while ( num>0 )  
	{  
		str1[j++] = num%10+'0';     // 0098651      098651   
		num /= 10;  
	}  

	if (dotPos < 1)  
	{  
		str[index++] = '0';  
		str[index++] = '.';  
		for (i=0; i<0-dotPos; i++)  
			str[index++] = '0';  
		for (i=0; i<j; i++)  
			str[index++] = str1[j-1-i];       
	}  
	else  
	{  
		for (i=0; i<dotPos; i++)  
			str[index++] = str1[j-1-i];  
		str[index++] = '.';  
		for (; i<j; i++)  
			str[index++] = str1[j-1-i];  
	}     

	// 清零       123.0000 --> 123.   
	while ( str[--index]=='0' );  
	index++;  
	// 清除最后面的一个小数点 '.'      123. --> 123   
	while ( str[--index]=='.' );  


	str[++index] = '\0';  
	return(str);  
}  

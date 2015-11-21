#include"includes.h"

#define  HUNDRED_MS   (uint32_t)(100e-3 * SYSPCLK)
#define TEN_SEC       (uint32_t)(10*SYSPCLK)


PARA_T para;
uint16_t Flash_ID;
uint32_t bitmap_addr;
uint32_t jedec_id=0;
static uint32_t config_addr;
#if defined(CODEPAGE)
uint32_t font_en_addr;
#endif

extern void config_init(void)
{
        uint32_t i;
        uint16_t cksum;
        uint16_t buf;

        F25L_ReadId((uint8_t *)(&buf));
        if(buf == 0xffff)
        {
            F25L_ReadJedecId((uint8_t *)(&jedec_id));
           Flash_ID = (uint16_t)(jedec_id >>8);
        }
        else
        {
            Flash_ID = buf;
        }
//----chang
#if defined(CODEPAGE)
    font_en_addr = 0x2400;
#endif

        switch(buf)
        {
        case 0x14C2:    // MX25L1606
        case 0x1402:    // S25FL016A
        case 0x148c:    // F25L016A
        default:
            config_addr = 0x200000-0x1000;
            bitmap_addr = 0x200000-0x14000;
            break;
        case 0x128c:    // F25L004
        case 0x4400:    // SST25LF040
            config_addr = 0x80000-0x1000;
            bitmap_addr = 0x80000-0x14000;
            break;
        case 0x48bf:    // SST25VF512
            config_addr = 0x10000-0x1000;
            bitmap_addr = 0x10000-0x4000;
            break;
        case 0x141c:
            config_addr = 0x200000-0x1000;
            bitmap_addr = 0x200000-0x14000;
            break;
        }
        F25L_Read(config_addr, (uint8_t *)&para, sizeof(para));
        for(i=0, cksum=0; i<(sizeof(para)-2); i++)
		{
			cksum += ((uint8_t *)&para)[i];
		}
       if((para.id_head == 0x12345678) && (cksum == para.cksum))
	   {

       }
       else
       {
          para.id_head = 0x12345678;
          para.com_baud = 0;		// 9600
          para.print_voltage= 0;
          para.character_code_page = 0;
          para.max_mark_length=250;
          para.line_after_mark=32;   //32*0.0625=1mm
       }
       if(para.character_code_page >= 0x30)
       {
          para.character_code_page = 0;
       }


}
extern uint32_t config_idx2baud(uint8_t idx)
{
	switch(idx)
	{
	case 0:
	default:
		return 9600L;
	case 1:
		return 19200L;
	case 2:
		return 38400L;
	case 3:
		return 57600L;
	case 4:
		return 115200L;
    case 5:
        return 4800L;
	}
}

#if 0
extern void config_mode(void)
{
    PARA_T para_b;
	uint8_t i,ch;
	uint16_t pt, cksum;
    uint32_t start,timeout;
  //   UARTInit(9600);
	start = timeout = getSysTICs();
   	pt = 0;
    while(1)
    {

      if(pt && (getElapsedSysTICs(start) > HUNDRED_MS))
      {
          pt = 0;
      }
      if(getElapsedSysTICs(timeout) > (TEN_SEC*3))    // 30 seconds
      {
//        printf("超时退出!\n");
        break;
      }
      if(PrintBufGetchar(&ch))
     {
     ((uint8_t *)&para_b)[pt++] = ch;// Getchar();

      if(pt >= (sizeof(para_b)))
      {
    	for(i=0, cksum=0; i<(sizeof(para_b)-2); i++)
    	{
    		cksum += ((uint8_t *)&para_b)[i];
    	}
    	if((para_b.id_head == 0x12345678) && (cksum == para_b.cksum))
    	{
            F25L_SectorErase(config_addr >> 12);
            if(Flash_ID== 0x148c)
            {
                F25L_AAIWordProgram(config_addr, (uint8_t *)&para_b, sizeof(para_b)/2);
            }
            else
            {
    		    F16_PageProgram(config_addr, (uint8_t *)&para_b, sizeof(para_b));
    		}
    		break;
        }
    	pt = 0;
      }
      start = getSysTICs();	// reset timer
    }


    }

    UARTInit(config_idx2baud(para.com_baud));
//    printf("参数下载完成!\n");

    }
#endif
#if defined(FONT_DOWNLOAD)
FONT_T font_b;
uint8_t font_download=0;
extern void font_download_mode(void)
{
	FONT_ACK_T Ack;
	//uint32_t jedec_id;
	uint8_t ch;
	uint16_t pt,ptt;//, cksum;
	//uint32_t start, timeout;
	uint32_t i,dl_addr,last_pack;
	UARTInit(115200);
	dl_addr = 0;
	pt=ptt=0;
	last_pack = 0;
	//start = timeout = getSysTICs();
	Ack.font_head = 0xaabbccdd;
	Ack.ack = 0;
	Ack.cksum = 0xffff;
	F25L_ReadId((uint8_t *)(&Flash_ID));

	last_pack = 0;
	dl_addr = 0;
	F25L_ChipErase();

	while(1)
	{
		if(PrintBufGetchar(&ch))
		{
			((uint8_t *)&font_b)[ptt++] = ch;
			pt++;
			if(ptt>=(10+Font_Size))
			{
				if(font_b.len > 0)
				{
					if(Flash_ID== 0x148C)
					{
						F25L_AAIWordProgram(dl_addr, font_b.font, sizeof(font_b.font)/2);
					}
					else	//(flashid == 0x141c) || (flashid == 0x1402) || flashid == 0x14C2
					{
						 F16_PageProgram(dl_addr,&font_b.font[0],Font_Size);
					}
				}
				dl_addr += sizeof(font_b.font);
				ptt=10;
				//Putchar('0');
			}
			if(pt>=(12+1024-2))
			{
				//PrintBufGetchar(&ch);
				Getchar();
				pt++;
				//PrintBufGetchar(&ch);
				Getchar();
				pt++;
			}
			if(pt >=(12+1024))
			{
				if((font_b.font_head == 0xaabbccdd))
				{
					//timeout = getSysTICs();
					if(font_b.pack == 0)
					{
						last_pack = font_b.pack;
						/*dl_addr = 0;
						F25L_ChipErase();*/
					}
					else if (font_b.pack == (last_pack + 1))    //正常包
					{

						last_pack = font_b.pack;
						//dl_addr += sizeof(font_b.font);
					}
					else if (font_b.pack == last_pack)          //retry包
					{
						Ack.pack = font_b.pack;
						UARTSend(((uint8_t *)&Ack),sizeof(Ack));
					}
					else    //出错，退出
					{
						break;
					}
					Ack.pack = font_b.pack;
					for(i=0; i<sizeof(Ack); i++)
						Putchar(((uint8_t *)&Ack)[i]);

					if(font_b.len < 1024)    //有效数据小于256则认为是最后一个包.
					{
						break;
					}
				}
				pt=0;
				ptt=0;
			}
			//start = getSysTICs();   // reset timer
		}
	}
	while ( !(LPC_UART->LSR & LSR_THRE) );
    UARTInit(config_idx2baud(para.com_baud));
    font_download=0;
}
#endif
extern void EraseConfig(void)
{
    F25L_SectorErase(config_addr >> 12);
}
extern void UpdateParaSector(PARA_T * para_b)
{

	uint16_t i, cksum;
    for(i=0, cksum=0; i<(sizeof(PARA_T)-2); i++)
	{
		cksum += ((uint8_t *)para_b)[i];
	}
    para_b->cksum = cksum;

    F25L_SectorErase(config_addr >> 12);

    if(Flash_ID == 0x148C)
    {
	    F25L_AAIWordProgram(config_addr, (uint8_t *)para_b, sizeof(PARA_T)/2);
	}
    else //if((flashid == 0x141c) || (flashid == 0x1402)||(flashid == 0x14C2))
    {
        F16_PageProgram(config_addr, (uint8_t *)para_b, sizeof(PARA_T));
    }
}




#include "Type.h"
#include "ThermalDetect.h"
#include "PaperDetect.h"

#ifdef DEBUG_VER
extern unsigned char debug_buffer[];
extern unsigned int debug_cnt;
#endif

#define TEMP_SNS_VALUE		((AD_Value[0][TEMP_SNS_OFFSET]+AD_Value[1][TEMP_SNS_OFFSET]+AD_Value[2][TEMP_SNS_OFFSET]+AD_Value[3][TEMP_SNS_OFFSET]\
	+AD_Value[4][TEMP_SNS_OFFSET]+AD_Value[5][TEMP_SNS_OFFSET]+AD_Value[6][TEMP_SNS_OFFSET]+AD_Value[7][TEMP_SNS_OFFSET]\
	+AD_Value[8][TEMP_SNS_OFFSET]+AD_Value[9][TEMP_SNS_OFFSET])/10)

/*
函数    把读取到的AD值，根据上拉或下拉电阻计算对应内部的电阻值
输入    ad:     读取到的ad值,单位:100欧姆
        uRes:   外部上拉电阻值
        dRes:   外部下拉电阻值
*/
uint32_t TranVtoR(uint32_t ad,uint32_t adMax,uint32_t uRes,uint32_t dRes)
{
    if (uRes)
    {   // 上拉电阻
        if (ad >= adMax)
            return (ad*uRes);
        else
            return ((ad*uRes)/(adMax-ad));
    }
    else
    {   // 下拉电阻
        if (ad == 0) ad = 1;
        return ((adMax-ad)*dRes/ad);
    }
}

/*
函数    把热敏机芯的热敏电阻值转换为对应温度
输入    res     热敏电阻值，单位:100欧姆
*/
const uint16_t restbl[25] = {8430,6230,4660,3520,2690,2080,
                        1610,1240, 968, 757, 595, 471,
                         375, 300, 242, 196, 159, 131,
                         108,  89,  74,  62,  52,  44,37};
int16_t TranRtoDegree(uint32_t res)
{
    uint16_t i;
    int16_t degree;

    // 热敏电阻对应温度表 -40 --> 75,间隔5度
    i = 0;
    while(i < 24)
    {
        if (res >= restbl[i+1]) break;
        i++;
    }
    if (i >= 24)
    {   // 超过80度
        degree = ((restbl[24]-res)*5)/(restbl[23]-restbl[24]) + 80;
    }
    else
    {
        degree = 5*(i+1) - ((res-restbl[i+1])*5)/(restbl[i]-restbl[i+1]) - 40;
    }

#ifdef DEBUG_VER
	//debug_buffer[debug_cnt] = degree;
	//debug_cnt++;
#endif

    return (degree);
}

int16_t  TPHTemperature(void)
{
    return TranRtoDegree(TranVtoR(TEMP_SNS_VALUE,0xfff,300,0));
}


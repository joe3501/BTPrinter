/**
***************************************************************************
* @file  rec_ecc.h
* @brief ECC�㷨ͷ�ļ�
***************************************************************************
*
* @version V0.0.1
* @author jiangcx
* @date 2010��09��06��
* @note 
*
***************************************************************************
*
* @copy
*
* �˴���Ϊ���ڽ������������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ���
* ���ƴ����������ڱ���˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2010 netcom</center></h1>
*/

/**
 * calculate_ecc - [NAND Interface] Calculate 3-byte ECC for 256/512-byte
 *			 block
 * @buf:	input buffer with raw data
 * @eccsize:	data bytes per ecc step (256 or 512)
 * @code:	output buffer with ECC
 */
void calculate_ecc(const unsigned char *buf, unsigned int eccsize, unsigned char *code);

/**
 * correct_data - [NAND Interface] Detect and correct bit error(s)
 * @buf:	raw data read from the chip
 * @read_ecc:	ECC from the chip
 * @calc_ecc:	the ECC calculated from raw data
 * @eccsize:	data bytes per ecc step (256 or 512)
 *
 * Detect and correct a 1 bit error for eccsize byte block
 */
int correct_data(unsigned char *buf,
				 unsigned char *read_ecc,
				 unsigned char *calc_ecc,
				 unsigned int   eccsize);
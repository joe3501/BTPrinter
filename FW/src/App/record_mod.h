/**
***************************************************************************
* @file  record_mod.h
* @brief spi-flash�ļ�ϵͳͷ�ļ�
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

#ifndef _RECORD_MOD_H_
#define _RECORD_MOD_H_

/**
*	��Դ����
*/
//#define	RECORD_DEBUG 
#define PARBLK						0			//������
#define REC1BLK						1		//��¼������1
#define REC2BLK						2		//��¼������2
#define REC3BLK						3		//��¼������3
#define REC4BLK						4		//��¼������4
#define REC5BLK						5		//��¼������5
#define REC6BLK						6		//��¼������6


typedef struct
{
	unsigned int	recblk_startadd;	// ��¼����ʼ��ַ
	unsigned int	recblk_totsize;		// ��¼���ܴ�С
	unsigned int	recblk_datadd;		// ��¼��������ʼ��ַ
	unsigned int	recblk_datsize;		// һ����¼���ֽ���
	unsigned int	recblk_maxcap;		// �ɴ�����ļ�¼��
	unsigned int	recblk_datspac;		// ÿ����¼��ռ�ռ���
	unsigned int	recblk_datotal;		// ��¼�����ݴ洢���ܴ�С
	unsigned int	recblk_datcnt;		// ��ǰ��Ч��¼��
	unsigned int	recblk_hdpt;		// ͷָ��
	unsigned char	recblk_eccen;		// ECC����
}tRECBLKInfo;

/**
* @brief  ��ȡ��¼������
* @param  rectype: ��¼����
* @param  recblkinfo: ��¼��Ϣ�ṹ��ָ��
* @retval    0:��ȡ�ɹ�
*           -1:��֧�ִ����ͼ�¼
*           -2:Flash��δ��ʼ��
*           -3:�ļ�ϵͳ��δ��ʼ��
*/
int get_recordinfo(unsigned char rectype, tRECBLKInfo *recblkinfo);


/**
 * @brief  ��ȡ��¼��������Ϣrecinfo[]����,��ִ�жϵ�ָ�����
 * @param  rectype: ����������
 *         recordsize: ÿ����¼�Ĵ�С���ر�ECCʱ���֧��2046������ECCʱ֧�����2023�ֽ�
 *         number: ��¼��ɴ�ż�¼�ĸ���
 * @retval    0: ��ʼ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: û���ҵ���¼��
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: дFlash����
 *           -6: ��¼�ĳ�����������ԭ����ʽ���Ĳ�ƥ��
 */
int record_init(unsigned char rectype, unsigned int recordsize, unsigned int number);

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
int record_format(unsigned char recopt, unsigned int recordsize, unsigned int number);

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
int record_read(unsigned char rectype, int index, unsigned char *record, unsigned int len);

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
int record_modify(unsigned char rectype, int index, unsigned char mrkflag);


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
int record_replace(unsigned char rectype, int index, unsigned char *pNewData);
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
int record_write(unsigned char rectype, unsigned char *record, unsigned int len);

/**
 * @brief  ��ȡ��������Ч��¼����
 * @param    rectype: ��¼������
 * @retval  0..count: ��¼����
 *                -1: ��֧�ִ����ͼ�¼
 *                -2: Flash��δ��ʼ��
 *                -3: �ļ�ϵͳ��δ��ʼ��
 */
int record_count(unsigned char rectype);

//ֱ�Ӵ��ڴ��з��ؼ�¼��
int record_count_ext(unsigned char rectype);

/**
* @brief  ɾ��ָ����¼�������м�¼
* @param  rectype: ��¼������
* @retval    0: ɾ���ɹ�
*           -1: ��֧�ִ����ͼ�¼
*           -2: Flash��δ��ʼ��
*           -3: �ļ�ϵͳ��δ��ʼ��
*           -4: дFlash����
*/
int record_delall(unsigned char rectype);


/**
 * @brief  ��ȡ������������Ϣrecinfo[]����,��ִ�жϵ�ָ�����
 * @param  recordsize: �������Ĵ�С
 * @retval    0: ��ʼ���ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: û���ҵ�������
 *           -4: ��RIT�����ļ�ϵͳ����
 *           -5: дFlash����
 *           -6: ��¼�ĳ�����������ԭ����ʽ���Ĳ�ƥ��
 */
int param_init(unsigned int paramsize);


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
int param_format(unsigned int paramsize);


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
int param_read(unsigned char *param, unsigned int len);


/**
 * @brief  д�����
 * @param  *param: ��ŵĵ�ַ
 * @param     len: д�볤��
 * @retval    0: д��ɹ�
 *           -1: ��֧�ִ����ͼ�¼
 *           -2: Flash��δ��ʼ��
 *           -3: �ļ�ϵͳ��δ��ʼ��
 *           -4: ����������ȴ��ڲ����������ɵĳ���
 *           -5: дFlash����
 */
int param_write(unsigned char *param, unsigned int len);

#endif /* _RECORD_MOD_H_ */
/*******************************************************************************
 *   Filename:       bsp_wdt.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    ���Ź�����ģ��ͷ�ļ�
 *
 *
 *   Notes:
 *   �������Ź�����ԭ���ڼ�ֵ�Ĵ�����IWDG_KR����д��0XCCCC����ʼ���ö������Ź���
 *   ��ʱ��������ʼ���临λֵOXFFF�ݼ���������������������ĩβ0X000��ʱ�򣬻����
 *   һ����λ�źţ�IWDG_RESET�������ۺ�ʱ��ֻҪ�Ĵ���IWDG_KR�б�д��0XAAAA��IWDG_RLR
 *   �е�ֵ�ͻᱻ���¼��ص��������дӶ�����������Ź���λ��
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/

#ifndef	__BSP_WDT_H__
#define	__BSP_WDT_H__

/*******************************************************************************
 * INCLUDES
 */
#include  <global.h>

/*******************************************************************************
 * CONSTANTS
 */
#define BSP_WDT_MODE_NONE   0       //��ʹ�ÿ��Ź�
#define BSP_WDT_MODE_EXT    1       //��ʹ���ⲿ���Ź�
#define BSP_WDT_MODE_INT    2       //��ʹ���ڲ����Ź�
#define BSP_WDT_MODE_ALL    3       //�ⲿ���ڲ����Ź�ͬʱʹ��


#define WDI_GPIO_PIN        GPIO_Pin_8             /* PC.00 */
#define WDI_GPIO_PORT       GPIOA
#define WDI_GPIO_RCC        RCC_APB2Periph_GPIOA


/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern INT8U BSP_WdtMode;          // 0:��ֹ��1���ⲿ���Ź� 2���ڲ����Ź���3��ͬʱʹ���ڲ����ⲿ���Ź�


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
uint8_t   BSP_WDT_Init      (uint8_t mode);
void      BSP_WDT_Rst       (void);
uint8_t   BSP_WDT_GetMode   (void);


#endif
/*******************************************************************************
 *                        end of file                                          *
 *******************************************************************************/
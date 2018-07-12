/*******************************************************************************
 *   Filename:       bsp_wdt.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    ���Ź�����ģ��
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

/*******************************************************************************
* INCLUDES
*/
#include <includes.h>
#include <bsp_wdt.h>

#define BSP_WDT_MODULE_EN 1
#if BSP_WDT_MODULE_EN > 0

// 0:��ֹ��1���ⲿ���Ź���2���ڲ����Ź���3��ͬʱʹ���ڲ����ⲿ���Ź�;
INT8U BSP_WdtMode  = 0;

/*******************************************************************************
* ��    �ƣ� BSP_WdtRst
* ��    �ܣ� ι��
* ��ڲ����� ��
* ���ڲ����� ��
* ���� ���ߣ� ������
* �������ڣ� 2014-08-18
* ��    �ģ�
* �޸����ڣ�
* ��    ע��
*******************************************************************************/
void  BSP_WDT_Rst(void)
{
    if ( BSP_WdtMode == BSP_WDT_MODE_NONE )
        return;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif
    /***********************************************
    * ������STM�ڲ����Ź�ι�� Reload IWDG counter
    */
    if ( ( BSP_WdtMode == BSP_WDT_MODE_INT ) || ( BSP_WdtMode == BSP_WDT_MODE_ALL ) ) {
        IWDG_ReloadCounter();
    }
//    /***********************************************
//    * �������ⲿ���Ź�ι��
//    */
//    if ( ( BSP_WdtMode == BSP_WDT_MODE_EXT ) || ( BSP_WdtMode == BSP_WDT_MODE_ALL ) ) {
//        CPU_SR_ALLOC();
//        OS_CRITICAL_ENTER();
//        GPIO_SetBits(WDI_GPIO_PORT, WDI_GPIO_PIN); 
//        Delay_Nus(5);
//        GPIO_ResetBits(WDI_GPIO_PORT, WDI_GPIO_PIN);
//        OS_CRITICAL_EXIT();
//    }
}

/*******************************************************************************
* ��    �ƣ� BSP_WDT_GetMode
* ��    �ܣ� ��ȡ����ʲô���Ź�
* ��ڲ����� ��
* ���ڲ����� ��
* ���� ���ߣ� ������
* �������ڣ� 2014-08-18
* ��    �ģ�
* �޸����ڣ�
* ��    ע�� ���ڲ��ģ��ⲿ�ģ������������ã���
*******************************************************************************/
uint8_t  BSP_WDT_GetMode(void)
{
    return BSP_WdtMode;
}

/*******************************************************************************
* ��    �ƣ� BSP_WDT_Init
* ��    �ܣ� �������Ź���ʼ��
* ��ڲ����� 0:��ֹ��1���ⲿ���Ź���2���ڲ����Ź���3��ͬʱʹ���ڲ����ⲿ���Ź�;
* ���ڲ����� 0����ʼ���ɹ� 1����ʼ��ʧ��
* ���� ���ߣ� ������
* �������ڣ� 2014-08-18
* ��    �ģ�
* �޸����ڣ�
* ��    ע��
*******************************************************************************/
uint8_t BSP_WDT_Init(uint8_t mode)
{
    BSP_WdtMode = mode;

    if ( mode == BSP_WDT_MODE_NONE )  //��ֹ��
        return 0;

    if ( ( mode == BSP_WDT_MODE_INT ) || ( mode == BSP_WDT_MODE_ALL ) ) { //ʹ���ڲ��������ڲ��ⲿ��һ����

        /* Check if the system has resumed from IWDG reset */
        if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {
            /* Clear reset flags */
            RCC_ClearFlag();
        } else {
            /* IWDGRST flag is not set */
        }

        //2.�������Ź�(IWDG)��ר�õ�40kHz �ĵ���ʱ��Ϊ��������ˣ���ʹ��ʱ�ӷ���
        //������Ҳ��Ȼ��Ч�����ڿ��Ź��ɴ�APB1 ʱ�ӷ�Ƶ��õ���ʱ��������ͨ����
        //���õ�ʱ�䴰�������Ӧ�ó���������Ĺ��ٻ�������Ϊ����ͨ��
        //IWDG_SetPrescaler(IWDG_Prescaler_32); ����ʱ�ӽ��з�Ƶ��4-256��
        //ͨ�����·�ʽι�� ��
        ///* Reload IWDG counter */
        //IWDG_ReloadCounter();
        //3. 0.625KHz ��ÿ���� Ϊ1.6ms
        //����ʱ 1000 �����ڣ���1000*1.6ms=1.6s
        //���Ź���ʱʱ��= IWDG_SetReload������ֵ / ���Ź�ʱ��Ƶ��
        //���Ź�ʱ��Ƶ��=LSI���ڲ�����ʱ�ӣ���Ƶ�ʣ�40KHz��/ ��Ƶ��

        RCC_LSICmd(ENABLE);                              //��LSI
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);

        /* Enable write access to IWDG_PR and IWDG_RLR registers */
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
        /* IWDG counter clock: 40KHz(LSI) / 64 * 2  = 0.625 / 2  KHz */
        IWDG_SetPrescaler(IWDG_Prescaler_128);
        /* 3.2s ,max 0xFFF  0~4095  */
        IWDG_SetReload(3000);
        IWDG_ReloadCounter();
        IWDG_Enable();
    }
//    if ( ( mode == BSP_WDT_MODE_EXT ) || ( mode == BSP_WDT_MODE_ALL ) ) { //ʹ���⹷�����ڹ��⹷һ����
//        GPIO_InitTypeDef  gpio_init;
//        RCC_APB2PeriphClockCmd(WDI_GPIO_RCC, ENABLE);
//
//        gpio_init.GPIO_Pin   = WDI_GPIO_PIN;
//        gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
//        gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
//
//        GPIO_Init(WDI_GPIO_PORT, &gpio_init);
//    }

    return 1;
}

/*******************************************************************************
*              end of file                                                    *
*******************************************************************************/
#endif
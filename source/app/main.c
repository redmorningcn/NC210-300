#include <includes.h>
#include <bsp_adc.h>
#include <bsp_boardID.h>
#include <bsp_wave_detect.h>
#include <app_wave_task.h>
#include <app.h>
#include <bsp.h>
#include <bsp_dac.h>
#include <bsp_flash.h>
#include <bsp_wdt.h>



/*******************************************************************************
* Description  :  ָʾ���������ٶ��ź����������ٶ��źſ��������ڵ��ã�100ms��
* Author       : 2018/4/13 ������, by redmorningcn
*******************************************************************************/
void    led_task(void)
{
    static  uint8   blinkcnt= 0;
    
    blinkcnt++;
    
    if(sCtrl.ch.para[0].period == 0 && sCtrl.ch.para[1].period  == 0)   //����������   
    {
        blinkcnt %= 20;                                     //����ͨ�����źţ�����  
    }
    else
    {
        blinkcnt %= 2;                                      //����ͨ�����źţ�����
    }
    
    if(blinkcnt == 0)                                       //�������
    {
        BSP_LED_On(1);
    }
    else
    {
        BSP_LED_Off(1);
    }
}

//�ο���ѹ����λmv
#define     FULL_VOLTAGE            (3300)
#define     STANDARD_VOLTAGE        (FULL_VOLTAGE/4)
#define     MAX_HIG_VOLTAGE         (3000)
#define     MAX_STANDARD_VOLTAGE    (1500)        

/*******************************************************************************
* Description  : ���òο���ѹ�����ڵ��ã�100ms��
                �ο���ѹΪ�ߵ�ƽ��0.09����ȡ10%��ѹ��0.9�������ߵ�ƽ��������10%λ��
                ����5�Σ��ο���ѹ��ֵƫ�����10%�����µ����ο���ѹ��
* Author       : 2018/4/16 ����һ, by redmorningcn
*******************************************************************************/
void    set_dac_task(void)
{
    static  uint32  sum;
    static  uint8   diffcnt = 0;    
    
    uint8   i;
    for(i = 0;i< 2;i++)
    if( sCtrl.ch.para[i].freq )                                           //ͨ��0����ͨ��1���ٶ��ź�
    {
        if(fabs(sCtrl.ch.stand_vol -  sCtrl.ch.para[i].Voh * 0.9) > (FULL_VOLTAGE/100)*2 )    //��ѹƫ��С��10% ��
        {
            if(sCtrl.ch.para[i].Voh < MAX_HIG_VOLTAGE)                    //�ߵ�ƽ���3V
            {
                sum += sCtrl.ch.para[i].Voh;
                diffcnt++;
            }
            
            if( diffcnt > 10 )                                              //����5�Σ��ɼ��ĸߵ�ƽ��ѹ�����õ�ѹ
            {
                sCtrl.ch.stand_vol =((sum / diffcnt)*9)/10;
                
                if( sCtrl.ch.stand_vol > MAX_STANDARD_VOLTAGE  )          //�޶��Ƚ����Ĳο���ѹ��1.5V��0.66V֮��
                {
                    sCtrl.ch.stand_vol   = MAX_STANDARD_VOLTAGE;          //1.5v
                }
                else if(sCtrl.ch.stand_vol < STANDARD_VOLTAGE)
                {
                    sCtrl.ch.stand_vol  = STANDARD_VOLTAGE;               //0.8V
                }
                
                bsp_set_dacvalue(sCtrl.ch.stand_vol);                     //�������ñȽ�ֵ                      
            }
        }
        else
        {
            sum     = 0;
            diffcnt = 0;
        }
    }else{                                                              // (���û���źţ�����Ĭ��ֵ����)
        if( sCtrl.ch.stand_vol > MAX_STANDARD_VOLTAGE  )                //�޶��Ƚ����Ĳο���ѹ��1.5V��0.66V֮��
        {
            sCtrl.ch.stand_vol  = MAX_STANDARD_VOLTAGE;                 //1.5v
        }
        else if(sCtrl.ch.stand_vol < STANDARD_VOLTAGE)
        {
            sCtrl.ch.stand_vol  = STANDARD_VOLTAGE;                      //0.8V
        }
        
        bsp_set_dacvalue(sCtrl.ch.stand_vol);                           //�������ñȽ�ֵ                      
    }
}

/**************************************************************
* Description  : ��ʼ��ϵͳ����
* Author       : 2018/5/30 ������, by redmorningcn
*/
void    InitCtrl(void)
{
    sCtrl.sys.paraflg.sysflg    = 0;        //����ϵͳ������sys��
    sCtrl.sys.paraflg.califlg   = 0;        //��������������cali��
    
    /**************************************************************
    * Description  : ��ȡУ׼ֵ
    * Author       : 2018/5/30 ������, by redmorningcn
    */
    BSP_FlashReadBytes(STORE_ADDR_CALI,
                       (u8 *)&sCtrl.calitab,
                       sizeof(sCtrl.calitab));
    
    for(u8  i = 0;i <sizeof(sCtrl.calitab)/sizeof(strCalibration);i++){
        //�������Զȼ��
        if(     sCtrl.calitab.CaliBuf[i].line < CALI_LINE_MIN 
            ||  sCtrl.calitab.CaliBuf[i].line > CALI_LINE_MAX
            ){
                sCtrl.calitab.CaliBuf[i].line   = CALI_LINE_BASE; //���Զȳ��ޣ���Ĭ��ֵ
                sCtrl.calitab.CaliBuf[i].Delta  = CALI_DELTA_BASE;//����ƫ��ޣ���Ĭ��ֵ

            }
        
        //����ƫ����
        if(     sCtrl.calitab.CaliBuf[i].Delta < CALI_DELTA_MIN
            ||  sCtrl.calitab.CaliBuf[i].Delta > CALI_DELTA_MAX
            ){
                sCtrl.calitab.CaliBuf[i].Delta = CALI_DELTA_BASE;//����ƫ��ޣ���Ĭ��ֵ
            }
    }
}

/**************************************************************
* Description  : �������
* Author       : 2018/5/30 ������, by redmorningcn
*/
void    store_para(void)
{
    if(sCtrl.sys.paraflg.califlg == 1){
        sCtrl.sys.paraflg.califlg = 0; 
        
        BSP_FlashWriteBytes(STORE_ADDR_CALI,
                        (u8 *)&sCtrl.calitab,
                        sizeof(sCtrl.calitab));
    }
    
    if(sCtrl.sys.paraflg.sysflg == 1){
        sCtrl.sys.paraflg.sysflg = 0;
        
        BSP_FlashWriteBytes(STORE_ADDR_SYS,
                            (u8 *)&sCtrl.sys,
                            sizeof(sCtrl.sys));
    }
}
        
    
/*******************************************************************************
* Description  : ��������ʱ�䲻���ȵĹ����ڴ�����
* Author       : 2018/4/16 ����һ, by redmorningcn
*******************************************************************************/
void    idle_task(void)      
{
    static  uint32  tick;
    if(sCtrl.sys.time > tick+100 ||  sCtrl.sys.time < tick) //100ms
    {
        tick = sCtrl.sys.time;                  //ʱ��
        
        led_task();                             //ָʾ�ƿ���
        
        set_dac_task();                         //���òο���ѹ
        
        store_para();                           //�������
    }
}

extern  void mod_bus_rx_task(void);

void main (void)
{
	BSP_Init();                                                 /* Initialize BSP functions                             */
	CPU_TS_TmrInit();
	/***********************************************
	* ������ ��ʼ���δ�ʱ��������ʼ��ϵͳ����ʱ�ӡ�
	*/
	sCtrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //ʱ��Ƶ��               /* Determine SysTick reference freq.              */
    
    /*******************************************************************************
    * Description  : �źŷ�ֵ��������Դ��ѹ����ʼ����
    * Author       : 2018/4/12 ������, by redmorningcn
    *******************************************************************************/
	Bsp_ADC_Init();
    
    /*******************************************************************************
    * Description  : �豸ID�Ż�ȡ��ʼ��
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    Init_boardID();
    sCtrl.sys.id = get_boardID();
    
    /*******************************************************************************
    * Description  : �ٶ�ͨ��ʱ���������ʼ��
    ��ʱ������+ȫ�ֶ�ʱ��ʱ�䣬��¼���β����ĸ�ʱ��㡣
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    init_ch_timepara_detect();
    
    /*******************************************************************************
    * Description  : �Ƚ����ο���ѹ��ʼ����������ʼ��
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    BSP_dac_init();
    
    /**************************************************************
    * Description  : ���Ѵ������������
    * Author       : 2018/5/29 ���ڶ�, by redmorningcn
    */
    InitCtrl();
    
    /*******************************************************************************
    * Description  : ����ͨ�ų�ʼ��
    * Author       : 2018/5/7 ����һ, by redmorningcn
    *******************************************************************************/
    MB_Init(1000);      //��ʼ��modbusƵ��					

    sCtrl.pch         = MB_CfgCh( sCtrl.sys.id,        	// ... Modbus Node # for this slave channel
                        //MB_CfgCh( ModbusNode,        	// ... Modbus Node # for this slave channel
                        MODBUS_SLAVE,           // ... This is a MASTER
                        500,                    // ... 0 when a slave
                        MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                        0,                      // ... Specify UART #1
                        57600,                  // ... Baud Rate
                        USART_WordLength_8b,    // ... Number of data bits 7 or 8
                        USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                        USART_StopBits_1,       // ... Number of stop bits 1 or 2
                        MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes

    
    BSP_WDT_Init(BSP_WDT_MODE_INT);             // ��ʼ�����Ź�
                                  
    while(1)
    {
        /*******************************************************************************
        * Description  : �����ٶ�ͨ����ʱ�����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_timepara();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : �����ź��쳣����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_ch_judge();
        
        /*******************************************************************************
        * Description  : ��������ʱ�䲻���еĵĹ����ڴ����У�ÿ100ms����һ��
        * Author       : 2018/4/16 ����һ, by redmorningcn
        *******************************************************************************/
        idle_task();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : ͨѶ����
        * Author       : 2018/5/7 ����һ, by redmorningcn
        *******************************************************************************/
        mod_bus_rx_task();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        BSP_WDT_Rst();
    }
}


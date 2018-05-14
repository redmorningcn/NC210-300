/*******************************************************************************
* Description  : ���㲨�β���
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>


/*******************************************************************************
* Description  : ͨ��ʱ���������
                 ����ͨ��buf�Ķ�дָ�룬ȷ���Ƿ���Ҫ������
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
*******************************************************************************/
void    app_calc_ch_timepara(void)
{
    uint8   p_write;
    uint8   p_read;
    uint64  starttime;
    uint64  endtime;
    
    uint64  periodtime;
    uint64  ratiotime;
    
    uint8   i;
        
    for(i = 0;i< 2;i++)
    {
        p_write = sCtrl.ch.test[i].p_write;
        p_read  = sCtrl.ch.test[i].p_read;
        
        /*******************************************************************************
        * Description  : �ٶ�ͨ��ʱ���������
        * Author       : 2018/3/13 ���ڶ�, by redmorningcn
        *******************************************************************************/
        if(     ( p_write > p_read) &&  (p_write > p_read+10)           
           ||   ( p_write < p_read) &&  (p_write + CH_TIMEPARA_BUF_SIZE > p_read+10)           
               )  
        {

            /*******************************************************************************
            * Description  : ��������(0.01us) �����ź�����һ���ٴγ��֣�ȡlow_up�ж�Ϊ��׼
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            
            p_read      =   (sCtrl.ch.test[i].p_read + 1) % CH_TIMEPARA_BUF_SIZE;       //��ֹԽ��
            endtime     =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            
            if(starttime > endtime)             //��ֹ��ת
            {
                endtime += 65536;
            }
            periodtime = endtime - starttime;
            
            sCtrl.ch.para[i].period = (periodtime * 1000*1000*100 )/ sCtrl.sys.cpu_freq;
            
            if(periodtime){
                sCtrl.ch.para[i].freq = sCtrl.sys.cpu_freq  / periodtime;   //����Ƶ��
                
                if(((sCtrl.sys.cpu_freq *10) % periodtime)> 4 )       //��������
                    sCtrl.ch.para[i].freq += 1;
            }
            
            /*******************************************************************************
            * Description  : ����ռ�ձ�(xx.xx%)��( hig_down -  low_up ) / period
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].low_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_down_cnt  ;            
            if(starttime > endtime)             //��ֹ��ת
            {
                endtime += 65536;
            }
            ratiotime = endtime - starttime;
            
            if( periodtime )
                sCtrl.ch.para[i].ratio = ( ratiotime * 100* 100 ) / periodtime; 
            
            /*******************************************************************************
            * Description  : ���������أ�0.01us��
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_up_cnt  ;            
            if(starttime > endtime)             //��ֹ��ת,
            {
                endtime += 65536;
            }
            else
            {
                sCtrl.ch.para[i].raise = ( endtime - starttime) *1000*1000*100 / sCtrl.sys.cpu_freq;
            }
            
            /*******************************************************************************
            * Description  : �����½���(0.01us)
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].hig_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_down_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].low_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_down_cnt  ;            
            if(starttime > endtime)             //��ֹ��ת
            {
                endtime += 65536;
            }
            else
            {
                sCtrl.ch.para[i].fail = ( endtime - starttime)*1000*1000*100/ sCtrl.sys.cpu_freq;
            }

            /*******************************************************************************
            * Description  : ������λ��(xx.xx��)
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                uint16      ptmp[2];
                
                ptmp[0]    = sCtrl.ch.test[0].p_write;
                ptmp[1]    = sCtrl.ch.test[1].p_write;
                
                p_read      =   (ptmp[0] + CH_TIMEPARA_BUF_SIZE -1)%CH_TIMEPARA_BUF_SIZE ;
                starttime   =   sCtrl.ch.test[0].time[p_read].low_up_time  * 65536 
                            +   sCtrl.ch.test[0].time[p_read].low_up_cnt  ;
                
                p_read      =   (ptmp[1] + CH_TIMEPARA_BUF_SIZE -1)%CH_TIMEPARA_BUF_SIZE ;

                endtime     =   sCtrl.ch.test[1].time[p_read].low_up_time  * 65536 
                            +   sCtrl.ch.test[1].time[p_read].low_up_cnt  ;            
                if(starttime > endtime)             //��ֹ��ת (����������źŽ��м���)
                {
                    endtime += periodtime;          //��һ����ʱ��
                }
                
                //   phase/360 = difftime/peirod
                sCtrl.ch.ch1_2phase = 360*(endtime - starttime)*100 / periodtime; 
            }

            //��ָ��++
            sCtrl.ch.test[i].p_read++ ;
            sCtrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
            
            //ȡ�źŵĸߵ͵�ƽ
        }
    }
}


/*******************************************************************************
* Description  : ȡ�źŵ�ƽ���ֵ
* Author       : 2018/3/29 ������, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    uint8   i;
    static  uint8   lockflg[2]={0,0};   
    uint16  p_wr;
    uint16  p_rd;
    
    for(i = 0;i< 2;i++)
    {
        /*******************************************************************************
        * Description  : ������������λ�ã��ɼ��ߵ͵�ƽ
                        �ߵ�ƽ�������أ�90%��ɼ���
                        �͵�ƽ���½��أ�10%��ɼ���
                        �ڸߡ��͵�ƽ���ɼ�һ�κ�lockflgʵ�֣����Ž����������ݲɼ���
        * Author       : 2018/3/29 ������, by redmorningcn
        *******************************************************************************/
        p_wr = sCtrl.ch.test[i].p_wr_vol;

        
        if(( sCtrl.ch.test[i].station.fall_90)   && lockflg[i] == 0 )    
        {
            lockflg[i] = 1;                                     //�����ź�
           
            sCtrl.ch.test[i].voltage[p_wr].ch_low_voltage =   Get_ADC(ADC_Channel_10+i);
        }
        
        if((sCtrl.ch.test[i].station.raise_10) && lockflg[i]  )
        //if(sCtrl.ch.test[i].station.raise_10 && lockflg[i]  )
        {
            lockflg[i] = 0;                                    //�����ź�,����ȡ��͵�ƽ����ִ��
            
            sCtrl.ch.test[i].voltage[p_wr].ch_hig_voltage = Get_ADC(ADC_Channel_10+i);
            
            /*******************************************************************************
            * Description  : ȡ�����ƽ��дָ��++
            * Author       : 2018/3/29 ������, by redmorningcn
            *******************************************************************************/
            if(i == 1)  //ȡ�����ƽ
                sCtrl.ch.test[i].voltage[p_wr].vcc_hig_voltage = Get_ADC(ADC_Channel_10+2);
            
            p_wr++;
            
            sCtrl.ch.test[i].p_wr_vol = p_wr % CH_VOLTAGE_BUF_SIZE;
        }
        
        /*******************************************************************************
        * Description  : ����ߵ͵�ƽ
        * Author       : 2018/3/29 ������, by redmorningcn
        *******************************************************************************/
        p_wr = sCtrl.ch.test[i].p_wr_vol;
        p_rd = sCtrl.ch.test[i].p_rd_vol;

        if(     ( p_wr > p_rd) &&  (p_wr > p_rd+10)           
           ||   ( p_wr < p_rd) &&  (p_wr + CH_VOLTAGE_BUF_SIZE > p_rd+10)           
            )  
        {
            uint32  sum;
            uint16  max,min;
            uint8   tmp8;
            uint16  tmp16;
            
            /*******************************************************************************
            * Description  : ��10�����У���ȥ���ֵ����Сֵ����ȡƽ��
            * Author       : 2018/3/29 ������, by redmorningcn
            *******************************************************************************/
            //����͵�ƽ
            tmp8 = 0;
            sum  = 0;
            max  = sCtrl.ch.test[i].voltage[p_rd].ch_low_voltage;
            min  = max;
            for(tmp8 = 0;tmp8< 10;tmp8++)
            {
                tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].ch_low_voltage;
                if(tmp16 > max)
                    max = tmp16;
                
                if(tmp16 < min)
                    min = tmp16;
                
                sum += tmp16;
            }
            sCtrl.ch.para[i].Vol = (sum - max - min)/8;
                
            //����ߵ�ƽ
            tmp8 = 0;
            sum  = 0;
            max  = sCtrl.ch.test[i].voltage[p_rd].ch_hig_voltage;
            min  = max;
            for(tmp8 = 0;tmp8< 10;tmp8++)
            {
                tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].ch_hig_voltage;
                if(tmp16 > max)
                    max = tmp16;
                
                if(tmp16 < min)
                    min = tmp16;
                
                sum += tmp16;
            }
            sCtrl.ch.para[i].Voh = (sum - max - min)/8;
            
            
            //���㹩���Դ
            if(i == 1)
            {
                tmp8 = 0;
                sum  = 0;
                max  = sCtrl.ch.test[i].voltage[p_rd].vcc_hig_voltage;
                min  = max;
                for(tmp8 = 0;tmp8< 10;tmp8++)
                {
                    tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].vcc_hig_voltage;
                    if(tmp16 > max)
                        max = tmp16;
                    
                    if(tmp16 < min)
                        min = tmp16;
                    
                    sum += tmp16;
                }
                
                sCtrl.ch.vcc_vol = (sum - max - min)/8;
            }
            
            /*******************************************************************************
            * Description  : ������ָ��
            * Author       : 2018/3/29 ������, by redmorningcn
            *******************************************************************************/
            sCtrl.ch.test[i].p_rd_vol = (p_rd + tmp8) % CH_VOLTAGE_BUF_SIZE;
        }
    }
}


/*******************************************************************************
* Description  : �ٶ��źŵ��쳣�ж�
* Author       : 2018/4/13 ������, by redmorningcn
*******************************************************************************/
void    app_ch_judge(void)
{
    static  uint32  tick;
    static  uint32  plusecnt[2];
    static  uint32  errcnt[2];
    uint8   i;
    
    if(sCtrl.sys.time > tick + 300 || sCtrl.sys.time < tick)       //300ms�ж�һ��
    {
        tick = sCtrl.sys.time;
            
        for(i = 0; i < 2;i++)
        {
            if(sCtrl.ch.test[i].pulse_cnt > plusecnt[i])
            {
                errcnt[i] = 0;
                sCtrl.ch.para[i].status       &= ~(0x01);     //�����ź�����  
                
            }
            else
            {
                errcnt[i]++;
                if(errcnt[i] > 2)                           //�쳣������(�ñ�ʶλ���������㣬�������ź�)
                {
                    sCtrl.ch.para[i].fail     = 0;
                    sCtrl.ch.para[i].freq     = 0;
                    sCtrl.ch.para[i].period   = 0;
                    sCtrl.ch.para[i].raise    = 0;
                    sCtrl.ch.para[i].ratio    = 0;
                    sCtrl.ch.para[i].status  |= 0x01;         //�������ź�
                    sCtrl.ch.para[i].Voh      = 0;
                    sCtrl.ch.para[i].Vol      = 0;
                    
                    if(i == 1){
                        sCtrl.ch.ch1_2phase   = 0;
                        sCtrl.ch.vcc_vol      = 0;
                    }
                    
                }
            }
            plusecnt[i] = sCtrl.ch.test[i].pulse_cnt;
        }
    }
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
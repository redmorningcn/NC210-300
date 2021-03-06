/*******************************************************************************
* Description  : 计算波形参数
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>
#include <Algorithm.h>

//时间参数计算值过滤
#define     CH_PARA_BUF_SIZE             (10)     
//边沿时间补偿系数
#define     CH_EDGE_CALI                 (1.1)

typedef   struct   _strChTimeParaFliter_{
    uint32              period[CH_PARA_BUF_SIZE];                         //周期，  0.00-2000000.00us （0.5Hz）
    uint32              freq[CH_PARA_BUF_SIZE];                           //频率，  0-100000hz              
    uint32              raise[CH_PARA_BUF_SIZE];                          //上升沿，0.00-50.00us
    uint32              fail[CH_PARA_BUF_SIZE];                           //下降沿，0.00-50.00us
    uint32              ratio[CH_PARA_BUF_SIZE];                          //占空比，0.00-100.00%
    uint32              phase[CH_PARA_BUF_SIZE];
    int32               Acceleration[CH_PARA_BUF_SIZE];                   //加速度
}strChTimeParaFliter;

strChTimeParaFliter     lsChTimeFliterBuf[2];
u32                     timetmpbuf[CH_PARA_BUF_SIZE];





/*******************************************************************************
* Description  : 通道时间参数计算
                 根据通道buf的读写指针，确定是否需要进运算
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void    app_calc_ch_timepara(void)
{
    uint8   p_write;
    uint8   p_read;

    u16     p_next     ;
    u32     now_time   ;   
    u32     next_time  ;   
    u32     now_cnt    ;   
    u32     next_cnt   ;   
    
    s32     periodtime; 
    s32     ratiotime;
    s32     raisetime;
    s32     failtime;
    s32     phasetime;
    
    u32     freq;
    s32     Acceleration;
    
    u32     pri_period[2];          //计算丢脉冲使用，原始周期值
        
    static  u8      i = 0;
    static  u8      pparabuf[2] = {0,0};
    
    
    i++;
    i %= 2;
        
    //for(i = 0;i< 2;i++)
    {
        p_write = Ctrl.ch.test[i].p_write;
        p_read  = Ctrl.ch.test[i].p_read;
        
        /*******************************************************************************
        * Description  : 速度通道时间参数运算
        * Author       : 2018/3/13 星期二, by redmorningcn
        *******************************************************************************/
        if(     ( p_write > p_read) &&  (p_write > p_read+2)           
           ||   ( p_write < p_read) &&  (p_write + CH_TIMEPARA_BUF_SIZE > p_read+2)           
               )  
        {
            /*******************************************************************************
            * Description  : 计算周期(0.01us) 周期信号任意一点再次出现，取low_up中断为标准
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   Ctrl.ch.test[i].p_read;
            p_next      = (p_read + 1) % CH_TIMEPARA_BUF_SIZE;
            
            now_time    = Ctrl.ch.test[i].time[p_read].hig_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_next].hig_up_time;
            next_cnt    = Ctrl.ch.test[i].time[p_next].hig_up_cnt;
            
            if(now_time <= next_time)
            {
                periodtime = (next_time - now_time) * 65536 + next_cnt - now_cnt;               //redmonringcn 20180719 取消64位乘法运算
                
                while(periodtime < 0)
                    periodtime += 65536;
                
                   pri_period[i] =  (periodtime * 100 )/ (Ctrl.sys.cpu_freq / (1000*1000));//计算丢脉冲时使用。
                       
                   App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].period,CH_PARA_BUF_SIZE,periodtime);  //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    periodtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].period, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                
            }else   //时钟翻转，直接跳过
            {
                //读指正++
                Ctrl.ch.test[i].p_read++ ;
                Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
                //continue;           //跳过此处循环
                return;
            }
            
            Ctrl.ch.para[i].period = (periodtime * 100 )/ (Ctrl.sys.cpu_freq / (1000*1000));
            
            
            if(periodtime){
                //Ctrl.ch.para[i].freq = Ctrl.sys.cpu_freq  / periodtime;                       //计算频率
                
                freq = Ctrl.sys.cpu_freq  / periodtime;                                         //计算频率
                  
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].freq,CH_PARA_BUF_SIZE,freq);   //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据

                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    periodtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].period, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                
                Ctrl.ch.para[i].freq = freq;
                
                if(((Ctrl.sys.cpu_freq *10) % periodtime)> 4 )                     //四舍五入
                    Ctrl.ch.para[i].freq += 1;
            }
            
            
            /**************************************************************
            * Description  : 如果频率为0，后面计算，直接赋值0
            * Author       : 2018/7/18 星期三, by redmorningcn
            */
            if(Ctrl.ch.para[i].freq    == 0 || periodtime == 0){
                Ctrl.ch.para[i].ratio  = 0;
                Ctrl.ch.para[i].raise  = 0;
                Ctrl.ch.para[i].fail   = 0; 
                Ctrl.ch.ch1_2phase     = 0;
                
                //读指正++
                Ctrl.ch.test[i].p_read++ ;
                Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
                //continue;           //跳过此处循环
                return;
            }
            
            
            
            /**************************************************************
            * Description  : 计算加速度
            a = K * fm * （fm-fn）/(m-n)
            k = 200 / 3.14 /1.05 = 60
            * Author       : 2018/7/31 星期二, by redmorningcn
            */
            if(pparabuf[i] > CH_PARA_BUF_SIZE){
                s32 freqend,freqstart;
                
                static  u8  acceltimes = 0;
                
                freqend     = lsChTimeFliterBuf[i].freq[CH_PARA_BUF_SIZE - 1];
                freqstart   = lsChTimeFliterBuf[i].freq[0];
                Acceleration =  freqend * (freqend - freqstart) / (60*CH_PARA_BUF_SIZE);     //计算加速度
                
                u8  accelbufsize = CH_PARA_BUF_SIZE;
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].Acceleration,accelbufsize,Acceleration);   //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据

                if(acceltimes > accelbufsize){
                    acceltimes = 0;
                    s8  addtimes    = 0;
                    int accelsum    = 0;
                    
                    for(u8 j= 0;j < accelbufsize  ;j++ ){
                        
                        accelsum += lsChTimeFliterBuf[i].Acceleration[j];
                        if(lsChTimeFliterBuf[i].Acceleration[j] > 100 ){
                            addtimes++;
                        }else if(lsChTimeFliterBuf[i].Acceleration[j] < -100 ){
                            addtimes--;
                        }
                    }
                    
                    if(abs(addtimes) == accelbufsize ){                         //全正或全负
                        Ctrl.ch.para[i].status.acceleration = accelsum/(accelbufsize * 100 );         
                        
                    }else{
                        Ctrl.ch.para[i].status.acceleration = 0;
                    }
                }
                acceltimes++;
                    
            }
                
            
            
            
            /*******************************************************************************
            * Description  : 计算占空比(xx.xx%)，( hig_down -  low_up ) / period
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].hig_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].hig_down_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].hig_down_cnt;
            
            if(now_time <= next_time)
            {
                ratiotime = (next_time - now_time) * 65536 + next_cnt - now_cnt;    //redmonringcn 20180719 取消64位乘法运算
                
                while(ratiotime < 0)
                    ratiotime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].ratio,CH_PARA_BUF_SIZE,ratiotime);  //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据
                
                if(pparabuf[i] > 10)
                    ratiotime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].ratio, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                
                
                if(periodtime)
                    Ctrl.ch.para[i].ratio = ( (uint64  )ratiotime * 100 * 100 ) / (periodtime ) ;                 
            }
            
            /*******************************************************************************
            * Description  : 计算上升沿（0.01us）
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].low_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].low_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].hig_up_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            if(now_time <= next_time)
            {
                raisetime = (next_time - now_time) * 65536 + next_cnt - now_cnt;    //redmonringcn 20180719 取消64位乘法运算
                
                while(raisetime < 0)
                    raisetime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].raise,CH_PARA_BUF_SIZE,raisetime);  //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    raisetime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].raise, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                
                Ctrl.ch.para[i].raise = CH_EDGE_CALI*(raisetime *100) / (Ctrl.sys.cpu_freq / (1000*1000)); 

                if(Ctrl.ch.para[i].raise  > 1500)
                    Ctrl.ch.para[i].raise  = 1500;
            }
            
            /*******************************************************************************
            * Description  : 计算下降沿(0.01us)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].hig_down_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_down_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].low_down_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].low_down_cnt;
            
            if(now_time <= next_time)
            {
                failtime = (next_time - now_time) * 65536 + next_cnt - now_cnt; //redmonringcn 20180719 取消64位乘法运算
                
                while(failtime < 0)
                    failtime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].fail,CH_PARA_BUF_SIZE,failtime);  //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    failtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].fail, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                                
                Ctrl.ch.para[i].fail = CH_EDGE_CALI*(failtime *100) / (Ctrl.sys.cpu_freq / (1000*1000)); 

                if(Ctrl.ch.para[i].fail < 25 )  {                               //r补偿采样电路误差 180712 (减去固有误差)
                    Ctrl.ch.para[i].fail /=  5;
                }else if(Ctrl.ch.para[i].fail < 45){
                    Ctrl.ch.para[i].fail -= 25;
                    
                    Ctrl.ch.para[i].fail = 5 + (Ctrl.ch.para[i].fail / 4);
                }else{
                    Ctrl.ch.para[i].fail -= 45;
                    
                    if(Ctrl.ch.para[i].fail < ( 5 + 5 ))
                        Ctrl.ch.para[i].fail = 5 + 5;
                }
                
                if(Ctrl.ch.para[i].fail  > 1500)
                    Ctrl.ch.para[i].fail  = 1500;
            }

            /*******************************************************************************
            * Description  : 计算相位差(xx.xx°)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                if(Ctrl.ch.para[1].freq  && Ctrl.ch.para[0].freq ){
                    
                    u8  ptmp    =(Ctrl.ch.test[0].p_write + CH_TIMEPARA_BUF_SIZE -1) % CH_TIMEPARA_BUF_SIZE ;   //redmonringcn 20180719 相位差实时计算
                    now_time    = Ctrl.ch.test[0].time[ptmp].low_up_time;
                    now_cnt     = Ctrl.ch.test[0].time[ptmp].low_up_cnt;
                    
                    ptmp        =(Ctrl.ch.test[1].p_write + CH_TIMEPARA_BUF_SIZE -1) % CH_TIMEPARA_BUF_SIZE ;
                    next_time   = Ctrl.ch.test[1].time[ptmp].low_up_time;
                    next_cnt    = Ctrl.ch.test[1].time[ptmp].low_up_cnt;
                    
                    phasetime = (next_time - now_time) * 65536 + next_cnt - now_cnt;    //redmonringcn 20180719 取消64位乘法运算
                    
                    while(phasetime < 0)
                        phasetime += periodtime;
                    
                    App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].phase,CH_PARA_BUF_SIZE,phasetime);  //将数据写入buf，保留最近CH_PARA_BUF_SIZE的数据
                    
                    if(pparabuf[i] > CH_PARA_BUF_SIZE)
                        phasetime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].phase, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //数据过滤
                    
                    if(periodtime)
                        Ctrl.ch.ch1_2phase = ((uint64)phasetime * 360 * 100) / periodtime ; 

                    if( Ctrl.ch.ch1_2phase   >  360*100)
                         Ctrl.ch.ch1_2phase   =  360*100;
                }
            }

            /**************************************************************
            * Description  : 传感器丢脉冲判断
            判断方法：对比前后脉冲周期，如果周期偏差大于%50，认为丢脉冲。
            * Author       : 2018/5/30 星期三, by redmorningcn
            */
            static  u32     plusetimes[2]   ={0,0};         //正常脉冲计算器。从产生错误开始计数
            static  u8      perioderrcnt[2] ={0,0};        //周期错误计数器。每产生一次错误加1，一圈内周期都正常，清除。
            static  u32     lastperiod[2]   ={0,0};         //上次周期
            
            //前后周期对比
            if(pri_period[i] * (Ctrl.sys.periodcali / 100) < lastperiod[i] ){
                plusetimes[i] = 0;
                perioderrcnt[i]++;
                
                if(perioderrcnt[i] > (Ctrl.sys.loseerrtimes)){
                    Ctrl.ch.para[i].status.lose = 1;   
                }
            }else{
                plusetimes[i]++;                        
                
                if(plusetimes[i] > CIRCLE_PLUSE_NUM*(1.05) )     // 如果产生的错误，经过1圈后，未在发生，则认为是误判。
                {
                    perioderrcnt[i] = 0;                        // 错误计数器清零
                    Ctrl.ch.para[i].status.lose = 0;   
                }
            }
            lastperiod[i] = pri_period[i];                      // 下次判断使用
            
            //读指正++
            Ctrl.ch.test[i].p_read++ ;
            Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
            
            //取信号的高低电平
            
            if(pparabuf[i] < CH_PARA_BUF_SIZE +5)
                pparabuf[i]++;
        }
    }
}

#define     CALC_NUM_VOL  (10)


/*******************************************************************************
* Description  : 取信号电平检测值
* Author       : 2018/3/29 星期四, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    static  uint8    i;
    static  uint8   lockflg[2]={0,0};   
    uint16   p_wr;
    uint16   p_rd;
    static  u32 systime     = 0;
    static  u8  nocalatimes = 0; 
    
    uint8    calcnum         = CALC_NUM_VOL;    //滤波常数
    uint8    divisor         = 3;
    uint16               pin[2] ={GPIO_Pin_7,GPIO_Pin_9};
    GPIO_TypeDef *       port[2]={GPIOC     ,GPIOC}; 
    
    /**************************************************************
    * Description  : 通道间循环执行
    * Author       : 2018/7/19 星期四, by redmorningcn
    */
    i++;
    i %=2;
    //for(i = 0;i< 2;i++)
    {
        /*******************************************************************************
        * Description  : 根据脉冲所在位置，采集高低电平
                        高电平：上升沿，90%后采集；
                        低电平：下降沿，10%后采集；
                        在高、低电平均采集一次后（lockflg实现），才进行下组数据采集。
        * Author       : 2018/3/29 星期四, by redmorningcn
        *******************************************************************************/
        p_wr = Ctrl.ch.test[i].p_wr_vol;

        
        if(( Ctrl.ch.test[i].station.fall_90)   && lockflg[i] == 0 )    
        {
            if(GPIO_ReadInputDataBit(port[i],pin[i]) == RESET){       //确定是在高位
                lockflg[i] = 1;                                      //互锁信号
                
                Ctrl.ch.test[i].voltage[p_wr].ch_low_voltage =   Get_ADC(ADC_Channel_10+i);
            }
        }
        
        if((Ctrl.ch.test[i].station.raise_10) && lockflg[i]  )
        //if(Ctrl.ch.test[i].station.raise_10 && lockflg[i]  )
        {
            if(GPIO_ReadInputDataBit(port[i],pin[i]) == SET){      //确定是在低位
                
                lockflg[i] = 0;                                       //互锁信号,必须取完低电平才能执行
                
                Ctrl.ch.test[i].voltage[p_wr].ch_hig_voltage = Get_ADC(ADC_Channel_10+i);
                
                /*******************************************************************************
                * Description  : 取供电电平及写指针++
                * Author       : 2018/3/29 星期四, by redmorningcn
                *******************************************************************************/
                if(i == 1)  //取供电电平
                    Ctrl.ch.test[i].voltage[p_wr].vcc_hig_voltage = Get_ADC(ADC_Channel_10+2);
                
                p_wr++;
                
                Ctrl.ch.test[i].p_wr_vol = p_wr % CH_VOLTAGE_BUF_SIZE;
            }
        }
        
        /*******************************************************************************
        * Description  : 计算高低电平
        * Author       : 2018/3/29 星期四, by redmorningcn
        *******************************************************************************/
        p_wr = Ctrl.ch.test[i].p_wr_vol;
        p_rd = Ctrl.ch.test[i].p_rd_vol;

        if(     ( p_wr > p_rd) &&  (p_wr > p_rd+ calcnum)           
           ||   ( p_wr < p_rd) &&  (p_wr + CH_VOLTAGE_BUF_SIZE > p_rd+calcnum)           
            )  
        {
            int32       voh,vol,vcc;  
            
            nocalatimes     = 0;                                                //
            systime         = Ctrl.sys.time;
            
            /*******************************************************************************
            * Description  : 在10个数中，除去最大值、最小值，再取平均
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            //计算低电平
            u16   buftmp[CALC_NUM_VOL];
            
            for(u8 j=0;j < calcnum;j++){
                buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_low_voltage;
            }
            vol = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            
                
            //计算高电平
            for(u8 j=0;j < 10;j++){
                buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_hig_voltage;
            }
            voh = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            

            //计算供电电源
            if(i == 1)
            {
                //计算高电平
                for(u8 j=0;j < 10;j++){
                    buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].vcc_hig_voltage;
                }
                vcc = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
                
                Ctrl.ch.vcc_vol     = (vcc * Ctrl.calitab.VccVol.line / CALI_LINE_BASE)       + Ctrl.calitab.VccVol.Delta;
            }
            
            /**************************************************************
            * Description  : 计算值修正
            * Author       : 2018/5/30 星期三, by redmorningcn
            */
            Ctrl.ch.para[i].Vol = (vol * Ctrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + Ctrl.calitab.CaliBuf[i+1].Delta;
            Ctrl.ch.para[i].Voh = (voh * Ctrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + Ctrl.calitab.CaliBuf[i+1].Delta;
            
            /*******************************************************************************
            * Description  : 调整读指针
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            Ctrl.ch.test[i].p_rd_vol = (p_rd + calcnum) % CH_VOLTAGE_BUF_SIZE;
        }
    }
    
    /**************************************************************
    * Description  : 如果长时间未进行采集信号VCC，
    * Author       : 2018/5/30 星期三, by redmorningcn
    */
    if(Ctrl.sys.time  > systime + 1000){
        systime = Ctrl.sys.time;
        nocalatimes++;
        
        if(nocalatimes >20 ){                           //无信号，重新设置参考电压
            nocalatimes = 0;
            int32 vcc = Get_ADC(ADC_Channel_10+2);      //采集电压
            if(vcc < 1550)                              //小于15V
                Ctrl.ch.vcc_vol     = (vcc * Ctrl.calitab.VccVol.line / CALI_LINE_BASE) + Ctrl.calitab.VccVol.Delta;
            else
                Ctrl.ch.vcc_vol     = Ctrl.sys.ref_limitvol_min;    
        }
    }
}


/*******************************************************************************
* Description  : 速度信号的异常判读
* Author       : 2018/4/13 星期五, by redmorningcn
*******************************************************************************/
void    app_ch_judge(void)
{
    static  uint32  tick;
    static  uint32  errcnt[2];
    uint8    i;
    static  int pluse[2];
        
    if(Ctrl.sys.time > tick + 300 || Ctrl.sys.time < tick)    //300ms判断一次
    {
        tick = Ctrl.sys.time;
            
        /**************************************************************
        * Description  : 无信号判断
        * Author       : 2018/7/18 星期三, by redmorningcn
        */
        for(i = 0; i < 2;i++)
        {
            if(pluse[i] !=  Ctrl.ch.test[i].pulse_cnt)
            {
                errcnt[i] = 0;
                
                Ctrl.ch.para[i].status.nopluse = 0;            //有脉冲信号        
            }
            else
            {   
                if(Ctrl.ch.para[i].status.nopluse == 0){       //有信號
                    errcnt[i]++;
                    
                    if(errcnt[i] > 3)                           //异常，处理(置标识位，数据清零，无脉冲信号)
                    {
                        Ctrl.ch.para[i].fail     = 0;
                        Ctrl.ch.para[i].freq     = 0;
                        Ctrl.ch.para[i].period   = 0;
                        Ctrl.ch.para[i].raise    = 0;
                        Ctrl.ch.para[i].ratio    = 0;
                        Ctrl.ch.para[i].status.flags    = 0;
                        Ctrl.ch.para[i].status.nopluse  = 1;    //无脉冲信号
                        Ctrl.ch.para[i].Voh      = 0;
                        Ctrl.ch.para[i].Vol      = 0;
                        
                        Ctrl.ch.ch1_2phase   = 0;
                    }
                }
            }
            pluse[i] = Ctrl.ch.test[i].pulse_cnt;
        }
    }
}


/*******************************************************************************
* 				end of file
*******************************************************************************/

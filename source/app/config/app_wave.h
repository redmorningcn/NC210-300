/*******************************************************************************
* Description  : ���μ��ṹ�嶨��
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/

#ifndef	APP_WAVE_H
#define	APP_WAVE_H

/*******************************************************************************
 * INCLUDES
 */
#include <global.h>
#include <bsp.h>


/*******************************************************************************
 * CONSTANTS
 */

 /*******************************************************************************
 * 				            ϵͳ������غ궨��                                  *
 *******************************************************************************/

/*******************************************************************************
 * MACROS
 */
//ͨ��ʱ�������������С
#define CH_TIMEPARA_BUF_SIZE    50
#define CH_VOLTAGE_BUF_SIZE     20

//�ź�λ��
#define CH_RAISE_10_STATUS      0x01
#define CH_RAISE_90_STATUS      0x02
#define CH_FALL_90_STATUS       0x04
#define CH_FALL_10_STATUS       0x08


/*******************************************************************************
* Description  : ͨ������ָ��
* Author       : 2018/3/14 ������, by redmorningcn
*******************************************************************************/
typedef struct  _strsignalchannelpara{
    uint32              period;                         //���ڣ�  0.00-2000000.00us ��0.5Hz��
    uint32              freq;                           //Ƶ�ʣ�  0-100000hz              
    uint16              raise;                          //�����أ�0.00-50.00us
    uint16              fail;                           //�½��أ�0.00-50.00us
    uint16              ratio;                          //ռ�ձȣ�0.00-100.00%
    uint16              Vol;                            //�͵�ƽ��0.00-30.00V
    uint16              Voh;                            //�ߵ�ƽ��0.00-30.00V
    
    union{
        struct{
            u8         lose    :1  ;                   //������
            u8         nopluse :1  ;                   //���ź�
            u8         rec     :6 ;
            
            s8         acceleration;                   //���ٶ�
        };
        u16             flags;
    }status;                         //ͨ��״̬
}strsignalchannelpara;



/*******************************************************************************
 * TYPEDEFS
 */



/*******************************************************************************
* Description  : ͨ���ṹ�塣���У�ͨ��ʱ������������㣻
                             ͨ����������ͨ���ľ���ָ�ꡣ
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
*******************************************************************************/
typedef struct
{
    /*******************************************************************************
    * Description  : ��������
    * Author       : 2018/3/14 ������, by redmorningcn
    *******************************************************************************/
    struct  {
        //ʱ��ָ��
        struct  {
            uint32  low_up_time;                            //10%λ�ã������أ��ж�ʱ�� 
            uint32  low_down_time;                          //10%λ�ã��½��أ��ж�ʱ��
            uint32  hig_up_time;                            //90%λ�ã������أ��ж�ʱ��
            uint32  hig_down_time;                          //90%λ�ã��½��أ��ж�ʱ��
            uint16  low_up_cnt;
            uint16  low_down_cnt;
            uint16  hig_up_cnt;
            uint16  hig_down_cnt;
        }time[CH_TIMEPARA_BUF_SIZE];
        
        uint16  p_write;                                    //��������д����
        uint16  p_read;
        uint32  pulse_cnt;                                  //����������ж��ź�����
        
        //�ź�����λ�ã���/��
        union  
        {
            struct __pluse_status__ {                      // �ź�״̬
                uint16  raise_10: 1;                        // ����λ�á���10%
                uint16  raise_90: 1;       	                // ����λ��--90%
                uint16  fall_90 : 1;       	                // �½�λ��--10
                uint16  fall_10 : 1;  	                    // ������δ����
                uint16  res     : 12;  	                    // ������δ����
                
                u8      res1    ;                           //Ԥ��
                s8      acceleration;                       //���ٶ�
            }station;
            uint32  pluse_status;
        };        
        
        //��ƽָ��
        struct {
            uint16  ch_low_voltage;                         //�źŵ͵�ƽ
            uint16  ch_hig_voltage;                         //�źŸߵ�ƽ
            uint16  vcc_hig_voltage;                        //ͨ�������ƽ
        }voltage[CH_VOLTAGE_BUF_SIZE];
        uint16  p_wr_vol;                                   //��ƽд��ָ��
        uint16  p_rd_vol;                                   //��ƽ����ָ��
    }test[2];                                               //ͨ���������
    
    
    /*******************************************************************************
    * Description  : ͨ������ָ��
    * Author       : 2018/3/14 ������, by redmorningcn
    *******************************************************************************/
    strsignalchannelpara    para[2];
    
    uint32  ch1_2phase;                                     //��λ�0.00-360.00��
    uint16  vcc_vol;                                        //�����ѹ
    uint16  stand_vol;                                      //�ο���ѹ
}strCoupleChannel;


//calibration
/**************************************************************
* Description  : У׼����
* Author       : 2018/5/22 ���ڶ�, by redmorningcn
*/
typedef struct {
    u32     line;       //�������Զ�  
    int16   Delta;      //����ƫ��
    int16   tmp;        //Ԥ��

}strCalibration;


/**************************************************************
* Description  : ����������
* Author       : 2018/5/22 ���ڶ�, by redmorningcn
*/
typedef struct{
    union   {
        struct{
            strCalibration  VccVol;         //��ƽ
            strCalibration  Ch1Vol;         //ͨ��1��ѹ
            strCalibration  Ch2Vol;         //ͨ��2��ѹ
        };
        strCalibration      CaliBuf[20];
    };
    
}strCaliTable;


/*******************************************************************************
 * GLOBAL VARIABLES
 */


/*******************************************************************************
 * 				end of file                                                    *
 *******************************************************************************/
#endif	/* GLOBLES_H */

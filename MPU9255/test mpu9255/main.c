//****************************************
// Update to MPU6050 by shinetop
// MCU: STC89C52
// 2012.3.1
// ����: ��ʾ���ٶȼƺ������ǵ�10λԭʼ����
//****************************************
// GY-52 MPU3050 IIC���Գ���
// ʹ�õ�Ƭ��STC89C51 
// ����11.0592M
// ��ʾ��LCD1602
// ���뻷�� Keil uVision2
// �ο��꾧��վ24c04ͨ�ų���
// ʱ�䣺2011��9��1��
// QQ��531389319
//****************************************
#include <REG52.H>	
#include <math.h>    //Keil library  
#include <stdio.h>   //Keil library	
#include <INTRINS.H>
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
//****************************************
// ����51��Ƭ���˿�
//****************************************
sbit    SCL=P2^1;			//IICʱ�����Ŷ���
sbit    SDA=P2^0;			//IIC�������Ŷ���

sbit  dula = P2^6;
sbit  wela = P2^7;
sbit  RS=P3^5;		//LCD1602����˿�		
sbit  RW=P3^6;		//LCD1602����˿�		
sbit  EN=P3^4;		//LCD1602����˿� 
//****************************************
// ����MPU6050�ڲ���ַ
//****************************************
#define	SMPLRT_DIV		0x19	//�����ǲ����ʣ�����ֵ��0x07(125Hz)
#define	CONFIG			0x1A	//��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)
#define	ACCEL_CONFIG	0x1C	//���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz)
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B	//��Դ����������ֵ��0x00(��������)
#define	WHO_AM_I			0x75	//IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��)
#define	SlaveAddress	0xD0	//IICд��ʱ�ĵ�ַ�ֽ����ݣ�+1Ϊ��ȡ
//****************************************
//�������ͼ�����
//****************************************
uchar dis[6];							//��ʾ����(-511��512)���ַ�����
int	dis_data;						//����

//****************************************
//��������
//****************************************

void  delayMs(unsigned int k);										//��ʱ
//LCD��غ���
void  InitLcd();														//��ʼ��lcd1602
void SeriPushSend(uchar send_data)
void  WriteDataLCM(uchar dat);									//LCD����
void  writeComm(uchar CMD);				//LCDָ��
void  DisplayOneChar(uchar X,uchar Y,uchar DData);			//��ʾһ���ַ�
void  DisplayListChar(uchar X,uchar Y,uchar *DData,L);	//��ʾ�ַ���
//MPU6050��������
void  InitMPU6050();													//��ʼ��MPU6050
void  Delay5us();
void  I2C_Start();
void  I2C_Stop();
void  I2C_SendACK(bit ack);
bit   I2C_RecvACK();
void  I2C_SendByte(uchar dat);
uchar I2C_RecvByte();
void  I2C_ReadPage();
void  I2C_WritePage();
void  display_ACCEL_x();
void  display_ACCEL_y();
void  display_ACCEL_z();
uchar Single_ReadI2C(uchar REG_Address);						//��ȡI2C����
void  Single_WriteI2C(uchar REG_Address,uchar REG_data);	//��I2Cд������
//****************************************
//����ת�ַ���
//****************************************
void SeriPushSend(uchar send_data)
{
	SBUF=send_data;
	while(!TI);
	TI=0;
}
//****************************************
//��ʱ
//****************************************
 void delayMs(uint a)
{
    uint i, j;
    for(i = a; i > 0; i--)
        for(j = 100; j > 0; j--);
 }
//****************************************
//LCD1602��ʼ��
//****************************************
void InitLcd()				
{			
	RW = 0; 
	dula = wela = 0;
	writeComm(0x38);
	writeComm(0x0c); 
	writeComm(0x06);
	writeComm(0x01);
}			
	
//****************************************
//LCD1602д������
//****************************************
void writeComm(uchar comm)
{
	RS = 0;    
	P0 = comm;
	EN = 1;
	_nop_();
	EN = 0;
	delayMs(1);
}	
//****************************************
//LCD1602д������
//****************************************
void WriteDataLCM(uchar dat)
{					
	RS = 1;
	P0 = dat;
	EN = 1;
	_nop_();
	EN = 0;
	delayMs(1);
}		
//****************************************
//LCD1602д��һ���ַ�
//****************************************
void DisplayOneChar(uchar X,uchar Y,uchar DData)
{						
	Y&=1;						
	X&=15;						
	if(Y)X|=0x40;					
	X|=0x80;					
	WriteDataLCM(DData);		
}						
//****************************************
//LCD1602��ʾ�ַ���
//****************************************
void DisplayListChar(uchar X,uchar Y,uchar *DData,L)
{
	uchar ListLength=0; 
	Y&=0x1;                
	X&=0xF;                
	while(L--)             
	{                       
		DisplayOneChar(X,Y,DData[ListLength]);
		ListLength++;  
		X++;                        
	}    
}
//**************************************
//��ʱ5΢��(STC90C52RC@12M)
//��ͬ�Ĺ�������,��Ҫ�����˺���
//������1T��MCUʱ,���������ʱ����
//**************************************
void Delay5us()
{
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
}
//**************************************
//I2C��ʼ�ź�
//**************************************
void I2C_Start()
{
    SDA = 1;                    //����������
    SCL = 1;                    //����ʱ����
    Delay5us();                 //��ʱ
    SDA = 0;                    //�����½���
    Delay5us();                 //��ʱ
    SCL = 0;                    //����ʱ����
}
//**************************************
//I2Cֹͣ�ź�
//**************************************
void I2C_Stop()
{
    SDA = 0;                    //����������
    SCL = 1;                    //����ʱ����
    Delay5us();                 //��ʱ
    SDA = 1;                    //����������
    Delay5us();                 //��ʱ
}
//**************************************
//I2C����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
//**************************************
void I2C_SendACK(bit ack)
{
    SDA = ack;                  //дӦ���ź�
    SCL = 1;                    //����ʱ����
    Delay5us();                 //��ʱ
    SCL = 0;                    //����ʱ����
    Delay5us();                 //��ʱ
}
//**************************************
//I2C����Ӧ���ź�
//**************************************
bit I2C_RecvACK()
{
    SCL = 1;                    //����ʱ����
    Delay5us();                 //��ʱ
    CY = SDA;                   //��Ӧ���ź�
    SCL = 0;                    //����ʱ����
    Delay5us();                 //��ʱ
    return CY;
}
//**************************************
//��I2C���߷���һ���ֽ�����
//**************************************
void I2C_SendByte(uchar dat)
{
    uchar i;
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;              //�Ƴ����ݵ����λ
        SDA = CY;               //�����ݿ�
        SCL = 1;                //����ʱ����
        Delay5us();             //��ʱ
        SCL = 0;                //����ʱ����
        Delay5us();             //��ʱ
    }
    I2C_RecvACK();
}
//**************************************
//��I2C���߽���һ���ֽ�����
//**************************************
uchar I2C_RecvByte()
{
    uchar i;
    uchar dat = 0;
    SDA = 1;                    //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        SCL = 1;                //����ʱ����
        Delay5us();             //��ʱ
        dat |= SDA;             //������               
        SCL = 0;                //����ʱ����
        Delay5us();             //��ʱ
    }
    return dat;
}
//**************************************
//��I2C�豸д��һ���ֽ�����
//**************************************
void Single_WriteI2C(uchar REG_Address,uchar REG_data)
{
    I2C_Start();                  //��ʼ�ź�
    I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�
    I2C_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ��
    I2C_SendByte(REG_data);       //�ڲ��Ĵ������ݣ�
    I2C_Stop();                   //����ֹͣ�ź�
}
//**************************************
//��I2C�豸��ȡһ���ֽ�����
//**************************************
uchar Single_ReadI2C(uchar REG_Address)
{
	uchar REG_data;
	I2C_Start();                   //��ʼ�ź�
	I2C_SendByte(SlaveAddress);    //�����豸��ַ+д�ź�
	I2C_SendByte(REG_Address);     //���ʹ洢��Ԫ��ַ����0��ʼ	
	I2C_Start();                   //��ʼ�ź�
	I2C_SendByte(SlaveAddress+1);  //�����豸��ַ+���ź�
	REG_data=I2C_RecvByte();       //�����Ĵ�������
	I2C_SendACK(1);                //����Ӧ���ź�
	I2C_Stop();                    //ֹͣ�ź�
	return REG_data;
}
//**************************************
//��ʼ��MPU6050
//**************************************
void InitMPU6050()
{
	Single_WriteI2C(PWR_MGMT_1, 0x00);	//�������״̬
	Single_WriteI2C(SMPLRT_DIV, 0x07);
	Single_WriteI2C(CONFIG, 0x06);
	Single_WriteI2C(GYRO_CONFIG, 0x18);
	Single_WriteI2C(ACCEL_CONFIG, 0x01);
}
//**************************************
//�ϳ�����
//**************************************
int GetData(uchar REG_Address)
{
	char H,L;
	H=Single_ReadI2C(REG_Address);
	L=Single_ReadI2C(REG_Address+1);
	return (H<<8)+L;   //�ϳ�����
}
//**************************************
//��1602����ʾ10λ����
//**************************************
void Display10BitData(int value,uchar x,uchar y)
{
	value/=64;							//ת��Ϊ10λ����
	lcd_printf(dis, value);			//ת��������ʾ
	DisplayListChar(x,y,dis,4);	//��ʼ�У��У���ʾ���飬��ʾ����
}

//*********************************************************
//������
//*********************************************************
void main()
{ 
	delayMs(500);		//�ϵ���ʱ		
	InitLcd();		//Һ����ʼ��
	InitMPU6050();	//��ʼ��MPU6050
	delayMs(150);
	while(1)
	{
		Display10BitData(GetData(ACCEL_XOUT_H),2,0);	//��ʾX����ٶ�
		Display10BitData(GetData(ACCEL_YOUT_H),7,0);	//��ʾY����ٶ�
		Display10BitData(GetData(ACCEL_ZOUT_H),12,0);	//��ʾZ����ٶ�
		Display10BitData(GetData(GYRO_XOUT_H),2,1);		//��ʾX����ٶ�
		Display10BitData(GetData(GYRO_YOUT_H),7,1);		//��ʾY����ٶ�
		Display10BitData(GetData(GYRO_ZOUT_H),12,1);	//��ʾZ����ٶ�
		delayMs(5000);
	}
}
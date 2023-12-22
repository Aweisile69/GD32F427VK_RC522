#include "gd32f4xx.h"
#include "rc522.h"
#include "systick.h"
#include "string.h"
#include "my_uart.h"
#include "stdio.h"
#include "485port.h"

// M1卡分为16个扇区，每个扇区由四个块（块0、块1、块2、块3）组成
// 将16个扇区的64个块按绝对地址编号为：0~63
// 第0个扇区的块0（即绝对地址0块），用于存放厂商代码，已经固化不可更改 
// 每个扇区的块0、块1、块2为数据块，可用于存放数据
// 每个扇区的块3为控制块（绝对地址为:块3、块7、块11.....）包括密码A，存取控制、密码B等
 
/*全局变量*/
unsigned char CT[2];//卡片类型
unsigned char SN[4]; //卡号（低字节在前，高字节在后）
unsigned char RFID[16];//存放RFID 
unsigned char lxl_bit=0;
unsigned char card1_bit=0;
unsigned char card2_bit=0;
unsigned char card3_bit=0;
unsigned char card4_bit=0;
unsigned char total=0;
unsigned char lxl[4]={196,58,104,217};
unsigned char card_1[4]={83,106,11,1};
unsigned char card_2[4]={208,121,31,57};
unsigned char card_3[4]={176,177,143,165};
unsigned char card_4[4]={5,158,10,136};
uint8_t KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};//密钥
uint8_t AUDIO_OPEN[6] = {0xAA, 0x07, 0x02, 0x00, 0x09, 0xBC};
unsigned char RFID1[16]={0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char status;
unsigned char s=0x08;
unsigned char ShowON; 
 
#define   RC522_DELAY()  delay_us(20)

//ID
char ss[255];
//char data[16];

unsigned char snr, buf[16], TagType[2], SelectedSnr[4], DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
unsigned char buf1[16];
unsigned char buf2[16];
int a = 1200;
char OK_status;

void RC522_Handel(void)
{
    uart_printf("寻卡中!\n");
    status = PcdRequest(PICC_REQALL,CT);//寻卡
    
    if(status == MI_OK)//射频范围内有14443 A卡
    {
        uart_printf("射频范围内有A卡!\n");
        status = MI_ERR;//修改标志位，进入下一步防冲撞
        status = PcdAnticoll(SN);//防冲撞
    }

    if (status == MI_OK)//防冲撞结束
    {
        uart_printf("防冲撞成功!\n");
        status = MI_ERR;//修改标志位，进入下一步选卡
        status = PcdSelect(SN);//选卡
    }

    if(status == MI_OK)//选卡成功
    {
        uart_printf("选卡成功!\n");
        status = MI_ERR;//修改标志位，进入下一步验证
        status = PcdAuthState(0x60,0x09,KEY,SN);//验证
    }

    if(status == MI_OK)//验证成功
    {
        uart_printf("验证成功!\n");
        status = MI_ERR;
        status = PcdRead(s,RFID);
    }
 
    if(status == MI_OK)//读卡成功
    {
        uart_printf("读卡成功!\n");
        status = MI_ERR;
        delay_1ms(100);
    }	
 
}

/**
 * @description: SPI通信下，MCU发送一个字节,MCU->RC522
 * @param {uint8_t} Byte 发送的数据
 * @return {*}
 */
void RC522_SendByte(uint8_t Byte)
{
    //MCU和RC522交换一个字节，换回来的数据不处理即可
    HardWare_SPI_SwapByte(Byte);
}

/**
 * @description: SPI通信下，MCU接收一个字节，MCU<-RC522
 * @return {uint8_t} 接收到的数据
 */
uint8_t RC522_ReceiveByte(void)
{
    //SPI通信的本质上是交换一个字节，所以这里依然调用swap
    //把一个无用的字节0xFF发送过去，交换得到我们需要的数据
    return HardWare_SPI_SwapByte(0);
}

/**
 * @description: 读RC522的寄存器
 * @param {uint8_t} ucAddress 寄存器地址
 * @return {uint8_t} 所读寄存器的内容
 */
uint8_t ReadRawRC(uint8_t ucAddress)
{
	uint8_t ucAddr,ucReturn;
    /**
     * 计算要发送给模块的地址字节
     * 根据RC522的SPI通信协议要求，地址字节由1位读写位，6位地址，1位RFU组成
     * bit7为读写位（1为读，0为写）
     * bit6-bit1为地址
     * bit0为RFU（必须为0）
     * 所以先将传入的地址左移1位，RC522的所有寄存器都是00xx xxxx，左移1位就
     * 变成了0xxx xxx0，中间6位的地址并没有变。
     * 然后与上0x7E(0111 1110)，将最低位置0
     * 最后或上0x80(1000 0000)，将最高位置1，表示读寄存器
    */
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;
	//开始SPI通信(SPI模式0)，这里需要参考MFRC522的数据手册，
	HardWare_SPI_Start();
	//先发送寄存器地址，让RC522将目标寄存器的内容加载到RC522的SPI移位寄存器
	RC522_SendByte(ucAddr);
	//发送一个无用字节，交换得到目标寄存器的内容
	ucReturn = RC522_ReceiveByte();
    //结束SPI通信
	HardWare_SPI_Stop();
	return ucReturn;
}

/**
 * @description: 写RC522的寄存器
 * @param {uint8_t} ucAddress 寄存器地址
 * @param {uint8_t} ucValue 要写入的值
 * @return {*}
 */
void WriteRawRC(uint8_t ucAddress,uint8_t ucValue)
{  
	uint8_t ucAddr;
    //先左移1为，在或上 0111 1110，保证最高位和最低位都为0，表示写寄存器
	ucAddr = ( ucAddress << 1 ) & 0x7E;
	//开始SPI通信
	HardWare_SPI_Start();
	//发送要写的寄存器的地址
	RC522_SendByte(ucAddr);
	//发送要写入的值
	RC522_SendByte(ucValue);
	//结束SPI通信
	HardWare_SPI_Stop();	
}

/**
 * @description: 对RC522的寄存器置位
 * @param {uint8_t} ucReg 寄存器地址
 * @param {uint8_t} ucMask 要置的位，比如要置第2，4位，ucMask = 0x14(0001 0100)
 * @return {*}
 */
void SetBitMask(uint8_t ucReg,uint8_t ucMask)  
{
    uint8_t ucTemp;
    //读取目标寄存器当前的值
    ucTemp = ReadRawRC (ucReg);
	//通过或运算，将指定的bit位，置1，这也称为掩码
    WriteRawRC(ucReg, ucTemp|ucMask);
}

/**
 * @description: 对RC522的寄存器清位
 * @param {uint8_t} ucReg 寄存器地址
 * @param {uint8_t} ucMask 要清的位，比如要清第bit2和bit4，那么ucMask = 0x14 (0001 0100)
 * @return {*}
 */
void ClearBitMask(uint8_t ucReg,uint8_t ucMask)  
{
    uint8_t ucTemp;
    //读取目标寄存器当前的值
    ucTemp = ReadRawRC(ucReg);
	//先将ucMask取反，再相与，就能达到指定位清0的目的
    WriteRawRC(ucReg,ucTemp&(~ucMask));
}

/**
 * @description: 开启天线
 * @return {*}
 */
void PcdAntennaOn(void)
{
    uint8_t uc;
    uc = ReadRawRC(TxControlReg);
    //TxControlReg寄存器的bit0和bit1，是用来控制天线工作的，置高电平开启天线输出调制。
    if (!(uc&0x03))
	SetBitMask(TxControlReg,0x03);
}

/**
 * @description: 关闭天线
 * @return {*}
 */
void PcdAntennaOff ( void )
{
    ClearBitMask(TxControlReg,0x03);
}


/**
 * @description: 复位RC522
 * @return {*}
 */
void PCD_Reset(void)
{
    /*硬件复位RC522*/
    Reset_Disable();
    delay_1us(1);
    Reset_Enable();
    delay_1us(1);
    Reset_Disable();
    delay_1us(1);
    /*软件复位RC522*/
    WriteRawRC(CommandReg,0x0F);
    //CommandReg的bit4为PowerDown,逻辑0表示RC522已准备好运行，逻辑1表示进入软掉电模式
    while(ReadRawRC(CommandReg)&0x10);
    delay_1us(1);
    // uart_TransmitByte(ReadRawRC(ModeReg));
    WriteRawRC(ModeReg,0x3D);            //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
    // uart_TransmitByte(ReadRawRC(ModeReg));
    // uart_TransmitByte(ReadRawRC(TReloadRegL));
    WriteRawRC(TReloadRegL,30);          //16位定时器低位    
    // uart_TransmitByte(ReadRawRC(TReloadRegL));
    // uart_TransmitByte(ReadRawRC(TReloadRegH));
    WriteRawRC(TReloadRegH,0);			     //16位定时器高位
    // uart_TransmitByte(ReadRawRC(TReloadRegH));
    // uart_TransmitByte(ReadRawRC(TModeReg));
    WriteRawRC(TModeReg,0x8D);				   //定义内部定时器的设置
    // uart_TransmitByte(ReadRawRC(TModeReg));
    // uart_TransmitByte(ReadRawRC(TPrescalerReg));
    WriteRawRC(TPrescalerReg,0x3E);			 //设置定时器分频系数
    // uart_TransmitByte(ReadRawRC(TPrescalerReg));
    // uart_TransmitByte(ReadRawRC(TxAutoReg));
    WriteRawRC(TxAutoReg,0x40);				   //调制发送信号为100%ASK	
    // uart_TransmitByte(ReadRawRC(TxAutoReg));
}

/**
 * @description: 配置RC522的工作模式
 * @param {uint8_t} ucType
 * @return {*}
 */
void M500PcdConfigISOType(uint8_t ucType)
{
	if (ucType == 'A')//A卡
    {
		ClearBitMask ( Status2Reg, 0x08 );
        WriteRawRC ( ModeReg, 0x3D );//3F
		WriteRawRC ( RxSelReg, 0x86 );//84
		WriteRawRC( RFCfgReg, 0x7F ); //4F
		WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		WriteRawRC ( TReloadRegH, 0 );
		WriteRawRC ( TModeReg, 0x8D );
		WriteRawRC ( TPrescalerReg, 0x3E );
		delay_1us(2);
		PcdAntennaOn ();//开天线
    }
}

/**
 * @description: 通过RC522和ISO14443卡通讯
 * @param {uint8_t} ucCommand RC522的命令字
 * @param {uint8_t *} pInData 通过RC522发送到卡片的数据
 * @param {uint8_t} ucInLenByte 发送数据的长度
 * @param {uint8_t *} pOutData 接收到的卡片返回的数据
 * @param {uint32_t *} pOutLenBit 返回的数据的位长度
 * @return {char} 状态值
 *                 = MI_OK，成功
 */
char PcdComMF522 ( uint8_t ucCommand, uint8_t * pInData, uint8_t ucInLenByte, uint8_t * pOutData, uint32_t * pOutLenBit )		
{
    char cStatus = MI_ERR;
    uint8_t ucIrqEn   = 0x00;
    uint8_t ucWaitFor = 0x00;
    uint8_t ucLastBits;
    uint8_t ucN;
    uint32_t ul;
 
    switch ( ucCommand )
    {
       case PCD_AUTHENT:		//Mifare认证
          ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
          ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
          break;
			 
       case PCD_TRANSCEIVE:		//接收发送 发送接收
          ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
          break;
			 
       default:
         break;
			 
    }
   
    WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 
    ClearBitMask ( ComIrqReg, 0x80 );			//Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC ( CommandReg, PCD_IDLE );		//写空闲命令
    SetBitMask ( FIFOLevelReg, 0x80 );			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除
    
    for ( ul = 0; ul < ucInLenByte; ul ++ )
		  WriteRawRC ( FIFODataReg, pInData [ ul ] );    		//写数据进FIFOdata
			
    WriteRawRC ( CommandReg, ucCommand );					//写命令
   
    
    if ( ucCommand == PCD_TRANSCEIVE )
			SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效
    
    ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms
		
    do 														//认证 与寻卡等待时间	
    {
         ucN = ReadRawRC ( ComIrqReg );							//查询事件中断
         ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//退出条件i=0,定时器中断，与写空闲命令
		
    ClearBitMask ( BitFramingReg, 0x80 );					//清理允许StartSend位
		
    if ( ul != 0 )
    {
		if ( ! (( ReadRawRC ( ErrorReg ) & 0x1B )) )			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
		{
			cStatus = MI_OK;
			
			if ( ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
			  cStatus = MI_NOTAGERR;   
				
			if ( ucCommand == PCD_TRANSCEIVE )
			{
				ucN = ReadRawRC ( FIFOLevelReg );			//读FIFO中保存的字节数
				
				ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	//最后接收到得字节的有效位数
				
				if ( ucLastBits )
					* pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
				else
					* pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效
				
				if ( ucN == 0 )	
                    ucN = 1;    
				
				if ( ucN > MAXRLEN )
					ucN = MAXRLEN;   
				
				for ( ul = 0; ul < ucN; ul ++ )
				  pOutData [ ul ] = ReadRawRC ( FIFODataReg );   
			}		
        }
			else
				cStatus = MI_ERR;   
//			printf(ErrorReg);
    }
   
   SetBitMask ( ControlReg, 0x80 );           // stop timer now
   WriteRawRC ( CommandReg, PCD_IDLE ); 
	
   return cStatus;
 
}

/**
 * @description: 寻卡
 * @param {uint8_t} ucReq_code 寻卡方式
 *                  = 0x52，寻感应区内所有符合14443A标准的卡
 *                  = 0x26，寻未进入休眠状态的卡
 * @param {uint8_t *} pTagType 卡片类型代码
 *                  = 0x4400，Mifare_UltraLight
 *                  = 0x0400，Mifare_One(S50)
 *                  = 0x0200，Mifare_One(S70)
 *                  = 0x0800，Mifare_Pro(X))
 *                  = 0x4403，Mifare_DESFire
 * @return {char} 状态值
 *                  = MI_OK，成功
 */
char PcdRequest(uint8_t ucReq_code,uint8_t * pTagType)
{
    char cStatus;
    uint8_t ucComMF522Buf[MAXRLEN];
    uint32_t ulLen;
    ClearBitMask(Status2Reg,0x08);	//将MIFARECyptol清零，不加密
    WriteRawRC(BitFramingReg,0x07);	//发送的最后一个字节的七位
    SetBitMask(TxControlReg,0x03);	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号
 
    ucComMF522Buf [ 0 ] = ucReq_code;		//存入卡片命令字
 
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//寻卡  
 
    if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡类型 
    {    
       * pTagType = ucComMF522Buf [ 0 ];
       * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
    }
     
    else
     cStatus = MI_ERR;
 
    return cStatus;
 
}

/**
 * @description: 防冲撞
 * @param {uint8_t *} pSnr 卡片序列号，4字节
 * @return {char} 状态值
 *                  = MI_OK，成功
 */
char PcdAnticoll ( uint8_t * pSnr )
{
    char cStatus;
    uint8_t uc, ucSnr_check = 0;
    uint8_t ucComMF522Buf [ MAXRLEN ]; 
	uint32_t ulLen;
 
    ClearBitMask ( Status2Reg, 0x08 );		//清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    WriteRawRC ( BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask ( CollReg, 0x80 );			//清ValuesAfterColl所有接收的位在冲突后被清除
   
    ucComMF522Buf [ 0 ] = 0x93;	//卡片防冲突命令
    ucComMF522Buf [ 1 ] = 0x20;
   
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信
	
    if ( cStatus == MI_OK)		//通信成功
    {
		for ( uc = 0; uc < 4; uc ++ )
        {
            * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//读出UID
            ucSnr_check ^= ucComMF522Buf [ uc ];
        }
			
        if ( ucSnr_check != ucComMF522Buf [ uc ] )
        		cStatus = MI_ERR;    
				 
    }
    
    SetBitMask ( CollReg, 0x80 );
 
    return cStatus;
	
}

/**
 * @description: 用RC522计算CRC16
 * @param {uint8_t *} pIndata 计算CRC16的数组
 * @param {uint8_t} ucLen 计算CRC16的数组字节长度
 * @param {uint8_t *} pOutData 存放计算结果存放的首地址
 * @return {*}
 */
void CalulateCRC ( uint8_t * pIndata, uint8_t ucLen, uint8_t * pOutData )
{
    uint8_t uc, ucN;
 
    ClearBitMask(DivIrqReg,0x04);
	
    WriteRawRC(CommandReg,PCD_IDLE);
	
    SetBitMask(FIFOLevelReg,0x80);
	
    for ( uc = 0; uc < ucLen; uc ++)
	    WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );   
 
    WriteRawRC ( CommandReg, PCD_CALCCRC );
	
    uc = 0xFF;
	
    do 
    {
        ucN = ReadRawRC ( DivIrqReg );
        uc --;
    } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
		
    pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
    pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );
	
}

/**
 * @description: 选定卡片
 * @param {uint8_t *} pSnr 卡片序列号，4字节
 * @return {char} 状态值
 *                  = MI_OK，成功
 */
char PcdSelect ( uint8_t * pSnr )
{
    char ucN;
    uint8_t uc;
	  uint8_t ucComMF522Buf [ MAXRLEN ]; 
    uint32_t  ulLen;
 
    ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
    ucComMF522Buf [ 1 ] = 0x70;
    ucComMF522Buf [ 6 ] = 0;
	
    for ( uc = 0; uc < 4; uc ++ )
    {
    	ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
    	ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
    }
		
    CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );
  
    ClearBitMask ( Status2Reg, 0x08 );
 
    ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
    
    if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
      ucN = MI_OK;  
    else
      ucN = MI_ERR;    
 
    return ucN;
	
}

/**
 * @description: 验证卡片密码
 * @param {uint8_t} ucAuth_mode 密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 * @param {uint8_t} ucAddr 块地址
 * @param {uint8_t *} pKey 密码
 * @param {uint8_t *} pSnr 卡片序列号
 * @return {char} 状态值
 *                 = MI_OK，成功
 */
char PcdAuthState ( uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t * pKey, uint8_t * pSnr )
{
    char cStatus;
	  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;
 
    ucComMF522Buf [ 0 ] = ucAuth_mode;
    ucComMF522Buf [ 1 ] = ucAddr;
	
    for ( uc = 0; uc < 6; uc ++ )
	    ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   
	
    for ( uc = 0; uc < 6; uc ++ )
	    ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   
 
    cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );
	
    if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )
			
		{
//			if(cStatus != MI_OK)
//					printf("666")	;		
//			else
//				printf("888");
			cStatus = MI_ERR; 
    }
		
    return cStatus;
		
}

/**
 * @description: 写数据到M1卡的一块
 * @param {uint8_t} ucAddr 块地址
 * @param {uint8_t *} pData 数据
 * @return {char} 状态值
 *                  = MI_OK，成功
 */
char PcdWrite ( uint8_t ucAddr, uint8_t * pData )
{
    char cStatus;
	  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;
 
    ucComMF522Buf [ 0 ] = PICC_WRITE;
    ucComMF522Buf [ 1 ] = ucAddr;
	
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
 
    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;   
        
    if ( cStatus == MI_OK )
    {
			memcpy(ucComMF522Buf, pData, 16);
      for ( uc = 0; uc < 16; uc ++ )
			  ucComMF522Buf [ uc ] = * ( pData + uc );  
			
      CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );
 
      cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
			
			if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;   
			
    } 
 
    return cStatus;
	
}

/**
 * @description: 读取M1的一块
 * @param {uint8_t} ucAddr 块地址
 * @param {uint8_t *} pData 读取的数据
 * @return {char} 状态值
 *                  = MI_OK，成功
 */
char PcdRead ( uint8_t ucAddr, uint8_t * pData )
{
    char cStatus;
	  uint8_t uc, ucComMF522Buf [ MAXRLEN ]; 
    uint32_t ulLen;
 
    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;
	
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
   
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
	
    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
			for ( uc = 0; uc < 16; uc ++ )
        * ( pData + uc ) = ucComMF522Buf [ uc ];   
    }
		
    else
      cStatus = MI_ERR;   
	
    return cStatus;
 
}

/**
 * @description: 命令卡片进入休眠模式
 * @return {char} 状态值
 *                 = MI_OK，成功
 */
char PcdHalt( void )
{
    uint8_t ucComMF522Buf [ MAXRLEN ]; 
    uint32_t  ulLen;
 
    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;
 
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    	PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
 
    return MI_OK;
	
}
 
 
void IC_CMT ( uint8_t * UID, uint8_t * KEY, uint8_t RW, uint8_t * Dat )
{
    uint8_t ucArray_ID [ 4 ] = { 0 };//先后存放IC卡的类型和UID(IC卡序列号)
 
    PcdRequest ( 0x52, ucArray_ID );//寻卡
 
    PcdAnticoll ( ucArray_ID );//防冲撞
 
    PcdSelect ( UID );//选定卡
 
    PcdAuthState ( 0x60, 0x10, KEY, UID );//校验
 
    if ( RW )//读写选择，1是读，0是写
        PcdRead ( 0x10, Dat );
 
    else 
        PcdWrite ( 0x10, Dat );
     
    PcdHalt ();	 
	 
}


void RC522_Init(void)
{
	HardWare_SPI_Init();//SPI外设初始化
    PCD_Reset();//RC522复位
	M500PcdConfigISOType('A');//设置工作方式
}

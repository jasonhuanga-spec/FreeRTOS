#include "LCD.h"

/**
 * @函数名称       测试
 * @说明           验证功能
 */
void LCDtest(void)
{
    LCD4SPI_WriteCMD(0xDF);
    LCD4SPI_WriteDATA(0x98);
    LCD4SPI_WriteDATA(0x53);

    LCD4SPI_WriteCMD(0xDE);
    LCD4SPI_WriteDATA(0x00);

    LCD4SPI_WriteCMD(0xB2);
    LCD4SPI_WriteDATA(0x25);

    LCD4SPI_WriteCMD(0xB7);
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0x29);//VGMP 4.5V
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0x51);//VGSP 4.5V

    LCD4SPI_WriteCMD(0xBB);
    LCD4SPI_WriteDATA(0x44);
    LCD4SPI_WriteDATA(0x2B);
    LCD4SPI_WriteDATA(0x37);
    LCD4SPI_WriteDATA(0x61);
    LCD4SPI_WriteDATA(0x63);
    LCD4SPI_WriteDATA(0xF0);

    // LCD4SPI_WriteCMD(0xBC);
    // LCD4SPI_WriteDATA(0x77);

    LCD4SPI_WriteCMD(0xC0);
    LCD4SPI_WriteDATA(0x44);
    LCD4SPI_WriteDATA(0xA4);

    LCD4SPI_WriteCMD(0xC1);
    LCD4SPI_WriteDATA(0x12);

    LCD4SPI_WriteCMD(0xC3);
    LCD4SPI_WriteDATA(0x7D);
    LCD4SPI_WriteDATA(0x07);
    LCD4SPI_WriteDATA(0x14);
    LCD4SPI_WriteDATA(0x06);
    LCD4SPI_WriteDATA(0xC8);
    LCD4SPI_WriteDATA(0x71);
    LCD4SPI_WriteDATA(0x6C);
    LCD4SPI_WriteDATA(0x77);

    LCD4SPI_WriteCMD(0xC4);
    LCD4SPI_WriteDATA(0x00);//00=60Hz 06=53Hz 08=42Hz
    LCD4SPI_WriteDATA(0x10);
    LCD4SPI_WriteDATA(0x94);//LN=296  Line
    LCD4SPI_WriteDATA(0x79);
    LCD4SPI_WriteDATA(0x25);
    LCD4SPI_WriteDATA(0x0A);
    LCD4SPI_WriteDATA(0x16);
    LCD4SPI_WriteDATA(0x79);
    LCD4SPI_WriteDATA(0x25);
    LCD4SPI_WriteDATA(0x0A);
    LCD4SPI_WriteDATA(0x16);
    LCD4SPI_WriteDATA(0x82);

    //SET_R_GAMMA Fit VOP4.5 G2.0 20231116
    LCD4SPI_WriteCMD(0xC8);	//G2.2	//G2.5
    LCD4SPI_WriteDATA(0x3F);	//0x3F	//0x3F
    LCD4SPI_WriteDATA(0x34);	//0x33	//0x32
    LCD4SPI_WriteDATA(0x2D);	//0x2B	//0x2A
    LCD4SPI_WriteDATA(0x26);	//0x25	//0x22
    LCD4SPI_WriteDATA(0x2B);	//0x29	//0x26
    LCD4SPI_WriteDATA(0x2B);	//0x28	//0x25
    LCD4SPI_WriteDATA(0x25);	//0x22	//0x1E
    LCD4SPI_WriteDATA(0x24);	//0x21	//0x1E
    LCD4SPI_WriteDATA(0x23);	//0x20	//0x1C
    LCD4SPI_WriteDATA(0x22);	//0x1F	//0x1C
    LCD4SPI_WriteDATA(0x20);	//0x1E	//0x19
    LCD4SPI_WriteDATA(0x17);	//0x14	//0x0E
    LCD4SPI_WriteDATA(0x14);	//0x0F	//0x09
    LCD4SPI_WriteDATA(0x0E);	//0x0A	//0x02
    LCD4SPI_WriteDATA(0x06);	//0x06	//0x06
    LCD4SPI_WriteDATA(0x00);	//0x00	//0x00
    LCD4SPI_WriteDATA(0x3F);	//0x3F	//0x3F
    LCD4SPI_WriteDATA(0x34);	//0x33	//0x32
    LCD4SPI_WriteDATA(0x2D);	//0x2B	//0x2A
    LCD4SPI_WriteDATA(0x26);	//0x25	//0x22
    LCD4SPI_WriteDATA(0x2B);	//0x29	//0x26
    LCD4SPI_WriteDATA(0x2B);	//0x28	//0x25
    LCD4SPI_WriteDATA(0x25);	//0x22	//0x1E
    LCD4SPI_WriteDATA(0x24);	//0x21	//0x1E
    LCD4SPI_WriteDATA(0x23);	//0x20	//0x1C
    LCD4SPI_WriteDATA(0x22);	//0x1F	//0x1C
    LCD4SPI_WriteDATA(0x20);	//0x1E	//0x19
    LCD4SPI_WriteDATA(0x17);	//0x14	//0x0E
    LCD4SPI_WriteDATA(0x14);	//0x0F	//0x09
    LCD4SPI_WriteDATA(0x0E);	//0x0A	//0x02
    LCD4SPI_WriteDATA(0x06);	//0x06	//0x06
    LCD4SPI_WriteDATA(0x00);	//0x00	//0x00


    LCD4SPI_WriteCMD(0xD0);
    LCD4SPI_WriteDATA(0x04);
    LCD4SPI_WriteDATA(0x06);
    LCD4SPI_WriteDATA(0x6B);
    LCD4SPI_WriteDATA(0x0F);
    LCD4SPI_WriteDATA(0x00);

    LCD4SPI_WriteCMD(0xD7);
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0x30);

    LCD4SPI_WriteCMD(0xE6);
    LCD4SPI_WriteDATA(0x10);

    LCD4SPI_WriteCMD(0xDE);
    LCD4SPI_WriteDATA(0x01);


    // LCD4SPI_WriteCMD(0xBB);
    // LCD4SPI_WriteDATA(0x04);

    // LCD4SPI_WriteCMD(0xD7);
    // LCD4SPI_WriteDATA(0x12);

    LCD4SPI_WriteCMD(0xB7);
    LCD4SPI_WriteDATA(0x03);
    LCD4SPI_WriteDATA(0x13);
    LCD4SPI_WriteDATA(0xEF);
    LCD4SPI_WriteDATA(0x35);
    LCD4SPI_WriteDATA(0x35);

    LCD4SPI_WriteCMD(0xC1);
    LCD4SPI_WriteDATA(0x14);
    LCD4SPI_WriteDATA(0x15);
    LCD4SPI_WriteDATA(0xC0);

    LCD4SPI_WriteCMD(0xC2);
    LCD4SPI_WriteDATA(0x06);
    LCD4SPI_WriteDATA(0x3A);

    LCD4SPI_WriteCMD(0xC4);
    LCD4SPI_WriteDATA(0x72);  // 00=60Hz 06=53Hz 08=42Hz
    LCD4SPI_WriteDATA(0x12);

    // LCD4SPI_WriteCMD(0xC5);
    // LCD4SPI_WriteDATA(0x03);

    LCD4SPI_WriteCMD(0xBE);
    LCD4SPI_WriteDATA(0x00);

    LCD4SPI_WriteCMD(0xDE);
    LCD4SPI_WriteDATA(0x00);

    LCD4SPI_WriteCMD(0x35);
    LCD4SPI_WriteDATA(0x00);

    LCD4SPI_WriteCMD(0x3A);
    LCD4SPI_WriteDATA(0x05);//0x06=RGB666  0x05=RGB565

    LCD4SPI_WriteCMD(0x2A);
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0x00);//Start_X=00
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0xEF);//End_X=239

    LCD4SPI_WriteCMD(0x2B);
    LCD4SPI_WriteDATA(0x00);
    LCD4SPI_WriteDATA(0x00);//Start_Y=00
    LCD4SPI_WriteDATA(0x01);
    LCD4SPI_WriteDATA(0x27);//End_Y=295


    LCD4SPI_WriteCMD(0x11);
    HAL_Delay(120);

    // LCD4SPI_WriteCMD(0xDE);
    // LCD4SPI_WriteDATA(0x03);


    // LCD4SPI_WriteCMD(0xB5);
    // LCD4SPI_WriteDATA(0x23);

    // LCD4SPI_WriteCMD(0xDE);
    // LCD4SPI_WriteDATA(0x00);


    LCD4SPI_WriteCMD(0x29);
    HAL_Delay(100);


    LCD4SPI_WriteCMD(0x2C);
    for (int i = 0; i < (71040 * 2); i++)
    {
        LCD4SPI_WriteDATA(0xFF);
    }
    

}



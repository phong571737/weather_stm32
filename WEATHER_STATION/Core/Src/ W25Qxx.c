#include "main.h"
#include "W25Qxx.h"

extern SPI_HandleTypeDef hspi2;
#define W25Q_SPI hspi2
void W25Q_Delay(uint32_t time){
	HAL_Delay (time);
}

void csLOW(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

void csHIGH(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}

void SPI_Write(uint8_t *data, uint8_t len)
{
	HAL_SPI_Transmit(&W25Q_SPI, data, len, 2000);
}

void SPI_Read(uint8_t *data, uint8_t len)
{
	HAL_SPI_Receive(&W25Q_SPI, data, len, 5000);
}

void W25Q_Reset(void)
{
	uint8_t tData[2];

	tData[0] = 0x66;  // enable Reset
	tData[1] = 0x99;  // Reset
    csLOW();
	SPI_Write(tData, 2);
	csHIGH(); // pull the HIGH
        W25Q_Delay(100);
}

uint32_t W25QReadID(){
	uint8_t tData = 0x9f;  // Read ID
	uint8_t rData[3];
	csLOW();  // pull the CS LOW
	SPI_Write(&tData, 1);
	SPI_Read(rData, 3);
	csHIGH();  // pull the HIGH
	return ((rData[0]<<16)|(rData[1]<<8)|rData[2]);// MFN ID : MEM ID : CAPACITY ID
}

void W25Q_WriteData(uint32_t address, uint8_t* data, uint32_t length) {
    uint8_t cmd[4];
    cmd[0] = 0x02;                          // Lệnh Write Data
    cmd[1] = (address >> 16) & 0xFF;        // Byte cao của địa chỉ
    cmd[2] = (address >> 8) & 0xFF;         // Byte giữa của địa chỉ
    cmd[3] = address & 0xFF;                // Byte thấp của địa chỉ

    csLOW();                                // Kéo chân CS xuống
    SPI_Write(cmd, 4);                      // Gửi lệnh và địa chỉ
    SPI_Write(data, length);                // Gửi dữ liệu
    csHIGH();                               // Kết thúc ghi
}

void W25Q_ReadData(uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint8_t cmd[4];
    cmd[0] = 0x03;                          // Lệnh Read Data
    cmd[1] = (address >> 16) & 0xFF;        // Byte cao của địa chỉ
    cmd[2] = (address >> 8) & 0xFF;         // Byte giữa của địa chỉ
    cmd[3] = address & 0xFF;                // Byte thấp của địa chỉ

    csLOW();                                // Kéo chân CS xuống
    SPI_Write(cmd, 4);                      // Gửi lệnh và địa chỉ
    SPI_Read(buffer, length);               // Đọc dữ liệu
    csHIGH();                               // Kết thúc đọc
}



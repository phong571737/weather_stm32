#ifndef INC_W25QXX_H
#define INC_W25QXX_H

#define IMAGE_START_ADDRESS 0x90000000
#define IMAGE_WIDTH 240
#define IMAGE_HEIGHT 320

void csLOW(void);
void csHIGH(void);
void SPI_Write(uint8_t *data, uint8_t len);
void SPI_Read(uint8_t *data, uint8_t len);
void W25Q_Delay(uint32_t time);
void W25Q_ReadData(uint32_t address, uint8_t *buffer, uint32_t length);
uint32_t W25QReadID();
void W25Q_Reset(void);
void W25Q_WriteData(uint32_t address, uint8_t* data, uint32_t length);

#endif

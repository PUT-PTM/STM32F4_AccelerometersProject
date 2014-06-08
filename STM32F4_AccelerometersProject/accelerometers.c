#include "accelerometers.h"

__IO uint32_t SPITimeout = SPI_FLAG_TIMEOUT;

struct Connection Accelerometer_Init(enum Accel accel, enum Interface iface) {
	int success = 1;

	SystemInit();
	SystemCoreClockUpdate();

	switch (iface) {
	case I2C:
		I2C1_init();
		switch (accel) {
		case ADXL:
			I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);
			I2C_write(I2C1, ADXL_REGISTRY_DATA_FORMAT);
			I2C_write(I2C1, ADXL_RANGE_4G);
			I2C_stop(I2C1);
			I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);
			I2C_write(I2C1, ADXL_REGISTRY_POWER_CTL);
			I2C_write(I2C1, ADXL_LOWPOWERMODE_ACTIVE);
			I2C_stop(I2C1);
			break;
		case LIS:
			I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);
			I2C_write(I2C1, LIS_REGISTRY_CTRL);
			I2C_write(I2C1, (LIS_LOWPOWERMODE_ACTIVE | LIS_DATARATE_100 |
				LIS_X_ENABLE | LIS_Y_ENABLE | LIS_Z_ENABLE |
				LIS_FULLSCALE_2_3 | LIS_SELFTEST_NORMAL));
			I2C_stop(I2C1);
			break;
		default:
			success = 0;
		}
		break;
	case SPI:
		SPI_init();
		uint8_t ctrl;
		switch (accel) {
		case ADXL:
			ctrl = ADXL_RANGE_4G;
			SPI_Write(&ctrl, ADXL_REGISTRY_DATA_FORMAT, 1);
			ctrl = ADXL_LOWPOWERMODE_POWERDOWN;
			SPI_Write(&ctrl, ADXL_REGISTRY_POWER_CTL, 1);
			ctrl = ADXL_LOWPOWERMODE_ACTIVE;
			SPI_Write(&ctrl, ADXL_REGISTRY_POWER_CTL, 1);
			break;
		case LIS:
			ctrl = (LIS_LOWPOWERMODE_ACTIVE | LIS_DATARATE_100 |
				LIS_X_ENABLE | LIS_Y_ENABLE | LIS_Z_ENABLE |
				LIS_FULLSCALE_2_3 | LIS_SELFTEST_NORMAL);
			SPI_Write(&ctrl, LIS_REGISTRY_CTRL, 1);
			break;
		default:
			success = 0;
		}
		break;
	default:
		success = 0;
	}

	if (!success) {
		return (struct Connection){0};
	}
	return (struct Connection){1, accel, iface};
}

// SPI
void SPI_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef init_struct;
	init_struct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	init_struct.GPIO_Mode = GPIO_Mode_OUT;
	init_struct.GPIO_OType = GPIO_OType_PP;
	init_struct.GPIO_Speed = GPIO_Speed_100MHz;
	init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &init_struct);

	RCC_APB2PeriphClockCmd(SPI_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(SPI_SCK_GPIO_CLK | SPI_MISO_GPIO_CLK | SPI_MOSI_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(SPI_CS_GPIO_CLK, ENABLE);
	GPIO_PinAFConfig(SPI_SCK_GPIO_PORT, SPI_SCK_SOURCE, SPI_SCK_AF);
	GPIO_PinAFConfig(SPI_MISO_GPIO_PORT, SPI_MISO_SOURCE, SPI_MISO_AF);
	GPIO_PinAFConfig(SPI_MOSI_GPIO_PORT, SPI_MOSI_SOURCE, SPI_MOSI_AF);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = SPI_SCK_PIN;
	GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI_MOSI_PIN;
	GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
	GPIO_Init(SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	SPI_InitTypeDef  SPI_InitStructure;
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_CS_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN);
}

// I2C
void I2C1_init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1); // SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA

	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStruct);

	I2C_Cmd(I2C1, ENABLE);
}

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction) {
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2Cx, ENABLE);

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2Cx, address, direction);

	if (direction == I2C_Direction_Transmitter) {
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	} else if (direction == I2C_Direction_Receiver) {
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

void I2C_write(I2C_TypeDef* I2Cx, uint8_t data) {
	I2C_SendData(I2Cx, data);

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
}

int8_t I2C_read_ack(I2C_TypeDef* I2Cx) {
	I2C_AcknowledgeConfig(I2Cx, ENABLE);

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

	int8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

int8_t I2C_read_nack(I2C_TypeDef* I2Cx) {
	I2C_AcknowledgeConfig(I2Cx, DISABLE);

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

	int8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

void I2C_stop(I2C_TypeDef* I2Cx) {
	I2C_GenerateSTOP(I2Cx, ENABLE);

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t i2c_read_registry(uint8_t axes) {
	uint8_t temp;
	I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, axes);
	I2C_stop(I2C1);

	I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Receiver);
	temp = I2C_read_nack(I2C1);
	I2C_GenerateSTOP(I2C1, ENABLE);
	return temp;
}

// SPI WRITE
void SPI_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
	if (NumByteToWrite > 0x01) {
		WriteAddr |= (uint8_t)SPI_MULTIPLEBYTE_CMD;
	}
	SPI_CS_LOW();

	SPI_SendByte(WriteAddr);
	while (NumByteToWrite >= 0x01) {
		SPI_SendByte(*pBuffer);
		NumByteToWrite--;
		pBuffer++;
	}

	SPI_CS_HIGH();
}

// SPI SEND BYTE
static uint8_t SPI_SendByte(uint8_t byte)
{
	SPITimeout = SPI_FLAG_TIMEOUT;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
		if ((SPITimeout--) == 0)
			return SPI_TIMEOUT_UserCallback();
	}

	SPI_I2S_SendData(SPI1, byte);

	SPITimeout = SPI_FLAG_TIMEOUT;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback();
	}

	return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

// SPI READ
void SPI_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
	if(NumByteToRead > 0x01) {
		ReadAddr |= (uint8_t)(SPI_READWRITE_CMD | SPI_MULTIPLEBYTE_CMD);
	} else {
		ReadAddr |= (uint8_t)SPI_READWRITE_CMD;
	}

	SPI_CS_LOW();
	SPI_SendByte(ReadAddr);
	
	while(NumByteToRead > 0x00) {
		*pBuffer = SPI_SendByte(SPI_DUMMY_BYTE);
		NumByteToRead--;
		pBuffer++;
	}

	SPI_CS_HIGH();
}

// DELAY
void Delay(__IO uint32_t nCount)
{
	while(nCount--)
	{
	}
}

// SPI TIMEOUT
uint32_t SPI_TIMEOUT_UserCallback(void)
{
	while (1)
	{
	}
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{
	while (1)
	{
	}
}
#endif

struct Axes Accelerometer_readAxes(struct Connection conn) {
	if (!conn.success) {
		return (struct Axes){0, 0, 0, 0};
	}

	struct Axes axes = {0, 0, 0, 1};
	uint8_t data[6];
	uint8_t temp;

	switch (conn.accel) {
	case ADXL:
		switch (conn.iface) {
		case I2C:
			I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);
			I2C_write(I2C1, ADXL_REGISTRY_DATAX0);
			I2C_stop(I2C1);

			I2C_start(I2C1, I2C_SLAVE_ADDRESS, I2C_Direction_Receiver);
			data[0] = I2C_read_ack(I2C1);
			data[1] = I2C_read_ack(I2C1);
			data[2] = I2C_read_ack(I2C1);
			data[3] = I2C_read_ack(I2C1);
			data[4] = I2C_read_ack(I2C1);
			data[5] = I2C_read_nack(I2C1);
			axes.x = data[1] << 8 | data[0];
			axes.y = data[3] << 8 | data[2];
			axes.z = data[5] << 8 | data[4];
			I2C_GenerateSTOP(I2C1, ENABLE);
			break;
		case SPI:
			SPI_Read(data, ADXL_REGISTRY_DATAX0, 6);
			axes.x = data[1] << 8 | data[0];
			axes.y = data[3] << 8 | data[2];
			axes.z = data[5] << 8 | data[4];
			break;
		}
		break;
	case LIS:
		switch (conn.iface) {
		case I2C:
			axes.x = i2c_read_registry(LIS_REGISTRY_DATAX);
			axes.y = i2c_read_registry(LIS_REGISTRY_DATAY);
			axes.z = i2c_read_registry(LIS_REGISTRY_DATAZ);
			break;
		case SPI:
			SPI_Read(&temp, LIS_REGISTRY_DATAX, 1);
			axes.x = temp;
			SPI_Read(&temp, LIS_REGISTRY_DATAY, 1);
			axes.y = temp;
			SPI_Read(&temp, LIS_REGISTRY_DATAZ, 1);
			axes.z = temp;
			break;
		}
		break;
	}

	return axes;
}

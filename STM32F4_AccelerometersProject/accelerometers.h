#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_spi.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

/*
	I2C:
	    -- SDA - PB7
	    -- SCL - PB8

*/

#define I2C_SLAVE_ADDRESS 0x3A

#define SPI_FLAG_TIMEOUT	((uint32_t)0x1000)
#define SPI_CLK	RCC_APB2Periph_SPI1

#define SPI_SCK_PIN	GPIO_Pin_5	/* PA.05 */
#define SPI_SCK_GPIO_PORT	GPIOA	/* GPIOA */
#define SPI_SCK_GPIO_CLK	RCC_AHB1Periph_GPIOA
#define SPI_SCK_SOURCE	GPIO_PinSource5
#define SPI_SCK_AF	GPIO_AF_SPI1

#define SPI_MISO_PIN	GPIO_Pin_6	/* PA.6 */
#define SPI_MISO_GPIO_PORT	GPIOA	/* GPIOA */
#define SPI_MISO_GPIO_CLK	RCC_AHB1Periph_GPIOA
#define SPI_MISO_SOURCE	GPIO_PinSource6
#define SPI_MISO_AF	GPIO_AF_SPI1

#define SPI_MOSI_PIN	GPIO_Pin_7	/* PA.7 */
#define SPI_MOSI_GPIO_PORT	GPIOA	/* GPIOA */
#define SPI_MOSI_GPIO_CLK	RCC_AHB1Periph_GPIOA
#define SPI_MOSI_SOURCE	GPIO_PinSource7
#define SPI_MOSI_AF	GPIO_AF_SPI1

#define SPI_CS_PIN	GPIO_Pin_7	/* PE.07 */
#define SPI_CS_GPIO_PORT	GPIOE	/* GPIOE */
#define SPI_CS_GPIO_CLK	RCC_AHB1Periph_GPIOE

#define SPI_CS_LOW()	GPIO_ResetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)
#define SPI_CS_HIGH()	GPIO_SetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)
#define SPI_READWRITE_CMD	((uint8_t)0x80)
#define SPI_MULTIPLEBYTE_CMD	((uint8_t)0x40)
#define SPI_DUMMY_BYTE	((uint8_t)0x00)

#define ADXL_REGISTRY_POWER_CTL	0x2D
#define ADXL_REGISTRY_DATA_FORMAT	0x31
#define ADXL_REGISTRY_DATAX0	0x32
#define ADXL_REGISTRY_DATAX1	0x33
#define ADXL_REGISTRY_DATAY0	0x34
#define ADXL_REGISTRY_DATAY1	0x35
#define ADXL_REGISTRY_DATAZ0	0x36
#define ADXL_REGISTRY_DATAZ1	0x37

#define ADXL_LOWPOWERMODE_POWERDOWN	((uint8_t)0x00)
#define ADXL_LOWPOWERMODE_ACTIVE	((uint8_t)0x08)
#define ADXL_RANGE_2G	((uint8_t)0x00)
#define ADXL_RANGE_4G	((uint8_t)0x01)
#define ADXL_RANGE_8G	((uint8_t)0x02)
#define ADXL_RANGE_16G	((uint8_t)0x03)

#define LIS_REGISTRY_CTRL	0x20
#define LIS_REGISTRY_DATAX	0x29
#define LIS_REGISTRY_DATAY	0x2B
#define LIS_REGISTRY_DATAZ	0x2D

#define LIS_DATARATE_100	((uint8_t)0x00)
#define LIS_DATARATE_100	((uint8_t)0x00)
#define LIS_LOWPOWERMODE_POWERDOWN	((uint8_t)0x00)
#define LIS_LOWPOWERMODE_ACTIVE	((uint8_t)0x40)
#define LIS_FULLSCALE_2_3	((uint8_t)0x00)
#define LIS_FULLSCALE_9_2	((uint8_t)0x20)
#define LIS_SELFTEST_NORMAL	((uint8_t)0x00)
#define LIS_SELFTEST_P	((uint8_t)0x10)
#define LIS_SELFTEST_M	((uint8_t)0x08)
#define LIS_X_ENABLE	((uint8_t)0x01)
#define LIS_Y_ENABLE	((uint8_t)0x02)
#define LIS_Z_ENABLE	((uint8_t)0x04)

enum Accel { LIS, ADXL };
enum Interface { SPI, I2C };

struct Connection {
	int success;
	enum Accel accel;
	enum Interface iface;
};

struct Axes {
	u16 x, y, z;
	int success;
};

struct Connection Accelerometer_Init(enum Accel accel, enum Interface iface);
struct Axes Accelerometer_readAxes(struct Connection conn);

// I2C
void I2C1_init(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
int8_t I2C_read_ack(I2C_TypeDef* I2Cx);
int8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);

// SPI
void SPI_init(void);
void SPI_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
static uint8_t SPI_SendByte(uint8_t byte);
void SPI_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI_TIMEOUT_UserCallback(void);
void Delay(__IO uint32_t nCount);

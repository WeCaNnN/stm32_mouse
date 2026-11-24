#include "pixart_port.h"
#include "gpio.h"
#include "delay.h"
#include "spi.h"
#include "user_config.h"

/* 本文件日志标签：必须在 elog.h 之前定义，log_a/e/w/i/d/v 会自动带上它 */
#define LOG_TAG     "pixart_port"
#include <elog.h>

#define SENSOR_CS_LOW   HAL_GPIO_WritePin(Sensor_CS_GPIO_Port, Sensor_CS_Pin, GPIO_PIN_RESET)
#define SENSOR_CS_HIGH  HAL_GPIO_WritePin(Sensor_CS_GPIO_Port, Sensor_CS_Pin, GPIO_PIN_SET)

/**
 * @brief 初始化传输层：CS 收回总线空闲态
 *
 * @return 无
 *
 * @note 系统外设初始化完毕后调用
 */
void pixart_port_init(void)
{
    SENSOR_CS_HIGH;
}

/**
 * @brief 写一个寄存器：地址字节（bit7=1 写方向）与数据字节连续发出
 *
 * @param[in] addr  寄存器地址（7 位）
 * @param[in] value 待写入的字节
 *
 * @return 无；SPI 失败仅打日志
 *
 */
void pixart_reg_write(uint8_t addr, uint8_t value)
{
    uint8_t frame[2] = {0};
    frame[0] = addr | PIXART_ADDR_WRITE_BIT;
    frame[1] = value;

    SENSOR_CS_LOW;
    delay_us(PIXART_T_NCS_SCLK_US);
    if (HAL_SPI_Transmit(&hspi1, frame, 2, PIXART_T_TIMEOUT) != HAL_OK)
    {
        #if PIXART_PORT_DEBUG
        log_e("spi write fail addr=0x%02X", (unsigned int)addr);
        #endif
    }
    delay_us(PIXART_T_SCLK_NCS_US);  
    SENSOR_CS_HIGH;
    delay_us(PIXART_T_SWW_US);        
}

/**
 * @brief 读一个寄存器：连续读长度 1 的特例
 *
 * @param[in] addr 寄存器地址（7 位）
 *
 * @return 寄存器值；SPI 失败时返回 0
 */
uint8_t pixart_reg_read(uint8_t addr)
{
    uint8_t value = 0U;
    pixart_seq_read(addr, &value, 1);
    return value;
}

/**
 * @brief 从指定寄存器起连续读 len 字节（Motion Burst 的传输形式）
 *
 * 地址后等一次 tSRAD，之后数据字节连续时钟、字节间不延时
 *
 * @param[in]  addr 寄存器起始地址（7 位）
 * @param[out] buf  数据缓冲区，长度 >= len
 * @param[in]  len  读取字节数，1~PIXART_SEQ_READ_MAX_LEN
 *
 * @return 无；参数非法或 SPI 失败仅打日志
 */
void pixart_seq_read(uint8_t addr, uint8_t *buf, uint8_t len)
{
    uint8_t addr_temp = addr;
    addr_temp = addr_temp & PIXART_ADDR_READ_MASK;   /* 清 bit7：读方向，防上层带错位 */


    if ((buf == NULL) || (len == 0U) || (len > PIXART_SEQ_READ_MAX_LEN))
    {
        #if PIXART_PORT_DEBUG
        log_e("seq_read param err len=%u", (unsigned int)len);
        #endif
        return;
    }

    SENSOR_CS_LOW;
    delay_us(PIXART_T_NCS_SCLK_US);

    if (HAL_SPI_Transmit(&hspi1, &addr_temp, 1, PIXART_T_TIMEOUT) != HAL_OK)
    {
        #if PIXART_PORT_DEBUG
        log_e("seq_read addr fail addr=0x%02X", (unsigned int)addr);
        #endif
        SENSOR_CS_HIGH;   /* 失败释放总线 */
        return;
    }
    delay_us(PIXART_T_SRAD_US);

    if (HAL_SPI_Receive(&hspi1, buf, len, PIXART_T_TIMEOUT) != HAL_OK)
    {
        #if PIXART_PORT_DEBUG
        log_e("seq_read data fail addr=0x%02X", (unsigned int)addr);
        #endif
    }
    SENSOR_CS_HIGH;
    delay_us(PIXART_T_SRR_US);
}

#include "sensor.h"
#include "pixart_port.h"
#include "user_config.h"
#include "gpio.h"

/* 本文件日志标签：必须在 elog.h 之前定义，log_i/e 会自动带上它 */
#define LOG_TAG     "paw3395"
#include <elog.h>

#define SENSOR_RESET_HIGH   HAL_GPIO_WritePin(Sensor_Reset_GPIO_Port, Sensor_Reset_Pin, GPIO_PIN_SET)
#define SENSOR_RESET_LOW    HAL_GPIO_WritePin(Sensor_Reset_GPIO_Port, Sensor_Reset_Pin, GPIO_PIN_RESET)

/**
 * @brief 初始化 PAW3395 传感器：复位脉冲、上电序列、ID 双重校验
 *
 *
 * @return true  传感器在线（ID 双校验通过：0x00=0x51 且 0x5F=0xAE）
 * @return false 校验失败或 SPI 通信异常
 */
bool sensor_paw_3395_init(void)
{
    bool ret = false;
    uint8_t id = 0U;
    uint8_t inverse_id = 0U;
    uint8_t reg;

    pixart_port_init();


    SENSOR_RESET_LOW;
    HAL_Delay(10);
    SENSOR_RESET_HIGH;
    HAL_Delay(100);   
    HAL_Delay(50);    

    pixart_reg_write(POWER_UP_RESET_CMD, POWER_UP_RESET_DATA);
    HAL_Delay(5);   
    /* 无论运动位状态如何，读 0x02~0x06 各一次清残留 datasheet要求 */
    for (reg = 0x02U; reg <= 0x06U; reg++)
    {
        (void)pixart_reg_read(reg);
    }

    id = pixart_reg_read(PAW3395_REG_PRODUCT_ID);
    inverse_id = pixart_reg_read(PAW3395_REG_PRODUCT_ID_INV);

    #if SENSOR_INIT_DEBUG
    log_i("paw3395 id=0x%02X iid=0x%02X", (unsigned int)id, (unsigned int)inverse_id);
    #endif

    
    ret = (id == PAW3395_ID_VAL) && (inverse_id == PAW3395_ID_INV_VAL);
    return ret;
}

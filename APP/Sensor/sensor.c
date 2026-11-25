#include "sensor.h"
#include "pixart_port.h"
#include "user_config.h"
#include "gpio.h"

/* 本文件日志标签：必须在 elog.h 之前定义，log_i/e 会自动带上它 */
#define LOG_TAG     "sensor"
#include <elog.h>


#define PAW3395_REG_MOTION_BURST  0x16U  /* Motion Burst 寄存器：读它启动连续报文输出 */
#define PAW3395_MOTION_BIT        0x80U  /* Burst BYTE[0] bit7：1=有新位移数据 */


static int16_t acc_x = 0;
static int16_t acc_y = 0;
/**
 * @brief 传感器位移采集任务：读 Motion Burst 并累加 X/Y 增量
 * @return 无
 */
void sensor_motion_task(void)
{
    uint8_t data_buf[6];   /*暂时只读6个byte*/

    pixart_seq_read(PAW3395_REG_MOTION_BURST, data_buf, 6);

    if ((data_buf[0] & PAW3395_MOTION_BIT) != 0U)   /* Motion 位：有新位移才有效 */
    {
        int16_t delta_x = (int16_t)(((uint16_t)data_buf[3] << 8) | (uint16_t)data_buf[2]);   /* Delta_X_L/H 拼装 */
        int16_t delta_y = (int16_t)(((uint16_t)data_buf[5] << 8) | (uint16_t)data_buf[4]);   /* Delta_Y_L/H 拼装 */

        acc_x += delta_x;
        acc_y += delta_y;

        #if SENSOR_DATA_DEBUG
        log_i("dx=%d dy=%d", (int)delta_x, (int)delta_y);
        #endif
    }
}

/**
 * @brief 取走位移累加值并清零
 *
 * @param[out] pX X 轴累加位移（可为负）；不允许 NULL
 * @param[out] pY Y 轴累加位移（可为负）；不允许 NULL
 *
 * @return 无
 */
void sensor_XY_data_get(int16_t *pX, int16_t *pY)
{
    if ((pX == NULL) || (pY == NULL))
    {
        #if SENSOR_DATA_DEBUG
        log_e("ERRO_sensor_XY_data_get");
        #endif
        return;
    }
    *pX = acc_x;
    *pY = acc_y;
    acc_x = 0;   /* 取走即清零：位移消费完毕，防止历史值重复上报 */
    acc_y = 0;
}

/**
 * @brief 回写位移增量
 *
 * @param[in] lastX 上次取走的 X 轴位移
 * @param[in] lastY 上次取走的 Y 轴位移
 *
 * @return 无
 */
void sensor_XY_data_acc(int16_t lastX, int16_t lastY)
{
    acc_x = acc_x + lastX;
    acc_y = acc_y + lastY;
}

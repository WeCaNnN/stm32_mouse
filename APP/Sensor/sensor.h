#ifndef __SENSOR__
#define __SENSOR__

#include <stdbool.h>
#include <stdint.h>

#define DPI_MIN 50
#define DPI_MAX 26000
/* ===== PAW3395 上电序列寄存器 ===== */
#define POWER_UP_RESET_CMD   0x3AU  /* Power_Up_Reset 寄存器地址 */
#define POWER_UP_RESET_DATA  0x5AU  /* 写入值：上电复位命令 */

/* ===== 身份寄存器与期望值 ===== */
#define PAW3395_REG_PRODUCT_ID      0x00U  /* Product ID 寄存器地址 */
#define PAW3395_REG_PRODUCT_ID_INV  0x5FU  /* Inverse Product ID 寄存器地址 */
#define PAW3395_ID_VAL              0x51U  /* Product ID 期望值 */
#define PAW3395_ID_INV_VAL          0xAEU  /* Inverse ID 期望值 = ~0x51 */


#define PAW3395_MOTION_CTRL         0x5CU //MOTION_CTRL寄存器
#define PAW3395_RESOLUTION_X_LOW    0x48U //x轴分辨率
#define PAW3395_RESOLUTION_X_HIGH   0x49U
#define PAW3395_SET_RESOLUTION      0x47U //提交/生效寄存器

#define PAW3395_RES_MOD_SHARED     0x00U  /* MOTION_CTRL 值：RES_MOD=0，X/Y 共用 X 分辨率寄存器 */ //bit7 MOT_set Motion脚的有效电平 0=低有效
                                                                                                 //bit2 RES_MOD 0=X/Y 共用 
                                                                                                 //RESOLUTION_X 一组寄存器；1=X/Y 各自用 X/Y 两组。复位值 0x02 → RES_MOD=1（各自）
#define PAW3395_RESOLUTION_APPLY   0x01U  /* SET_RESOLUTION 值：提交命令，新分辨率装载生效 */

bool sensor_paw_3395_init(void);
void sensor_motion_task(void);
void sensor_XY_data_get(int16_t *pX, int16_t *pY);
void sensor_XY_data_acc(int16_t lastX, int16_t lastY);
void sensor_dpi_config(uint16_t dpi);
#endif

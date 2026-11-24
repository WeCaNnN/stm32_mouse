#ifndef __SENSOR__
#define __SENSOR__

#include <stdbool.h>
#include <stdint.h>

/* ===== PAW3395 上电序列寄存器 ===== */
#define POWER_UP_RESET_CMD   0x3AU  /* Power_Up_Reset 寄存器地址 */
#define POWER_UP_RESET_DATA  0x5AU  /* 写入值：上电复位命令 */

/* ===== 身份寄存器与期望值 ===== */
#define PAW3395_REG_PRODUCT_ID      0x00U  /* Product ID 寄存器地址 */
#define PAW3395_REG_PRODUCT_ID_INV  0x5FU  /* Inverse Product ID 寄存器地址 */
#define PAW3395_ID_VAL              0x51U  /* Product ID 期望值 */
#define PAW3395_ID_INV_VAL          0xAEU  /* Inverse ID 期望值 = ~0x51 */

bool sensor_paw_3395_init(void);

#endif

#ifndef __PIXART_PORT__
#define __PIXART_PORT__

#include <stdint.h>

/* ===== 串口协议地址字节格式===== */
#define PIXART_ADDR_WRITE_BIT   0x80U   /* 地址字节 bit7：1=写方向，0=读方向 */
#define PIXART_ADDR_READ_MASK   0x7FU   /* 方向位掩码：清 bit7 得 7 位纯地址 */

/* ===== 手册 Table 4 命令级时序下限（全为最小值，过等无害，单位 µs）===== */
#define PIXART_T_NCS_SCLK_US   1     /* NCS↓ → 首时钟，手册 120ns */
#define PIXART_T_SCLK_NCS_US   1     /* 末时钟 → NCS↑（写操作），手册 1µs */
#define PIXART_T_SRAD_US       2     /* 读地址 → 数据可读 */
#define PIXART_T_SWW_US        5     /* 写 → 下一命令（写尾延时用它，兼满足 tSWR） */
#define PIXART_T_SWR_US        5     /* 写 → 读（与 SWW 同值，写尾延时已一并覆盖） */
#define PIXART_T_SRR_US        2     /* 读 → 下一命令，兼覆盖 burst 收尾 tEXIT 500ns */

/* ===== 限制 ===== */
#define PIXART_T_TIMEOUT        10   /* SPI 传输超时，单位 ms：12 字节 @9MHz 实际 ~11µs，百倍余量 */
#define PIXART_SEQ_READ_MAX_LEN 12   /* 连续读长度上限：Motion Burst 报文 BYTE[0]~[11] */

#define SENSOR_CS_LOW   HAL_GPIO_WritePin(Sensor_CS_GPIO_Port, Sensor_CS_Pin, GPIO_PIN_RESET)
#define SENSOR_CS_HIGH  HAL_GPIO_WritePin(Sensor_CS_GPIO_Port, Sensor_CS_Pin, GPIO_PIN_SET)


void    pixart_port_init(void);
void    pixart_reg_write(uint8_t addr, uint8_t value);
uint8_t pixart_reg_read(uint8_t addr);
void    pixart_seq_read(uint8_t addr, uint8_t *buf, uint8_t len);

#endif /* __PIXART_PORT__ */

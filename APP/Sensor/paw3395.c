#include "sensor.h"
#include "pixart_port.h"
#include "delay.h"
#include "user_config.h"
#include "gpio.h"

/* 本文件日志标签：必须在 elog.h 之前定义，log_i/e 会自动带上它 */
#define LOG_TAG     "paw3395"
#include <elog.h>

/* ===== OTP 装载握手（手册 6.2 步骤 138~139）===== */
#define PAW3395_REG_BANK_SEL     0x7FU  /* 页选择寄存器：写入页号切换寄存器页 */
#define PAW3395_REG_OTP_STATUS   0x6CU  /* OTP 装载完成状态：读到 0x80 表示装载完毕 */
#define PAW3395_OTP_DONE         0x80U  /* OTP 装载完成标志值 */
#define PAW3395_OTP_POLL_MAX     60U    /* 轮询上限：1ms 间隔 × 60 = 最多等 60ms */
#define PAW3395_BANK_FAIL        0x14U  /* 装载失败分支切到的页号（手册步骤 139） */

#define SENSOR_RESET_HIGH   HAL_GPIO_WritePin(Sensor_Reset_GPIO_Port, Sensor_Reset_Pin, GPIO_PIN_SET)
#define SENSOR_RESET_LOW    HAL_GPIO_WritePin(Sensor_Reset_GPIO_Port, Sensor_Reset_Pin, GPIO_PIN_RESET)

static void paw3395_reg_config(void);

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

    /* NRESET 脉冲：低 10ms 让 C19 放电充分，拉高后等 RC（10k×10µF≈100ms）
     * 充电完成芯片才真正退出复位；再等 50ms 为手册 6.1 步骤 2 */
    SENSOR_RESET_LOW;
    HAL_Delay(10);
    SENSOR_RESET_HIGH;
    HAL_Delay(100);
    HAL_Delay(50);

    /* 手册 6.1 步骤 4/5：写 Power_Up_Reset 后等 5ms */
    pixart_reg_write(POWER_UP_RESET_CMD, POWER_UP_RESET_DATA);
    HAL_Delay(5);

    /* 手册 6.1 步骤 6：装载上电初始化寄存器（含 OTP 握手） */
    paw3395_reg_config();

    /* 手册 6.1 步骤 7：无论运动位状态如何，读 0x02~0x06 各一次清残留 */
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

/**
 * @brief 装载上电初始化寄存器配置
 * @return 无
 */
static void paw3395_reg_config(void)
{
    uint8_t otp_status = 0U;
    uint8_t poll_cnt = 0U;

    /* 6.2 #1~2：页 07，疑似阶段命令 */
    pixart_reg_write(0x7F, 0x07);
    pixart_reg_write(0x40, 0x41);

    /* 6.2 #3~4：页 00，0x40=0x80 疑似启动装载 */
    pixart_reg_write(0x7F, 0x00);
    pixart_reg_write(0x40, 0x80);

    /* 6.2 #5~9：页 0E，四连字节 0D 1B E8 D5，疑似解锁魔数 */
    pixart_reg_write(0x7F, 0x0E);
    pixart_reg_write(0x55, 0x0D);
    pixart_reg_write(0x56, 0x1B);
    pixart_reg_write(0x57, 0xE8);
    pixart_reg_write(0x58, 0xD5);

    /* 6.2 #10~15：页 14，装载控制参数 */
    pixart_reg_write(0x7F, 0x14);
    pixart_reg_write(0x42, 0xBC);
    pixart_reg_write(0x43, 0x74);
    pixart_reg_write(0x4B, 0x20);
    pixart_reg_write(0x4D, 0x00);
    pixart_reg_write(0x53, 0x0E);

    /* 6.2 #16~30：页 05，最大配置块之一 */
    pixart_reg_write(0x7F, 0x05);
    pixart_reg_write(0x44, 0x04);
    pixart_reg_write(0x4D, 0x06);
    pixart_reg_write(0x51, 0x40);
    pixart_reg_write(0x53, 0x40);
    pixart_reg_write(0x55, 0xCA);
    pixart_reg_write(0x5A, 0xE8);
    pixart_reg_write(0x5B, 0xEA);
    pixart_reg_write(0x61, 0x31);
    pixart_reg_write(0x62, 0x64);
    pixart_reg_write(0x6D, 0xB8);
    pixart_reg_write(0x6E, 0x0F);
    pixart_reg_write(0x70, 0x02);
    pixart_reg_write(0x4A, 0x2A);
    pixart_reg_write(0x60, 0x26);

    /* 6.2 #31~38：页 06 */
    pixart_reg_write(0x7F, 0x06);
    pixart_reg_write(0x6D, 0x70);
    pixart_reg_write(0x6E, 0x60);
    pixart_reg_write(0x6F, 0x04);
    pixart_reg_write(0x53, 0x02);
    pixart_reg_write(0x55, 0x11);
    pixart_reg_write(0x7A, 0x01);   /* 手册 6.2 #37 此处为 0x74=0x01；按参考工程取值 */
    pixart_reg_write(0x7D, 0x51);

    /* 6.2 #39~43：回页 07 */
    pixart_reg_write(0x7F, 0x07);
    pixart_reg_write(0x41, 0x10);
    pixart_reg_write(0x42, 0x32);
    pixart_reg_write(0x43, 0x00);

    /* 6.2 #44：页 08 */
    pixart_reg_write(0x7F, 0x08);
    pixart_reg_write(0x71, 0x4F);

    /* 6.2 #45~61：页 09，0x1F/0x03 成组交替，疑似阈值表 */
    pixart_reg_write(0x7F, 0x09);
    pixart_reg_write(0x62, 0x1F);
    pixart_reg_write(0x63, 0x1F);
    pixart_reg_write(0x65, 0x03);
    pixart_reg_write(0x66, 0x03);
    pixart_reg_write(0x67, 0x1F);
    pixart_reg_write(0x68, 0x1F);
    pixart_reg_write(0x69, 0x03);
    pixart_reg_write(0x6A, 0x03);
    pixart_reg_write(0x6C, 0x1F);
    pixart_reg_write(0x6D, 0x1F);
    pixart_reg_write(0x51, 0x04);
    pixart_reg_write(0x53, 0x20);
    pixart_reg_write(0x54, 0x20);
    pixart_reg_write(0x71, 0x0C);
    pixart_reg_write(0x72, 0x07);
    pixart_reg_write(0x73, 0x07);

    /* 6.2 #62~65：页 0A */
    pixart_reg_write(0x7F, 0x0A);
    pixart_reg_write(0x4A, 0x14);
    pixart_reg_write(0x4C, 0x14);
    pixart_reg_write(0x55, 0x19);

    /* 6.2 #66~71：回页 14 */
    pixart_reg_write(0x7F, 0x14);
    pixart_reg_write(0x4B, 0x30);
    pixart_reg_write(0x4C, 0x03);
    pixart_reg_write(0x61, 0x0B);
    pixart_reg_write(0x62, 0x0A);
    pixart_reg_write(0x63, 0x02);

    /* 6.2 #72~76：页 15 */
    pixart_reg_write(0x7F, 0x15);
    pixart_reg_write(0x4C, 0x02);
    pixart_reg_write(0x56, 0x02);
    pixart_reg_write(0x41, 0x91);
    pixart_reg_write(0x4D, 0x0A);

    /* 6.2 #77~92：页 0C */
    pixart_reg_write(0x7F, 0x0C);
    pixart_reg_write(0x4A, 0x10);
    pixart_reg_write(0x4B, 0x0C);
    pixart_reg_write(0x4C, 0x40);   /* 手册 6.2 #80 此处为 0x0C；按参考工程取值 */
    pixart_reg_write(0x41, 0x25);
    pixart_reg_write(0x55, 0x18);
    pixart_reg_write(0x56, 0x14);
    pixart_reg_write(0x49, 0x0A);
    pixart_reg_write(0x42, 0x00);
    pixart_reg_write(0x43, 0x2D);
    pixart_reg_write(0x44, 0x0C);
    pixart_reg_write(0x54, 0x1A);
    pixart_reg_write(0x5A, 0x0D);
    pixart_reg_write(0x5F, 0x1E);
    pixart_reg_write(0x5B, 0x05);
    pixart_reg_write(0x5E, 0x0F);

    /* 6.2 #93~115：页 0D，最长段，(值,0x00) 成对，疑似快门/曝光校准组 */
    pixart_reg_write(0x7F, 0x0D);
    pixart_reg_write(0x48, 0xDD);
    pixart_reg_write(0x4F, 0x03);
    pixart_reg_write(0x52, 0x49);
    pixart_reg_write(0x51, 0x00);
    pixart_reg_write(0x54, 0x5B);
    pixart_reg_write(0x53, 0x00);
    pixart_reg_write(0x56, 0x64);
    pixart_reg_write(0x55, 0x00);
    pixart_reg_write(0x58, 0xA5);
    pixart_reg_write(0x57, 0x02);
    pixart_reg_write(0x5A, 0x29);
    pixart_reg_write(0x5B, 0x47);
    pixart_reg_write(0x5C, 0x81);
    pixart_reg_write(0x5D, 0x40);
    pixart_reg_write(0x71, 0xDC);
    pixart_reg_write(0x70, 0x07);
    pixart_reg_write(0x73, 0x00);
    pixart_reg_write(0x72, 0x08);
    pixart_reg_write(0x75, 0xDC);
    pixart_reg_write(0x74, 0x07);
    pixart_reg_write(0x77, 0x00);
    pixart_reg_write(0x76, 0x08);

    /* 6.2 #116~117：页 10 */
    pixart_reg_write(0x7F, 0x10);
    pixart_reg_write(0x4C, 0xD0);

    /* 6.2 #118~137：回公共页 00 收尾；#137 的 0x22=0x01 疑似"启动 OTP 装载"命令 */
    pixart_reg_write(0x7F, 0x00);
    pixart_reg_write(0x4F, 0x63);
    pixart_reg_write(0x4E, 0x00);
    pixart_reg_write(0x52, 0x63);
    pixart_reg_write(0x51, 0x00);
    pixart_reg_write(0x54, 0x54);
    pixart_reg_write(0x5A, 0x10);
    pixart_reg_write(0x77, 0x4F);
    pixart_reg_write(0x47, 0x01);
    pixart_reg_write(0x5B, 0x40);
    pixart_reg_write(0x64, 0x60);
    pixart_reg_write(0x65, 0x06);
    pixart_reg_write(0x66, 0x13);
    pixart_reg_write(0x67, 0x0F);
    pixart_reg_write(0x78, 0x01);
    pixart_reg_write(0x79, 0x9C);
    pixart_reg_write(0x40, 0x00);
    pixart_reg_write(0x55, 0x02);
    pixart_reg_write(0x23, 0x70);
    pixart_reg_write(0x22, 0x01);

    /* 6.2 #138：等 1ms（DWT 忙等，满足手册 139 步 ±1% 的间隔容差要求） */
    delay_us(1000);

    /* 6.2 #139：1ms 间隔轮询 OTP 状态直到装载完成（0x80），最多 60 次 */
    for (poll_cnt = 0U; poll_cnt < PAW3395_OTP_POLL_MAX; poll_cnt++)
    {
        otp_status = pixart_reg_read(PAW3395_REG_OTP_STATUS);
        if (otp_status == PAW3395_OTP_DONE)
        {
            #if SENSOR_INIT_DEBUG
            log_i("paw3395 reg init success");
            #endif
            break;
        }
        delay_us(1000);
    }

    /* 装载失败：按手册失败分支切页清状态后继续收尾（芯片以未完整装载状态运行） */
    if (poll_cnt == PAW3395_OTP_POLL_MAX)
    {
        #if SENSOR_INIT_DEBUG
        log_e("paw3395 reg init error !");
        #endif
        pixart_reg_write(PAW3395_REG_BANK_SEL, PAW3395_BANK_FAIL);
        pixart_reg_write(PAW3395_REG_OTP_STATUS, 0x00);
        pixart_reg_write(PAW3395_REG_BANK_SEL, 0x00);
    }

    /* 6.2 #140~144：收尾，末条确认回到页 00（后续 ID 读取不受换页影响） */
    pixart_reg_write(0x22, 0x00);
    pixart_reg_write(0x55, 0x00);
    pixart_reg_write(0x7F, 0x07);
    pixart_reg_write(0x40, 0x40);
    pixart_reg_write(0x7F, 0x00);
}

#include "delay.h"
#include "stm32f1xx_hal.h"   /* DWT/CoreDebug/SystemCoreClock：内核 CMSIS 定义，随 HAL 头引入 */

/**
 * @brief 忙等指定 CPU 周期数（含循环自身开销，实际等待 >= cycles）
 *
 * @param[in] cycles 最少等待的 CPU 周期数
 *
 * @return 无
 *
 * @note 无符号减法使 32 位回绕不影响差值，CYCCNT 使能后永不清零、自由运行
 */
static void delay_wait_cycles(uint32_t cycles)
{
    uint32_t start_cycles = DWT->CYCCNT;
    while ((DWT->CYCCNT - start_cycles) < cycles)
    {
        /* 空转：时序约束全为最小值型，过等无害 */
    }
}

void delay_init(void)
{
    /* DWT 是内核调试单元：先开 TRCENA 总开关，再使能 CYCCNT 计数 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    /* 懒使能兜底：CYCCNT 不走数时差值恒为 0，忙等会死循环——
     * 一行检查换掉"忘调 delay_init 就挂死"的隐患 */
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        delay_init();
    }

    /* SystemCoreClock/1e6 = 每 µs 周期数（72MHz 下为 72，整除无误差），
     * 变量现算不硬编码，时钟改动自动跟随 */
    delay_wait_cycles(us * (SystemCoreClock / 1000000U));
}

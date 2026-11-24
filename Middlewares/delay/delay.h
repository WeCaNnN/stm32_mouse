#ifndef __DELAY_H__
#define __DELAY_H__

#include <stdint.h>


/**
 * @brief 初始化延时模块：使能 DWT 周期计数器
 *
 * @return 无
 *
 * @note 跳过本函数直接调 delay_us 也不会死等（函数内带懒使能兜底），
 *       但正常初始化流程应在 main 里显式调用一次
 */
void delay_init(void);

/**
 * @brief 忙等指定微秒数
 *
 * 实际等待 >= us：等待期间被中断抢占只会拉长等待，方向安全。
 *
 * @param[in] us 等待时长，单位微秒。内部换算 us * (SystemCoreClock/1e6) 在
 *               us 过大时会 uint32 溢出变短（72MHz 下约 >5900 万），
 *               本接口定位 µs 级等待，勿用作秒级延时
 *
 * @return 无
 */
void delay_us(uint32_t us);

#endif /* __DELAY_H__ */

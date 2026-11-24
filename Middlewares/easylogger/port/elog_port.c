/**
 * @file elog_port.c
 * @brief EasyLogger 移植层：日志输出后端接 SEGGER RTT 通道 0，不占用串口
 *
 * 之所以选 RTT：SWD 调试口本来就要接，RTT 通过内存缓冲区传输，不占引脚、
 * 不依赖波特率，也不会像阻塞式串口那样拖慢中断。
 */
#include <elog.h>
#include "stm32f1xx.h"   /* __get_PRIMASK / __set_PRIMASK */
#include "SEGGER_RTT.h"

/* 输出锁的嵌套计数与加锁前的 PRIMASK 值：EasyLogger 在主循环和中断里都会被调用，
   用计数保存/恢复，保证嵌套调用时不会提前打开中断 */
static uint32_t s_output_lock_nest = 0U;
static uint32_t s_primask_before_lock = 0U;

/**
 * @brief EasyLogger 移植层初始化：启动 RTT
 *
 * @return ELOG_NO_ERR 恒成功
 */
ElogErrCode elog_port_init(void)
{
    SEGGER_RTT_Init();
    return ELOG_NO_ERR;
}

/**
 * @brief EasyLogger 移植层反初始化（RTT 无需释放资源，空实现）
 */
void elog_port_deinit(void)
{
}

/**
 * @brief 输出一条已格式化的日志，写入 RTT 上行通道 0
 *
 * @param[in] log  已格式化的日志字符串，以 '\0' 结尾，可为非 NUL 结尾的定长片段
 * @param[in] size 日志长度（字节）
 */
void elog_port_output(const char *log, size_t size)
{
    SEGGER_RTT_Write(0, log, size);
}

/**
 * @brief 加输出锁：关中断，防止主循环与 USB 中断的日志互相穿插
 */
void elog_port_output_lock(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    if (s_output_lock_nest == 0U)
    {
        s_primask_before_lock = primask;
    }
    s_output_lock_nest++;
}

/**
 * @brief 解输出锁：最外层调用时恢复加锁前的中断状态
 */
void elog_port_output_unlock(void)
{
    if (s_output_lock_nest > 0U)
    {
        s_output_lock_nest--;
        if (s_output_lock_nest == 0U)
        {
            __set_PRIMASK(s_primask_before_lock);
        }
    }
}

/**
 * @brief 返回时间戳字符串（本工程未启用时间格式，返回空串）
 *
 * 各级别均未使能 ELOG_FMT_TIME（见 main.c 的 app_log_init），elog.c 不会调用到本函数；
 * 保留空实现以满足移植接口。
 *
 * @return 空字符串
 */
const char *elog_port_get_time(void)
{
    return "";
}

/**
 * @brief 返回进程名（无 RTOS，返回空串）
 *
 * @return 空字符串
 */
const char *elog_port_get_p_info(void)
{
    return "";
}

/**
 * @brief 返回线程名（无 RTOS，返回空串）
 *
 * @return 空字符串
 */
const char *elog_port_get_t_info(void)
{
    return "";
}

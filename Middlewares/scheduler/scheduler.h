/**
 ******************************************************************************
 * @file    scheduler.h
 * @brief   简单时间片轮询任务调度器
 ******************************************************************************
 * 
 * 所有 API 只允许在主循环上下文调用，不支持在中断里增删/启停任务。
 *
 * 典型用法：
 *          scheduler_register(led_task, 500);   // 初始化阶段注册任务
 *          while (1) { scheduler_run(); }       // 主循环只调这一个函数
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported types ------------------------------------------------------------*/
/**
 * @brief 调度器任务回调函数类型：无参数、无返回值
 */
typedef void (*task_fn_t)(void);

/* Exported functions --------------------------------------------------------*/
void scheduler_reset(void);
uint8_t scheduler_register(task_fn_t callback, uint32_t interval_time);
void scheduler_enable(uint8_t task_id);
void scheduler_disable(uint8_t task_id);
void scheduler_run(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __SCHEDULER_H__ */

/*-------------------------------End of File----------------------------------*/

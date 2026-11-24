/**
 ******************************************************************************
 * @file    scheduler.c
 * @brief   简单时间片轮询任务调度器
 ******************************************************************************
 * HAL_GetTick()作为时间基（HAL_Init 已把 SysTick 配为 1ms）。主循环里反复调
 * scheduler_run()，到点的任务回调被依次执行；回调执行时间应远小于最小任务
 * 周期，否则其他任务会被拖慢（本调度器无抢占、无优先级，按注册顺序轮询）。
 *
 * 注意：所有 API 只在主循环上下文使用，不支持中断里增删/启停任务（若要在
 * 中断里启停任务，需把任务表访问包进临界区后再放开限制）。
 ******************************************************************************
 */
#include "scheduler.h"

/* Private define ------------------------------------------------------------*/

#define SCH_TASK_MAX    (15U)

/* Private typedef -----------------------------------------------------------*/
/* 调度器任务状态：注册后先 READY，首个周期到点转 RUN 开始周期执行；disable 转 STOP */
typedef enum
{
    SCH_READY = 0,   /* 已注册，等首个周期到点后启动 */
    SCH_RUN   = 1,   /* 周期执行中，到点即调回调 */
    SCH_STOP  = 2,   /* 已停用，scheduler_run() 跳过该任务 */
} sch_state_e;

typedef struct
{
    task_fn_t   callback;       /* 任务回调函数 */
    uint32_t    interval_time;  /* 执行周期，单位 ms；0 表示每次 scheduler_run() 都执行 */
    uint32_t    last_time;      /* 上次执行时刻（HAL_GetTick 毫秒数），差值比较天然免回绕问题 */
    sch_state_e state;          /* 当前状态 */
} sch_task_t;

/* Private variables ---------------------------------------------------------*/
/* 任务表与已注册数：只在主循环上下文读写，无并发访问，故不加 volatile */
static uint8_t    sch_task_num = 0;
static sch_task_t sch_task_list[SCH_TASK_MAX] = {0};

/* Private functions ---------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*------------------------------ Public functions ----------------------------*/
/*----------------------------------------------------------------------------*/
/**
 * @brief  清空任务表，所有已注册任务作废（主循环启动前调用一次）
 *
 * @return 无
 */
void scheduler_reset(void)
{
    sch_task_num = 0;
    for (uint8_t i = 0; i < SCH_TASK_MAX; i++)
    {
        sch_task_list[i].callback      = NULL;
        sch_task_list[i].interval_time = 0;
        sch_task_list[i].last_time     = 0;
        sch_task_list[i].state         = SCH_STOP;
    }
}

/**
 * @brief  注册任务：把回调函数和执行周期挂到调度器任务表
 *
 * @param[in]  callback      任务回调函数，不能为 NULL；同一回调重复注册会被拒绝
 * @param[in]  interval_time 执行周期，单位 ms；0 表示每次 scheduler_run() 都执行
 *
 * @return 1  注册成功
 * @return 0  失败：任务表已满，或该回调已注册过
 *
 * @note 任务 ID 即任务表下标（从 0 开始，按注册顺序分配）。本函数不返回 ID，
 *       需要 scheduler_enable/disable 的模块应在注册时自行记录顺序，这与原版
 *       GS98 工程一致（该工程也从未使用 enable/disable）。
 */
uint8_t scheduler_register(task_fn_t callback, uint32_t interval_time)
{
    if ((callback == NULL) || (sch_task_num >= SCH_TASK_MAX))
    {
        return 0;
    }

    /* 逐个比对已注册任务的回调，同一函数不允许重复注册 */
    for (uint8_t i = 0; i < sch_task_num; i++)
    {
        if (sch_task_list[i].callback == callback)
        {
            return 0;
        }
    }

    sch_task_list[sch_task_num].callback      = callback;
    sch_task_list[sch_task_num].interval_time = interval_time;
    sch_task_list[sch_task_num].last_time     = 0;
    sch_task_list[sch_task_num].state         = SCH_READY;   /* 首个周期到点后才开始执行 */
    sch_task_num++;

    return 1;
}

/**
 * @brief  启用指定任务，从当前时刻起重新计时开始周期执行
 *
 * @param[in]  task_id 任务 ID（任务表下标，0 开始），须小于已注册任务数
 *
 * @return 无
 */
void scheduler_enable(uint8_t task_id)
{
    if (task_id < sch_task_num)
    {
        if (sch_task_list[task_id].callback != NULL)
        {
            sch_task_list[task_id].last_time = HAL_GetTick();
            sch_task_list[task_id].state     = SCH_READY;
        }
    }
}

/**
 * @brief  停用指定任务，scheduler_run() 不再调度它，任务信息保留
 *
 * @param[in]  task_id 任务 ID（任务表下标，0 开始），须小于已注册任务数
 *
 * @return 无
 */
void scheduler_disable(uint8_t task_id)
{
    if (task_id < sch_task_num)
    {
        if (sch_task_list[task_id].callback != NULL)
        {
            sch_task_list[task_id].last_time = 0;
            sch_task_list[task_id].state     = SCH_STOP;
        }
    }
}

/**
 * @brief  调度一轮任务：遍历任务表，执行所有到期的任务回调
 *
 * 主循环里反复调用。用"当前时刻 - 上次执行时刻 >= 周期"判断到期，无符号数
 * 差值比较天然免回绕问题。READY 状态的任务首个周期到点时只刷新时刻并转 RUN，
 * 保证每个任务两次执行至少间隔一个 interval_time。
 */
void scheduler_run(void)
{
    uint32_t now;
    uint32_t elapsed_ms;

    for (uint8_t i = 0; i < sch_task_num; i++)
    {
        now        = HAL_GetTick();
        elapsed_ms = now - sch_task_list[i].last_time;

        if (elapsed_ms >= sch_task_list[i].interval_time)
        {
            if (sch_task_list[i].state == SCH_RUN)
            {
                if (sch_task_list[i].callback != NULL)
                {
                    sch_task_list[i].callback();
                }
                sch_task_list[i].last_time = now;
            }
            else if (sch_task_list[i].state == SCH_READY)
            {
                sch_task_list[i].last_time = now;
                sch_task_list[i].state     = SCH_RUN;
            }
        }
    }
}

/*---------------------------------End of File--------------------------------*/

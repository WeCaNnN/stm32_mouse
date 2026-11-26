#include "user_config.h"
#include "sensor.h"
#include "profile.h"

/**
 * @brief 加载鼠标的一些默认参数
 */
void mouse_param_init(void)
{
    sensor_dpi_config(DEFAULT_DPI);
}

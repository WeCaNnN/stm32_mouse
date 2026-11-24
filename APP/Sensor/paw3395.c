#include "sensor.h"
#include "pixart_port.h"
#include "user_config.h"

/* 
sensor Init 1-成功 0-失败
 */
bool sensor_paw_3395_init(void)
{

    pixart_port_init();
    
    return 0;
}

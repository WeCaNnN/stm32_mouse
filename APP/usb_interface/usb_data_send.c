#include "usb_data_send.h"
#include "main.h"
#include "user_config.h"
#include "usbd_hid.h"          /* USBD_HID_SendReport */
#include "usb_device.h"
#include "sensor.h"            /* sensor_XY_data_get/acc：位移取走与回滚 */

/* 本文件日志标签：必须在 elog.h 之前定义，log_w/e 会自动带上它 */
#define LOG_TAG     "usb_report"
#include <elog.h>

typedef struct {
    uint8_t buttons;   // bit0=左, bit1=右, bit2=中
    int16_t x;
    int16_t y;
    int8_t  wheel;
}__attribute__((packed))  mouse_report_t;

typedef char mouse_report_size_check[(sizeof(mouse_report_t) == 6) ? 1 : -1]; /* sizeof 不为 6 时数组长度为负，编译失败 */

#define BTN_LEFT   0x01
#define BTN_RIGHT  0x02
#define BTN_MIDDLE 0x04


static mouse_report_t mouse_report = {0};


/**
 * @brief 鼠标报文上报任务：打包并发送HID鼠标报文
 */
void mouse_report_task(void)
{
    int16_t sensor_x = 0;
    int16_t sensor_y = 0;

    sensor_XY_data_get(&sensor_x,&sensor_y);

    //按键
    mouse_report.buttons = 0;
    //X Y 位移
    mouse_report.x = sensor_x;
    mouse_report.y = sensor_y;
    //滚轮
    mouse_report.wheel = 0;


    if(USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&mouse_report, sizeof(mouse_report_t)) != USBD_OK)
    {
        #if USB_MOUSE_DEBUG
        log_w("usb_report_busy");
        #endif
        //上一包发送未完成，回写xy轴位移累积到下一次
        sensor_XY_data_acc(sensor_x,sensor_y);
    }
}


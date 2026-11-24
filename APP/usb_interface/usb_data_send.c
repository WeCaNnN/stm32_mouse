#include "usb_data_send.h"
#include "main.h"
#include "user_config.h"
#include "usbd_hid.h"          /* USBD_HID_SendReport */
#include "usb_device.h"

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
    #if USB_MOUSE_INTERFACE_TEST
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&mouse_report, sizeof(mouse_report_t));
    #endif

}


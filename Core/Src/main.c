/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_config.h"
/* 本文件日志标签：必须在 elog.h 之前定义，log_a/e/w/i/d/v 会自动带上它；
 * 其他 .c 文件要打日志时，同样在 include elog.h 之前定义各自的 LOG_TAG */
#define LOG_TAG     "main"
#include <elog.h>
#include "SEGGER_RTT.h"   /* Error_Handler 里 elog 可能未初始化，需裸写 RTT */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* LED 翻转间隔(ms)：亮、灭各占一个间隔，完整闪烁周期是它的 2 倍 */
#define LED_BLINK_INTERVAL_MS 500U

/* 日志前缀格式（elog_set_fmt 用）：不带 [ ] 时间/进程/线程外壳 */
#define LOG_FMT_WITH_LINE (ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_LINE)  /* A/E/D/V：级别+标签+行号 */
#define LOG_FMT_SHORT     (ELOG_FMT_LVL | ELOG_FMT_TAG)                  /* I/W：仅级别+标签 */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if SELF_TEST_LOG
/**
 * @brief 日志链路自检：逐级输出一行，覆盖全部 5 个日志级别
 *
 * 开机调用一次，RTT Viewer 里 6 行全可见（含配色）说明 RTT+EasyLogger 链路正常；
 * 由 APP/user_config.h 的 SELF_TEST_LOG 控制是否编译。
 */
static void log_self_test(void)
{
  log_v("boot: RTT + EasyLogger chain test start");
  log_d("SYSCLK=72MHz (HSE 8M x PLL9), APB1=36MHz");
  log_i("EasyLogger -> SEGGER RTT ch0");
  log_w("WARN level sample: lvl+tag prefix only");
  log_e("ERROR level sample: full-field prefix");
  log_a("ASSERT level sample: all 5 levels visible means RTT OK");
}
#endif

/**
 * @brief 初始化 EasyLogger 日志框架（输出后端为 SEGGER RTT 通道 0，见 elog_port.c）
 *
 * 各级别的输出格式：A/E/D/V 带行号，I/W 仅级别+标签；各级都不带 [ ] 时间/进程/线程
 * 外壳——定位问题靠行号，行号在 Keil 里可直接跳转
 */
static void app_log_init(void)
{
    elog_init();

    /* 各级别日志输出格式 */
    elog_set_fmt(ELOG_LVL_ASSERT,  LOG_FMT_WITH_LINE);
    elog_set_fmt(ELOG_LVL_ERROR,   LOG_FMT_WITH_LINE);
    elog_set_fmt(ELOG_LVL_WARN,    LOG_FMT_SHORT);
    elog_set_fmt(ELOG_LVL_INFO,    LOG_FMT_SHORT);
    elog_set_fmt(ELOG_LVL_DEBUG,   LOG_FMT_WITH_LINE);
    elog_set_fmt(ELOG_LVL_VERBOSE, LOG_FMT_WITH_LINE);
    elog_start();

    #if SELF_TEST_LOG
    log_self_test();
    #endif
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  app_log_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    #if LED_PC13_TOGGLE_TEST
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(LED_BLINK_INTERVAL_MS);
    #endif
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  SEGGER_RTT_WriteString(0, "E/HAL: Error_Handler entered, clock/periph init failed\n");
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

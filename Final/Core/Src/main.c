#include "main.h"

#define RESET_BUTTON_PIN GPIO_PIN_0
#define INC_BUTTON_PIN   GPIO_PIN_1
#define DEC_BUTTON_PIN   GPIO_PIN_2
#define BUTTON_GPIO_PORT GPIOA

#define LED_PIN GPIO_PIN_5
#define LED_GPIO_PORT GPIOA

#define SEG_A_PIN GPIO_PIN_3
#define SEG_B_PIN GPIO_PIN_4
#define SEG_C_PIN GPIO_PIN_5
#define SEG_D_PIN GPIO_PIN_6
#define SEG_E_PIN GPIO_PIN_7
#define SEG_F_PIN GPIO_PIN_8
#define SEG_G_PIN GPIO_PIN_9
#define SEG_DP_PIN GPIO_PIN_10
#define SEG_GPIO_PORT GPIOB

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

volatile uint8_t counter = 0;
volatile uint8_t inc_pressed = 0;
volatile uint8_t dec_pressed = 0;
volatile uint32_t inc_press_time = 0;
volatile uint32_t dec_press_time = 0;
volatile uint32_t last_activity_time = 0;
volatile uint8_t inactivity_mode = 0;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void display_number(uint8_t number);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();

    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);

    last_activity_time = HAL_GetTick();

    while (1) {
        uint32_t current_time = HAL_GetTick();

        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, RESET_BUTTON_PIN) == GPIO_PIN_SET) {
            counter = 0;
            last_activity_time = current_time;
            inactivity_mode = 0;
        }

        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, INC_BUTTON_PIN) == GPIO_PIN_SET) {
            last_activity_time = current_time;
            inactivity_mode = 0;

            if (!inc_pressed) {
                inc_pressed = 1;
                inc_press_time = current_time;
                counter = (counter + 1) % 10;
            } else if (current_time - inc_press_time >= 3000) {
                inc_pressed = 2; // Long press detected
            }
        } else {
            inc_pressed = 0;
        }

        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, DEC_BUTTON_PIN) == GPIO_PIN_SET) {
            last_activity_time = current_time;
            inactivity_mode = 0;

            if (!dec_pressed) {
                dec_pressed = 1;
                dec_press_time = current_time;
                if (counter == 0) {
                    counter = 9;
                } else {
                    counter--;
                }
            } else if (current_time - dec_press_time >= 3000) {
                dec_pressed = 2; // Long press detected
            }
        } else {
            dec_pressed = 0;
        }

        if (!inactivity_mode && (current_time - last_activity_time >= 10000)) {
            inactivity_mode = 1;
        }

        display_number(counter);
        HAL_Delay(200);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (inc_pressed == 2) {
            counter = (counter + 1) % 10;
        }
        if (dec_pressed == 2) {
            if (counter == 0) {
                counter = 9;
            } else {
                counter--;
            }
        }

        // Toggle LED every second
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    } else if (htim->Instance == TIM3) {
        static uint8_t inactivity_countdown = 0;
        if (inactivity_mode) {
            if (counter > 0) {
                counter--;
            } else {
                inactivity_mode = 0;
            }
        }
    }
}

void display_number(uint8_t number) {
    HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN |
                                      SEG_E_PIN | SEG_F_PIN | SEG_G_PIN | SEG_DP_PIN, GPIO_PIN_RESET);

    switch (number) {
        case 0:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | 
                                              SEG_E_PIN | SEG_F_PIN, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_B_PIN | SEG_C_PIN, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_D_PIN | SEG_E_PIN | 
                                              SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 3:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | 
                                              SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 4:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_B_PIN | SEG_C_PIN | SEG_F_PIN | SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 5:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_C_PIN | SEG_D_PIN | SEG_F_PIN | 
                                              SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 6:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN | 
                                              SEG_F_PIN | SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 7:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN, GPIO_PIN_SET);
            break;
        case 8:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | 
                                              SEG_E_PIN | SEG_F_PIN | SEG_G_PIN, GPIO_PIN_SET);
            break;
        case 9:
            HAL_GPIO_WritePin(SEG_GPIO_PORT, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | 
                                              SEG_F_PIN | SEG_G_PIN, GPIO_PIN_SET);
            break;
    }
}

void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = RESET_BUTTON_PIN | INC_BUTTON_PIN | DEC_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN |
                          SEG_E_PIN | SEG_F_PIN | SEG_G_PIN | SEG_DP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SEG_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 9999; // Assuming 10kHz clock -> 1ms tick
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999; // 1 second period
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
}

void MX_TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 9999; // Assuming 10kHz clock -> 1ms tick
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999; // 1 second period
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RED_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RED_LED_GPIO_Port, &GPIO_InitStruct);

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
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

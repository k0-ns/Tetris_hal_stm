/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HEIGHT 32
#define WIDTH 8
#define PIXEL_OFF 46
#define PIXEL_ON 64
#define TIME_SLEEP 100
#define ROTATE_KEY GPIO_PIN_12
#define DROP_KEY GPIO_PIN_10
#define LEFT_KEY GPIO_PIN_1
#define RIGHT_KEY GPIO_PIN_13
#define AMOUNT_OF_ADJ 3
#define TICKS_FOR_MOVE 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum {
    ITEM_L_FRONT,
    ITEM_L_BACK,
    ITEM_SQUARE,
    ITEM_LINE,
    ITEM_Z,
    ITEM_Z_BACK,
    ITEM_T,
    ITEM_PENIS
};

enum {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    ROTATION,
    NOWHERE
};

enum {
    N = 0,
    U = -1,
    UU = -2,
    D = 1,
    R = 1,
    L = -1
};

struct item_t {
    u8 type;
    u8 rotation;
    u8 x;
    u8 y;
};

u8 matrix[HEIGHT][WIDTH];
struct item_t item;
u8 timer = 0;
u8 p_rand = 0;

i8 L_FRONT[AMOUNT_OF_ADJ][2] = { {N,U},{N,D},{R,D} };
i8 L_BACK[AMOUNT_OF_ADJ][2] = { {N,U},{N,D},{L,D} };
i8 SQUARE[AMOUNT_OF_ADJ][2] = { {N,U},{R,U},{R,N} };
i8 LINE[AMOUNT_OF_ADJ][2] = { {N,U},{N,D},{N,UU} };
i8 Z[AMOUNT_OF_ADJ][2] = { {N,U},{L,U},{R,N} };
i8 Z_BACK[AMOUNT_OF_ADJ][2] = { {N,U},{L,N},{R,U} };
i8 T[AMOUNT_OF_ADJ][2] = { {N,U},{L,N},{R,N} };
i8 PENIS[AMOUNT_OF_ADJ][2] = { {N,U},{L,D},{R,D} };

void SetDIN(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, SET);
}

void ResetDIN(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, RESET);
}

void ToggleDIN(){
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
}



void SetCS(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, SET);
}

void ResetCS(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, RESET);
}

void PulseCS(){
	SetCS();
	ResetCS();
}

void ToggleCS(){
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
}

void SetCLK(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, SET);
}

void ResetCLK(){
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, RESET);
}

void PulseCLK(){
	  SetCLK();
	  ResetCLK();
}


void SendBtis(u16 mes){
	for (u8 i = 0; i < 16; i++){
		if(mes >> 15 == 0b1){
			SetDIN();
		} else {
			ResetDIN();
		}
		PulseCLK();
		mes <<= 1;
	}
	PulseCS();
	ResetDIN();

}

void SendRawBtis(u16 mes){
	for (u8 i = 0; i < 16; i++){
		if(mes >> 15 == 0b1){
			SetDIN();
		} else {
			ResetDIN();
		}
		PulseCLK();
		mes <<= 1;
	}
	ResetDIN();

}



// Вместо текущей инициализации используйте:
void InitMAX7219Chain() {
    // Отправляем команды 4 раза (по одной на каждый чип в цепи)
    for(uint8_t i = 0; i < 4; i++) {
        SendBtis(0x0C01);  // Включить
        SendBtis(0x0900);  // No decode
        SendBtis(0x0A0F);  // Яркость
        SendBtis(0x0B07);  // Все разряды
        SendBtis(0x0F00);  // Нормальный режим
    }

    // Очистка всех 4 чипов
    for(uint8_t digit = 1; digit <= 8; digit++) {
        for(uint8_t chip = 0; chip < 4; chip++) {
            SendBtis((digit << 8) | 0x00);
        }
    }
}
void Send(uint8_t reg, uint8_t data){
	SendBtis(((u16)reg << 8) | data);
}

void SendRaw(uint8_t reg, uint8_t data){
	SendRawBtis(((u16)reg << 8) | data);
}
// Для отправки данных во все 4 чипа:
void SendToAllChips(uint8_t reg, uint8_t data) {
    for(uint8_t i = 0; i < 4; i++) {
        SendBtis(((u16)reg << 8) | data);
    }
}

// Или для разных данных в каждый чип (от последнего к первому):
void SendToEachChip(uint8_t reg, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4) {
    SendBtis(((u16)reg << 8) | data4); // Последний чип
    SendBtis(((u16)reg << 8) | data3);
    SendBtis(((u16)reg << 8) | data2);
    SendBtis(((u16)reg << 8) | data1); // Первый чип
}

u8 RandomGen(u8 min, u8 max) {
    max++;
    return (p_rand % (max - min) + min);
}

void ResetMatrix() {
    for (u8 i = 0; i < HEIGHT; i++) {
        for (u8 j = 0; j < WIDTH; j++) {
            matrix[i][j] = PIXEL_OFF;
        }
    }
}

void PrintMatrix() {
	for(u8 i = 0; i < 8; i++){
		for(u8 j = 0; j < 4; j++){
			u8 word = 0b0;
			for(u8 k = 0; k < 8; k++){
				word <<= 1;
				if (matrix[j*8+k][i] == PIXEL_ON){
					word |= 0b1;
				}
			}
			SendRaw(8 - i, word);
		}
		PulseCS();
	}
}

u8 GetPixel(u8 x, u8 y) {
    if (x < 0 || x >= WIDTH) return PIXEL_ON;
    if (y < 0 || x >= HEIGHT) return PIXEL_ON;

    return matrix[y][x];
}

u8 IsAllowedPixel(u8 x, u8 y) {
    return (GetPixel(x, y) == PIXEL_OFF) ? 1 : 0;
}

u8 IsAllowedPixelRight(u8 x, u8 y) {
    return (GetPixel(x + 1, y) == PIXEL_OFF) ? 1 : 0;
}

u8 IsAllowedPixelLeft(u8 x, u8 y) {
    return (GetPixel(x - 1, y) == PIXEL_OFF) ? 1 : 0;
}

u8 IsAllowedPixelDown(u8 x, u8 y) {
    return (GetPixel(x, y + 1) == PIXEL_OFF) ? 1 : 0;
}

void SetPixel(u8 x, u8 y) {
    if (x < 0 || x >= WIDTH) return;
    if (y < 0 || x >= HEIGHT) return;

    matrix[y][x] = PIXEL_ON;
}

void ResetPixel(u8 x, u8 y) {
    if (x < 0 || x >= WIDTH) return;
    if (y < 0 || x >= HEIGHT) return;

    matrix[y][x] = PIXEL_OFF;
}

u8 TogglePixel(u8 x, u8 y) {
    if (x < 0 || x >= WIDTH) return 0;
    if (y < 0 || x >= HEIGHT) return 0;

    if (matrix[y][x] == PIXEL_ON) {
        matrix[y][x] = PIXEL_OFF;
    }
    else {
        matrix[y][x] = PIXEL_ON;
    }
    return 0;
}

void CopyItem(i8 item_rotated[AMOUNT_OF_ADJ][2], i8 copyable_item[AMOUNT_OF_ADJ][2]) {
    for (int i = 0; i < AMOUNT_OF_ADJ; i++) {
        item_rotated[i][0] = copyable_item[i][0];
        item_rotated[i][1] = copyable_item[i][1];
    }
}

void FillItem(i8 item_rotated[AMOUNT_OF_ADJ][2]) {
    switch (item.type)
    {
    case ITEM_L_FRONT:
        CopyItem(item_rotated, L_FRONT);
        break;
    case ITEM_L_BACK:
        CopyItem(item_rotated, L_BACK);
        break;
    case ITEM_SQUARE:
        CopyItem(item_rotated, SQUARE);
        break;
    case ITEM_LINE:
        CopyItem(item_rotated, LINE);
        break;
    case ITEM_Z:
        CopyItem(item_rotated, Z);
        break;
    case ITEM_Z_BACK:
        CopyItem(item_rotated, Z_BACK);
        break;
    case ITEM_T:
        CopyItem(item_rotated, T);
        break;
    case ITEM_PENIS:
        CopyItem(item_rotated, PENIS);
        break;
    }
}

void CheckPosition(u8 * value, u8(*Function)(u8, u8)) {
    u8 x = item.x;
    u8 y = item.y;
    *value &= Function(x, y);
    i8 item_rotated[AMOUNT_OF_ADJ][2];
    FillItem(item_rotated);
    for (int i = 0; i < item.rotation; i++) {
        for (int i = 0; i < AMOUNT_OF_ADJ; i++) {
            i8 bucket = item_rotated[i][0];
            item_rotated[i][0] = -item_rotated[i][1];
            item_rotated[i][1] = bucket;
        }
    }
    *value &= Function(x + item_rotated[0][0], y + item_rotated[0][1]);
    *value &= Function(x + item_rotated[1][0], y + item_rotated[1][1]);
    *value &= Function(x + item_rotated[2][0], y + item_rotated[2][1]);
}

u8 IsMoveable(u8 direction) {
    u8(*checkFunction)(u8, u8) = IsAllowedPixel;
    switch (direction)
    {
    case LEFT:
        checkFunction = IsAllowedPixelLeft;
        break;
    case RIGHT:
        checkFunction = IsAllowedPixelRight;
        break;
    case DOWN:
        checkFunction = IsAllowedPixelDown;
        break;
    default:
        break;
    }
    u8 is_moveable = 1;
    is_moveable &= checkFunction(item.x, item.y);
    if (direction == ROTATION) {
        item.rotation = (item.rotation + 1) % 4;
    }
    CheckPosition(&is_moveable, checkFunction);
    if (direction == ROTATION) {
        item.rotation = (item.rotation + 3) % 4;
    }
    return is_moveable;
}

void ResetItem() {
    item.type = RandomGen(ITEM_L_FRONT, ITEM_T);
    item.x = WIDTH / 2;
    item.y = 2;
    item.rotation = RandomGen(UP, LEFT);
    if (!IsMoveable(NOWHERE)) {
        ResetMatrix();
    }
}

void ToggleItem() {
    u8 nothing = 0;
    CheckPosition(&nothing, TogglePixel);
}

u8 GetKeyState(u8 arg){
	return 0;
}

u8 IsRotate() {
    return HAL_GPIO_ReadPin(GPIOB, ROTATE_KEY) ? 0 : 1;
}

u8 IsDrop() {
    return HAL_GPIO_ReadPin(GPIOB, DROP_KEY) ? 0 : 1;
}

u8 IsLeft() {
    return HAL_GPIO_ReadPin(GPIOB, LEFT_KEY) ? 0 : 1;
}

u8 IsRight() {
    return HAL_GPIO_ReadPin(GPIOB, RIGHT_KEY) ? 0 : 1;
}

void DeleteLine(u8 y) {
    for (u8 i = y; i > 1; i--) {
        for (u8 j = 0; j < WIDTH; j++) {
            matrix[i][j] = matrix[i - 1][j];
        }
    }
}

void CheckLines() {
    for (u8 i = HEIGHT - 1; i > 1; i--) {
        for (u8 j = 0; j < WIDTH; j++) {
            if (matrix[i][j] == PIXEL_OFF) {
                break;
            }
            if (j == WIDTH - 1) {
                DeleteLine(i);
                i = HEIGHT;
            }
        }
    }
}

void Update() {
    if (!IsMoveable(DOWN)) {
        ToggleItem();
        CheckLines();
        ResetItem();
        return;
    }
    timer = (timer + 1) % TICKS_FOR_MOVE;
    if(timer == 0) item.y++;
    if (IsDrop()) {
        while (IsMoveable(DOWN)) {
            item.y++;
        }
        return;
    }
    if (IsRotate() && IsMoveable(ROTATION)) {
        item.rotation = (item.rotation + 1) % 4;
    }
    if (IsLeft() && IsMoveable(LEFT)) {
        item.x--;
    }
    if (IsRight() && IsMoveable(RIGHT)) {
        item.x++;
    }
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t time = 100;
  ResetCLK();
  ResetCS();
  ResetDIN();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, RESET);
	HAL_Delay(time);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, SET);
	HAL_Delay(time);

  SendBtis(0x0C01);  // 0x0C - регистр Shutdown, 0x01 - нормальный режим (не shutdown)
  SendBtis(0x0900);  // 0x09 - регистр Decode Mode, 0x00 - без декодирования
  SendBtis(0x0A01);  // 0x0A - регистр Intensity, 0x0F - максимальная яркость
  SendBtis(0x0B07);  // 0x0B - регистр Scan Limit, 0x07 - все 8 цифр/строк
  SendBtis(0x0F00);  // 0x0F - регистр Display Test, 0x00 - нормальный режим (не тест)

  // Очистка дисплея
  for(uint8_t i = 1; i <= 8; i++) {
	  SendToAllChips(i , 0x00); // Отправка 0x00 во все регистры цифр
  }
  ResetItem();
  ResetMatrix();

  while (1) {
      // Зажигаем разные сегменты на разных чипах

	  p_rand++;
	  Update();
	  ToggleItem();
	  PrintMatrix();
	  HAL_Delay(TIME_SLEEP);
	  ToggleItem();


      // Очищаем все
      /*SendRaw(1, 0x01);
      SendRaw(2, 0x02);
      SendRaw(3, 0x03);
      SendRaw(4, 0x04);
      PulseCS();
      HAL_Delay(500);
      SendToAllChips(1, 0x00);
      SendToAllChips(2, 0x00);
      SendToAllChips(3, 0x00);
      SendToAllChips(4, 0x00);
      HAL_Delay(500);*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DIN_Pin|CS_Pin|CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(VCC_GPIO_Port, VCC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN_Pin CS_Pin CLK_Pin */
  GPIO_InitStruct.Pin = DIN_Pin|CS_Pin|CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : VCC_Pin */
  GPIO_InitStruct.Pin = VCC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(VCC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LEFT_Pin DROP_Pin ROTATE_Pin RIGHT_Pin */
  GPIO_InitStruct.Pin = LEFT_Pin|DROP_Pin|ROTATE_Pin|RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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

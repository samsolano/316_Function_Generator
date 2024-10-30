#include "main.h"


void SystemClock_Config(void);
void DAC_Init(void);
void Keypad_Init(void);
void Timer_Init(void);

void TIM2_IRQHandler(void);
int  Keypad_Output(void);
int  get_key_pressed(int row, int col);
void DAC_Write(uint16_t value);
int DAC_volt_conv(int value);
void Poll_Keypad(void);

void Square();
void Sine();
void Triangle();
void Sawtooth();


#define HZ_100 1
#define HZ_200 2
#define HZ_300 3
#define HZ_400 4
#define HZ_500 5
#define SINE_WAVE 6
#define TRIANGLE_WAVE 7
#define SAWTOOTH_WAVE 8
#define SQUARE_WAVE 9
#define FIFTY_DUTY 0
#define DECREASE_DUTY 10
#define INCREASE_DUTY 11
#define MAX_SAMPLE_SPEED .0000225
//#define MAX_SAMPLE_SPEED .0001579


//global variables

uint8_t Hertz = 1;
uint8_t Waveform = 9;
uint8_t Duty_Cycle = 5;
uint8_t Voltage_WE = 0;



/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	HAL_Init();
	SystemClock_Config();
	Keypad_Init();
	Timer_Init();
	DAC_Init();

	//to get to 3V in 10ms (100Hz) it takes 22.5us so there are 10ms / 22.5us = 444.4 samples
	//to get to 3V in 5ms (200Hz) it takes 11.25us so there are 5ms / 22.5us =  222.2 samples

	//set up interrupts to be 44.4Khz

//	DAC_Write(1703);

	while(1)
	{
//		DAC_Write(1500);
		Poll_Keypad();

		if(Waveform == SINE_WAVE)
		{
			Sine();
		}
		else if(Waveform == TRIANGLE_WAVE)
		{
			Triangle();
		}
		else if(Waveform == SAWTOOTH_WAVE)
		{
			Sawtooth();
		}
		else if(Waveform == SQUARE_WAVE)
		{
			Square();
		}

	}

}

void Sine()
{


	const uint16_t Sine_Table[148] = {
		1338, 1366, 1394, 1423, 1451, 1479, 1507, 1536, 1564, 1591,
		1619, 1647, 1675, 1702, 1729, 1756, 1783, 1810, 1836, 1862,
		1888, 1914, 1940, 1965, 1990, 2014, 2039, 2063, 2086, 2110,
		2133, 2155, 2178, 2200, 2221, 2242, 2263, 2283, 2303, 2323,
		2342, 2360, 2378, 2396, 2413, 2429, 2446, 2461, 2476, 2491,
		2505, 2519, 2532, 2544, 2556, 2568, 2579, 2589, 2599, 2608,
		2616, 2624, 2632, 2639, 2645, 2651, 2656, 2660, 2664, 2667,
		2670, 2672, 2674, 2675, 2675, 2675, 2674, 2672, 2670, 2667,
		2664, 2660, 2656, 2651, 2645, 2639, 2632, 2624, 2616, 2608,
		2599, 2589, 2579, 2568, 2556, 2544, 2532, 2519, 2505, 2491,
		2476, 2461, 2446, 2429, 2413, 2396, 2378, 2360, 2342, 2323,
		2303, 2283, 2263, 2242, 2221, 2200, 2178, 2155, 2133, 2110,
		2086, 2063, 2039, 2014, 1990, 1965, 1940, 1914, 1888, 1862,
		1836, 1810, 1783, 1756, 1729, 1702, 1675, 1647, 1619, 1591,
		1564, 1536, 1507, 1479, 1451, 1423, 1394, 1366
	};


	uint16_t ARR_Count = 0;
	uint16_t Sine_Table_Index = 0;
	uint16_t Switch_Phase = 0;


//		uint16_t ARR_Target_Period = ((10000000 / (100 * Hertz)) / (MAX_SAMPLE_SPEED * 10000000)); //444 @ 100Hz

	uint8_t Hertz_Change = Hertz;
	uint16_t ARR_Target_Period;

	if(Hertz == 1)
	{
		ARR_Target_Period = 148;
	}
	else if(Hertz == 2)
	{
		ARR_Target_Period = 74;
	}
	else if(Hertz == 3)
	{
		ARR_Target_Period = 49;
	}
	else if(Hertz == 4)
	{
		ARR_Target_Period = 37;
	}
	else if(Hertz == 5)
	{
		ARR_Target_Period = 30;
	}


	while(Waveform == SINE_WAVE)
	{
		Poll_Keypad();

		if(Hertz_Change != Hertz)
		{
			break;
		}

		if(ARR_Count >= (ARR_Target_Period * 2)) //reset
		{
			ARR_Count = 0;
			Sine_Table_Index = 0;
			Switch_Phase = 0;
		}

		if(ARR_Count < ARR_Target_Period) //going from half to peak
		{
			DAC_Write(Sine_Table[Sine_Table_Index]);
			Sine_Table_Index += Hertz;
			ARR_Count++;
		}
		else if((ARR_Count >= ARR_Target_Period) & (ARR_Count < (ARR_Target_Period * 2)))
		{
			if(Switch_Phase == 0)
			{
				Sine_Table_Index = 0;
				Switch_Phase = 1;
			}
			DAC_Write(2675 - Sine_Table[Sine_Table_Index]);
			Sine_Table_Index += Hertz;
			ARR_Count++;
		}

		Voltage_WE = 0;
	}

}

void Square()
{
	uint16_t ARR_Count = 0;

	uint8_t Hertz_Change = Hertz;
	uint8_t Duty_Change = Duty_Cycle;


//	double temp = (((1.0 / (100.0 * Hertz)) / (MAX_SAMPLE_SPEED))); //888
//	uint16_t ARR_Target_Period = (uint16_t)temp;
	uint16_t ARR_Target_Period;

	if(Hertz == 1)
	{
		ARR_Target_Period = 444;
	}
	else if(Hertz == 2)
	{
		ARR_Target_Period = 220;
	}
	else if(Hertz == 3)
	{
		ARR_Target_Period = 147;
	}
	else if(Hertz == 4)
	{
		ARR_Target_Period = 111;
	}
	else if(Hertz == 5)
	{
		ARR_Target_Period = 88;
	}

	uint16_t High_Length = (ARR_Target_Period * Duty_Cycle) / 10;
	uint16_t Low_Length = ARR_Target_Period - High_Length;

	if(Hertz == 1)
	{
		High_Length = 44 + ((Duty_Cycle - 1) * 44);
		Low_Length = ARR_Target_Period - High_Length;
	}

	while(Waveform == SQUARE_WAVE)
	{
		Poll_Keypad();
		if((Hertz_Change != Hertz) | (Duty_Change != Duty_Cycle))
		{
			break;
		}



		if(Voltage_WE)
		{
			if(ARR_Count == 0)
			{
				DAC_Write(2650);
			}
			if(ARR_Count == High_Length)
			{
				DAC_Write(0);
			}
			if(ARR_Count == High_Length + Low_Length)
			{
				DAC_Write(2650);
				ARR_Count = 0;
			}
			ARR_Count++;
			Voltage_WE = 0;
		}
	}

}

void Triangle()
{

	uint16_t ARR_Count = 0;
//	uint16_t ARR_Target_Period = ((10000000 / (100 * Hertz)) / (MAX_SAMPLE_SPEED * 10000000)) / 2; //222 at 100
	uint8_t Hertz_Change = Hertz;

	uint16_t ARR_Target_Period;

	if(Hertz == 1)
	{
		ARR_Target_Period = 120;
	}
	else if(Hertz == 2)
	{
		ARR_Target_Period = 60;
	}
	else if(Hertz == 3)
	{
		ARR_Target_Period = 40;
	}
	else if(Hertz == 4)
	{
		ARR_Target_Period = 30;
	}
	else if(Hertz == 5)
	{
		ARR_Target_Period = 24;
	}

		uint16_t Voltage_Step = (2675 / ARR_Target_Period);

	while(Waveform == TRIANGLE_WAVE)
	{
		Poll_Keypad();

		if(Hertz_Change != Hertz)
		{
			break;
		}

		if(Voltage_WE)
		{
			if(ARR_Count == (ARR_Target_Period * 2))
			{
				ARR_Count = 0;
			}
			if(ARR_Count <= ARR_Target_Period)
			{
				DAC_Write(Voltage_Step * ARR_Count);
				ARR_Count++;
			}
			else if(ARR_Count > ARR_Target_Period)
			{
				DAC_Write(5350 - Voltage_Step * ARR_Count);
				ARR_Count++;
			}
			Voltage_WE = 0;
		}
	}

}

void Sawtooth()
{
	uint16_t ARR_Count = 0;
//	uint16_t ARR_Target_Period = ((10000000 / (100 * Hertz)) / (MAX_SAMPLE_SPEED * 10000000)); //444 times
	uint8_t Hertz_Change = Hertz;

	uint16_t ARR_Target_Period;

	if(Hertz == 1)
	{
		ARR_Target_Period = 222;
	}
	else if(Hertz == 2)
	{
		ARR_Target_Period = 111;
	}
	else if(Hertz == 3)
	{
		ARR_Target_Period = 74;
	}
	else if(Hertz == 4)
	{
		ARR_Target_Period = 55;
	}
	else if(Hertz == 5)
	{
		ARR_Target_Period = 44;
	}

	uint16_t Voltage_Step = (2650 / ARR_Target_Period);

	while(Waveform == SAWTOOTH_WAVE)
	{
		Poll_Keypad();

		if(Hertz_Change != Hertz)
		{
			break;
		}

		if(Voltage_WE)
		{
			if(ARR_Count == ARR_Target_Period)
			{
				DAC_Write(0);
				ARR_Count = 0;
			}
			else
			{
				DAC_Write(Voltage_Step * ARR_Count);
				ARR_Count++;
			}
			Voltage_WE = 0;
		}
	}

}

void Poll_Keypad()
{


	uint8_t Key_Pressed = Keypad_Output();

	if((Key_Pressed >= 6) & (Key_Pressed <= 9))
	{
		Waveform = Key_Pressed;
	}
	if((Key_Pressed >= 1) & (Key_Pressed <= 5))
	{
		Hertz = Key_Pressed;
	}
	if(Key_Pressed == 10)
	{
		if(Duty_Cycle > 1)
		{
			Duty_Cycle--;
		}
		for(uint32_t i = 0; i < 300000; i++) {}
	}
	if(Key_Pressed == 11)
	{
		if(Duty_Cycle < 9)
		{
			Duty_Cycle++;
		}
		for(uint32_t i = 0; i < 300000; i++) {}
	}
	if(Key_Pressed == 0)
	{
		Duty_Cycle = 5;
	}

}

void TIM2_IRQHandler()
{
	GPIOC->ODR |= (GPIO_ODR_OD0);
  if (TIM2->SR & TIM_SR_UIF) 		// Check which flag was reason for interrupt in the Status Register
  {
	Voltage_WE = 1;
	TIM2->SR &= ~(TIM_SR_UIF); // Clear interrupt flag
  }
  GPIOC->ODR &= ~(GPIO_ODR_OD0);
}

void Timer_Init()
{

	  RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);         // Turn on timer TIM2 RCC
	  TIM2->ARR  = 360 - 1;                           //Auto Reload Register chooses when Timer starts over
	  TIM2->DIER |= TIM_DIER_UIE;   //Enable Interrupt when ARR value is reached
	  TIM2->SR &= ~(TIM_SR_UIF); //This clears both flags
	  TIM2->CR1 |= TIM_CR1_CEN;	//Turn on the Counter
	  NVIC->ISER[0] = (1 << TIM2_IRQn);             // enable interrupts for TIM2 in NVIC
	  __enable_irq();                               // enable interrupts globally
}


void DAC_Write(uint16_t value)
{
	const uint16_t FIFOEMPTYMASK = 14 << 11; //Signal to enable DAC
	GPIOA->ODR &= ~(GPIO_ODR_OD4); //put chip select low
	SPI1->DR = (DAC_volt_conv(value) | FIFOEMPTYMASK); //Write correct value plus DAC enable signal to DAC
	while (SPI1->SR & SPI_SR_BSY) {} //Make sure that all data is written
	GPIOA->ODR |= GPIO_ODR_OD4; //put chip select high
	return;
}

int DAC_volt_conv(int value)
{
	if(value > 3000)
	{
		return(4095);
	}
	return ((value * 4095) / 3000);  //Normalize value based on highest value and DAC ability
}


int Keypad_Output()
{
	int buttonPress = 0;
	int row = -1;
	int column = -1;


	if(buttonPress == 0)
	{
		// Set all cols high
		GPIOC->ODR |= (1 << GPIO_ODR_OD10_Pos | 1 << GPIO_ODR_OD11_Pos | 1 << GPIO_ODR_OD12_Pos);

		//row 1
		if((GPIOC->IDR & GPIO_IDR_ID5) != 0)
		{
		  row = 0;
		  buttonPress = 1;
		  GPIOC->ODR &= ~(1 << GPIO_ODR_OD10_Pos | 1 << GPIO_ODR_OD11_Pos | 1 << GPIO_ODR_OD12_Pos);
		}
		//row 2
		else if((GPIOC->IDR & GPIO_IDR_ID6) != 0)
		{
		  row = 1;
		  buttonPress = 1;
		  GPIOC->ODR &= ~(1 << GPIO_ODR_OD10_Pos | 1 << GPIO_ODR_OD11_Pos | 1 << GPIO_ODR_OD12_Pos);
		}
		//row 3
		else if((GPIOC->IDR & GPIO_IDR_ID8) != 0)
		{
		  row = 2;
		  buttonPress = 1;
		  GPIOC->ODR &= ~(1 << GPIO_ODR_OD10_Pos | 1 << GPIO_ODR_OD11_Pos | 1 << GPIO_ODR_OD12_Pos);
		}
		//row 4
		else if((GPIOC->IDR & GPIO_IDR_ID9) != 0)
		{
		  row = 3;
		  buttonPress = 1;
		  GPIOC->ODR &= ~(1 << GPIO_ODR_OD10_Pos | 1 << GPIO_ODR_OD11_Pos | 1 << GPIO_ODR_OD12_Pos);
		}

	}
	if(buttonPress == 1)
	{

		//col One
		GPIOC->ODR |= (1 << GPIO_ODR_OD10_Pos);
		if((GPIOC->IDR & (GPIO_IDR_ID5 | GPIO_IDR_ID6 | GPIO_IDR_ID8 | GPIO_IDR_ID9)) != 0)
		{
		  column = 0;
		}
		GPIOC->ODR &= ~(1 << GPIO_ODR_OD10_Pos);

		//col Two
		GPIOC->ODR |= (1 << GPIO_ODR_OD11_Pos);
		if((GPIOC->IDR & (GPIO_IDR_ID5 | GPIO_IDR_ID6 | GPIO_IDR_ID8 | GPIO_IDR_ID9)) != 0)
		{
		  column = 1;
		}
		GPIOC->ODR &= ~(1 << GPIO_ODR_OD11_Pos);

		//col Two
		GPIOC->ODR |= (1 << GPIO_ODR_OD12_Pos);
		if((GPIOC->IDR & (GPIO_IDR_ID5 | GPIO_IDR_ID6 | GPIO_IDR_ID8 | GPIO_IDR_ID9)) != 0)
		{
		  column = 2;
		}
		GPIOC->ODR &= ~(1 << GPIO_ODR_OD12_Pos);
	}

	return get_key_pressed(row, column);
}

int get_key_pressed(int row, int col)
{

	  int keypad[4][3] =
	  		  		  {
	  		  		      {1, 2, 3},
	  		  		      {4, 5, 6},
	  		  		      {7, 8, 9},
	  		  		      {10, 0, 11}
	  		  		  };

    if (row >= 0 && row < 4 && col >= 0 && col < 3) {
        return keypad[row][col];  // Return the key value
    }
    return -1;  // Return null if invalid
}



void Keypad_Init()
{

	RCC->AHB2ENR |=  (RCC_AHB2ENR_GPIOCEN);

	//For input rows, pins 5,6,8,9

	GPIOC->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE8| GPIO_MODER_MODE9);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8| GPIO_PUPDR_PUPD9);
	GPIOC->PUPDR |= (2 << GPIO_PUPDR_PUPD5_Pos | 2 << GPIO_PUPDR_PUPD6_Pos | 2 << GPIO_PUPDR_PUPD8_Pos | 2 << GPIO_PUPDR_PUPD9_Pos);
	GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED5_Pos | 3 << GPIO_OSPEEDR_OSPEED6_Pos | 3 << GPIO_OSPEEDR_OSPEED8_Pos | 3 << GPIO_OSPEEDR_OSPEED9_Pos);



	//For output cols, pins 10,11,12

	GPIOC->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
	GPIOC->MODER |= (1 << GPIO_MODER_MODE10_Pos | 1 << GPIO_MODER_MODE11_Pos | 1 << GPIO_MODER_MODE12_Pos);
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
	GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED10_Pos | 3 << GPIO_OSPEEDR_OSPEED11_Pos | 3 << GPIO_OSPEEDR_OSPEED12_Pos);


//	//For LEDS, pins 0,1,2,3

		  GPIOC->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
		  GPIOC->MODER |= (1 << GPIO_MODER_MODE0_Pos | 1 << GPIO_MODER_MODE1_Pos| 1 << GPIO_MODER_MODE2_Pos| 1 << GPIO_MODER_MODE3_Pos);
		  GPIOC->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
		  GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
		  GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED0_Pos | 3 << GPIO_OSPEEDR_OSPEED1_Pos | 3 << GPIO_OSPEEDR_OSPEED2_Pos | 3 << GPIO_OSPEEDR_OSPEED3_Pos);
		  GPIOC->ODR &= ~(GPIO_ODR_OD1);
}


void DAC_Init()
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;    //enable SPI 1 Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   //enable GPIO A Clock

//CS is PA4

	GPIOA->MODER   &= ~(GPIO_MODER_MODE4);
	GPIOA->MODER   |= (GPIO_MODER_MODE4_0);
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT4);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED4);
	GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPD4);
	GPIOA->ODR     |= GPIO_ODR_OD4;


//COPI is PA7

	GPIOA->MODER   &= ~(GPIO_MODER_MODE7);
	GPIOA->MODER   |= (GPIO_MODER_MODE7_1);
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT7);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED7);
	GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPD7);


//SCLK is PA5

	GPIOA->MODER   &= ~(GPIO_MODER_MODE5);
	GPIOA->MODER   |= (GPIO_MODER_MODE5_1);
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT5);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED5);
	GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPD5);


	GPIOA->AFR[0]   &= ~(GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL7);
	GPIOA->AFR[0]   |= ( (5 << GPIO_AFRL_AFSEL4_Pos) | (5 << GPIO_AFRL_AFSEL5_Pos) | (5 << GPIO_AFRL_AFSEL7_Pos) );


	SPI1->CR1 &= ~(SPI_CR1_BR);
	SPI1->CR1 |= (SPI_CR1_BR_0); //baud rate set to rate/4

	SPI1->CR1 |= SPI_CR1_CPOL;
	SPI1->CR1 |= SPI_CR1_CPHA; //polarity is 0 and phase is 1

	SPI1->CR1 &= ~(SPI_CR1_RXONLY); //bidirectional mode

	SPI1->CR1 &= ~(SPI_CR1_LSBFIRST); //msb first

	SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI); //setting slave select management

	SPI1->CR1 |= SPI_CR1_MSTR;      //set STM as controller
	SPI1->CR1 |= SPI_CR1_SPE;       //enable SPI

	SPI1->CR2 &= ~(SPI_CR2_DS);
	SPI1->CR2 |= (15 << SPI_CR2_DS_Pos);
	SPI1->CR1 |= SPI_CR1_SPE;

}





/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

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

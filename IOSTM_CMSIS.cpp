/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016, 2017 by Andy Uribe CA6JAU
 *   Copyright (C) 2017,2018,2020,2023 by Jonathan Naylor G4KLX
 *   Copyright (C) 2017 by Wojciech Krutnik N0CALL
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Config.h"
#include "Globals.h"
#include "IO.h"

#if defined(STM32F1)

#if defined(STM32F1_POG)
/*
Pin definitions for STM32F1 POG Board:

PTT      PB12   output
COSLED   PB5    output
LED      PB4    output

MODE1    PB7    output
MODE2    PB6    output
MODE3    PB8    output
MODE4    PB9    output

RX       PB0    analog input (ADC1_8)
TX       PA4    analog output (DAC_OUT1)

EXT_CLK  PA15   input (AF: TIM2_CH1_ETR)

USART1_TXD PA9  output (AF)
USART1_RXD PA10  input (AF)
*/

#define PIN_PTT           12
#define PORT_PTT          GPIOB
#define BB_PTT            *((bitband_t)BITBAND_PERIPH(&PORT_PTT->ODR, PIN_PTT))
#define PIN_COSLED        5
#define PORT_COSLED       GPIOB
#define BB_COSLED         *((bitband_t)BITBAND_PERIPH(&PORT_COSLED->ODR, PIN_COSLED))
#define PIN_LED           4
#define PORT_LED          GPIOB
#define BB_LED            *((bitband_t)BITBAND_PERIPH(&PORT_LED->ODR, PIN_LED))

#define PIN_MODE1         7
#define PORT_MODE1        GPIOB
#define BB_MODE1          *((bitband_t)BITBAND_PERIPH(&PORT_MODE1->ODR, PIN_MODE1))
#define PIN_MODE2         6
#define PORT_MODE2        GPIOB
#define BB_MODE2          *((bitband_t)BITBAND_PERIPH(&PORT_MODE2->ODR, PIN_MODE2))
#define PIN_MODE3         8
#define PORT_MODE3        GPIOB
#define BB_MODE3          *((bitband_t)BITBAND_PERIPH(&PORT_MODE3->ODR, PIN_MODE3))
#define PIN_MODE4         9
#define PORT_MODE4        GPIOB
#define BB_MODE4          *((bitband_t)BITBAND_PERIPH(&PORT_MODE4->ODR, PIN_MODE4))

#define PIN_RX            0
#define PIN_RX_ADC_CH     8
#define PORT_RX           GPIOB
#define PIN_TX            4
#define PIN_TX_DAC_CH     1
#define PORT_TX           GPIOA

#define PIN_EXT_CLK       15
#define SRC_EXT_CLK       15
#define PORT_EXT_CLK      GPIOA

#define PIN_USART1_TXD    9
#define PORT_USART1_TXD   GPIOA
#define PIN_USART1_RXD    10
#define PORT_USART1_RXD   GPIOA

#else   // defined(STM32F1_POG)
#error "Either STM32F1_POG, or sth need to be defined"
#endif  // defined(STM32F1_POG)

const uint16_t DC_OFFSET = 2048U;

// Sampling frequency
#define SAMP_FREQ   24000

extern "C" {
  void TIM2_IRQHandler() {
    if ((TIM2->SR & TIM_SR_UIF) == TIM_SR_UIF) {
      TIM2->SR = ~TIM_SR_UIF;   // clear UI flag
      io.interrupt();
    }
  }
}


void delay(uint32_t cnt)
{
  while(cnt--)
    asm("nop");
}

// source: http://www.freddiechopin.info/
void GPIOConfigPin(GPIO_TypeDef *port_ptr, uint32_t pin, uint32_t mode_cnf_value)
{
	volatile uint32_t *cr_ptr;
	uint32_t cr_value;

	cr_ptr = &port_ptr->CRL;  // configuration of pins [0,7] is in CRL

	if (pin >= 8)			        // is pin in [8; 15]?
	{									        // configuration of pins [8,15] is in CRH
		cr_ptr++;               // advance to next struct element CRL -> CRH
		pin -= 8;               // crop the pin number
	}

	cr_value = *cr_ptr;			  // localize the CRL / CRH value

	cr_value &= ~(0xF << (pin * 4));	// clear the MODE and CNF fields (now that pin is an analog input)
	cr_value |= (mode_cnf_value << (pin * 4));	        // save new MODE and CNF value for desired pin

	*cr_ptr = cr_value;				// save localized value to CRL / CRL
}

#if defined(STM32F1_POG)
void FancyLEDEffect()
{
  bitband_t foo[] = {&BB_LED, &BB_COSLED, &BB_PTT, &BB_MODE2, &BB_MODE1, &BB_MODE3, &BB_MODE4};
   
  for(int i=0; i<7; i++){
    *foo[i] = 0x01;
  }
  GPIOConfigPin(PORT_USART1_TXD, PIN_USART1_TXD, GPIO_CRL_MODE0_1);
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x00;
  delay(SystemCoreClock/1000*100);
  for(int i=0; i<7; i++){
    *foo[i] = 0x00;
  }
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x01;
  delay(SystemCoreClock/1000*20);
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x00;
  delay(SystemCoreClock/1000*10);
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x01;
  
  *foo[0] = 0x01;
  for(int i=1; i<7; i++){
    delay(SystemCoreClock/1000*10);
    *foo[i-1] = 0x00;
    *foo[i] = 0x01;
  }
  for(int i=5; i>=0; i--){
    delay(SystemCoreClock/1000*10);
    *foo[i+1] = 0x00;
    *foo[i] = 0x01;
  }
  delay(SystemCoreClock/1000*10);
  *foo[5+1-6] = 0x00;
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x00;
  delay(SystemCoreClock/1000*10);
  *((bitband_t)BITBAND_PERIPH(&PORT_USART1_TXD->ODR, PIN_USART1_TXD)) = 0x01;
  GPIOConfigPin(PORT_USART1_TXD, PIN_USART1_TXD, GPIO_CRL_MODE0_1|GPIO_CRL_CNF0_1);
  delay(SystemCoreClock/1000*50);
}
#endif

static inline void GPIOInit()
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN
                  | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN
                  | RCC_APB2ENR_AFIOEN;	// enable all GPIOs
                 
	// set all ports to input with pull-down
	GPIOA->CRL = 0x88888888;
	GPIOA->CRH = 0x88888888;
	GPIOA->ODR = 0;
	GPIOB->CRL = 0x88888888;
	GPIOB->CRH = 0x88888888;
	GPIOB->ODR = 0;
	GPIOC->CRL = 0x88888888;
	GPIOC->CRH = 0x88888888;
	GPIOC->ODR = 0;
	GPIOD->CRL = 0x88888888;
	GPIOD->CRH = 0x88888888;
	GPIOD->ODR = 0;
  
  // configure ports
  GPIOConfigPin(PORT_PTT,    PIN_PTT,    GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_COSLED, PIN_COSLED, GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_LED,    PIN_LED,    GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_COS,    PIN_COS,    GPIO_CRL_CNF0_1);
  
  GPIOConfigPin(PORT_MODE1,  PIN_MODE1,  GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_MODE2,  PIN_MODE2,  GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_MODE3,  PIN_MODE3,  GPIO_CRL_MODE0_1);
  GPIOConfigPin(PORT_MODE4,  PIN_MODE4,  GPIO_CRL_MODE0_1);

  GPIOConfigPin(PORT_RX, PIN_RX, 0);
  GPIOConfigPin(PORT_TX, PIN_TX, 0);

#if defined(EXTERNAL_OSC)
#if defined(STM32F1_POG)
  GPIOConfigPin(PORT_EXT_CLK, PIN_EXT_CLK, GPIO_CRL_CNF0_0);
  AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_TIM2_REMAP) | AFIO_MAPR_TIM2_REMAP_0;
#endif
#endif

  GPIOConfigPin(PORT_USART1_TXD, PIN_USART1_TXD, GPIO_CRL_MODE0_1|GPIO_CRL_CNF0_1);
  GPIOConfigPin(PORT_USART1_RXD, PIN_USART1_RXD, GPIO_CRL_CNF0_0);
  
  AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | AFIO_MAPR_SWJ_CFG_1;
}


static inline void ADCInit()
{
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_ADCPRE_Msk)
              | RCC_CFGR_ADCPRE_DIV6; // ADC clock divided by 6 (72MHz/6 = 12MHz)
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  // Set sampling time to 7.5 cycles
  if (PIN_RX_ADC_CH < 10) {
    ADC1->SMPR2 = ADC_SMPR2_SMP0_0 << (3*PIN_RX_ADC_CH); 
  } else {
    ADC1->SMPR1 = ADC_SMPR1_SMP10_0 << (3*PIN_RX_ADC_CH);
  }

  // Set conversion channel (single conversion)
  ADC1->SQR3 = PIN_RX_ADC_CH;

  // Enable ADC
  ADC1->CR2 |= ADC_CR2_ADON;

  // Start calibration
  delay(6*2);
  ADC1->CR2 |= ADC_CR2_CAL;
  while((ADC1->CR2 & ADC_CR2_CAL) == ADC_CR2_CAL)
    ;
  
  // Trigger first conversion
  ADC1->CR2 |= ADC_CR2_ADON;
}

static inline void DACInit()
{
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;

  DAC->CR = DAC_CR_EN1;
}

static inline void TimerInit()
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

#if defined(EXTERNAL_OSC)
  // Enable external clock, prescaler /4
  TIM2->SMCR = TIM_SMCR_ECE | TIM_SMCR_ETPS_1;
#endif

  // TIM2 output frequency
  TIM2->PSC = 0;
#if defined(EXTERNAL_OSC)
  // TimerCount = ExternalOsc
  //              /4 (external clock prescaler)
  //              /SAMP_FREQ
  TIM2->ARR = (uint16_t) ((EXTERNAL_OSC/(4*SAMP_FREQ)) - 1);
#else
  TIM2->ARR = (uint16_t) ((SystemCoreClock/(SAMP_FREQ)) - 1);
#endif

  // Enable TIse 1%-tolerance comM2 interrupt
  TIM2->DIER = TIM_DIER_UIE;
  NVIC_EnableIRQ(TIM2_IRQn);
  
  // Enable TIM2
  TIM2->CR1 |= TIM_CR1_CEN;
}

void CIO::initInt()
{
  GPIOInit();
  ADCInit();
  DACInit();
#if defined(STM32F1_POG)
  FancyLEDEffect();
#endif
}

void CIO::startInt()
{
  TimerInit();
   
  BB_COSLED = 0;
  BB_LED = 1;
}

void CIO::interrupt()
{
  uint16_t sample = 0U;

  bitband_t eoc = (bitband_t)BITBAND_PERIPH(&ADC1->SR, ADC_SR_EOS_Pos);
  bitband_t adon = (bitband_t)BITBAND_PERIPH(&ADC1->CR2, ADC_CR2_ADON_Pos);

  if (*eoc) {
    // trigger next conversion
    *adon = 1;
    
    m_txBuffer.get(sample);
    DAC->DHR12R1 = sample;  // Send the value to the DAC

    // Read value from ADC1 and ADC2
    sample = ADC1->DR;   // read conversion result; EOC is cleared by this read
    m_rxBuffer.put(sample);
  }

  m_ledCount++;
}

void CIO::setLEDInt(bool on)
{
  BB_LED = !!on;
}

void CIO::setPTTInt(bool on)
{
  BB_PTT = !!on;
}

void CIO::setCOSInt(bool on)
{
  BB_COSLED = !!on;
}

void CIO::setMode1Int(bool on)
{
  BB_MODE1 = !!on;
}

void CIO::setMode2Int(bool on)
{
  BB_MODE2 = !!on;
}

void CIO::setMode3Int(bool on)
{
  BB_MODE3 = !!on;
}

void CIO::setMode4Int(bool on)
{
  BB_MODE4 = !!on;
}

void CIO::delayInt(unsigned int dly)
{
  delay(dly);
}

uint8_t CIO::getCPU() const
{
  return 2U;
}

void CIO::getUDID(uint8_t* buffer)
{
#if defined(STM32F105xC)
  ::memcpy(buffer, (void *)0x1FFFF7E8, 12U);
#endif
}

#endif


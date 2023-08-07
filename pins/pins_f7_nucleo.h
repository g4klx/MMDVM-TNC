/*
 *   Copyright (C) 2019,2020 by BG5HHP
 *   Copyright (C) 2023 by Jonathan Naylor, G4KLX
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

#ifndef _PINS_F7_NUCLEO_H
#define _PINS_F7_NUCLEO_H

/*
Pin definitions for STM32F7 Nucleo boards (ST Morpho header):

PTT      PB13   output           CN12 Pin30
COSLED   PB14   output           CN12 Pin28
LED      PA5    output           CN12 Pin11
COS      PB15   input            CN12 Pin26

MODE1    PB10   output           CN12 Pin25
MODE2    PB4    output           CN12 Pin27
MODE3    PB5    output           CN12 Pin29
MODE4    PB3    output           CN12 Pin31

MMODE1   PC4    output           CN12 Pin34
MMODE2   PC5    output           CN12 Pin6
MMODE3   PC2    output           CN11 Pin35
MMODE4   PC3    output           CN11 Pin37

RX       PA0    analog input     CN11 Pin28
RSSI     PA1    analog input     CN11 Pin30
TX       PA4    analog output    CN11 Pin32

EXT_CLK  PA15   input            CN11 Pin17
*/

#define PIN_COS           GPIO_Pin_15
#define PORT_COS          GPIOB
#define RCC_Per_COS       RCC_AHB1Periph_GPIOB

#define PIN_PTT           GPIO_Pin_13
#define PORT_PTT          GPIOB
#define RCC_Per_PTT       RCC_AHB1Periph_GPIOB

#define PIN_COSLED        GPIO_Pin_14
#define PORT_COSLED       GPIOB
#define RCC_Per_COSLED    RCC_AHB1Periph_GPIOB

#define PIN_LED           GPIO_Pin_5
#define PORT_LED          GPIOA
#define RCC_Per_LED       RCC_AHB1Periph_GPIOA

#define PIN_MODE1         GPIO_Pin_10
#define PORT_MODE1        GPIOB
#define RCC_Per_MODE1     RCC_AHB1Periph_GPIOB

#define PIN_MODE2         GPIO_Pin_4
#define PORT_MODE2        GPIOB
#define RCC_Per_MODE2     RCC_AHB1Periph_GPIOB

#define PIN_MODE3           GPIO_Pin_5
#define PORT_MODE3          GPIOB
#define RCC_Per_MODE3       RCC_AHB1Periph_GPIOB

#define PIN_MODE4         GPIO_Pin_3
#define PORT_MODE4        GPIOB
#define RCC_Per_MODE4     RCC_AHB1Periph_GPIOB

#if defined(MODE_PINS)
#define PIN_MMODE1        GPIO_Pin_4
#define PORT_MMODE1       GPIOC
#define RCC_Per_MMODE1    RCC_AHB1Periph_GPIOC

#define PIN_MMODE2        GPIO_Pin_5
#define PORT_MMODE2       GPIOC
#define RCC_Per_MMODE2    RCC_AHB1Periph_GPIOC

#define PIN_MMODE3        GPIO_Pin_2
#define PORT_MMODE3       GPIOC
#define RCC_Per_MMODE3    RCC_AHB1Periph_GPIOC

#define PIN_MMODE4        GPIO_Pin_3
#define PORT_MMODE4       GPIOC
#define RCC_Per_MMODE4    RCC_AHB1Periph_GPIOC
#endif

#define PIN_EXT_CLK       GPIO_Pin_15
#define SRC_EXT_CLK       GPIO_PinSource15
#define PORT_EXT_CLK      GPIOA

#define PIN_RX            GPIO_Pin_0
#define PIN_RX_CH         ADC_Channel_0
#define PORT_RX           GPIOA
#define RCC_Per_RX        RCC_AHB1Periph_GPIOA

#define PIN_RSSI          GPIO_Pin_1
#define PIN_RSSI_CH       ADC_Channel_1
#define PORT_RSSI         GPIOA
#define RCC_Per_RSSI      RCC_AHB1Periph_GPIOA

#define PIN_TX            GPIO_Pin_4
#define PIN_TX_CH         DAC_Channel_1

#endif

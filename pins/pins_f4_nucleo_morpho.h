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

#ifndef _PINS_F4_NUCLEO_MORPHO_H
#define _PINS_F4_NUCLEO_MORPHO_H

/*
Pin definitions for STM32F4 Nucleo boards (ST Morpho header):

PTT      PB13   output           CN10 Pin30
COSLED   PB14   output           CN10 Pin28
LED      PA5    output           CN10 Pin11

MODE1    PB10   output           CN10 Pin25
MODE2    PB4    output           CN10 Pin27
MODE3    PB5    output           CN10 Pin29
MODE4    PB3    output           CN10 Pin31

RX       PA0    analog input     CN7 Pin28
TX       PA4    analog output    CN7 Pin32

EXT_CLK  PA15   input            CN7 Pin17
*/

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

#define PIN_MODE3         GPIO_Pin_5
#define PORT_MODE3        GPIOB
#define RCC_Per_MODE3     RCC_AHB1Periph_GPIOB

#define PIN_MODE4         GPIO_Pin_3
#define PORT_MODE4        GPIOB
#define RCC_Per_MODE4     RCC_AHB1Periph_GPIOB

#define PIN_EXT_CLK       GPIO_Pin_15
#define SRC_EXT_CLK       GPIO_PinSource15
#define PORT_EXT_CLK      GPIOA

#define PIN_RX            GPIO_Pin_0
#define PIN_RX_CH         ADC_Channel_0
#define PORT_RX           GPIOA
#define RCC_Per_RX        RCC_AHB1Periph_GPIOA

#define PIN_TX            GPIO_Pin_4
#define PIN_TX_CH         DAC_Channel_1

#endif

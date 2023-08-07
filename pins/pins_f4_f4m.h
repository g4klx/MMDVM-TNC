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

#ifndef _PINS_F4_F4M_H
#define _PINS_F4_F4M_H

/*
Pin definitions for MMDVM-F4M Pi-Hat F0DEI board:

PTT      PB13   output
COSLED   PB14   output
LED      PB15   output
COS      PC0    input

MODE1    PC7    output
MODE2    PC8    output
MODE3    PA8    output
MODE4    PC9    output

RX       PA0    analog input
RSSI     PA7    analog input
TX       PA4    analog output

EXT_CLK  PA15   input
*/

#define PIN_COS           GPIO_Pin_0
#define PORT_COS          GPIOC
#define RCC_Per_COS       RCC_AHB1Periph_GPIOC

#define PIN_PTT           GPIO_Pin_13
#define PORT_PTT          GPIOB
#define RCC_Per_PTT       RCC_AHB1Periph_GPIOB

#define PIN_COSLED        GPIO_Pin_14
#define PORT_COSLED       GPIOB
#define RCC_Per_COSLED    RCC_AHB1Periph_GPIOB

#define PIN_LED           GPIO_Pin_15
#define PORT_LED          GPIOB
#define RCC_Per_LED       RCC_AHB1Periph_GPIOB

#define PIN_MODE1         GPIO_Pin_7
#define PORT_MODE1        GPIOC
#define RCC_Per_MODE1     RCC_AHB1Periph_GPIOC

#define PIN_MODE2         GPIO_Pin_8
#define PORT_MODE2        GPIOC
#define RCC_Per_MODE2     RCC_AHB1Periph_GPIOC

#define PIN_MODE3         GPIO_Pin_8
#define PORT_MODE3        GPIOA
#define RCC_Per_MODE3     RCC_AHB1Periph_GPIOA

#define PIN_MODE4         GPIO_Pin_9
#define PORT_MODE4        GPIOC
#define RCC_Per_MODE4     RCC_AHB1Periph_GPIOC

#define PIN_EXT_CLK       GPIO_Pin_15
#define SRC_EXT_CLK       GPIO_PinSource15
#define PORT_EXT_CLK      GPIOA

#define PIN_RX            GPIO_Pin_0
#define PIN_RX_CH         ADC_Channel_0
#define PORT_RX           GPIOA
#define RCC_Per_RX        RCC_AHB1Periph_GPIOA

#define PIN_RSSI          GPIO_Pin_7
#define PIN_RSSI_CH       ADC_Channel_7
#define PORT_RSSI         GPIOA
#define RCC_Per_RSSI      RCC_AHB1Periph_GPIOA

#define PIN_TX            GPIO_Pin_4
#define PIN_TX_CH         DAC_Channel_1

#endif

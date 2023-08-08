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

#ifndef _PINS_F4_DRCC_HHP446_H
#define _PINS_F4_DRCC_HHP446_H

/*
Pin definitions for DRCC_DVM BG5HHP F446 board rev1

TX/PTT_LED  PB12   output
RX/COS_LED  PB5    output
STATUS_LED  PB10   output

MODE1       N/A
MODE2       N/A
MODE3       N/A
MODE4       N/A

RX          PA0    analog input
TX          PA4    analog output

EXT_CLK     N/A

UART1_TX    PA9    output
UART1_RX    PA10   output          Host Data Communication

UART2_TX    PA2    output
UART2_RX    PA3    output          Nextion Data Communication

I2C1_SCL    PB6    output
I2C1_SDA    PB7    output          OLED Data Communication as master

*/

#define PIN_PTT           GPIO_Pin_12
#define PORT_PTT          GPIOB
#define RCC_Per_PTT       RCC_AHB1Periph_GPIOB

#define PIN_COSLED        GPIO_Pin_5
#define PORT_COSLED       GPIOB
#define RCC_Per_COSLED    RCC_AHB1Periph_GPIOB

#define PIN_LED           GPIO_Pin_10
#define PORT_LED          GPIOB
#define RCC_Per_LED       RCC_AHB1Periph_GPIOB

#define PIN_TXLED         GPIO_Pin_4
#define PORT_TXLED        GPIOB
#define RCC_Per_TXLED     RCC_AHB1Periph_GPIOB

// #define PIN_MODE4         GPIO_Pin_3
// #define PORT_MODE4        GPIOB
// #define RCC_Per_MODE4     RCC_AHB1Periph_GPIOB

// #define PIN_MODE1         GPIO_Pin_10
// #define PORT_MODE1        GPIOB
// #define RCC_Per_MODE1     RCC_AHB1Periph_GPIOB

// #define PIN_MODE2         GPIO_Pin_4
// #define PORT_MODE2        GPIOB
// #define RCC_Per_MODE2     RCC_AHB1Periph_GPIOB

// #define PIN_MODE3         GPIO_Pin_5
// #define PORT_MODE3        GPIOB
// #define RCC_Per_MODE3     RCC_AHB1Periph_GPIOB

#define PIN_EXT_CLK       GPIO_Pin_15
#define SRC_EXT_CLK       GPIO_PinSource15
#define PORT_EXT_CLK      GPIOA

#define PIN_RX            GPIO_Pin_0
#define PIN_RX_CH         ADC_Channel_0
#define PORT_RX           GPIOA
#define RCC_Per_RX        RCC_AHB1Periph_GPIOA

#define PIN_TX            GPIO_Pin_4
#define PIN_TX_CH         DAC_Channel_1
#define PORT_TX           GPIOA
#define RCC_Per_TX        RCC_AHB1Periph_GPIOA

#endif

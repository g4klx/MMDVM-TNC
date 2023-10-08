/*
 *   Copyright (C) 2016,2017,2018,2020,2023 by Jonathan Naylor G4KLX
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

#if defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)

#if defined(EXTERNAL_OSC)
#define PIN_LED                3
#else
#define PIN_LED                13
#endif
#define PIN_PTT                5
#define PIN_COSLED             6
#define PIN_MODE1              9
#define PIN_MODE2              10
#define PIN_MODE3              11
#define PIN_MODE4              12
#define PIN_ADC                5        // A0,  Pin 14

#define PDB_CHnC1_TOS 0x0100
#define PDB_CHnC1_EN  0x0001

const uint16_t DC_OFFSET = 2048U;

extern "C" {
  void adc0_isr()
  {
    io.interrupt();
  }
}

void CIO::initInt()
{
  // Set up the TX, COS and LED pins
  pinMode(PIN_PTT,    OUTPUT);
  pinMode(PIN_COSLED, OUTPUT);
  pinMode(PIN_LED,    OUTPUT);

#if defined(MODE_LEDS)
  // Set up the mode output pins
  pinMode(PIN_MODE1,  OUTPUT);
  pinMode(PIN_MODE2,  OUTPUT);
  pinMode(PIN_MODE3,  OUTPUT);
  pinMode(PIN_MODE4,  OUTPUT);
#endif
}

void CIO::startInt()
{
  // Initialise the DAC
  SIM_SCGC2 |= SIM_SCGC2_DAC0;
  DAC0_C0    = DAC_C0_DACEN | DAC_C0_DACRFS;                          // 3.3V VDDA is DACREF_2

  // Initialise ADC0
  SIM_SCGC6 |= SIM_SCGC6_ADC0;
  ADC0_CFG1  = ADC_CFG1_ADIV(1) | ADC_CFG1_ADICLK(1) |                // Single-ended 12 bits, long sample time
               ADC_CFG1_MODE(1) | ADC_CFG1_ADLSMP;
  ADC0_CFG2  = ADC_CFG2_MUXSEL | ADC_CFG2_ADLSTS(2);                  // Select channels ADxxxb
  ADC0_SC2   = ADC_SC2_REFSEL(0) | ADC_SC2_ADTRG;                     // Voltage ref external, hardware trigger
  ADC0_SC3   = ADC_SC3_AVGE | ADC_SC3_AVGS(0);                        // Enable averaging, 4 samples

  ADC0_SC3  |= ADC_SC3_CAL;
  while (ADC0_SC3 & ADC_SC3_CAL)                                      // Wait for calibration
    ;

  uint16_t sum0 = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 +                 // Plus side gain
                  ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
  sum0 = (sum0 / 2U) | 0x8000U;
  ADC0_PG    = sum0;

  ADC0_SC1A  = ADC_SC1_AIEN | PIN_ADC;                                // Enable ADC interrupt, use A0
  NVIC_ENABLE_IRQ(IRQ_ADC0);

#if defined(EXTERNAL_OSC)
  // Set ADC0 to trigger from the LPTMR at 48 kHz
  SIM_SOPT7   = SIM_SOPT7_ADC0ALTTRGEN |                              // Enable ADC0 alternate trigger
                SIM_SOPT7_ADC0TRGSEL(14);                             // Trigger ADC0 by LPTMR0

  CORE_PIN13_CONFIG = PORT_PCR_MUX(3);

  SIM_SCGC5  |= SIM_SCGC5_LPTIMER;                                    // Enable Low Power Timer Access
  LPTMR0_CSR  = 0;                                                    // Disable
  LPTMR0_PSR  = LPTMR_PSR_PBYP;                                       // Bypass prescaler/filter
  LPTMR0_CMR  = (EXTERNAL_OSC / 48000) - 1;                           // Frequency divided by CMR + 1
  LPTMR0_CSR  = LPTMR_CSR_TPS(2) |                                    // Pin: 0=CMP0, 1=xtal, 2=pin13
                LPTMR_CSR_TMS;                                        // Mode Select, 0=timer, 1=counter
  LPTMR0_CSR |= LPTMR_CSR_TEN;                                        // Enable
#else
  // Setup PDB for ADC0 at 48 kHz
  SIM_SCGC6  |= SIM_SCGC6_PDB;                                        // Enable PDB clock
  PDB0_MOD    = (F_BUS / 48000) - 1;                                  // Timer period - 1
  PDB0_IDLY   = 0;                                                    // Interrupt delay
  PDB0_CH0C1  = PDB_CHnC1_TOS | PDB_CHnC1_EN;                         // Enable pre-trigger for ADC0
  PDB0_SC     = PDB_SC_TRGSEL(15) | PDB_SC_PDBEN |                    // SW trigger, enable interrupts, continuous mode
                PDB_SC_PDBIE | PDB_SC_CONT | PDB_SC_LDOK;             // No prescaling
  PDB0_SC    |= PDB_SC_SWTRIG;                                        // Software trigger (reset and restart counter)
#endif

  digitalWrite(PIN_PTT,    LOW);
  digitalWrite(PIN_COSLED, LOW);
  digitalWrite(PIN_LED,    HIGH);
}

void CIO::interrupt()
{
  uint16_t sample = DC_OFFSET;

  m_txBuffer.get(sample);
  *(int16_t *)&(DAC0_DAT0L) = sample;

  if ((ADC0_SC1A & ADC_SC1_COCO) == ADC_SC1_COCO) {
    sample = ADC0_RA;
    m_rxBuffer.put(sample);
  }

  m_ledCount++;
}

void CIO::setLEDInt(bool on)
{
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

void CIO::setPTTInt(bool on)
{
  digitalWrite(PIN_PTT, on ? HIGH : LOW);
}

void CIO::setCOSInt(bool on)
{
  digitalWrite(PIN_COSLED, on ? HIGH : LOW);
}

void CIO::setMode1Int(bool on)
{
  digitalWrite(PIN_MODE1, on ? HIGH : LOW);
}

void CIO::setMode2Int(bool on) 
{
  digitalWrite(PIN_MODE2, on ? HIGH : LOW);
}

void CIO::setMode3Int(bool on)
{
  digitalWrite(PIN_MODE3, on ? HIGH : LOW);
}

void CIO::setMode4Int(bool on) 
{
  digitalWrite(PIN_MODE4, on ? HIGH : LOW);
}

void CIO::delayInt(unsigned int dly)
{
  delay(dly);
}

uint8_t CIO::getCPU() const
{
  return 1U;
}

void CIO::getUDID(uint8_t* buffer)
{
  ::memcpy(buffer, (void *)0x4058, 16U);
}

#endif


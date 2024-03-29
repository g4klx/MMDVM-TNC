This is the source code of the MMDVM-TNC firmware that supports standard 1200 bps AFSK AX.25, 9600 bps C4FSK IL2P in a 12.5 kHz bandwidth, and 19200 bps C4FSK IL2P in a 12.5 kHz bandwidth. The 9600 and 19200 bps mode uses the same on-air waveform as DMR, but is incompatible with it in every sense. One big difference is that there is no correct way in which the deviation is decoded and so the receive side is able to detect and decode transmissions of either sense. This is why there is no transmit or receive invert settings to be found anywhere.

Standard KISS command over the MMDVM serial port are used, the speed of which is set to 115200 baud, although this can be changed in Config.h at compile time.

The KISS SET HARDWARE command has two versions that allow it to control the modem (all of these settings may also be set in Config.h at compile time).

A SET HARDWARE command with a single one byte argument sets the mode. The modes are 1200 bps AFSK AX.25 is mode 1, 9600 bps C4FSK IL2P is mode 2, and 19200 bps C4FSK IL2P is mode 3. The mode is shown on the modem LEDs with D-Star showing mode 1, DMR for mode 2, and YSF for mode 3. The other version of the command has four one byte arguments, the first byte being the Receive Level which has a range of 0 to 255, the second byte is the mode 1 Transmit Level which may be between 0 and 255, the third byte is the mode 2 Transmit Level which is between 0 and 255, and the fourth byte is the mode 3 Transmit Level which is also between 0 and 255.

Simple debugging is optionally available over the modems display serial port, usually used for Nextion displays, and these are output at 38400 baud. These may be switched on and off in Config.h.

It runs on the the ST-Micro STM32F4xxx and STM32F7xxx processors.

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.

Portions of the ARM support code include the following copyright:

   Copyright (c) 2011 - 2013 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

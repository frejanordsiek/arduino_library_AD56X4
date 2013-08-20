/* Copyright (c) 2013, Freja Nordsiek
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:
   
     Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
   
     Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
   
     Neither the name of the {organization} nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
   AD56X4.h: Library for controlling the Analog Devices AD56X4
                 Quad DAC family:
                   AD5624   12-bit
                   AD5664   16-bit
                   AD5624R  12-bit with internal reference
                   AD5644R  14-bit with internal reference
                   AD5664R  16-bit with internal reference
             Data sheet can be found at http://www.analog.com/en/digital-to-analog-converters/da-converters/ad5624r/products/product.html?doc=AD5624R_5644R_5664R.PDF
   
   Author:   Freja Nordsiek
   Notes:
   History:  * 2013-08-15 Created.
*/

/* http://www.analog.com/en/digital-to-analog-converters/da-converters/ad5624r/products/product.html?doc=AD5624R_5644R_5664R.PDF

   The Analog Devices AD56X4 are Quad channel 1X-bit Digital to
   Analog Converter (DAC) controlled by SPI. It can use an external
   reference or its own internal reference if present (an R is
   appended to the chip name) and has various shutdown modes for
   each channel. Each channel has separate DAC (output) and input
   (buffer) registers and commands exist to set just the input
   register, assign the input to the DAC, or set both.
   
   The chip is an SPI slave that expects SPI_MODE1 for its polarity
   and phase (read on falling edge and clock low while idle) that
   can operate at up to 50 MHz, which is far above what the arduino
   can do (8 MHz for the Uno). The DIN, SCLK, and SYNC pins in the
   chip documentation correspond to the Arduino MOSI, SCK, and SS
   pins (doesn't have to use the SS for the arduino, but it is a
   a reasonable default pin). Messages must be sent MSB first and
   are 24 bits. The first two bits are junk and are ignored. The
   next three bits are the command bits, which tell the DAC what
   operation to do. The next three bits are the address bits, which
   tells what channel (or all) should be set. The last two whole
   bytes are a word specifying the value to set the appropriate
   register on the DAC channel if setting a channel, or other needed
   information if doing another sort of command. For the 14-bit and
   12-bit chips, the last 2 and 4 bits respectively are ignored if
   an input or DAC register is being set. The command bits and address
   bits are
   
   Command bits
   | C2 | C1 | C0 | Command
   ------------------------------------------------------------
   | 0  | 0  | 0  | Write to input register N
   | 0  | 0  | 1  | Update DAC register N
   | 0  | 1  | 0  | Write to input register N and update on all
   | 0  | 1  | 1  | Write to and update channel N
   | 1  | 0  | 0  | Power up-down
   | 1  | 0  | 1  | Reset
   | 1  | 1  | 0  | Set LDAC register
   | 1  | 1  | 1  | Internal reference on/off (if present)
   ------------------------------------------------------------
   
   Address bits
   | A2 | A1 | A0 | Channel/s
   --------------------------
   | 0  | 0  | 0  | A
   | 0  | 0  | 1  | B
   | 0  | 1  | 0  | C
   | 0  | 1  | 1  | D
   | 1  | 1  | 1  | All
   --------------------------
   
   For the power up-down command, the power mode specified with
   data bits DB5 and DB4 are applied to the channels specified by
   ones in the bit mask made by DB3 to DB0 (in channel D to A
   order). The power modes are
   
   Power up-down mode
   | DB5 | DB4 | mode
   ------------------------------
   |  0  |  0  | normal operation
   |  0  |  1  | 1k to ground
   |  1  |  0  | 100k to ground
   |  1  |  1  | tri-state
   ------------------------------
   
   For the reset command, DB0 controls exactly what is reset.
   Regardless of its value, the input and DAC registers are all
   reset to zero. If DB0 is set to one; then the LDAC register
   is reset to zero, all channels powered up to normal mode, and
   the internal reference turned off (means external reference is
   used).
   
   For the set LDAC register command, whether to just set the DAC
   register to the input register every time it is changed or not,
   is set for each channel with DB3 to DB0 corresponding to channel
   D through A. A zero means that setting the input register does
   not automatically set the DAC register (value isn't written out
   as output yet) and a one means that the DAC register (and hence
   the output) is set the the input register the moment the input
   register is changed.
   
   For the internal reference on/off command, setting DB0 to zero
   turns it off and setting it to one turns it on. When off, the
   external reference is used as the voltage reference. This
   command only works in the R versions that have an internal
   reference.
   
*/

#ifndef AD56X4_h
#define AD56X4_h

#include "Arduino.h"
#include <SPI.h>

/* For the various defined values, they are chosen so that they
   are exactly the values that need to be put into the SPI
   message. This makes it a lot easier to construct the message.
*/

#define AD56X4_COMMAND_WRITE_INPUT_REGISTER            B00000000
#define AD56X4_COMMAND_UPDATE_DAC_REGISTER             B00001000
#define AD56X4_COMMAND_WRITE_INPUT_REGISTER_UPDATE_ALL B00010000
#define AD56X4_COMMAND_WRITE_UPDATE_CHANNEL            B00011000
#define AD56X4_COMMAND_POWER_UPDOWN                    B00100000
#define AD56X4_COMMAND_RESET                           B00101000
#define AD56X4_COMMAND_SET_LDAC                        B00110000
#define AD56X4_COMMAND_REFERENCE_ONOFF                 B00111000

#define AD56X4_CHANNEL_A                               B00000000
#define AD56X4_CHANNEL_B                               B00000001
#define AD56X4_CHANNEL_C                               B00000010
#define AD56X4_CHANNEL_D                               B00000011
#define AD56X4_CHANNEL_ALL                             B00000111

#define AD56X4_SETMODE_INPUT                           AD56X4_COMMAND_WRITE_INPUT_REGISTER
#define AD56X4_SETMODE_INPUT_DAC                       AD56X4_COMMAND_WRITE_UPDATE_CHANNEL
#define AD56X4_SETMODE_INPUT_DAC_ALL                   AD56X4_COMMAND_WRITE_INPUT_REGISTER_UPDATE_ALL

#define AD56X4_POWERMODE_NORMAL                        B00000000
#define AD56X4_POWERMODE_POWERDOWN_1K                  B00010000
#define AD56X4_POWERMODE_POWERDOWN_100K                B00100000
#define AD56X4_POWERMODE_TRISTATE                      B00110000



class AD56X4Class
{
  
  public:
  
    static void setChannel (int SS_pin, byte setMode, byte channel,
                            word value);
    static void setChannel (int SS_pin, byte setMode, word values[]);
    static void setChannel (int SS_pin, byte setMode, word value_D,
                            word value_C, word value_B, word value_A);
                            
    static void updateChannel (int SS_pin, byte channel);
    
    static void powerUpDown (int SS_pin, byte powerMode,
                             boolean channels[]);
    static void powerUpDown (int SS_pin, byte powerMode,
                             boolean channel_D, boolean channel_C,
                             boolean channel_B, boolean channel_A);
    static void powerUpDown (int SS_pin, byte powerModes[]);
    
    static void reset (int SS_pin, boolean fullReset);
    
    static void setInputMode (int SS_pin, boolean channels[]);
    static void setInputMode (int SS_pin, boolean channel_D,
                              boolean channel_C, boolean channel_B,
                              boolean channel_A);
    
    static void useInternalReference (int SS_pin, boolean yesno);
    
  private:
    inline static void powerUpDown (int SS_pin, byte powerMode,
                                    byte channelMask);
    
    inline static void setInputMode (int SS_pin, byte channelMask);
    
    static word makeChannelMask (boolean channels[]);
    static word makeChannelMask (boolean channel_D, boolean channel_C,
                                 boolean channel_B, boolean channel_A);
    
    static void writeMessage (int SS_pin, byte command, byte address,
                              word data);
    
};

extern AD56X4Class AD56X4;

#endif 

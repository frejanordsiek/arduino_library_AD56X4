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
   AD56X4.cpp: Library for controlling the Analog Devices AD56X4
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
   can do (8 MHz for the Uno). Messages must be sent MSB first and
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

#include "Arduino.h"
#include <SPI.h>
#include <AD56X4.h>

AD56X4Class AD56X4;

/* Commands the AD564X DAC whose Slave Select pin is SS_pin to
   set the values of the specified channel/s. The values are word
   with the 12/14/16-bit values the channels should be set at (last
   4 and 2 bits are ignored for 12-bit and 14-bit DACs). Either
   one channel can be set (or all channels set to the same value)
   with a channel byte that must be one of
   
   AD56X4_CHANNEL_A
   AD56X4_CHANNEL_B
   AD56X4_CHANNEL_C
   AD56X4_CHANNEL_D
   AD56X4_CHANNEL_ALL
   
   or separate values (as an array in D to A order or as four
   separate arguments) be set to each one. There are three ways
   they can be set, or set modes. They are
   
   AD56X4_SETMODE_INPUT          Set the channel/s's input register.
   AD56X4_SETMODE_INPUT_DAC      Set both the input and DAC
                                   registers for the channel/s.
   AD56X4_SETMODE_INPUT_DAC_ALL  Set channel/s's input register and
                                   then update all DAC registers
                                   from the input registers.
*/
void AD56X4Class::setChannel (int SS_pin, byte setMode, byte channel,
                              word value)
{
  // Don't do anything if we weren't given a valid setMode.
  if (setMode == AD56X4_SETMODE_INPUT 
      || setMode == AD56X4_SETMODE_INPUT_DAC 
      || setMode == AD56X4_SETMODE_INPUT_DAC_ALL)
    AD56X4.writeMessage(SS_pin,setMode,channel,value);
}
void AD56X4Class::setChannel (int SS_pin, byte setMode, word values[])
{
  // Don't do anything if we weren't given a valid setMode.
  if (setMode == AD56X4_SETMODE_INPUT 
      || setMode == AD56X4_SETMODE_INPUT_DAC 
      || setMode == AD56X4_SETMODE_INPUT_DAC_ALL)
    {
      // It luckily turns out that channels A through D are numbers
      // 0 through 3, which we will exploit in the for loop.
      for (int i = 3; i >= 0; i--)
        AD56X4.writeMessage(SS_pin,setMode,i,values[3-i]);
    }
}
void AD56X4Class::setChannel (int SS_pin, byte setMode, word value_D,
                              word value_C, word value_B, word value_A)
{
  word values[] = {value_D,value_C,value_B,value_A};
  AD56X4.setChannel(SS_pin,setMode,values);
}

/* Commands the AD564X DAC whose Slave Select pin is SS_pin to
   update the output (DAC register) of the specified channel from
   its buffer (input register). The valid channel choices are
   
   AD56X4_CHANNEL_A
   AD56X4_CHANNEL_B
   AD56X4_CHANNEL_C
   AD56X4_CHANNEL_D
   AD56X4_CHANNEL_ALL
*/
void AD56X4Class::updateChannel (int SS_pin, byte channel)
{
  AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_UPDATE_DAC_REGISTER,
                            channel,0);
}



/* Commands the AD564X DAC whose Slave Select pin is SS_pin to set
   the given power mode for the specified channels. The power modes
   are
   
   AD56X4_POWERMODE_NORMAL             normal operation (power up)
   AD56X4_POWERMODE_POWERDOWN_1K       connected to ground by 1k
   AD56X4_POWERMODE_POWERDOWN_100K     connected to ground by 100k
   AD56X4_POWERMODE_POWERDOWN_TRISTATE power down in tristate.
   
   A power mode can be applied to a set of channels specified by
   a channel mask (bits 3 through 0 correspond to channels D
   through A), a boolean array (in channel D through A order), or
   four boolean arguments. Or, an array of power modes can be
   applied to each channel (in D through A order).
*/
void AD56X4Class::powerUpDown (int SS_pin, byte powerMode,
                               byte channelMask)
{
  AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_POWER_UPDOWN,0,
                      word((B00110000 & powerMode)
                      | (B00001111 & channelMask)));
}
void AD56X4Class::powerUpDown (int SS_pin, byte powerMode,
                               boolean channels[])
{
  AD56X4.powerUpDown(SS_pin,powerMode,
                     AD56X4.makeChannelMask(channels));
}
void AD56X4Class::powerUpDown (int SS_pin, byte powerMode,
                               boolean channel_D, boolean channel_C,
                               boolean channel_B, boolean channel_A)
{
  AD56X4.powerUpDown(SS_pin,powerMode,
                     AD56X4.makeChannelMask(channel_D,channel_C,
                     channel_B,channel_A));
}
void AD56X4Class::powerUpDown (int SS_pin, byte powerModes[])
{
  // Go through each channel making a mask for just that channel
  // and apply the given power mode.
  
  byte channelMask = 1;
  for (int i = 0; i < 4; i++)
    {
      AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_POWER_UPDOWN,0,
                          word((B00110000 & powerModes[i])
                          | (B00001111 & channelMask)));
      channelMask = channelMask << 1;
    }
}



/* Commands the AD56X4 DAC whose Slave Select pin is SS_pin to
   reset. The DAC (output) and input (buffer) registers are set
   to zero, and if doing a full reset, the channels are all
   powered up, the external reference is used (internal turned off
   if present), and all channels set so that writing to the input
   register does not auto update the DAC register (output).
*/
void AD56X4Class::reset (int SS_pin, boolean fullReset)
{
  AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_RESET,0, word(fullReset));
}



/* Commands the AD564X DAC whose Slave Select pin is SS_pin which
   channels are to have their DAC register (output) updated
   immediately when the input register (buffer) is set. True is
   for auto update and false is for not. It can either be given
   as a channel mask (bits 3 through 0 correspond to channels D
   through A), a boolean array (in channel D through A order), or
   four boolean arguments.
*/
void AD56X4Class::setInputMode (int SS_pin, byte channelMask)
{
  AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_SET_LDAC,0,
                      word(channelMask));
}
void AD56X4Class::setInputMode (int SS_pin, boolean channels[])
{
  AD56X4.setInputMode(SS_pin,AD56X4.makeChannelMask(channels));
}
void AD56X4Class::setInputMode (int SS_pin, boolean channel_D,
                                boolean channel_C, boolean channel_B,
                                boolean channel_A)
{
  AD56X4.setInputMode(SS_pin,
                      AD56X4.makeChannelMask(channel_D,channel_C,
                                             channel_B,channel_A));
}



/* Commands the AD564X DAC whose Slave Select pin is SS_pin whether
   to use the internal voltage reference or not (use external).
   Should only be used with chips having an internal reference,
   which are the ones whose name ends in an R.
*/
void AD56X4Class::useInternalReference (int SS_pin, boolean yesno)
{
  AD56X4.writeMessage(SS_pin,AD56X4_COMMAND_REFERENCE_ONOFF,0,
                      word(yesno));
}



/* Create channel masks (word with bits 3 through 0 corresponding
   to channels D through A) from an array of booleans for each
   channel (D to A order) or from 4 argument booleans, one for each
   channel.
*/
word AD56X4Class::makeChannelMask (boolean channels[])
{
  return word((byte(channels[0]) << 3) | (byte(channels[1]) << 2)
              | (byte(channels[2]) << 1) | byte(channels[3]));
}
word AD56X4Class::makeChannelMask (boolean channel_D,
                                   boolean channel_C,
                                   boolean channel_B,
                                   boolean channel_A)
{
  return word((byte(channel_D) << 3) | (byte(channel_C) << 2)
              | (byte(channel_B) << 1) | byte(channel_A));
}



/* Writes a 24 bit message to the AD56X4 DAC whose Slave Select
   pin is SS_pin. The message is composed of a command instructing
   the chip what to do, an address telling it which channel/s to
   operate on, and a 2-byte unsigned integer data which could be
   the value to set a channel register to or other control data for
   other commands.
*/
void AD56X4Class::writeMessage (int SS_pin, byte command,
                                byte address, word data)
{
  
  // Set the SPI mode to SPI_MODE1 and the bit order to MSB first.
  
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  
  // The clock speed doesn't matter too much as the chip can go up
  // to 50 MHz while the arduino can only go up 8 MHz, so we will
  // leave it.
  
  // Set the Slave Select pin to low so that the DAC knows to
  // listen for a command.
  
  digitalWrite(SS_pin,LOW);
  
  // The first byte is composed of two bits of nothing, then
  // the command bits, and then the address bits. Masks are
  // used for each set of bits and then the fields are OR'ed
  // together.
  
  SPI.transfer((command & B00111000) | (address & B00000111));
  
  // Send the data word. Must be sent byte by byte, MSB first.
  
  SPI.transfer(highByte(data));
  SPI.transfer(lowByte(data));
  
  // Set the Slave Select pin back to high since we are done
  // sending the command.
  
  digitalWrite(SS_pin,HIGH);
  
}

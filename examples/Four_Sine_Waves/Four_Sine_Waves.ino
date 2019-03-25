/* Copyright (c) 2013, Freja Nordsiek
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or
   without modification, are permitted provided that the
   following conditions are met:
   
   1. Redistributions of source code must retain the above
   copyright notice, this list of conditions and the following disclaimer.
   
   2. Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials
   provided with the distribution.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Analog Sine Waves
   
   This example controls an Analog Devices AD56X4 Quad Channel DAC
   (Digital to Analog Converter) by SPI and writes a 10 Hz sine
   wave to each channel that are all 90 degrees out of phase. The
   outputs are updated synchronously.
   
   Author:   Freja Nordsiek
   Notes:
   History:  * 2013-08-17 Created.
*/

#include "Arduino.h"
#include "math.h"
#include <SPI.h>
#include <AD56X4.h>

// Output pin for the Slave Select SPI line of the AD56X4.

int AD56X4_SS_pin = 10;

// Define the sine wave frequencies for each channel in Hz, the
// initial phases in degrees, the offsets, and the amplitudes.

float frequencies[] = {1e1, 1e1, 1e1, 1e1};
float phases[] = {0, 90, 180, 270};
float offsets[] = {32768, 32768, 32768, 32768};
float amplitudes[] = {32600, 32600, 32600, 32600};

void setup()
{
  
  // Setup SPI. This means setting pin 10 to output (arduino must
  // be the master), the AD56X4 Slave Select pin to output, the
  // SPI clock (chip's 50 MHz is way faster than our 8 MHz max),
  // and start SPI.
  
  pinMode(10,OUTPUT);
  pinMode(AD56X4_SS_pin,OUTPUT);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  
  // Reset the AD56X4, which will power it up and set all outputs
  // to zero.
  
  AD56X4.reset(AD56X4_SS_pin,true);
  
  
}

void loop()
{
  
  // Get the current time.
  
  float t = 1e-6 * float(micros());
  
  // Array to help in giving the library the right channel.
  
  byte channels[] = {AD56X4_CHANNEL_A, AD56X4_CHANNEL_B,
                     AD56X4_CHANNEL_C, AD56X4_CHANNEL_D};
  
  // Calculate the sine wave and put it in the input register for
  // each channel (not output by the DAC yet).
  
  for (int i = 0; i < 4; i++)
    {
      
      // Calculate the sine wave value.
      
      float y = offsets[i] + amplitudes[i]
                * sin(2.0*M_PI*(t*frequencies[i] + phases[i]/360.0));
      
      // Convert to a word (unsigned int) making sure to keep it
      // in the range [0, 0xFFFF].
      
      word output = (word)y;
      
      if (y > 0xFFFF)
        output = 0xFFFF;
      else if (y < 0)
        output = 0;
        
      // Write the output to the i'th channel making sure that only
      // the input register is set (not output yet).
        
      AD56X4.setChannel(AD56X4_SS_pin, AD56X4_SETMODE_INPUT,
                        channels[i], output);
      
    }
  
  // Update all the DAC registers so that the outputs of the DAC
  // are now the new sine wave values. This is synchronous.
  
  AD56X4.updateChannel(AD56X4_SS_pin, AD56X4_CHANNEL_ALL);
  
}

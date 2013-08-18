Analog Devices AD56X4 12/14/16-Bit Quad Channel DAC Arduino Library
===================================================================

An [Arduino](http://arduino.cc) library to control an Analog Devices [AD56X4](http://www.analog.com/en/digital-to-analog-converters/da-converters/ad5624r/products/product.html?doc=AD5624R_5644R_5664R.PDF) series 12/14/16-bit Quad channel DAC (Digital to Analog Converter) by SPI. This library interfaces the following chips

* AD5624  : 12-bit
* AD5664  : 16-bit
* AD5624R : 12-bit with an internal voltage reference that can be used
* AD5644R : 14-bit with an internal voltage reference that can be used
* AD5664R : 16-bit with an internal voltage reference that can be used


Copyright (c) 2013, Freja Nordsiek

This software is distributed under a [3-Clause BSD License](./LICENSE.txt).



Making The Library Package
--------------------------

On a system with `make`, the provided [Makefile](./Makefile) can be used to build a `.zip` file containing the whole library by simply running `make` or `make package`. The package will be named `AD65X4_VERSION.zip` where `VERSION` is the current package [version](./VERSION.txt).



Using The Library
-----------------

An [Arduino](http://arduino.cc) program using this library must first include the library header in addition the the SPI library header.

```C++
#include <SPI.h>
#include <AD56X4.h>
```

All library functions require that the Arduino pin that is the Slave Select for the AD654X (SYNC pin on the device) be given as the first argument. Hence, the calling format looks like

```C++
AD56X4.function(AD56X4_SS_pin, ...);
```

When initializing, the Arduino's Slave Select pin (10) must be set to OUTPUT in addition to whatever pin is used for the Slave Select of the AD56X4 device (pin 10 is a good candidate to avoid wasting a pin). As the AD56X4 device can work with up to 50 MHz SPI messages while the Arduino's maximum speed at the time of this writing (2013-08-17) is 8 MHz, the particular clock divider setting doesn't really matter, though setting it as fast as possible to `SPI_CLOCK_DIV2` will reduce the time it takes to give the AD56X4 commands. Unless it is desirable to keep the previous state of the AD65X4 on Arduino power up or reset, one should do a full reset of the device with `AD56X4.reset(AD56X4_SS_pin, true)`. An initialization sequence would look like

```C++
pinMode(10,OUTPUT);
pinMode(AD56X4_SS_pin,OUTPUT);
SPI.setClockDivider(SPI_CLOCK_DIV2);
SPI.begin();

AD56X4.reset(AD56X4_SS_pin,true);
```

Note, when any library function is called, the [SPI Bit Order](http://arduino.cc/en/Reference/SPISetBitOrder) is set to `MSBFIRST` and the [SPI Data Mode](http://arduino.cc/en/Reference/SPISetDataMode) is set to `SPI_MODE1`. These are not changed back at the end of the function call, so they will need to set before using the SPI bus with another device.

The AD56X4 series DACs all have the value they are outputting and a buffer holding the next value to output. These are referred to as the DAC and input registers respectively. Setting the input registers (function `AD56X4.setChannel`) does not change the analog voltages on the channels unless the AD56X4 is told to update the output (DAC register) at the same time (optional set modes `AD56X4_SETMODE_INPUT_DAC` or `AD56X4_SETMODE_INPUT_DAC_ALL`) or that is the default setting for the channel (function `AD56X4.setInputMode`). The function `AD56X4.updateChannel` is used to update the output (DAC register) to the current value of the input register.



Example Code
------------

An example program, [Four_Sine_Waves](./examples/Four_Sine_Waves/Four_Sine_Waves.ino), is provided that writes 2.2 kHz sine waves to each channel that are out of phase with each other in 90 degree increments.

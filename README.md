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



Hardware Information
--------------------

All the AD56X4 series are connected in a similar way in order for the Arduino to communicate with it by SPI (Serial Peripheral Interface). The DIN pin on the chip (pin 8 on the MSOP and LFCSP packages and pin C1 on the WLCSP package) corresponds to the Arduino's MOSI pin (pin 11 on the Uno, pin 51 on the Mega, and ICSP-4 on all that have the ICSP header). The SCLK pin on the chip (pin 7 on the MSOP and LFCSP packages and pin D1 on the WLCSP package) corresponds to the Arduino's SCK pin (pin 13 on the Uno, pin 52 on the Mega, and ICSP-3 on all that have the ICSP header). The SYNC pin on the chip (pin 6 on the MSOP and LFCSP packages and pin D2 on the WLCSP package) is the Slave Select (SS) pin, which might as well be the Arduino's SS pin (pin 10 on the Uno and pin 53 on the Mega). The SS pin to use has to be given as the first argument of every public function in this library (makes it easier to have multiple SPI devices with different SS pins).

The chip can receive data by SPI at speeds up to 50 MHz, which is significantly greater than the Uno and Mega can do at the present time (8 MHz as of 2013-08-20). So, the SPI clock divider setting doesn't really matter except for how long it will take for commands to complete.

The chips all use a voltage reference that is the upper rail for the analog outputs and determines the resolution of the output voltage. All the chips can take an external voltage reference (pin 16 on the MSOP and LFCSP and pin A1 on the WFCSP). The AD56X4R series have an internal voltage reference that can optionally be used instead (off by default). It is 1.25 V for the AD56X4R-3 series and 2.5 V for the AD56X4R-5 series.



Using The Library
-----------------

An [Arduino](http://arduino.cc) program using this library must first include the library header in addition the the SPI library header.

```Arduino
#include <SPI.h>
#include <AD56X4.h>
```

All library functions require that the Arduino pin that is the Slave Select for the AD654X (SYNC pin on the device) be given as the first argument. Hence, the calling format looks like

```Arduino
AD56X4.function(AD56X4_SS_pin, ...);
```

When initializing, the Arduino's Slave Select pin (10) must be set to OUTPUT in addition to whatever pin is used for the Slave Select of the AD56X4 device (pin 10 is a good candidate to avoid wasting a pin). As the AD56X4 device can work with up to 50 MHz SPI messages while the Arduino's maximum speed at the time of this writing (2013-08-20) is 8 MHz, the particular clock divider setting doesn't really matter, though setting it as fast as possible to `SPI_CLOCK_DIV2` will reduce the time it takes to give the AD56X4 commands. Unless it is desirable to keep the previous state of the AD65X4 on Arduino power up or reset, one should do a full reset of the device with `AD56X4.reset(AD56X4_SS_pin, true)`. An initialization sequence would look like

```Arduino
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



Library Functions
-----------------

*   ```Arduino
    void setChannel(int SS_pin, byte setMode, byte channel, word value)
    void setChannel(int SS_pin, byte setMode, word values[])
    void setChannel(int SS_pin, byte setMode, word value_D, word value_C, word value_B, word value_A)
    ```
    
    Sets the value/s of the specified channel/s on the chip (Slave Select pin `SS_pin`). Values (last argument/s) have to be `word` types (unsigned 16-bit integers, which are `unsigned int` as well). For 12 and 14-bit chips, the last 4 and 2 bits respectively are ignored. `setMode` specifies how the registers are set when the channels/s are set, and the valid values are listed below
    
    *   `AD56X4_SETMODE_INPUT`          Set only the input register (doesn't change voltage output).
    *   `AD56X4_SETMODE_INPUT_DAC`      Set both the input and DAC registers (voltage output is updated).
    *   `AD56X4_SETMODE_INPUT_DAC_ALL`  Set the input register for that channel and cause all input registers to then be copied into their respective DAC registers (output updated on all channels).
    
    For the first overload (calling method), only one channel (or all channels) are set to `value` (or all channels set to it). The valid values of `channel` are
    
    *   `AD56X4_CHANNEL_A`    Channel A only.
    *   `AD56X4_CHANNEL_B`    Channel B only.
    *   `AD56X4_CHANNEL_C`    Channel C only.
    *   `AD56X4_CHANNEL_D`    Channel D only.
    *   `AD56X4_CHANNEL_ALL`  All channels.
    
    They CANNOT be bitwise OR'ed together. In the second and third calling overloads, each channel is set to the given values, which can either be a 4-element array (channel D to A order) or four separate arguments.

*   ```Arduino
    void updateChannel(int SS_pin, byte channel)
    ````
    
    For the chip with the Slave Select pin `SS_pin`, the DAC register (and therefore voltage output) of the specified channel (see `setChannel` above for how channels are specified) is set to the value of its input register. If `setChannel` was called with `setMode = AD56X4_SETMODE_INPUT`, then this function will have to be called on the channel in order for the output voltage to be updated.

*   ```Arduino
    void powerUpDown(int SS_pin, byte powerMode, boolean channels[])
    void powerUpDown(int SS_pin, byte powerMode, boolean channel_D, boolean channel_C, boolean channel_B, boolean channel_A)
    void powerUpDown(int SS_pin, byte powerModes[])
    ```
    
    For the chip with the Slave Select pin `SS_pin`, the power state/mode of the channels are set. The valid power mode/s are
    
    *   `AD56X4_POWERMODE_NORMAL`          On.
    *   `AD56X4_POWERMODE_POWERDOWN_1K`    Off and connected by a 1k resistor to ground.
    *   `AD56X4_POWERMODE_POWERDOWN_100K`  Off and connected by a 100k resistor to ground.
    *   `AD56X4_POWERMODE_TRISTATE`        Off in a tri-state operation.
    
    For the first and second overloads (function calling methods), one power mode `powerMode` is applied to the specified channels. The channels are specified either as a 4-element `boolean` array (channel D to A order) or as four `boolean` arguments. `true` means set to the given power mode and `false` means not (keep current power mode). The third overload sets the power mode of each channel to the power modes given in the 4-element array `powerModes[]` (channel D to A order).

*   ```Arduino    
    void reset(int SS_pin, boolean fullReset)
    ```
    
    Resets the AD56X4 chip with Slave Select pin `SS_pin`. The DAC and input registers are all reset to 0, and thus the analog voltages are set to zero. If `fullReset` is `true`, then a full reset is done which resets the channel powerMode to on (`AD56X4_POWERMODE_NORMAL`), the external voltage reference is used (internal reference, if present, turned off), and the input mode (LDAC register in the chip data sheet) is reset (updating the input registers does not cause automatic updates of the DAC registers from their values). 

*   ```Arduino
    void setInputMode(int SS_pin, boolean channels[])
    void setInputMode(int SS_pin, boolean channel_D, boolean channel_C, boolean channel_B, boolean channel_A)
    ```
    
    Sets the input modes of each channel on the chip (Slave Select pin `SS_pi`). The modes are `boolean` with `true` meaning that setting the input register automatically sets the DAC register (and therefore voltage output) to the same value and `false` meaning no auto update of the DAC register. The modes for each channel can either be given by a 4-element `boolean` array (channel D to A order) or as four separate input arguments.

*   ```Arduino
    void useInternalReference (int SS_pin, boolean yesno)
    ```
    
    Choose whether the chip (Slave Select pin `SS_pin`) uses the internal voltage reference (`yesno = true`) or the external reference pin (`yesno = false`). Only applicable for the chips that have an internal reference (AD56XR).

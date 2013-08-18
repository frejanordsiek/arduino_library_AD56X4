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

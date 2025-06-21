# libsd
A simple library for interfacing with SD memory cards via SPI.

## Building
The project is still a work in progress and is largely untested.  To build for the RP2040 (currently the only supported chip) use the following steps (Windows install):
mkdir build
cd build
cmake -DTARGET_PLATFORM=rp2040 -G "NMake Makefiles" ..
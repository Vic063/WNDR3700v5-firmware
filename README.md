Project WNDR3700v5-firmware

# README

These tools were created to build a firmware image for the Netgear WNDR3700v5.
You MUST have a prebuilt firmware file from OpenWRT (for example).

The solution contains 2 projects:
* WNDR3700v5-Firmware: dedicated to build the bin file with adding all Sercomm signatures
* zipImage: make the final image firmware file from the previous built bin file

# COMPILATION

The code can be build both on Windows using Visual Studio or linux using g++.
g++ *.cpp -o WNDR3700v5-Firmware

# KNOWN BUGS

I was unable to reproduce the exact same output file that can give the official zipImage program
(given by Netgear) on Windows. You should use the zipImage program on linux which can be found here:
http://www.downloads.netgear.com/files/GPL/WNDR3700v5_V1.0.0.17_src.tar.gz.zip

The code was wrote after reverse engineering researches on the official program used to flash the Netgear 
with their official firmware. The code is messy and might contain bugs, but it should do the job when
used in normal condition.

#LICENSE

You are free to use my code, modify it and redistribute it but don't forget to mention me if you do it :)
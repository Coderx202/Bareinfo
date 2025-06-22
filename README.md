# Bareinfo
A Simple tool to see your system info for almost all Linux distro

## Features
- Shows CPU Name, vendor and cores
- Shows BIOS/UEFI vendor, date, version and release
- Shows Motherboard vendor, name and system vendor
- Shows product name (laptop or computer name)
- Shows kernel and build info
- Shows Package manager name
- Doesn't need sudo privileges
- Allows user to export the output to a file
- Shows the distro name
- Shows RAM info (How much ram there is and how much is free)
## Compatibility
This program works with most distros out of the box, but distros like alpine and containers are not supported due to alpine missing some stuff and the same for containers but VM's are semi supported

Tested distros:
- Fedora (Real Hardware)
- Debian (Real Hardware)
- Arch (Virtual Machine)
- OpenSUSE (Virtual Machine)


## Building
```sh
git clone https://github.com/Coderx202/Bareinfo.git
cd Bareinfo
g++ -o bareinfo Bareinfo.cpp
```
## Usage
if you have downloaded the binary from the release do these steps to execute the app

```sh
chmod +x Bareinfo
./Bareinfo
```


Simply type in this command in the same directory as the executable
```sh
./bareinfo
```
If you want to export the output to a file don't use the right and left bitwise operators because of the ANSI escape codes instead type
```sh
./bareinfo --export-to-file #First way
./bareinfo -export #second way
./bareinfo --export #third way
```

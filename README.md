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

## Compatibility
This program works with most distros out of the box, but distros like alpine and containers are not supported due to alpine missing some stuff and the same for containers but VM's are semi supported

Tested distros:
- Fedora
- Debian
- Arch
- OpenSUSE


## Unstable and beta version
The Unstable/Beta version of 1.1 is here but it has some issues like sometimes it simply doesn't work on debian

Distros tested for the beta version:
- Fedora (Fully works)
- Debian (Has minor issues probably with the environment might not be the actual app)
- OpenSUSE (Fully works)



## Building
```sh
git clone https://github.com/Coderx202/Bareinfo.git
cd Bareinfo
g++ -o bareinfo Bareinfo.cpp
```
## Usage
Simply type in this command in the same directory as the executable
```sh
./bareinfo
```

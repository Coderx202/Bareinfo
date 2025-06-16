#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class CPUInfo{
public:
std::string getCPUName()   {return readline("/proc/cpuinfo", "model name");   }
std::string getCPUCores()  {return readline("/proc/cpuinfo", "cpu cores");    }
std::string getCPUVendor() {return readline("/proc/cpuinfo", "vendor_id");    }

private:

std::string readline(const std::string& path, const std::string& word) {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(word) != std::string::npos) {
            return line.substr(line.find(":") + 2);
        }
    }

    return "N/A"; 
}

};


class BIOSInfo {
public:
    std::string getBIOSVendor()     { return read("/sys/class/dmi/id/bios_vendor");  }
    std::string getBIOSVersion()    { return read("/sys/class/dmi/id/bios_version"); }
    std::string getBIOSDate()       { return read("/sys/class/dmi/id/bios_date");    }
    std::string getBIOSRelease()    { return read("/sys/class/dmi/id/bios_release");    }
    
private:
    std::string read(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        if (file && std::getline(file, line)) {
            return line;
        }
        return "N/A";
    }
};



class MotherboardInfo{
public:
	std::string getMotherboardName() {return read("/sys/class/dmi/id/board_name");}
	std::string getMotherboardVendor() {return read("/sys/class/dmi/id/board_vendor");}
	std::string getSystemVendor() {return read("/sys/class/dmi/id/sys_vendor");}
    std::string getProductName(){return read("/sys/class/dmi/id/product_name");}
private:
    std::string read(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        if (file && std::getline(file, line)) {
            return line;
        }
        return "N/A";
    }

};


void meminfo(){
    std::ifstream file("/proc/meminfo");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("MemTotal") != std::string::npos) {
            std::cout << "Total RAM:          " << line.substr(line.find(":") + 8) << std::endl;
            break;
        }
    }
}

void CheckSecureBoot(){
    int ret = system("mokutil --sb-state 2>/dev/null | grep -q 'SecureBoot enabled'");

    if(ret == 0){
        std::cout << "Secure Boot:        Enabled\n";
    }
    else {
        std::cout << "Secure boot:        Disabled\n";
    }


}





std::string getKernelInfo(){
    std::ifstream file("/proc/sys/kernel/osrelease");
    std::string line;
    if (file && std::getline(file, line)){
        return line;
    }
    else {
        return "N/A";
    }
}


std::string shell(){

    system("ps -p $$ -o comm= > shell.txt");

    std::ifstream file("shell.txt");
    std::string line;
    std::getline(file, line);
    std::remove("shell.txt");
    return line;
}


std::string getPackageManager() {
    const std::vector<std::pair<std::string, std::string>> managers = {
        {"apt", "/usr/bin/apt"},
        {"dnf", "/usr/bin/dnf"},
        {"yum", "/usr/bin/yum"},
        {"pacman", "/usr/bin/pacman"},
        {"zypper", "/usr/bin/zypper"},
        {"emerge", "/usr/bin/emerge"},
        {"nix", "/run/current-system/sw/bin/nix-env"}
    };

    for (const auto& [name, path] : managers) {
        if (fs::exists(path)) {
            return name;
        }
    }

    return "unknown";
}




std::string BuildInfo(){
    std::ifstream build("/proc/version");
    std::string line;
    
    std::getline(build, line);
    return line;
}


int main(){

    BIOSInfo bios;
    MotherboardInfo Motherboard;
    CPUInfo CPU;

    const std::string RED    = "\033[31m";
    const std::string GREEN  = "\033[32m";
    const std::string BLUE   = "\033[34m";
    const std::string CYAN   = "\033[36m";
    const std::string YELLOW = "\033[33m";
    const std::string RESET  = "\033[0m";

    std::cout << BLUE << "CPU Model:          " << RESET << CPU.getCPUName() << "\n";
    std::cout << BLUE << "CPU Cores:          " << RESET << CPU.getCPUCores() << "\n";
    std::cout << BLUE << "CPU Vendor:         " << RESET << CPU.getCPUVendor() << "\n";

    meminfo();
    CheckSecureBoot(); 

    std::cout << RED << "BIOS/UEFI Vendor:   " << RESET << bios.getBIOSVendor() << "\n";
    std::cout << RED << "BIOS/UEFI Version:  " << RESET << bios.getBIOSVersion() << "\n";
    std::cout << RED << "BIOS/UEFI Date:     " << RESET << bios.getBIOSDate() << "\n";
    std::cout << RED << "BIOS/UEFI Release:  " << RESET << bios.getBIOSRelease() << "\n";

    std::cout << GREEN << "Motherboard Name:   " << RESET << Motherboard.getMotherboardName() << "\n";
    std::cout << GREEN << "Motherboard Vendor: " << RESET << Motherboard.getMotherboardVendor() << "\n";

    std::cout << CYAN << "System Vendor:      " << RESET << Motherboard.getSystemVendor() << "\n";
    std::cout << CYAN << "Product Name:       " << RESET << Motherboard.getProductName() << "\n";

    std::cout << YELLOW << "Kernel:             " << RESET << getKernelInfo() << "\n";
    std::cout << YELLOW << "Default Shell:      " << RESET << shell() << "\n";
    std::cout << YELLOW << "Build Info:         " << RESET << BuildInfo() << "\n";
    std::cout << YELLOW << "Package Manager:    " << RESET << getPackageManager() << "\n";
}

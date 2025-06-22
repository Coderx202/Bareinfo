#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib> 
#include <cmath>

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




bool CheckSecureBoot(){
    int ret = system("mokutil --sb-state 2>/dev/null | grep -q 'SecureBoot enabled'");

    if(ret == 0){
        return true;
    }
    else {
        return false;
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

double getRAMInfo(const std::string &arg){

    std::ifstream meminfo("/proc/meminfo");
    std::string key;
    long value;
    std::string unit;

    if (arg == "RAM"){
            while (meminfo >> key >> value >> unit) {
        if (key == "MemTotal:") {
            double gb = value / 1048576.0;
            return gb;
        }
    }
    }

    if (arg == "FREE"){
            while (meminfo >> key >> value >> unit) {
        if (key == "MemAvailable:") {
            double gb = value / 1048576.0;
            return gb;
        }
    }
    }


    return 0.0;
}


std::string BuildInfo(){
    std::ifstream build("/proc/version");
    std::string line;
    
    std::getline(build, line);
    if (line.empty()){
        return "N/A";
    }
    return line;
}

std::string getDistroInfo(){

    std::ifstream file("/etc/os-release");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("PRETTY_NAME") != std::string::npos) {
            return line.substr(line.find("=") + 1);
        }
    }

    return "N/A"; 

}

std::string getBootMode() {
    return fs::exists("/sys/firmware/efi") ? "UEFI" : "BIOS";
}

void ExportToFile(){

    BIOSInfo bios;
    MotherboardInfo Motherboard;
    CPUInfo CPU;


    std::ofstream file("bareinfo.txt");

    file << "CPU Model:          " << CPU.getCPUName() << "\n";
    file << "CPU Cores:          " << CPU.getCPUCores() << "\n";
    file << "CPU Vendor:         " << CPU.getCPUVendor() << "\n";
    file << "BIOS/UEFI Vendor:   " << bios.getBIOSVendor() << "\n";
    file << "BIOS/UEFI Version:  " << bios.getBIOSVersion() << "\n";
    file << "BIOS/UEFI Date:     " << bios.getBIOSDate() << "\n";
    file << "BIOS/UEFI Release:  " << bios.getBIOSRelease() << "\n";

    file << "Motherboard Name:   " << Motherboard.getMotherboardName() << "\n";
    file << "Motherboard Vendor: " << Motherboard.getMotherboardVendor() << "\n";

    file << "System Vendor:      " << Motherboard.getSystemVendor() << "\n";
    file << "Product Name:       " << Motherboard.getProductName() << "\n";

    file << "Kernel:             " << getKernelInfo() << "\n";
    file << "Default Shell:      " << shell() << "\n";
    file << "Build Info:         " << BuildInfo() << "\n";
    file << "Boot Mode:          " << getBootMode() << "\n";
    file << "Package Manager:    " << getPackageManager() << "\n";
    file << "Distro name:        " << getDistroInfo() << "\n";
    file << "Secure Boot state:  " << CheckSecureBoot() << "\n";
    file << "Total RAM:          " << getRAMInfo("RAM") << "GB" << "\n";
    file << "Free RAM:           " <<  getRAMInfo("FREE") << "GB" << "\n";
    file.close();
}


int main(int argc, char *argv[]){

    BIOSInfo bios;
    MotherboardInfo Motherboard;
    CPUInfo CPU;


    if (argc > 1) {
            std::string arg1 = argv[1];

            if (arg1 == "--export-to-file" || arg1 == "-export" || arg1 == "--export") {
                ExportToFile();
                exit(0);
            } else {
                std::cerr << "Unknown argument: " << arg1 << std::endl;
                exit(1);
            }
    }

    const std::string RED    = "\033[31m";
    const std::string GREEN  = "\033[32m";
    const std::string BLUE   = "\033[34m";
    const std::string CYAN   = "\033[36m";
    const std::string YELLOW = "\033[33m";
    const std::string RESET  = "\033[0m";
    const std::string MAGENTA = "\033[35m";

    std::cout << BLUE << "CPU Model:          " << RESET << CPU.getCPUName() << "\n";
    std::cout << BLUE << "CPU Cores:          " << RESET << CPU.getCPUCores() << "\n";
    std::cout << BLUE << "CPU Vendor:         " << RESET << CPU.getCPUVendor() << "\n";

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
    std::cout << YELLOW << "Boot Mode:          " << RESET << getBootMode() << "\n";
    std::cout << YELLOW << "Package Manager:    " << RESET << getPackageManager() << "\n";

    std::cout << MAGENTA << "Distro name:       " << RESET << getDistroInfo() << "\n";
    std::cout << MAGENTA << "Secure Boot state:  " << RESET << CheckSecureBoot() << "\n"; 
    std::cout << MAGENTA << "Total RAM:          " << RESET << getRAMInfo("RAM") << "GB" <<"\n";
    std::cout << MAGENTA << "Free RAM:           " << RESET << getRAMInfo("FREE") << "GB" << "\n";

}

#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <array>
#include <chrono>

namespace fs = std::filesystem;

// Reads a line from a file matching a specific keyword
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

// =============================
// CPU INFO CLASS
// =============================
class CPUInfo {
public:
    std::string getCPUName()   { return readline("/proc/cpuinfo", "model name"); }
    std::string getCPUCores()  { return readline("/proc/cpuinfo", "cpu cores"); }
    std::string getCPUVendor() { return readline("/proc/cpuinfo", "vendor_id"); }
};

// =============================
// BIOS INFO CLASS
// =============================
class BIOSInfo {
public:
    std::string getBIOSVendor()  { return read("/sys/class/dmi/id/bios_vendor"); }
    std::string getBIOSVersion() { return read("/sys/class/dmi/id/bios_version"); }
    std::string getBIOSDate()    { return read("/sys/class/dmi/id/bios_date"); }
    std::string getBIOSRelease() { return read("/sys/class/dmi/id/bios_release"); }

private:
    std::string read(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        if (file && std::getline(file, line)) return line;
        return "N/A";
    }
};

// =============================
// MOTHERBOARD INFO CLASS
// =============================
class MotherboardInfo {
public:
    std::string getMotherboardName()   { return read("/sys/class/dmi/id/board_name"); }
    std::string getMotherboardVendor() { return read("/sys/class/dmi/id/board_vendor"); }
    std::string getSystemVendor()      { return read("/sys/class/dmi/id/sys_vendor"); }
    std::string getProductName()       { return read("/sys/class/dmi/id/product_name"); }

private:
    std::string read(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        if (file && std::getline(file, line)) return line;
        return "N/A";
    }
};

// =============================
// SYSTEM FUNCTIONS
// =============================
std::string CheckSecureBoot() {
    const fs::path efiPath = "/sys/firmware/efi";
    const fs::path varsPath = efiPath / "efivars";

    if (!fs::exists(efiPath)) return "N/A (Legacy BIOS)";
    if (!fs::exists(varsPath)) return "N/A (No efivars directory)";

    std::string secureBootPath;
    for (const auto& entry : fs::directory_iterator(varsPath)) {
        std::string name = entry.path().filename().string();
        if (name.rfind("SecureBoot-", 0) == 0) {
            secureBootPath = entry.path();
            break;
        }
    }

    if (secureBootPath.empty()) return "N/A (No SecureBoot variable)";

    std::ifstream file(secureBootPath, std::ios::binary);
    if (!file.is_open()) return "Unknown (Permission denied)";

    // Check file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size <= 0) return "Unknown (Empty variable)";
    file.seekg(0, std::ios::beg);

    unsigned char data[5] = {0};
    file.read(reinterpret_cast<char*>(data), std::min<std::streamsize>(size, 5));

    if (!file) return "Unknown (Read error)";

    // Try byte 4 (standard layout), fallback to byte 0 (some kernels)
    unsigned char state = 0;
    if (size > 4)
        state = data[4];
    else
        state = data[0];

    return (state == 1) ? "Enabled" : "Disabled";
}



std::string getKernelInfo() {
    std::ifstream file("/proc/sys/kernel/osrelease");
    std::string line;
    if (file && std::getline(file, line)) return line;
    return "N/A";
}

std::string shell() {
    const char* sh = getenv("SHELL");
    return sh ? std::string(sh) : "N/A";
}

std::string getPackageManager() {
    const std::vector<std::pair<std::string, std::vector<std::string>>> managers = {
        {"apt", {"/usr/bin/apt", "/usr/bin/apt-get"}},
        {"dnf", {"/usr/bin/dnf"}},
        {"yum", {"/usr/bin/yum"}},
        {"pacman", {"/usr/bin/pacman"}},
        {"yay", {"/usr/bin/yay"}},
        {"paru", {"/usr/bin/paru"}},
        {"zypper", {"/usr/bin/zypper"}},
        {"emerge", {"/usr/bin/emerge"}},
        {"nix", {"/run/current-system/sw/bin/nix-env"}},
        {"snap", {"/usr/bin/snap"}},
        {"flatpak", {"/usr/bin/flatpak"}},
        {"apk", {"/sbin/apk"}},
        {"brew", {"/home/linuxbrew/.linuxbrew/bin/brew", "/usr/bin/brew"}},
        {"conda", {"/usr/bin/conda"}},
        {"pip", {"/usr/bin/pip"}},
        {"pkg", {"/usr/sbin/pkg"}},
        {"guix", {"/usr/bin/guix"}}
    };

    std::vector<std::string> foundManagers;

    for (const auto& [name, paths] : managers) {
        for (const auto& path : paths) {
            if (fs::exists(path)) {
                foundManagers.push_back(name);
                break;
            }
        }
    }

    if (foundManagers.empty()) return "unknown";

    std::ostringstream oss;
    for (size_t i = 0; i < foundManagers.size(); ++i) {
        if (i) oss << ", ";
        oss << foundManagers[i];
    }
    return oss.str();
}

double getRAMInfo(const std::string &arg) {
    std::ifstream meminfo("/proc/meminfo");
    std::string key, unit;
    long value;

    if (!meminfo.is_open()) return 0.0;

    while (meminfo >> key >> value >> unit) {
        if (arg == "RAM" && key == "MemTotal:") {
            return value / 1048576.0;
        }
        if (arg == "FREE" && key == "MemAvailable:") {
            return value / 1048576.0;
        }
    }

    return 0.0;
}

std::string BuildInfo() {
    std::ifstream build("/proc/version");
    std::string line;
    std::getline(build, line);
    return line.empty() ? "N/A" : line;
}

std::string getDistroInfo() {
    std::ifstream file("/etc/os-release");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("PRETTY_NAME=") != std::string::npos) {
            size_t start = line.find('=') + 1;
            if (line[start] == '"') start++;
            size_t end = line.find_last_of('"');
            return line.substr(start, end - start);
        }
    }
    return "N/A";
}

std::string getBootMode() {
    return fs::exists("/sys/firmware/efi") ? "UEFI" : "BIOS";
}

// =============================
// EXPORT FUNCTIONS
// =============================
void ExportToHTML() {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n"
         << "<title>bareinfo.json Viewer</title>\n<style>\n"
         << "body{background:#1e1e1e;color:#dcdcdc;font-family:monospace;padding:20px;}"
         << "h2{color:#72bcd4;} .section{margin-bottom:20px;}"
         << "</style>\n</head>\n<body>\n<h1>System Information</h1>\n<div id=\"output\"></div>\n"
         << "<script>\nfunction displaySection(title,obj){const s=document.createElement('div');s.className='section';"
         << "const h=document.createElement('h2');h.textContent=title;s.appendChild(h);"
         << "for(const k in obj){const l=document.createElement('div');"
         << "const v=typeof obj[k]==='string'?`\"${obj[k]}\"`:obj[k];"
         << "l.textContent=`${k}: ${v}`;s.appendChild(l);}return s;}"
         << "fetch('bareinfo.json').then(r=>r.json()).then(d=>{const o=document.getElementById('output');"
         << "for(const s in d){o.appendChild(displaySection(s,d[s]));}})"
         << ".catch(e=>document.getElementById('output').textContent='Error: '+e.message);"
         << "</script>\n</body>\n</html>\n";

    std::ofstream HTML("Bareinfo.html");
    HTML << html.str();
}

void ExportToFile() {
    BIOSInfo bios;
    MotherboardInfo mb;
    CPUInfo cpu;

    std::ofstream file("bareinfo.txt");

    file << "CPU Model:          " << cpu.getCPUName() << "\n";
    file << "CPU Cores:          " << cpu.getCPUCores() << "\n";
    file << "CPU Vendor:         " << cpu.getCPUVendor() << "\n";
    file << "BIOS/UEFI Vendor:   " << bios.getBIOSVendor() << "\n";
    file << "BIOS/UEFI Version:  " << bios.getBIOSVersion() << "\n";
    file << "BIOS/UEFI Date:     " << bios.getBIOSDate() << "\n";
    file << "BIOS/UEFI Release:  " << bios.getBIOSRelease() << "\n";
    file << "Motherboard Name:   " << mb.getMotherboardName() << "\n";
    file << "Motherboard Vendor: " << mb.getMotherboardVendor() << "\n";
    file << "System Vendor:      " << mb.getSystemVendor() << "\n";
    file << "Product Name:       " << mb.getProductName() << "\n";
    file << "Kernel:             " << getKernelInfo() << "\n";
    file << "Default Shell:      " << shell() << "\n";
    file << "Build Info:         " << BuildInfo() << "\n";
    file << "Boot Mode:          " << getBootMode() << "\n";
    file << "Package Manager:    " << getPackageManager() << "\n";
    file << "Distro name:        " << getDistroInfo() << "\n";
    file << "Secure Boot state:  " << CheckSecureBoot() << "\n";
    file << "Total RAM:          " << getRAMInfo("RAM") << " GB\n";
    file << "Free RAM:           " << getRAMInfo("FREE") << " GB\n";
}

void ExportToJSON() {
    BIOSInfo bios;
    MotherboardInfo mb;
    CPUInfo cpu;

    std::ofstream jsonFile("bareinfo.json");
    jsonFile << "{\n"
             << "  \"CPU\": {\n"
             << "    \"Model\": \"" << cpu.getCPUName() << "\",\n"
             << "    \"Cores\": \"" << cpu.getCPUCores() << "\",\n"
             << "    \"Vendor\": \"" << cpu.getCPUVendor() << "\"\n"
             << "  },\n"
             << "  \"BIOS\": {\n"
             << "    \"Vendor\": \"" << bios.getBIOSVendor() << "\",\n"
             << "    \"Version\": \"" << bios.getBIOSVersion() << "\",\n"
             << "    \"Date\": \"" << bios.getBIOSDate() << "\",\n"
             << "    \"Release\": \"" << bios.getBIOSRelease() << "\"\n"
             << "  },\n"
             << "  \"Motherboard\": {\n"
             << "    \"Name\": \"" << mb.getMotherboardName() << "\",\n"
             << "    \"Vendor\": \"" << mb.getMotherboardVendor() << "\",\n"
             << "    \"SystemVendor\": \"" << mb.getSystemVendor() << "\",\n"
             << "    \"ProductName\": \"" << mb.getProductName() << "\"\n"
             << "  },\n"
             << "  \"System\": {\n"
             << "    \"Kernel\": \"" << getKernelInfo() << "\",\n"
             << "    \"DefaultShell\": \"" << shell() << "\",\n"
             << "    \"BuildInfo\": \"" << BuildInfo() << "\",\n"
             << "    \"BootMode\": \"" << getBootMode() << "\",\n"
             << "    \"PackageManager\": \"" << getPackageManager() << "\",\n"
             << "    \"Distro\": \"" << getDistroInfo() << "\",\n"
             << "    \"SecureBoot\": \"" << CheckSecureBoot() << "\",\n"
             << "    \"TotalRAM_GB\": " << getRAMInfo("RAM") << ",\n"
             << "    \"FreeRAM_GB\": " << getRAMInfo("FREE") << "\n"
             << "  }\n"
             << "}\n";
}

// =============================
// MAIN
// =============================
int main(int argc, char *argv[]) {
    
    BIOSInfo bios;
    MotherboardInfo mb;
    CPUInfo cpu;

    if (argc > 1) {
        std::string arg1 = argv[1];

        if (arg1 == "--export-to-file" || arg1 == "-export" || arg1 == "--export") {
            ExportToFile();
            return 0;
        } else if (arg1 == "--ExportToHTML" || arg1 == "-ExportToHTML") {
            ExportToHTML();
            return 0;
        } else if (arg1 == "--ExportToJSON" || arg1 == "-ExportToJSON") {
            ExportToJSON();
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg1 << std::endl;
            return 1;
        }
    }

    const std::string BLUE   = "\033[34m";
    const std::string RED    = "\033[31m";
    const std::string GREEN  = "\033[32m";
    const std::string CYAN   = "\033[36m";
    const std::string YELLOW = "\033[33m";
    const std::string MAGENTA= "\033[35m";
    const std::string RESET  = "\033[0m";

    std::cout << BLUE << "CPU Model:          " << RESET << cpu.getCPUName() << "\n";
    std::cout << BLUE << "CPU Cores:          " << RESET << cpu.getCPUCores() << "\n";
    std::cout << BLUE << "CPU Vendor:         " << RESET << cpu.getCPUVendor() << "\n";

    std::cout << RED << "BIOS/UEFI Vendor:   " << RESET << bios.getBIOSVendor() << "\n";
    std::cout << RED << "BIOS/UEFI Version:  " << RESET << bios.getBIOSVersion() << "\n";
    std::cout << RED << "BIOS/UEFI Date:     " << RESET << bios.getBIOSDate() << "\n";
    std::cout << RED << "BIOS/UEFI Release:  " << RESET << bios.getBIOSRelease() << "\n";

    std::cout << GREEN << "Motherboard Name:   " << RESET << mb.getMotherboardName() << "\n";
    std::cout << GREEN << "Motherboard Vendor: " << RESET << mb.getMotherboardVendor() << "\n";

    std::cout << CYAN << "System Vendor:      " << RESET << mb.getSystemVendor() << "\n";
    std::cout << CYAN << "Product Name:       " << RESET << mb.getProductName() << "\n";

    std::cout << YELLOW << "Kernel:             " << RESET << getKernelInfo() << "\n";
    std::cout << YELLOW << "Default Shell:      " << RESET << shell() << "\n";
    std::cout << YELLOW << "Build Info:         " << RESET << BuildInfo() << "\n";
    std::cout << YELLOW << "Boot Mode:          " << RESET << getBootMode() << "\n";
    std::cout << YELLOW << "Package Manager:    " << RESET << getPackageManager() << "\n";

    std::cout << MAGENTA << "Distro name:        " << RESET << getDistroInfo() << "\n";
    std::cout << MAGENTA << "Secure Boot state:  " << RESET << CheckSecureBoot() << "\n";
    std::cout << MAGENTA << "Total RAM:          " << RESET << getRAMInfo("RAM") << " GB\n";
    std::cout << MAGENTA << "Free RAM:           " << RESET << getRAMInfo("FREE") << " GB\n";
    return 0;
}

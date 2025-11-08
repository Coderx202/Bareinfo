#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib> 
#include <cmath>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <array>
#include <chrono>

namespace fs = std::filesystem;

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

std::string exec(const char* cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}


class CPUInfo{
public:
std::string getCPUName()   {return readline("/proc/cpuinfo", "model name");   }
std::string getCPUCores()  {return readline("/proc/cpuinfo", "cpu cores");    }
std::string getCPUVendor() {return readline("/proc/cpuinfo", "vendor_id");    }
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





std::string CheckSecureBoot() {
    const fs::path varsPath = "/sys/firmware/efi/vars";

    if (!fs::exists("/sys/firmware/efi") || !fs::exists(varsPath)) {
        return "N/A"; // Not UEFI or vars missing
    }

    std::error_code ec; // for non-throwing versions
    for (auto& entry : fs::directory_iterator(varsPath, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) continue; // skip entries we can't open

        std::string fname = entry.path().filename().string();
        if (fname.find("SecureBoot-") != std::string::npos) {
            std::ifstream file(entry.path() / "data", std::ios::binary);
            if (file) {
                char val;
                file.read(&val, 1);
                return (val != 0) ? "Enabled" : "Disabled";
            }
        }
    }

    return "N/A";
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
    const char* sh = getenv("SHELL");
    return sh ? std::string(sh) : "N/A";
}



std::string getPackageManager() {
    const std::vector<std::pair<std::string, std::string>> managers = {
        {"apt", "/usr/bin/apt"},
        {"apt-get", "/usr/bin/apt-get"},
        {"dnf", "/usr/bin/dnf"},
        {"yum", "/usr/bin/yum"},
        {"pacman", "/usr/bin/pacman"},
        {"yay", "/usr/bin/yay"},
        {"paru", "/usr/bin/paru"},
        {"zypper", "/usr/bin/zypper"},
        {"emerge", "/usr/bin/emerge"},
        {"nix", "/run/current-system/sw/bin/nix-env"},
        {"snap", "/usr/bin/snap"},
        {"flatpak", "/usr/bin/flatpak"},
        {"apk", "/sbin/apk"},
        {"brew", "/home/linuxbrew/.linuxbrew/bin/brew"},
        {"brew", "/usr/bin/brew"},
        {"conda", "/usr/bin/conda"},
        {"pip", "/usr/bin/pip"},
        {"pkg", "/usr/sbin/pkg"},
        {"guix", "/usr/bin/guix"}
    };

    std::vector<std::string> foundManagers;

    for (const auto& [name, path] : managers) {
        if (fs::exists(path)) {
            foundManagers.push_back(name);
        }
    }
        if (foundManagers.empty()) {
        return "unknown";
    }

    std::ostringstream oss;
    for (size_t i = 0; i < foundManagers.size(); ++i) {
        if (i) oss << ", ";
        oss << foundManagers[i];
    }
    return oss.str();
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


void ExportToHTML() {
	//Please don't change anything here, the stream has the code for the HTML file so DON'T TOUCH IT OR ELSE THE PAGE WONT WORK
    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n";
    html << "<head>\n";
    html << "    <meta charset=\"UTF-8\">\n";
    html << "    <title>bareinfo.json Viewer</title>\n";
    html << "    <style>\n";
    html << "        body {\n";
    html << "            background-color: #1e1e1e;\n";
    html << "            color: #dcdcdc;\n";
    html << "            font-family: monospace;\n";
    html << "            padding: 20px;\n";
    html << "        }\n";
    html << "        h2 {\n";
    html << "            color: #72bcd4;\n";
    html << "        }\n";
    html << "        .section {\n";
    html << "            margin-bottom: 20px;\n";
    html << "        }\n";
    html << "    </style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "\n";
    html << "    <h1>System Information</h1>\n";
    html << "    <div id=\"output\"></div>\n";
    html << "\n";
    html << "    <script>\n";
    html << "        function displaySection(title, obj) {\n";
    html << "            const section = document.createElement('div');\n";
    html << "            section.className = 'section';\n";
    html << "\n";
    html << "            const heading = document.createElement('h2');\n";
    html << "            heading.textContent = title;\n";
    html << "            section.appendChild(heading);\n";
    html << "\n";
    html << "            for (const key in obj) {\n";
    html << "                const line = document.createElement('div');\n";
    html << "                const value = typeof obj[key] === \"string\" ? `\"${obj[key]}\"` : obj[key];\n";
    html << "                line.textContent = `${key}: ${value}`;\n";
    html << "                section.appendChild(line);\n";
    html << "            }\n";
    html << "\n";
    html << "            return section;\n";
    html << "        }\n";
    html << "\n";
    html << "        fetch('bareinfo.json')\n";
    html << "            .then(response => response.json())\n";
    html << "            .then(data => {\n";
    html << "                const output = document.getElementById('output');\n";
    html << "                for (const section in data) {\n";
    html << "                    output.appendChild(displaySection(section, data[section]));\n";
    html << "                }\n";
    html << "            })\n";
    html << "            .catch(error => {\n";
    html << "                document.getElementById('output').textContent = \"Error loading JSON: \" + error.message;\n";
    html << "            });\n";
    html << "    </script>\n";
    html << "\n";
    html << "</body>\n";
    html << "</html>\n";


    std::ofstream HTML("Bareinfo.html");

    HTML << html.str();

    HTML.close();


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
    file << "Default Shell:      " << shell();
    file << "Build Info:         " << BuildInfo() << "\n";
    file << "Boot Mode:          " << getBootMode() << "\n";
    file << "Package Manager:    " << getPackageManager() << "\n";
    file << "Distro name:        " << getDistroInfo() << "\n";
    file << "Secure Boot state:  " << CheckSecureBoot() << "\n";
    file << "Total RAM:          " << getRAMInfo("RAM") << "GB" << "\n";
    file << "Free RAM:           " <<  getRAMInfo("FREE") << "GB" << "\n";
    file.close();
}

void ExportToJSON() {

    BIOSInfo bios;
    MotherboardInfo Motherboard;
    CPUInfo CPU;

    std::ofstream jsonFile("bareinfo.json");
    jsonFile << "{\n";
    jsonFile << "  \"CPU\": {\n";
    jsonFile << "    \"Model\": \"" << CPU.getCPUName() << "\",\n";
    jsonFile << "    \"Cores\": " << CPU.getCPUCores() << ",\n";
    jsonFile << "    \"Vendor\": \"" << CPU.getCPUVendor() << "\"\n";
    jsonFile << "  },\n";
    jsonFile << "  \"BIOS\": {\n";
    jsonFile << "    \"Vendor\": \"" << bios.getBIOSVendor() << "\",\n";
    jsonFile << "    \"Version\": \"" << bios.getBIOSVersion() << "\",\n";
    jsonFile << "    \"Date\": \"" << bios.getBIOSDate() << "\",\n";
    jsonFile << "    \"Release\": \"" << bios.getBIOSRelease() << "\"\n";
    jsonFile << "  },\n";
    jsonFile << "  \"Motherboard\": {\n";
    jsonFile << "    \"Name\": \"" << Motherboard.getMotherboardName() << "\",\n";
    jsonFile << "    \"Vendor\": \"" << Motherboard.getMotherboardVendor() << "\",\n";
    jsonFile << "    \"SystemVendor\": \"" << Motherboard.getSystemVendor() << "\",\n";
    jsonFile << "    \"ProductName\": \"" << Motherboard.getProductName() << "\"\n";
    jsonFile << "  },\n";
    jsonFile << "  \"System\": {\n";
    jsonFile << "    \"Kernel\": \"" << getKernelInfo() << "\",\n";
    jsonFile << "    \"DefaultShell\": \"" << shell() << "\",\n";
    jsonFile << "    \"BuildInfo\": \"" << BuildInfo() << "\",\n";
    jsonFile << "    \"BootMode\": \"" << getBootMode() << "\",\n";
    jsonFile << "    \"PackageManager\": \"" << getPackageManager() << "\",\n";
    jsonFile << "    \"Distro\": " << getDistroInfo() << ",\n";
    jsonFile << "    \"SecureBoot\": \"" << CheckSecureBoot() << "\",\n";
    jsonFile << "    \"TotalRAM_GB\": " << getRAMInfo("RAM") << ",\n";
    jsonFile << "    \"FreeRAM_GB\": " << getRAMInfo("FREE") << "\n";
    jsonFile << "  }\n";
    jsonFile << "}\n";
    jsonFile.close();

  
}



int main(int argc, char *argv[]){
    auto start = std::chrono::high_resolution_clock::now();

    BIOSInfo bios;
    MotherboardInfo Motherboard;
    CPUInfo CPU;

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
    
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Execution time: " << duration.count() << " microseconds\n";

}                                                

#include <Controller.h>
#include <MOS6502.h>
#include <iostream>
#include <iomanip>
#include <fstream>

Controller::Controller(std::ifstream &ROM) {
    // Load ROM in memory
    // ROM.seekg(0x0010, std::ios::beg);
    // ROM.read((char*)(&memory[ROMADDR]), 3);
    // ROM.seekg(0x0010, std::ios::beg);
    // ROM.read((char*)(&memory[0xC000]), 0x4000);
    ROM.seekg(0, std::ios::beg);
    ROM.read((char*)(&memory[0]), 0x4000);
}

void Controller::run() {
    while(1) {
        CPU.executeOP(memory);
    }
}
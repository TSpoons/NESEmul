#pragma once
#include <MOS6502.h>
#include <PPUCHIP.h>
#include <iostream>
#include <fstream>

class Controller
{
public:
    Controller(std::ifstream &ROM);
    void run();
    
private:
    const int ROMADDR = 0x8000;

    uint8_t memory[0xFFFF];

    MOS6502 CPU;
    PPUCHIP PPU;
};
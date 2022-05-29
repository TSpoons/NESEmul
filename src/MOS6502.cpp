#include <MOS6502.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

MOS6502::MOS6502() {
    CPULogFile.open("../ROMS/CPULogFile.txt", std::ofstream::out | std::ofstream::trunc);

    // Fill opcode lookup table
    for (int i = 0; i < 256; i++) {
        opcodeLookup.push_back(NULL);
    }
    for (int i = 0; i < sizeof(opcodes) / sizeof(opcodeDef); i++) {
        opcodeLookup[opcodes[i].opcodeValue] = &opcodes[i];
    }
    SR.set(interrupt);
    SR.set(none);
    totalClk = 7;
}

void MOS6502::executeOP(uint8_t (&memory)[0xFFFF]) {
    int clk = 0;

    logBuf = "";
    std:: string regLogBuf = getRegisterLog();
    std::stringstream stream;
    stream << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << (int)PC;
    logBuf += stream.str(); 
    logBuf += "  ";

    int opcode = getByte(memory);
    opcodeFuncPtr op = opcodeLookup[opcode]->funcPtr;
    (this->*op)(clk, memory);

    logBuf.resize((size_t)15, ' ');
    logBuf += opcodeLookup[opcode]->funcName;
    logBuf.resize((size_t)26, ' ');

    logBuf += regLogBuf;
    CPULogFile.write(&logBuf[0], logBuf.size());
    CPULogFile.flush();

    totalClk += clk;
}


std::string MOS6502::getRegisterLog() {
    std::stringstream stream;
    stream 
    << "A:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)AC << " " 
    << "X:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)X << " " 
    << "Y:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)Y << " " 
    << "SR:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << SR.to_ulong() << " "
    << "SP:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)SP << " " 
    << "CYC:" << std::dec << totalClk << "\n";
    return stream.str();
}

void MOS6502::setReg(uint8_t &reg, uint8_t val) {
    SR.set(zero, val == 0);
    SR.set(negative, val & 0x80);
    reg = val;
}

uint8_t MOS6502::getByte(uint8_t (&memory)[0xFFFF]) {
    std::stringstream stream;
    stream << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)memory[PC];
    logBuf += stream.str(); 
    logBuf += " ";
    return memory[PC++];
}

uint16_t MOS6502::addPgCross(uint8_t LSB, uint8_t addValue, uint8_t MSB, int &clk, bool addClk) {
    uint16_t LSBAdd = LSB + addValue;
    uint16_t MSBAdd = MSB;
    if ((LSBAdd > 0xFF) && addClk) {
        clk++;
    }
    return (MSBAdd << 8) + LSBAdd;
}

uint8_t MOS6502::zpModeAddr(uint8_t (&memory)[0xFFFF]) {
    return getByte(memory);
}

uint16_t MOS6502::zpindModeAddr(uint8_t addValue, uint8_t (&memory)[0xFFFF]) {
    return (getByte(memory) + addValue) & 0xFF;
}

uint16_t MOS6502::absModeAddr(uint8_t (&memory)[0xFFFF]) {
    uint8_t LSB = getByte(memory);
    uint8_t MSB = getByte(memory);
    return (MSB << 8) + LSB;
}

uint16_t MOS6502::absindModeAddr(uint8_t addValue, uint8_t (&memory)[0xFFFF], int &clk, bool addClk) {
    uint8_t LSB = getByte(memory);
    uint8_t MSB = getByte(memory);
    return addPgCross(LSB, addValue, MSB, clk, addClk);
}

uint16_t MOS6502::indxModeAddr(uint8_t (&memory)[0xFFFF]) {
    uint16_t memAddr = getByte(memory) + X;
    uint8_t LSB = memory[memAddr & 0xFF];
    uint8_t MSB = memory[((memAddr + 1) & 0xFF)];
    return (MSB << 8) + LSB;
}

uint16_t MOS6502::indyModeAddr(uint8_t (&memory)[0xFFFF], int &clk, bool addClk) {
    uint8_t memAddr = getByte(memory);
    uint8_t LSB = memory[memAddr];
    uint8_t MSB = memory[(memAddr + 1) & 0xFF];
    return addPgCross(LSB, Y, MSB, clk, addClk);
}

// LDA  load accumulator 
void MOS6502::LDA_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, getByte(memory));
    clk += 2;
}
void MOS6502::LDA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::LDA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::LDA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::LDA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::LDA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::LDA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::LDA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}

// LDX  load X
void MOS6502::LDX_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, getByte(memory));
    clk += 2;
}
void MOS6502::LDX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::LDX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[zpindModeAddr(Y, memory)]);
    clk += 4;
}
void MOS6502::LDX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::LDX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
// LDY  load Y 
void MOS6502::LDY_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, getByte(memory));
    clk += 2;
}
void MOS6502::LDY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::LDY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::LDY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::LDY_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
// STA  store accumulator 
void MOS6502::STA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr(memory)] = AC;
    clk += 3;
}
void MOS6502::STA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X, memory)] = AC;
    clk += 4;
}
void MOS6502::STA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr(memory)] = AC;
    clk += 4;
}
void MOS6502::STA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(X, memory, clk, false)] = AC;
    clk += 5;
}
void MOS6502::STA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(Y, memory, clk, false)] = AC;
    clk += 5;
}
void MOS6502::STA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[indxModeAddr(memory)] = AC;
    clk += 6;
}
void MOS6502::STA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[indyModeAddr(memory, clk, false)] = AC;
    clk += 6;
}
// STX  store X 
void MOS6502::STX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr(memory)] = X;
    clk += 3;
}
void MOS6502::STX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(Y, memory)] = X;
    clk += 4;
}
void MOS6502::STX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr(memory)] = X;
    clk += 4;
}
// STY  store Y 
void MOS6502::STY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr(memory)] = Y;
    clk += 3;
}
void MOS6502::STY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X, memory)] = Y;
    clk += 4;
}
void MOS6502::STY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr(memory)] = Y;
    clk += 4;
}
// TAX  transfer accumulator to X 
void MOS6502::TAX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, AC);
    clk += 2;
}
// TAY  transfer accumulator to Y 
void MOS6502::TAY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, AC);
    clk += 2;
}
// TSX  transfer stack pointer to X 
void MOS6502::TSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, SP);
    clk += 2;
}
// TXA  transfer X to accumulator 
void MOS6502::TXA(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, X);
    clk += 2;
}
// TXS  transfer X to stack pointer 
void MOS6502::TXS(int &clk, uint8_t (&memory)[0xFFFF]){
    SP = X;
    clk += 2;
}
// TYA  transfer Y to accumulator 
void MOS6502::TYA(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, Y);
    clk += 2;
}

// Stack instructions
uint16_t MOS6502::SPToAddr(){
    return 0x0100 | SP;
}

void MOS6502::pushToStack(uint8_t value, uint8_t (&memory)[0xFFFF]) {
    memory[SPToAddr()] = value;
    SP--;
}

uint8_t MOS6502::pullFromStack(uint8_t (&memory)[0xFFFF]) {
    SP++;
    uint8_t value = memory[SPToAddr()];
    return value;
}

// PHA  push accumulator 
void MOS6502::PHA(int &clk, uint8_t (&memory)[0xFFFF]){
    pushToStack(AC, memory);
    clk += 3;
}
// PHP  push processor status registers
void MOS6502::PHP(int &clk, uint8_t (&memory)[0xFFFF]){
    pushToStack(SR.to_ulong() | 0b00010000, memory);
    clk += 3;
}
// PLA  pull accumulator 
void MOS6502::PLA(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, pullFromStack(memory));
    clk += 4;
}
// PLP  pull processor status register 
void MOS6502::PLP(int &clk, uint8_t (&memory)[0xFFFF]){
    SR = (pullFromStack(memory) & 0b11101111) | 0b00100000;
    clk += 4; 
}

// Decrements and increments

// DEC  decrement (memory) 
void MOS6502::DEC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    setReg(memory[addr], memory[addr] - 1);
    clk += 5;
}
void MOS6502::DEC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    setReg(memory[addr], memory[addr] - 1);
    clk += 6;
}
void MOS6502::DEC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    setReg(memory[addr], memory[addr] - 1);
    clk += 6;
}
void MOS6502::DEC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    setReg(memory[addr], memory[addr] - 1);
    clk += 7;
}
// DEX  decrement X 
void MOS6502::DEX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, X - 1);
    clk += 2;
}
// DEY  decrement Y 
void MOS6502::DEY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, Y - 1);
    clk += 2;
}
// INC  increment (memory) 
void MOS6502::INC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    setReg(memory[addr], memory[addr] + 1);
    clk += 5;
}
void MOS6502::INC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    setReg(memory[addr], memory[addr] + 1);
    clk += 6;
}
void MOS6502::INC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    setReg(memory[addr], memory[addr] + 1);
    clk += 6;
}
void MOS6502::INC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    setReg(memory[addr], memory[addr] + 1);
    clk += 7;
}
// INX  increment X 
void MOS6502::INX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, X + 1);
    clk += 2;
}
// INY  increment Y 
void MOS6502::INY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, Y + 1);
    clk += 2;
}

// Arithmetic operations

void MOS6502::carryTest(uint16_t value) {
    SR.set(carry, (value & 0xFF00) != 0);
}

void MOS6502::overflowTest(uint8_t value) {
    // Both negative
    if ((value & 0b10000000) != 0 && (AC & 0b10000000) != 0)
        // set if result is positive
        SR.set(overflow, ((value + AC) & 0b10000000) == 0);
    // Both positive
    else if ((value & 0b10000000) == 0 && (AC & 0b10000000) == 0) {
        // set if result is negative
        int x = (value + AC);
        SR.set(overflow, ((value + AC) & 0b10000000) != 0);
    }
    else 
        SR.set(overflow, false);
}

void MOS6502::add(uint8_t value) {
    uint16_t sum = AC + value + SR.test(carry);
    uint8_t result = sum & 0xFFU;
    SR.set(carry, sum >> 8);  
    SR.set(overflow, !!((AC ^ result) & (value ^ result) & 0x80U));  
    setReg(AC, result);
}

void MOS6502::sub(uint8_t value) {
    add(value ^ 0xFFU);
}

// ADC  add with carry (prepare by CLC)                         
void MOS6502::ADC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    add(getByte(memory));
    clk += 2;
}
void MOS6502::ADC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::ADC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::ADC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::ADC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::ADC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::ADC_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::ADC_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    add(memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}
// SBC  subtract with carry (prepare by SEC)                
void MOS6502::SBC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(getByte(memory));
    clk += 2;
}
void MOS6502::SBC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::SBC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::SBC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::SBC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::SBC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::SBC_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::SBC_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    sub(memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}

// Logical operations

// AND  and (with accumulator) 
void MOS6502::AND_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & getByte(memory));
    clk += 2;
}
void MOS6502::AND_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::AND_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::AND_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::AND_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::AND_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::AND_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::AND_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}
// EOR  exclusive or (with accumulator)
void MOS6502::EOR_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ getByte(memory));
    clk += 2;
}
void MOS6502::EOR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::EOR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::EOR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::EOR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::EOR_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::EOR_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::EOR_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}
// ORA  (inclusive) or with accumulator 
void MOS6502::ORA_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | getByte(memory));
    clk += 2;
}
void MOS6502::ORA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::ORA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::ORA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::ORA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::ORA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::ORA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::ORA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}

// Shift and rotate instructions
void MOS6502::ASLMem(uint8_t &memVal) {
    SR.set(carry, memVal & 0x80);
    setReg(memVal, memVal << 1);
}

void MOS6502::LSRMem(uint8_t &memVal) {
    SR.set(carry, memVal & 0x01);
    setReg(memVal, memVal >> 1);
}

void MOS6502::ROLMem(uint8_t &memVal) {
    uint8_t carryVal = memVal & 0x80;
    setReg(memVal, (memVal << 1) | SR.test(carry));
    SR.set(carry, carryVal);
}

void MOS6502::RORMem(uint8_t &memVal) {
    uint8_t carryVal = memVal & 0x01;
    setReg(memVal, (memVal | (SR.test(carry) << 8)) >> 1);
    SR.set(carry, carryVal);
}

// ASL  arithmetic shift left (shifts in a zero bit on the right) 
void MOS6502::ASL_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, AC & 0x80);
    setReg(AC, AC << 1);
    clk += 2;
}
void MOS6502::ASL_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[zpModeAddr(memory)]);
    clk += 5;
}
void MOS6502::ASL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[zpindModeAddr(X, memory)]);
    clk += 6;
}
void MOS6502::ASL_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[absModeAddr(memory)]);
    clk += 6;
}
void MOS6502::ASL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[absindModeAddr(X, memory, clk, false)]);
    clk += 7;
}
// LSR  logical shift right (shifts in a zero bit on the left) 
void MOS6502::LSR_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, AC & 0x01);
    setReg(AC, AC >> 1);
    clk += 2;
}
void MOS6502::LSR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[zpModeAddr(memory)]);
    clk += 5;
}
void MOS6502::LSR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[zpindModeAddr(X, memory)]);
    clk += 6;
}
void MOS6502::LSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[absModeAddr(memory)]);
    clk += 6;
}
void MOS6502::LSR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[absindModeAddr(X, memory, clk, false)]);
    clk += 7;
}
// ROL  rotate left (shifts in carry bit on the right) 
void MOS6502::ROL_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t carryVal = AC & 0x80;
    setReg(AC, (AC << 1) | SR.test(carry));
    SR.set(carry, carryVal);
    clk += 2;
}
void MOS6502::ROL_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[zpModeAddr(memory)]);
    clk += 5;
}
void MOS6502::ROL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[zpindModeAddr(X, memory)]);
    clk += 6;
}
void MOS6502::ROL_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[absModeAddr(memory)]);
    clk += 6;
}
void MOS6502::ROL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[absindModeAddr(X, memory, clk, false)]);
    clk += 7;
}
// ROR  rotate right (shifts in zero bit on the left) 
void MOS6502::ROR_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t carryVal = AC & 0x01;
    setReg(AC, (AC >> 1) | (SR.test(carry) << 7));
    SR.set(carry, carryVal);
    clk += 2;
}
void MOS6502::ROR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[zpModeAddr(memory)]);
    clk += 5;
}
void MOS6502::ROR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[zpindModeAddr(X, memory)]);
    clk += 6;
}
void MOS6502::ROR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[absModeAddr(memory)]);
    clk += 6;
}
void MOS6502::ROR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[absindModeAddr(X, memory, clk, false)]);
    clk += 7;
}

// Flag instructions

// CLC  clear carry 
void MOS6502::CLC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, false);
    clk += 2;
}
// CLD  clear decimal (BCD arithmetics disabled)
void MOS6502::CLD(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(decimal, false);
    clk += 2;
}
// CLI  clear interrupt disable 
void MOS6502::CLI(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(interrupt, false);
    clk += 2;
}
// CLV  clear overflow 
void MOS6502::CLV(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(overflow, false);
    clk += 2;
}
// SEC  set carry 
void MOS6502::SEC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry);
    clk += 2;
}
// SED  set decimal (BCD arithmetics enabled) 
void MOS6502::SED(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(decimal);
    clk += 2;
}
// SEI  set interrupt disable 
void MOS6502::SEI(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(interrupt);
    clk += 2;
}

// Comparisons

void MOS6502::CMPTest(uint8_t reg, uint8_t val) { 
    SR.set(carry, reg >= val);
    SR.set(zero, reg == val);
    SR.set(negative, (reg - val) & 0b10000000);
}

// CMP  compare (with accumulator)
void MOS6502::CMP_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, getByte(memory));
    clk += 2;
}
void MOS6502::CMP_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::CMP_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[zpindModeAddr(X, memory)]);
    clk += 4;
}
void MOS6502::CMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absModeAddr(memory)]);
    clk += 4;
}
void MOS6502::CMP_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absindModeAddr(X, memory, clk, true)]);
    clk += 4;
}
void MOS6502::CMP_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absindModeAddr(Y, memory, clk, true)]);
    clk += 4;
}
void MOS6502::CMP_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[indxModeAddr(memory)]);
    clk += 6;
}
void MOS6502::CMP_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[indyModeAddr(memory, clk, true)]);
    clk += 5;
}
// CPX  compare with X 
void MOS6502::CPX_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, getByte(memory));
    clk += 2;
}
void MOS6502::CPX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::CPX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, memory[absModeAddr(memory)]);
    clk += 4;
}
// CPY  compare with Y 
void MOS6502::CPY_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, getByte(memory));
    clk += 2;
}
void MOS6502::CPY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, memory[zpModeAddr(memory)]);
    clk += 3;
}
void MOS6502::CPY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, memory[absModeAddr(memory)]);
    clk += 4;
}

// Conditional branch instructions
void MOS6502::checkBranchPgCross(int8_t jump, int &clk){
    if ((PC % 256 + jump) > 255) {
        clk++;
    }
}

// BCC  branch on carry clear 
void MOS6502::BCC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (!SR.test(carry)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BCS  branch on carry set 
void MOS6502::BCS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (SR.test(carry)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BEQ  branch on equal (zero set) 
void MOS6502::BEQ(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (SR.test(zero)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BMI  branch on minus (negative set) 
void MOS6502::BMI(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (SR.test(negative)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BNE  branch on not equal (zero clear) 
void MOS6502::BNE(int &clk, uint8_t (&memory)[0xFFFF]){
    int8_t jump = getByte(memory);
    if (!SR.test(zero)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BPL   branch on plus (negative clear) 
void MOS6502::BPL(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (!SR.test(negative)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    }
    clk += 2; 
}
// BVC  branch on overflow clear 
void MOS6502::BVC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (!SR.test(overflow)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk = 1; // 3 cycles no matter the adress
    } 
    clk += 2;
}
// BVS  branch on overflow set 
void MOS6502::BVS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte(memory);
    if (SR.test(overflow)){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}

// Jumps and subroutines

// JMP  jump 
void MOS6502::JMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    PC = absModeAddr(memory);
    clk += 3;
}
void MOS6502::JMP_IND(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    if ((addr & 0x00FF) == 0xFF) {
        PC = (memory[addr & 0xFF00] << 8) + memory[addr];
    }
    else 
        PC = (memory[addr + 1] << 8) + memory[addr];
    clk += 5;
}
// JSR  jump subroutine 
void MOS6502::JSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    pushToStack((PC - 1) >> 8, memory);
    pushToStack((PC - 1) & 0x00FF, memory);
    PC = addr;
    clk += 6;
}
// RTS  return from subroutine 
void MOS6502::RTS_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = pullFromStack(memory);
    uint8_t MSB = pullFromStack(memory);
    PC = ((MSB << 8) + LSB) + 1;
    clk += 6;
}

// Interrupts

// BRK  break / software interrupt 
void MOS6502::BRK_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    pushToStack((PC + 1) >> 8, memory);
    pushToStack((PC + 1) & 0x00FF, memory);
    pushToStack(SR.to_ulong(), memory);
    PC = memory[INTERRUPTVEC] + (memory[INTERRUPTVEC + 1] << 8);
    SR.set(brk);
    SR.set(interrupt);
    clk += 7;
}
// RTI  return from interrupt 
void MOS6502::RTI_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    SR = pullFromStack(memory);
    SR.set(brk, false);
    SR.set(none);

    uint8_t LSB = pullFromStack(memory);
    uint8_t MSB = pullFromStack(memory);
    PC = (MSB << 8) + LSB;
    clk += 6;
}

// Other

// BIT  bit test (accumulator & memory) 
void MOS6502::BIT_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[zpModeAddr(memory)];
    SR.set(zero, !(AC & value));
    SR.set(negative, (value & 0b10000000) != 0);
    SR.set(overflow, (value & 0b01000000) != 0);
    clk += 3;
}
void MOS6502::BIT_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[absModeAddr(memory)];
    SR.set(zero, !(AC & value));
    SR.set(negative, (value & 0b10000000) != 0);
    SR.set(overflow, (value & 0b01000000) != 0);
    clk += 4;
}
// NOP  no operation 
void MOS6502::NOP_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 2;
}

// Illegal opcodes
void MOS6502::ALR_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t andValue = AC & getByte(memory);
    SR.set(carry, andValue & 0x01);
    setReg(AC, andValue >> 1);
    clk += 2;
}
void MOS6502::ANC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, AC & 0x80);
    setReg(AC, AC & getByte(memory));
    clk += 2;
}
// unstable, not implemented
void MOS6502::ANE_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 2;
}
void MOS6502::ARR_IM(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::DCP_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 5;
}
void MOS6502::DCP_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 6;
}
void MOS6502::DCP_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 6;
}
void MOS6502::DCP_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 7;
}
void MOS6502::DCP_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 7;
}
void MOS6502::DCP_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 8;
}
void MOS6502::DCP_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    memory[addr]--;
    CMPTest(AC, memory[addr]);
    clk += 8;
}
void MOS6502::ISC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    memory[addr]++;
    sub(memory[addr]);
    clk += 5;
}
void MOS6502::ISC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    memory[addr]++;
    sub(memory[addr]);
    clk += 6;
}
void MOS6502::ISC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    memory[addr]++;
    sub(memory[addr]);
    clk += 6;
}
void MOS6502::ISC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    memory[addr]++;
    sub(memory[addr]);
    clk += 7;
}
void MOS6502::ISC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    memory[addr]++;
    sub(memory[addr]);
    clk += 7;
}
void MOS6502::ISC_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    memory[addr]++;
    sub(memory[addr]);
    clk += 8;
}
void MOS6502::ISC_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    memory[addr]++;
    sub(memory[addr]);
    clk += 8;
}
void MOS6502::LAS_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::LAX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[zpModeAddr(memory)];
    setReg(AC, value);
    setReg(X, value);
    clk += 3;
}
void MOS6502::LAX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[zpindModeAddr(Y, memory)];
    setReg(AC, value);
    setReg(X, value);
    clk += 4;
}
void MOS6502::LAX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[absModeAddr(memory)];
    setReg(AC, value);
    setReg(X, value);
    clk += 4;
}
void MOS6502::LAX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[absindModeAddr(Y, memory, clk, true)];
    setReg(AC, value);
    setReg(X, value);
    clk += 4;
}
void MOS6502::LAX_INDX(int &clk, uint8_t (&memory)[0xFFFF]) {
    uint8_t value = memory[indxModeAddr(memory)];
    setReg(AC, value);
    setReg(X, value);
    clk += 6;
}
void MOS6502::LAX_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t value = memory[indyModeAddr(memory, clk, true)];
    setReg(AC, value);
    setReg(X, value);
    clk += 5;
}
// unstable, not implemented
void MOS6502::LXA_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 2;
}
void MOS6502::RLA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 5;
}
void MOS6502::RLA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 6;
}
void MOS6502::RLA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 6;
}
void MOS6502::RLA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 7;
}
void MOS6502::RLA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 7;
}
void MOS6502::RLA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 8;
}
void MOS6502::RLA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    ROLMem(memory[addr]);
    setReg(AC, AC & memory[addr]);
    clk += 8;
}
void MOS6502::RRA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 5;
}
void MOS6502::RRA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 6;
}
void MOS6502::RRA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 6;
}
void MOS6502::RRA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 7;
}
void MOS6502::RRA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 7;
}
void MOS6502::RRA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 8;
}
void MOS6502::RRA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    RORMem(memory[addr]);
    add(memory[addr]);
    clk += 8;
}
void MOS6502::SAX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr(memory)] = AC & X;
    clk += 3;
}
void MOS6502::SAX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(Y, memory)] = AC & X;
    clk += 4;
}
void MOS6502::SAX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr(memory)] = AC & X;
    clk += 4;
}
void MOS6502::SAX_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[indxModeAddr(memory)] = AC & X;
    clk += 6;
}
void MOS6502::SBX_IM(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::SHA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::SHA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::SHX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::SHY_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::SLO_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 5;
}
void MOS6502::SLO_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 6;
}
void MOS6502::SLO_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 6;
}
void MOS6502::SLO_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 7;
}
void MOS6502::SLO_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 7;
}
void MOS6502::SLO_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 8;
}
void MOS6502::SLO_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    ASLMem(memory[addr]);
    setReg(AC, AC | memory[addr]);
    clk += 8;
}
void MOS6502::SRE_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpModeAddr(memory);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 5;
}
void MOS6502::SRE_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = zpindModeAddr(X, memory);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 6;
}
void MOS6502::SRE_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absModeAddr(memory);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 6;
}
void MOS6502::SRE_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(X, memory, clk, false);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 7;
}
void MOS6502::SRE_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = absindModeAddr(Y, memory, clk, false);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 7;
}
void MOS6502::SRE_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indxModeAddr(memory);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 8;
}
void MOS6502::SRE_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t addr = indyModeAddr(memory, clk, false);
    LSRMem(memory[addr]);
    setReg(AC, AC ^ memory[addr]);
    clk += 8;
}
void MOS6502::TAS_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){}
void MOS6502::USBC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    SBC_IM(clk, memory);
}
void MOS6502::NOP_0B2C(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 2;
}
void MOS6502::NOP_1B2C(int &clk, uint8_t (&memory)[0xFFFF]){
    getByte(memory);
    clk += 2;
}
void MOS6502::NOP_2B2C(int &clk, uint8_t (&memory)[0xFFFF]){
    getByte(memory);
    getByte(memory);
    clk += 2;
}
void MOS6502::NOP_1B3C(int &clk, uint8_t (&memory)[0xFFFF]){
    getByte(memory);
    clk += 3;
}
void MOS6502::NOP_1B4C(int &clk, uint8_t (&memory)[0xFFFF]){
    getByte(memory);
    clk += 4;
}
void MOS6502::NOP_2B4C(int &clk, uint8_t (&memory)[0xFFFF]){
    getByte(memory);
    getByte(memory);
    clk += 4;
}
void MOS6502::NOP_2B45C(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = getByte(memory);
    uint8_t MSB = getByte(memory);
    addPgCross(LSB, X, MSB, clk, true);
    clk += 4;
}

void MOS6502::JAM(int &clk, uint8_t (&memory)[0xFFFF]){}
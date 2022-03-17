#include <MOS6502.h>
#include <vector>
#include <iostream>
#include <iomanip>


MOS6502::MOS6502() {
    // Fill opcode lookup table
    for (int i = 0; i < 256; i++) {
        opcodeLookup.push_back(NULL);
    }
    for (int i = 0; i < sizeof(opcodes) / sizeof(opcodeDef); i++) {
        opcodeLookup[opcodes[i].opcodeValue] = &opcodes[i];
    }

    totalClk = 7;
}

void MOS6502::init(std::ifstream &ROM) {
    // Load ROM in memory
    ROM.seekg(0x0010, std::ios::beg);
    ROM.read((char*)(&memory[ROMADDR]), 0x4000);
    ROM.seekg(0x0010, std::ios::beg);
    ROM.read((char*)(&memory[0xC000]), 0x4000);
}

void MOS6502::execute() {
    while(1){
        int clk = 0;

        logBuf = "";
        std:: string regLogBuf = getRegisterLog();
        std::stringstream stream;
        stream << std::setw(2) << std::setfill('0') << std::hex << (int)PC;
        logBuf += stream.str(); 
        logBuf += " ";


        int opcode = getByte();
        opcodeFuncPtr op = opcodeLookup[opcode]->funcPtr;
        (this->*op)(clk, memory);

        logBuf.resize((size_t)15, ' ');
        logBuf += opcodeLookup[opcode]->funcName;
        logBuf.resize((size_t)30, ' ');

        logBuf += regLogBuf;
        std::cout << logBuf;

        totalClk += clk;
    }
}

std::string MOS6502::getRegisterLog() {
    std::stringstream stream;
    stream 
    << "AC: " << std::setw(2) << std::setfill('0') << std::hex << AC << " " 
    << "X: " << std::setw(2) << std::setfill('0') << std::hex << " " 
    << "Y: " << std::setw(2) << std::setfill('0') << std::hex << Y << " " 
    << "SP: " << std::setw(2) << std::setfill('0') << std::hex << SP << " " 
    << "SR: " << std::setw(2) << std::setfill('0') << std::hex << SR << " "
    << "CYC: " << std::dec << totalClk << "\n";
    return stream.str();
}

void MOS6502::setReg(uint8_t reg, uint8_t val) {
    if (val == 0) SR.set(zero);
    else SR.set(zero, false);

    if (val & 0x80) SR.set(negative);
    else SR.set(negative, false);

    reg = val;
}

uint8_t MOS6502::getByte() {
    std::stringstream stream;
    stream << std::setw(2) << std::setfill('0') << std::hex << (int)memory[PC];
    logBuf += stream.str(); 
    logBuf += " ";
    return memory[PC++];
}

uint16_t MOS6502::addPgCross(uint8_t LSB, uint8_t addValue, uint8_t MSB, int &clk, bool addClk) {
    uint16_t LSBAdd = LSB + addValue;
    if ((LSBAdd > 0x00FF) & addClk) {
        MSB += LSBAdd & 0xFF00;
        clk++;
    }
    return (LSBAdd & 0x00FF) + (MSB & 0xFF00);
}

void MOS6502::carryTest(uint16_t value) {
    if ((value & 0xFF00) != 0) SR.set(carry);
    else SR.set(carry, false);
}

uint8_t MOS6502::zpModeAddr() {
    return getByte();
}

uint16_t MOS6502::zpindModeAddr(uint8_t addValue) {
    return (getByte() + addValue) % 0xFF;
}

uint16_t MOS6502::absModeAddr() {
    uint8_t LSB = getByte();
    uint8_t MSB = getByte();
    return MSB << 8 + LSB;
}

uint16_t MOS6502::absindModeAddr(uint8_t addValue, int &clk, bool addClk) {
    return addPgCross(getByte(), X, getByte(), clk, addClk);
}

uint16_t MOS6502::indxModeAddr() {
    uint16_t memAddr = getByte() + X;
    return memory[memAddr + 1] << 8 + memory[memAddr];
}

uint16_t MOS6502::indyModeAddr(int &clk, bool addClk) {
    uint8_t memAddr = getByte();
    return addPgCross(memory[memAddr], Y, memory[memAddr + 1], clk, addClk);
}

void MOS6502::ASLMem(uint8_t &memVal) {
    SR.set(carry, memVal & 0x80);
    memVal = memVal << 1;
}

void MOS6502::LSRMem(uint8_t &memVal) {
    SR.set(carry, memVal & 0x01);
    memVal = memVal >> 1;
}

void MOS6502::ROLMem(uint8_t &memVal) {
    uint8_t carryVal = memVal & 0x80;
    memVal = (memVal << 1) | SR[carry];
    SR.set(carry, carryVal);
}

void MOS6502::RORMem(uint8_t &memVal) {
    uint8_t carryVal = memVal & 0x01;
    memVal = (memVal >> 1) | (SR[carry] << 8);
    SR.set(carry, carryVal);
}

// LDA  load accumulator 
void MOS6502::LDA_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, getByte());
    clk += 2;
}
void MOS6502::LDA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::LDA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[(getByte() + X) % 0xFF]);
    clk += 4;
}
void MOS6502::LDA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::LDA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
void MOS6502::LDA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
void MOS6502::LDA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[indxModeAddr()]);
    clk += 6;
}
void MOS6502::LDA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[indyModeAddr(clk, true)]);
    clk += 6;
}

// LDX  load X
void MOS6502::LDX_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, getByte());
    clk += 2;
}
void MOS6502::LDX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::LDX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[zpindModeAddr(Y)]);
    clk += 4;
}
void MOS6502::LDX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::LDX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(X, memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
// LDY  load Y 
void MOS6502::LDY_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, getByte());
    clk += 2;
}
void MOS6502::LDY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::LDY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[zpindModeAddr(X)]);
    clk += 4;
}
void MOS6502::LDY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::LDY_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(Y, memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
// STA  store accumulator 
void MOS6502::STA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr()] = AC;
    clk += 3;
}
void MOS6502::STA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X)] = AC;
    clk += 4;
}
void MOS6502::STA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr()] = AC;
    clk += 4;
}
void MOS6502::STA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(X, clk, true)] = AC;
    clk += 5;
}
void MOS6502::STA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(Y, clk, true)] = AC;
    clk += 5;
}
void MOS6502::STA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[indxModeAddr()] = AC;
    clk += 6;
}
void MOS6502::STA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[indyModeAddr(clk, false)] = AC;
    clk += 6;
}
// STX  store X 
void MOS6502::STX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr()] = X;
    clk += 3;
}
void MOS6502::STX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(Y)] = X;
    clk += 4;
}
void MOS6502::STX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr()] = X;
    clk += 4;
}
// STY  store Y 
void MOS6502::STY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr()] = Y;
    clk += 3;
}
void MOS6502::STY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X)] = Y;
    clk += 4;
}
void MOS6502::STY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr()] = Y;
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

// PHA  push accumulator 
void MOS6502::PHA(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[++SP] = AC;
    clk += 3;
}
// PHP  push processor status registers
void MOS6502::PHP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[++SP] = SR.to_ulong();
    clk += 3;
}
// PLA  pull accumulator 
void MOS6502::PLA(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, memory[SP--]);
    clk += 4;
}
// PLP  pull processor status register 
void MOS6502::PLP(int &clk, uint8_t (&memory)[0xFFFF]){
    SR = memory[SP--];
    clk += 4; 
}

// Decrements and increments

// DEC  decrement (memory) 
void MOS6502::DEC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpModeAddr()]--;
    clk += 5;
}
void MOS6502::DEC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X)]--;
    clk += 6;
}
void MOS6502::DEC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr()]--;
    clk += 6;
}
void MOS6502::DEC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(X, clk, false)]--;
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
    memory[zpModeAddr()]++;
    clk += 5;
}
void MOS6502::INC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[zpindModeAddr(X)]++;
    clk += 6;
}
void MOS6502::INC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absModeAddr()]++;
    clk += 6;
}
void MOS6502::INC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[absindModeAddr(X, clk, false)]++;
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

// ADC  add with carry (prepare by CLC)                         ********MISSING OVERFLOW********
void MOS6502::ADC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + getByte();
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 2;
}
void MOS6502::ADC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[zpModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 3;
}
void MOS6502::ADC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[zpindModeAddr(X)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::ADC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[absModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::ADC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[absindModeAddr(X, clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::ADC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[absindModeAddr(Y, clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::ADC_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[indxModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 6;
}
void MOS6502::ADC_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC + memory[indyModeAddr(clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 5;
}
// SBC  subtract with carry (prepare by SEC)                ********UNSURE OF FLAGS AND CARRIES*********
void MOS6502::SBC_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - getByte();
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 2;
}
void MOS6502::SBC_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[zpModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 3;
}
void MOS6502::SBC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[zpindModeAddr(X)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::SBC_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[absModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::SBC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[absindModeAddr(X, clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::SBC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[absindModeAddr(Y, clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 4;
}
void MOS6502::SBC_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[indxModeAddr()];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 6;
}
void MOS6502::SBC_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    uint16_t result = AC - memory[indyModeAddr(clk, true)];
    carryTest(result);
    setReg(AC, result & 0x00FF);
    clk += 5;
}

// Logical operations

// AND  and (with accumulator) 
void MOS6502::AND_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & getByte());
    clk += 2;
}
void MOS6502::AND_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::AND_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[zpindModeAddr(X)]);
    clk += 4;
}
void MOS6502::AND_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::AND_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
void MOS6502::AND_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
void MOS6502::AND_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indxModeAddr()]);
    clk += 6;
}
void MOS6502::AND_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indyModeAddr(clk, true)]);
    clk += 5;
}
// EOR  exclusive or (with accumulator)
void MOS6502::EOR_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ getByte());
    clk += 2;
}
void MOS6502::EOR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::EOR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[zpindModeAddr(X)]);
    clk += 4;
}
void MOS6502::EOR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::EOR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC ^ memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
void MOS6502::EOR_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
void MOS6502::EOR_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indxModeAddr()]);
    clk += 6;
}
void MOS6502::EOR_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC & memory[indyModeAddr(clk, true)]);
    clk += 5;
}
// ORA  (inclusive) or with accumulator 
void MOS6502::ORA_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | getByte());
    clk += 2;
}
void MOS6502::ORA_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::ORA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[zpindModeAddr(X)]);
    clk += 4;
}
void MOS6502::ORA_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::ORA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
void MOS6502::ORA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
void MOS6502::ORA_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[indxModeAddr()]);
    clk += 6;
}
void MOS6502::ORA_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    setReg(AC, AC | memory[indyModeAddr(clk, true)]);
    clk += 5;
}

// Shift and rotate instructions

// ASL  arithmetic shift left (shifts in a zero bit on the right) 
void MOS6502::ASL_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, AC & 0x80);
    setReg(AC, AC << 1);
    clk += 2;
}
void MOS6502::ASL_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[zpModeAddr()]);
    clk += 5;
}
void MOS6502::ASL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[zpindModeAddr(X)]);
    clk += 6;
}
void MOS6502::ASL_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[absModeAddr()]);
    clk += 6;
}
void MOS6502::ASL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    ASLMem(memory[absindModeAddr(X, clk, false)]);
    clk += 7;
}
// LSR  logical shift right (shifts in a zero bit on the left) 
void MOS6502::LSR_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    SR.set(carry, AC & 0x01);
    setReg(AC, AC >> 1);
    clk += 2;
}
void MOS6502::LSR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[zpModeAddr()]);
    clk += 5;
}
void MOS6502::LSR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[zpindModeAddr(X)]);
    clk += 6;
}
void MOS6502::LSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[absModeAddr()]);
    clk += 6;
}
void MOS6502::LSR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    LSRMem(memory[absindModeAddr(X, clk, false)]);
    clk += 7;
}
// ROL  rotate left (shifts in carry bit on the right) 
void MOS6502::ROL_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t carryVal = AC & 0x80;
    setReg(AC, (AC << 1) | SR[carry]);
    SR.set(carry, carryVal);
    clk += 2;
}
void MOS6502::ROL_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[zpModeAddr()]);
    clk += 5;
}
void MOS6502::ROL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[zpindModeAddr(X)]);
    clk += 6;
}
void MOS6502::ROL_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[absModeAddr()]);
    clk += 6;
}
void MOS6502::ROL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    ROLMem(memory[absindModeAddr(X, clk, false)]);
    clk += 7;
}
// ROR  rotate right (shifts in zero bit on the left) 
void MOS6502::ROR_ACC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t carryVal = AC & 0x01;
    setReg(AC, (AC >> 1) | (SR[carry] << 7));
    SR.set(carry, carryVal);
    clk += 2;
}
void MOS6502::ROR_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[zpModeAddr()]);
    clk += 5;
}
void MOS6502::ROR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[zpindModeAddr(X)]);
    clk += 6;
}
void MOS6502::ROR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[absModeAddr()]);
    clk += 6;
}
void MOS6502::ROR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    RORMem(memory[absindModeAddr(X, clk, false)]);
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
    SR.set(interrupt);        clk++;

    clk += 2;
}

// Comparisons

void MOS6502::CMPTest(uint8_t reg, uint8_t val) { 
    reg >= val ? SR.set(carry) : SR.set(carry, false);
    reg == val ? SR.set(zero) : SR.set(zero, false);
    reg != val ? SR.set(negative) : SR.set(negative, false);
}

// CMP  compare (with accumulator)
void MOS6502::CMP_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, getByte());
    clk += 2;
}
void MOS6502::CMP_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::CMP_ZPX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[zpindModeAddr(X)]);
    clk += 4;
}
void MOS6502::CMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absModeAddr()]);
    clk += 4;
}
void MOS6502::CMP_ABSX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absindModeAddr(X, clk, true)]);
    clk += 4;
}
void MOS6502::CMP_ABSY(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[absindModeAddr(Y, clk, true)]);
    clk += 4;
}
void MOS6502::CMP_INDX(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[indxModeAddr()]);
    clk += 6;
}
void MOS6502::CMP_INDY(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(AC, memory[indyModeAddr(clk, true)]);
    clk += 5;
}
// CPX  compare with X 
void MOS6502::CPX_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, getByte());
    clk += 2;
}
void MOS6502::CPX_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::CPX_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(X, memory[absModeAddr()]);
    clk += 4;
}
// CPY  compare with Y 
void MOS6502::CPY_IM(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, getByte());
    clk += 2;
}
void MOS6502::CPY_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, memory[zpModeAddr()]);
    clk += 3;
}
void MOS6502::CPY_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    CMPTest(Y, memory[absModeAddr()]);
    clk += 4;
}

// Conditional branch instructions
void MOS6502::checkBranchPgCross(uint8_t jump, int &clk){
    if ((PC % 256 + jump) > 256) clk++;
}

// BCC  branch on carry clear 
void MOS6502::BCC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (!SR[carry]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BCS  branch on carry set 
void MOS6502::BCS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (SR[carry]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BEQ  branch on equal (zero set) 
void MOS6502::BEQ(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (SR[zero]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BMI  branch on minus (negative set) 
void MOS6502::BMI(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (SR[negative]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BNE  branch on not equal (zero clear) 
void MOS6502::BNE(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (!SR[zero]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}
// BPL   branch on plus (negative clear) 
void MOS6502::BPL(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (!SR[negative]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    }
    clk += 2; 
}
// BVC  branch on overflow clear 
void MOS6502::BVC(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (!SR[overflow]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk = 1; // 3 cycles no matter the adress
    } 
    clk += 2;
}
// BVS  branch on overflow set 
void MOS6502::BVS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t jump = getByte();
    if (SR[overflow]){
        checkBranchPgCross(jump, clk);
        PC += jump;
        clk++;
    } 
    clk += 2;
}

// Jumps and subroutines

// JMP  jump 
void MOS6502::JMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = getByte();
    uint8_t MSB = getByte();
    PC = (MSB << 8) + LSB;
    clk += 3;
}
void MOS6502::JMP_IND(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = getByte();
    uint8_t MSB = getByte();
    PC += (MSB << 8) + LSB;
    clk += 5;
}
// JSR  jump subroutine 
void MOS6502::JSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = getByte();
    uint8_t MSB = getByte();
    memory[++SP] = PC;
    PC = (MSB << 8) + LSB;
    clk += 6;
}
// RTS  return from subroutine 
void MOS6502::RTS_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    uint8_t LSB = memory[SP--];
    uint8_t MSB = memory[SP--];
    PC = MSB << 8 + LSB;
    clk += 6;
}

// Interrupts

// BRK  break / software interrupt 
void MOS6502::BRK_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    memory[++SP] = (PC + 1) >> 8;
    memory[++SP] = (PC + 1) & 0x00FF;
    memory[++SP] = SR.to_ulong();
    PC = memory[INTERRUPTVEC] + (memory[INTERRUPTVEC + 1] << 8);
    SR.set(brk);
    SR.set(interrupt);
    clk += 7;
}
// RTI  return from interrupt 
void MOS6502::RTI_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    SR = memory[SP--];
    SR.set(brk, false);

    uint8_t LSB = memory[SP--];
    uint8_t MSB = memory[SP--];
    PC = (MSB << 8) + LSB;
    clk += 6;
}

// Other

// BIT  bit test (accumulator & memory) 
void MOS6502::BIT_ZP(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 3;
}
void MOS6502::BIT_ABS(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 4;
}
// NOP  no operation 
void MOS6502::NOP_IMP(int &clk, uint8_t (&memory)[0xFFFF]){
    clk += 2;
}
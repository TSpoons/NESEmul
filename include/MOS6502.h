#pragma once
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>

class MOS6502
{
public:
    MOS6502();
    void init(std::ifstream &ROM);
    void executeOP(uint8_t (&memory)[0xFFFF]);

private:
    const int INTERRUPTVEC = 0xFFFE;
    int totalClk;

    std::ofstream CPULogFile;
    std::string logBuf;
    std::string getRegisterLog();

    enum Flags
    {
        carry,
        zero,
        interrupt,
        decimal, // decimal mode not implemented
        brk,
        none,
        overflow,
        negative
    };

    Flags flag = Flags();

    // Registers
    uint16_t PC = 0xC000;
    uint16_t SP = 0xFD;
    uint8_t AC = 0;
    uint8_t X = 0;
    uint8_t Y = 0;
    std::bitset<8> SR;

    typedef void (MOS6502::*opcodeFuncPtr)(int &, uint8_t (&memory)[0xFFFF]);

    struct opcodeDef
    {
        opcodeFuncPtr funcPtr;
        uint8_t opcodeValue;
        std::string funcName;
    };

    std::vector<opcodeDef *> opcodeLookup;

    void setReg(uint8_t &reg, uint8_t val);
    uint8_t getByte(uint8_t (&memory)[0xFFFF]);

    uint16_t addPgCross(uint8_t LSB, uint8_t addValue, uint8_t MSB, int &clk, bool addClk);
    void carryTest(uint16_t value);
    void overflowTest(uint8_t value);
    void add(uint8_t value);
    void sub(uint8_t value);
    void CMPTest(uint8_t reg, uint8_t val);
    void checkBranchPgCross(int8_t jump, int &clk);

    uint16_t SPToAddr();
    void pushToStack(uint8_t value, uint8_t (&memory)[0xFFFF]);
    uint8_t pullFromStack(uint8_t (&memory)[0xFFFF]);

    uint8_t zpModeAddr(uint8_t (&memory)[0xFFFF]);
    uint16_t zpindModeAddr(uint8_t addValue, uint8_t (&memory)[0xFFFF]);
    uint16_t absModeAddr(uint8_t (&memory)[0xFFFF]);
    uint16_t absindModeAddr(uint8_t addValue, uint8_t (&memory)[0xFFFF], int &clk, bool addClk);
    uint16_t indxModeAddr(uint8_t (&memory)[0xFFFF]);
    uint16_t indyModeAddr(uint8_t (&memory)[0xFFFF], int &clk, bool addClk);

    void ASLMem(uint8_t &memVal);
    void LSRMem(uint8_t &memVal);
    void ROLMem(uint8_t &memVal);
    void RORMem(uint8_t &memVal);

    // opcodes

    // Transfer

    // LDA  load accumulator
    void LDA_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // LDX  load X
    void LDX_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDX_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDX_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    // LDY  load Y
    void LDY_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDY_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDY_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void LDY_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // STA  store accumulator
    void STA_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void STA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // STX  store X
    void STX_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void STX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]);
    void STX_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    // STY  store Y
    void STY_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void STY_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void STY_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    // TAX  transfer accumulator to X
    void TAX(int &clk, uint8_t (&memory)[0xFFFF]);
    // TAY  transfer accumulator to Y
    void TAY(int &clk, uint8_t (&memory)[0xFFFF]);
    // TSX  transfer stack pointer to X
    void TSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // TXA  transfer X to accumulator
    void TXA(int &clk, uint8_t (&memory)[0xFFFF]);
    // TXS  transfer X to stack pointer
    void TXS(int &clk, uint8_t (&memory)[0xFFFF]);
    // TYA  transfer Y to accumulator
    void TYA(int &clk, uint8_t (&memory)[0xFFFF]);

    // Stack instructions

    // PHA  push accumulator
    void PHA(int &clk, uint8_t (&memory)[0xFFFF]);
    // PHP  push processor status register (with break flag set)
    void PHP(int &clk, uint8_t (&memory)[0xFFFF]);
    // PLA  pull accumulator
    void PLA(int &clk, uint8_t (&memory)[0xFFFF]);
    // PLP  pull processor status register
    void PLP(int &clk, uint8_t (&memory)[0xFFFF]);

    // Decrements and increments

    // DEC  decrement (memory)
    void DEC_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void DEC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void DEC_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void DEC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // DEX  decrement X
    void DEX(int &clk, uint8_t (&memory)[0xFFFF]);
    // DEY  decrement Y
    void DEY(int &clk, uint8_t (&memory)[0xFFFF]);
    // INC  increment (memory)
    void INC_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void INC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void INC_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void INC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // INX  increment X
    void INX(int &clk, uint8_t (&memory)[0xFFFF]);
    // INY  increment Y
    void INY(int &clk, uint8_t (&memory)[0xFFFF]);

    // Arithmetic operations

    // ADC  add with carry (prepare by CLC)
    void ADC_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ADC_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // SBC  subtract with carry (prepare by SEC)
    void SBC_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SBC_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    // Logical operations

    // AND  and (with accumulator)
    void AND_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void AND_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // EOR  exclusive or (with accumulator)
    void EOR_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void EOR_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // ORA  (inclusive) or with accumulator
    void ORA_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ORA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    // Shift and rotate instructions

    // ASL  arithmetic shift left (shifts in a zero bit on the right)
    void ASL_ACC(int &clk, uint8_t (&memory)[0xFFFF]);
    void ASL_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ASL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ASL_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ASL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // LSR  logical shift right (shifts in a zero bit on the left)
    void LSR_ACC(int &clk, uint8_t (&memory)[0xFFFF]);
    void LSR_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void LSR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void LSR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // ROL  rotate left (shifts in carry bit on the right)
    void ROL_ACC(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROL_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROL_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROL_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROL_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    // ROR  rotate right (shifts in zero bit on the left)
    void ROR_ACC(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROR_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROR_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROR_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ROR_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);

    // Flag instructions

    // CLC  clear carry
    void CLC(int &clk, uint8_t (&memory)[0xFFFF]);
    // CLD  clear decimal (BCD arithmetics disabled)
    void CLD(int &clk, uint8_t (&memory)[0xFFFF]);
    // CLI  clear interrupt disable
    void CLI(int &clk, uint8_t (&memory)[0xFFFF]);
    // CLV  clear overflow
    void CLV(int &clk, uint8_t (&memory)[0xFFFF]);
    // SEC  set carry
    void SEC(int &clk, uint8_t (&memory)[0xFFFF]);
    // SED  set decimal (BCD arithmetics enabled)
    void SED(int &clk, uint8_t (&memory)[0xFFFF]);
    // SEI  set interrupt disable
    void SEI(int &clk, uint8_t (&memory)[0xFFFF]);

    // Comparisons

    // CMP  compare (with accumulator)
    void CMP_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void CMP_INDY(int &clk, uint8_t (&memory)[0xFFFF]);
    // CPX  compare with X
    void CPX_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void CPX_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void CPX_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    // CPY  compare with Y
    void CPY_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void CPY_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void CPY_ABS(int &clk, uint8_t (&memory)[0xFFFF]);

    // Conditional branch instructions

    // BCC  branch on carry clear
    void BCC(int &clk, uint8_t (&memory)[0xFFFF]);
    // BCS  branch on carry set
    void BCS(int &clk, uint8_t (&memory)[0xFFFF]);
    // BEQ  branch on equal (zero set)
    void BEQ(int &clk, uint8_t (&memory)[0xFFFF]);
    // BMI  branch on minus (negative set)
    void BMI(int &clk, uint8_t (&memory)[0xFFFF]);
    // BNE  branch on not equal (zero clear)
    void BNE(int &clk, uint8_t (&memory)[0xFFFF]);
    // BPL   branch on plus (negative clear)
    void BPL(int &clk, uint8_t (&memory)[0xFFFF]);
    // BVC  branch on overflow clear
    void BVC(int &clk, uint8_t (&memory)[0xFFFF]);
    // BVS  branch on overflow set
    void BVS(int &clk, uint8_t (&memory)[0xFFFF]);

    // Jumps and subroutines

    // JMP  jump
    void JMP_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void JMP_IND(int &clk, uint8_t (&memory)[0xFFFF]);
    // JSR  jump subroutine
    void JSR_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    // RTS  return from subroutine
    void RTS_IMP(int &clk, uint8_t (&memory)[0xFFFF]);

    // Interrupts

    // BRK  break / software interrupt
    void BRK_IMP(int &clk, uint8_t (&memory)[0xFFFF]);
    // RTI  return from interrupt
    void RTI_IMP(int &clk, uint8_t (&memory)[0xFFFF]);

    // Other

    // BIT  bit test (accumulator & memory)
    void BIT_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void BIT_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    // NOP  no operation
    void NOP_IMP(int &clk, uint8_t (&memory)[0xFFFF]);

    // Illegal opcodes
    void ALR_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void ANC_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void ANE_IM(int &clk, uint8_t (&memory)[0xFFFF]);
    void ARR_IM(int &clk, uint8_t (&memory)[0xFFFF]);

    void DCP_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void DCP_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void ISC_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void ISC_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void LAS_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);

    void LAX_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void LAX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]);
    void LAX_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void LAX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void LAX_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void LAX_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void LXA_IM(int &clk, uint8_t (&memory)[0xFFFF]);

    void RLA_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RLA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void RRA_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void RRA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void SAX_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void SAX_ZPY(int &clk, uint8_t (&memory)[0xFFFF]);
    void SAX_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void SAX_INDX(int &clk, uint8_t (&memory)[0xFFFF]);

    void SBX_IM(int &clk, uint8_t (&memory)[0xFFFF]);

    void SHA_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void SHA_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void SHX_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);

    void SHY_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);

    void SLO_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SLO_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void SRE_ZP(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_ZPX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_ABS(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_ABSX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_INDX(int &clk, uint8_t (&memory)[0xFFFF]);
    void SRE_INDY(int &clk, uint8_t (&memory)[0xFFFF]);

    void TAS_ABSY(int &clk, uint8_t (&memory)[0xFFFF]);

    void USBC_IM(int &clk, uint8_t (&memory)[0xFFFF]);

    void NOP_0B2C(int &clk, uint8_t (&memory)[0xFFFF]);
    void NOP_1B2C(int &clk, uint8_t (&memory)[0xFFFF]);
    void NOP_2B2C(int &clk, uint8_t (&memory)[0xFFFF]);
    void NOP_1B3C(int &clk, uint8_t (&memory)[0xFFFF]);
    void NOP_1B4C(int &clk, uint8_t (&memory)[0xFFFF]);
    void NOP_2B4C(int &clk, uint8_t (&memory)[0xFFFF]); 
    void NOP_2B45C(int &clk, uint8_t (&memory)[0xFFFF]);

    void JAM(int &clk, uint8_t (&memory)[0xFFFF]);

    opcodeDef opcodes[256] = {
        {&MOS6502::LDA_IM, 0xA9, "LDA_IM"},
        {&MOS6502::LDA_ZP, 0xA5, "LDA_ZP"},
        {&MOS6502::LDA_ZPX, 0xB5, "LDA_ZPX"},
        {&MOS6502::LDA_ABS, 0xAD, "LDA_ABS"},
        {&MOS6502::LDA_ABSX, 0xBD, "LDA_ABSX"},
        {&MOS6502::LDA_ABSY, 0xB9, "LDA_ABSY"},
        {&MOS6502::LDA_INDX, 0xA1, "LDA_INDX"},
        {&MOS6502::LDA_INDY, 0xB1, "LDA_INDY"},
        {&MOS6502::LDX_IM, 0xA2, "LDX_IM"},
        {&MOS6502::LDX_ZP, 0xA6, "LDX_ZP"},
        {&MOS6502::LDX_ZPY, 0xB6, "LDX_ZPY"},
        {&MOS6502::LDX_ABS, 0xAE, "LDX_ABS"},
        {&MOS6502::LDX_ABSY, 0xBE, "LDX_ABSY"},
        {&MOS6502::LDY_IM, 0xA0, "LDY_IM"},
        {&MOS6502::LDY_ZP, 0xA4, "LDY_ZP"},
        {&MOS6502::LDY_ZPX, 0xB4, "LDY_ZPX"},
        {&MOS6502::LDY_ABS, 0xAC, "LDY_ABS"},
        {&MOS6502::LDY_ABSX, 0xBC, "LDY_ABSX"},
        {&MOS6502::STA_ZP, 0x85, "STA_ZP"},
        {&MOS6502::STA_ZPX, 0x95, "STA_ZPX"},
        {&MOS6502::STA_ABS, 0x8D, "STA_ABS"},
        {&MOS6502::STA_ABSX, 0x9D, "STA_ABSX"},
        {&MOS6502::STA_ABSY, 0x99, "STA_ABSY"},
        {&MOS6502::STA_INDX, 0x81, "STA_INDX"},
        {&MOS6502::STA_INDY, 0x91, "STA_INDY"},
        {&MOS6502::STX_ZP, 0x86, "STX_ZP"},
        {&MOS6502::STX_ZPY, 0x96, "STX_ZPY"},
        {&MOS6502::STX_ABS, 0x8E, "STX_ABS"},
        {&MOS6502::STY_ZP, 0x84, "STY_ZP"},
        {&MOS6502::STY_ZPX, 0x94, "STY_ZPX"},
        {&MOS6502::STY_ABS, 0x8C, "STY_ABS"},
        {&MOS6502::TAX, 0xAA, "TAX"},
        {&MOS6502::TAY, 0xA8, "TAY"},
        {&MOS6502::TSX, 0xBA, "TSX"},
        {&MOS6502::TXA, 0x8A, "TXA"},
        {&MOS6502::TXS, 0x9A, "TXS"},
        {&MOS6502::TYA, 0x98, "TYA"},
        {&MOS6502::PHA, 0x48, "PHA"},
        {&MOS6502::PHP, 0x08, "PHP"},
        {&MOS6502::PLA, 0x68, "PLA"},
        {&MOS6502::PLP, 0x28, "PLP"},
        {&MOS6502::DEC_ZP, 0xC6, "DEC_ZP"},
        {&MOS6502::DEC_ZPX, 0xD6, "DEC_ZPX"},
        {&MOS6502::DEC_ABS, 0xCE, "DEC_ABS"},
        {&MOS6502::DEC_ABSX, 0xDE, "DEC_ABSX"},
        {&MOS6502::DEX, 0xCA, "DEX"},
        {&MOS6502::DEY, 0x88, "DEY"},
        {&MOS6502::INC_ZP, 0xE6, "INC_ZP"},
        {&MOS6502::INC_ZPX, 0xF6, "INC_ZPX"},
        {&MOS6502::INC_ABS, 0xEE, "INC_ABS"},
        {&MOS6502::INC_ABSX, 0xFE, "INC_ABSX"},
        {&MOS6502::INX, 0xE8, "INX"},
        {&MOS6502::INY, 0xC8, "INY"},
        {&MOS6502::ADC_IM, 0x69, "ADC_IM"},
        {&MOS6502::ADC_ZP, 0x65, "ADC_ZP"},
        {&MOS6502::ADC_ZPX, 0x75, "ADC_ZPX"},
        {&MOS6502::ADC_ABS, 0x6D, "ADC_ABS"},
        {&MOS6502::ADC_ABSX, 0x7D, "ADC_ABSX"},
        {&MOS6502::ADC_ABSY, 0x79, "ADC_ABSY"},
        {&MOS6502::ADC_INDX, 0x61, "ADC_INDX"},
        {&MOS6502::ADC_INDY, 0x71, "ADC_INDY"},
        {&MOS6502::SBC_IM, 0xE9, "SBC_IM"},
        {&MOS6502::SBC_ZP, 0xE5, "SBC_ZP"},
        {&MOS6502::SBC_ZPX, 0xF5, "SBC_ZPX"},
        {&MOS6502::SBC_ABS, 0xED, "SBC_ABS"},
        {&MOS6502::SBC_ABSX, 0xFD, "SBC_ABSX"},
        {&MOS6502::SBC_ABSY, 0xF9, "SBC_ABSY"},
        {&MOS6502::SBC_INDX, 0xE1, "SBC_INDX"},
        {&MOS6502::SBC_INDY, 0xF1, "SBC_INDY"},
        {&MOS6502::AND_IM, 0x29, "AND_IM"},
        {&MOS6502::AND_ZP, 0x25, "AND_ZP"},
        {&MOS6502::AND_ZPX, 0x35, "AND_ZPX"},
        {&MOS6502::AND_ABS, 0x2D, "AND_ABS"},
        {&MOS6502::AND_ABSX, 0x3D, "AND_ABSX"},
        {&MOS6502::AND_ABSY, 0x39, "AND_ABSY"},
        {&MOS6502::AND_INDX, 0x21, "AND_INDX"},
        {&MOS6502::AND_INDY, 0x31, "AND_INDY"},
        {&MOS6502::EOR_IM, 0x49, "EOR_IM"},
        {&MOS6502::EOR_ZP, 0x45, "EOR_ZP"},
        {&MOS6502::EOR_ZPX, 0x55, "EOR_ZPX"},
        {&MOS6502::EOR_ABS, 0x4D, "EOR_ABS"},
        {&MOS6502::EOR_ABSX, 0x5D, "EOR_ABSX"},
        {&MOS6502::EOR_ABSY, 0x59, "EOR_ABSY"},
        {&MOS6502::EOR_INDX, 0x41, "EOR_INDX"},
        {&MOS6502::EOR_INDY, 0x51, "EOR_INDY"},
        {&MOS6502::ORA_IM, 0x09, "ORA_IM"},
        {&MOS6502::ORA_ZP, 0x05, "ORA_ZP"},
        {&MOS6502::ORA_ZPX, 0x15, "ORA_ZPX"},
        {&MOS6502::ORA_ABS, 0x0D, "ORA_ABS"},
        {&MOS6502::ORA_ABSX, 0x1D, "ORA_ABSX"},
        {&MOS6502::ORA_ABSY, 0x19, "ORA_ABSY"},
        {&MOS6502::ORA_INDX, 0x01, "ORA_INDX"},
        {&MOS6502::ORA_INDY, 0x11, "ORA_INDY"},
        {&MOS6502::ASL_ACC, 0x0A, "ASL_ACC"},
        {&MOS6502::ASL_ZP, 0x06, "ASL_ZP"},
        {&MOS6502::ASL_ZPX, 0x16, "ASL_ZPX"},
        {&MOS6502::ASL_ABS, 0x0E, "ASL_ABS"},
        {&MOS6502::ASL_ABSX, 0x1E, "ASL_ABSX"},
        {&MOS6502::LSR_ACC, 0x4A, "LSR_ACC"},
        {&MOS6502::LSR_ZP, 0x46, "LSR_ZP"},
        {&MOS6502::LSR_ZPX, 0x56, "LSR_ZPX"},
        {&MOS6502::LSR_ABS, 0x4E, "LSR_ABS"},
        {&MOS6502::LSR_ABSX, 0x5E, "LSR_ABSX"},
        {&MOS6502::ROL_ACC, 0x2A, "ROL_ACC"},
        {&MOS6502::ROL_ZP, 0x26, "ROL_ZP"},
        {&MOS6502::ROL_ZPX, 0x36, "ROL_ZPX"},
        {&MOS6502::ROL_ABS, 0x2E, "ROL_ABS"},
        {&MOS6502::ROL_ABSX, 0x3E, "ROL_ABSX"},
        {&MOS6502::ROR_ACC, 0x6A, "ROR_ACC"},
        {&MOS6502::ROR_ZP, 0x66, "ROR_ZP"},
        {&MOS6502::ROR_ZPX, 0x76, "ROR_ZPX"},
        {&MOS6502::ROR_ABS, 0x6E, "ROR_ABS"},
        {&MOS6502::ROR_ABSX, 0x7E, "ROR_ABSX"},
        {&MOS6502::CLC, 0x18, "CLC"},
        {&MOS6502::CLD, 0xD8, "CLD"},
        {&MOS6502::CLI, 0x58, "CLI"},
        {&MOS6502::CLV, 0xB8, "CLV"},
        {&MOS6502::SEC, 0x38, "SEC"},
        {&MOS6502::SED, 0xF8, "SED"},
        {&MOS6502::SEI, 0x78, "SEI"},
        {&MOS6502::CMP_IM, 0xC9, "CMP_IM"},
        {&MOS6502::CMP_ZP, 0xC5, "CMP_ZP"},
        {&MOS6502::CMP_ZPX, 0xD5, "CMP_ZPX"},
        {&MOS6502::CMP_ABS, 0xCD, "CMP_ABS"},
        {&MOS6502::CMP_ABSX, 0xDD, "CMP_ABSX"},
        {&MOS6502::CMP_ABSY, 0xD9, "CMP_ABSY"},
        {&MOS6502::CMP_INDX, 0xC1, "CMP_INDX"},
        {&MOS6502::CMP_INDY, 0xD1, "CMP_INDY"},
        {&MOS6502::CPX_IM, 0xE0, "CPX_IM"},
        {&MOS6502::CPX_ZP, 0xE4, "CPX_ZP"},
        {&MOS6502::CPX_ABS, 0xEC, "CPX_ABS"},
        {&MOS6502::CPY_IM, 0xC0, "CPY_IM"},
        {&MOS6502::CPY_ZP, 0xC4, "CPY_ZP"},
        {&MOS6502::CPY_ABS, 0xCC, "CPY_ABS"},
        {&MOS6502::BCC, 0x90, "BCC"},
        {&MOS6502::BCS, 0xB0, "BCS"},
        {&MOS6502::BEQ, 0xF0, "BEQ"},
        {&MOS6502::BMI, 0x30, "BMI"},
        {&MOS6502::BNE, 0xD0, "BNE"},
        {&MOS6502::BPL, 0x10, "BPL"},
        {&MOS6502::BVC, 0x50, "BVC"},
        {&MOS6502::BVS, 0x70, "BVS"},
        {&MOS6502::JMP_ABS, 0x4C, "JMP_ABS"},
        {&MOS6502::JMP_IND, 0x6C, "JMP_IND"},
        {&MOS6502::JSR_ABS, 0x20, "JSR_ABS"},
        {&MOS6502::RTS_IMP, 0x60, "RTS_IMP"},
        {&MOS6502::RTI_IMP, 0x40, "RTI_IMP"},
        {&MOS6502::BIT_ZP, 0x24, "BIT_ZP"},
        {&MOS6502::BIT_ABS, 0x2C, "BIT_ABS"},
        {&MOS6502::NOP_IMP, 0xEA, "NOP_IMP"},
        {&MOS6502::ALR_IM, 0x4B, "ALR_IM"},
        {&MOS6502::ANC_IM, 0x0B, "ANC_IM"},
        {&MOS6502::ANC_IM, 0x2B, "ANC_IM"},
        {&MOS6502::ANE_IM, 0x8B, "ANE_IM"},
        {&MOS6502::ARR_IM, 0x6B, "ARR_IM"},
        {&MOS6502::DCP_ZP, 0xC7, "DCP_ZP"},
        {&MOS6502::DCP_ZPX, 0xD7, "DCP_ZPX"},
        {&MOS6502::DCP_ABS, 0xCF, "DCP_ABS"},
        {&MOS6502::DCP_ABSX, 0xDF, "DCP_ABSX"},
        {&MOS6502::DCP_ABSY, 0xDB, "DCP_ABSY"},
        {&MOS6502::DCP_INDX, 0xC3, "DCP_INDX"},
        {&MOS6502::DCP_INDY, 0xD3, "DCP_INDY"},
        {&MOS6502::ISC_ZP, 0xE7, "ISC_ZP"},
        {&MOS6502::ISC_ZPX, 0xF7, "ISC_ZPX"},
        {&MOS6502::ISC_ABS, 0xEF, "ISC_ABS"},
        {&MOS6502::ISC_ABSX, 0xFF, "ISC_ABSX"},
        {&MOS6502::ISC_ABSY, 0xFB, "ISC_ABSY"},
        {&MOS6502::ISC_INDX, 0xE3, "ISC_INDX"},
        {&MOS6502::ISC_INDY, 0xF3, "ISC_INDY"},
        {&MOS6502::LAS_ABSY, 0xBB, "LAS_ABSY"},
        {&MOS6502::LAX_ZP, 0xA7, "LAX_ZP"},
        {&MOS6502::LAX_ZPY, 0xB7, "LAX_ZPY"},
        {&MOS6502::LAX_ABS, 0xAF, "LAX_ABS"},
        {&MOS6502::LAX_ABSY, 0xBF, "LAX_ABSY"},
        {&MOS6502::LAX_INDX, 0xA3, "LAX_INDX"},
        {&MOS6502::LAX_INDY, 0xB3, "LAX_INDY"},
        {&MOS6502::LXA_IM, 0xAB, "LXA_IM"},
        {&MOS6502::RLA_ZP, 0x27, "RLA_ZP"},
        {&MOS6502::RLA_ZPX, 0x37, "RLA_ZPX"},
        {&MOS6502::RLA_ABS, 0x2F, "RLA_ABS"},
        {&MOS6502::RLA_ABSX, 0x3F, "RLA_ABSX"},
        {&MOS6502::RLA_ABSY, 0x3B, "RLA_ABSY"},
        {&MOS6502::RLA_INDX, 0x23, "RLA_INDX"},
        {&MOS6502::RLA_INDY, 0x33, "RLA_INDY"},
        {&MOS6502::RRA_ZP, 0x67, "RRA_ZP"},
        {&MOS6502::RRA_ZPX, 0x77, "RRA_ZPX"},
        {&MOS6502::RRA_ABS, 0x6F, "RRA_ABS"},
        {&MOS6502::RRA_ABSX, 0x7F, "RRA_ABSX"},
        {&MOS6502::RRA_ABSY, 0x7B, "RRA_ABSY"},
        {&MOS6502::RRA_INDX, 0x63, "RRA_INDX"},
        {&MOS6502::RRA_INDY, 0x73, "RRA_INDY"},
        {&MOS6502::SAX_ZP, 0x87, "SAX_ZP"},
        {&MOS6502::SAX_ZPY, 0x97, "SAX_ZPY"},
        {&MOS6502::SAX_ABS, 0x8F, "SAX_ABS"},
        {&MOS6502::SAX_INDX, 0x83, "SAX_INDX"},
        {&MOS6502::SBX_IM, 0xCB, "SBX_IM"},
        {&MOS6502::SHA_ABSY, 0x9F, "SHA_ABSY"},
        {&MOS6502::SHA_INDY, 0x93, "SHA_INDY"},
        {&MOS6502::SHX_ABSY, 0x9E, "SHX_ABSY"},
        {&MOS6502::SHY_ABSX, 0x9C, "SHY_ABSX"},
        {&MOS6502::SLO_ZP, 0x07, "SLO_ZP"},
        {&MOS6502::SLO_ZPX, 0x17, "SLO_ZPX"},
        {&MOS6502::SLO_ABS, 0x0F, "SLO_ABS"},
        {&MOS6502::SLO_ABSX, 0x1F, "SLO_ABSX"},
        {&MOS6502::SLO_ABSY, 0x1B, "SLO_ABSY"},
        {&MOS6502::SLO_INDX, 0x03, "SLO_INDX"},
        {&MOS6502::SLO_INDY, 0x13, "SLO_INDY"},
        {&MOS6502::SRE_ZP, 0x47, "SRE_ZP"},
        {&MOS6502::SRE_ZPX, 0x57, "SRE_ZPX"},
        {&MOS6502::SRE_ABS, 0x4F, "SRE_ABS"},
        {&MOS6502::SRE_ABSX, 0x5F, "SRE_ABSX"},
        {&MOS6502::SRE_ABSY, 0x5B, "SRE_ABSY"},
        {&MOS6502::SRE_INDX, 0x43, "SRE_INDX"},
        {&MOS6502::SRE_INDY, 0x53, "SRE_INDY"},
        {&MOS6502::TAS_ABSY, 0x9B, "TAS_ABSY"},
        {&MOS6502::USBC_IM, 0xEB, "USBC_IM"},
        {&MOS6502::NOP_0B2C, 0x1A, "NOP_0B2C"},
        {&MOS6502::NOP_0B2C, 0x3A, "NOP_0B2C"},
        {&MOS6502::NOP_0B2C, 0x5A, "NOP_0B2C"},
        {&MOS6502::NOP_0B2C, 0x7A, "NOP_0B2C"},
        {&MOS6502::NOP_0B2C, 0xDA, "NOP_0B2C"},
        {&MOS6502::NOP_0B2C, 0xFA, "NOP_0B2C"},
        {&MOS6502::NOP_1B2C, 0x80, "NOP_1B2C"},
        {&MOS6502::NOP_2B2C, 0x82, "NOP_2B2C"},
        {&MOS6502::NOP_2B2C, 0x89, "NOP_2B2C"},
        {&MOS6502::NOP_2B2C, 0xC2, "NOP_2B2C"},
        {&MOS6502::NOP_2B2C, 0xE2, "NOP_2B2C"},
        {&MOS6502::NOP_1B3C, 0x04, "NOP_1B3C"},
        {&MOS6502::NOP_1B3C, 0x44, "NOP_1B3C"},
        {&MOS6502::NOP_1B3C, 0x64, "NOP_1B3C"},
        {&MOS6502::NOP_1B4C, 0x14, "NOP_1B4C"},
        {&MOS6502::NOP_1B4C, 0x34, "NOP_1B4C"},
        {&MOS6502::NOP_1B4C, 0x54, "NOP_1B4C"},
        {&MOS6502::NOP_1B4C, 0x74, "NOP_1B4C"},
        {&MOS6502::NOP_1B4C, 0xD4, "NOP_1B4C"},
        {&MOS6502::NOP_1B4C, 0xF4, "NOP_1B4C"},
        {&MOS6502::NOP_2B4C, 0x0C, "NOP_2B4C"},
        {&MOS6502::NOP_2B45C, 0x1C, "NOP_2B45C"},
        {&MOS6502::NOP_2B45C, 0x3C, "NOP_2B45C"},
        {&MOS6502::NOP_2B45C, 0x5C, "NOP_2B45C"},
        {&MOS6502::NOP_2B45C, 0x7C, "NOP_2B45C"},
        {&MOS6502::NOP_2B45C, 0xDC, "NOP_2B45C"},
        {&MOS6502::NOP_2B45C, 0xFC, "NOP_2B45C"},
        {&MOS6502::JAM, 0x02, "JAM"},
        {&MOS6502::JAM, 0x12, "JAM"},
        {&MOS6502::JAM, 0x22, "JAM"},
        {&MOS6502::JAM, 0x32, "JAM"},
        {&MOS6502::JAM, 0x42, "JAM"},
        {&MOS6502::JAM, 0x52, "JAM"},
        {&MOS6502::JAM, 0x62, "JAM"},
        {&MOS6502::JAM, 0x72, "JAM"},
        {&MOS6502::JAM, 0x92, "JAM"},
        {&MOS6502::JAM, 0xB2, "JAM"},
        {&MOS6502::JAM, 0xD2, "JAM"},
        {&MOS6502::JAM, 0xF2, "JAM"},
        {&MOS6502::BRK_IMP, 0x00, "BRK_IMP"},
    };
};

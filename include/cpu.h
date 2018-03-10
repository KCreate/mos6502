/*
 * This file is part of the MOS 6502 Emulator
 * (https://github.com/KCreate/mos6502)
 *
 * MIT License
 *
 * Copyright (c) 2017 - 2018 Leonard Schütz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdint>
#include <iostream>  // for std::ostream

#include "bus.h"

#pragma once

namespace M6502 {

// Interrupt vectors
//
// These addresses contain the value that is loaded into the program counter
// when the corresponding interrupt is triggered
//
// maskable external interrupt
static constexpr uint16_t kVecIRQ = 0xFFFE;

// maskable software interrupt
static constexpr uint16_t kVecBRK = 0xFFFE;

// non-maskable hardware interrupt
static constexpr uint16_t kVecNMI = 0xFFFA;

// reset signal, sent at boot or at runtime
static constexpr uint16_t kVecRES = 0xFFFC;

// Masks for the STATUS register
static constexpr uint8_t kMaskSign = 0x80;
static constexpr uint8_t kMaskOverflow = 0x40;
static constexpr uint8_t kMaskConstant = 0x20;
static constexpr uint8_t kMaskBreak = 0x10;
static constexpr uint8_t kMaskDecimal = 0x08;
static constexpr uint8_t kMaskInterrupt = 0x04;
static constexpr uint8_t kMaskZero = 0x02;
static constexpr uint8_t kMaskCarry = 0x01;

// Some constants for stack handling
static constexpr uint16_t kStackBase = 0x0100;
static constexpr uint16_t kStackReset = 0xFF;

// Virtual CPU for the MOS 6502
class CPU {
public:
  CPU(Bus* b);

  // Begin executing
  void start();

  // Execute a single instruction
  void cycle();

  // Dump debugging information to a stream
  void dump_state(std::ostream& out);

  // Reset interrupt
  //
  // TODO: Make this thread safe
  void interrupt_reset();

private:
  // A single CPU instruction
  //
  // Contains function pointers to the address mode
  // and instruction implementation
  typedef void (CPU::*CodeExec)(uint16_t);
  typedef uint16_t (CPU::*AddrExec)();
  struct Instruction {
    AddrExec addr;
    CodeExec code;
  };

  // Dipsatch table indexed by instruction opcode
  //
  // This table is populated at CPU construction
  Instruction dispatch_table[256];

  // Executes a single instruction
  void exec_instruction(Instruction instruction);

  // Read and write single bytes from the bus
  Bus* bus;

  // Accumulator register
  //
  // This is the only register that is able to perform math operations
  uint8_t A;

  // X Index register
  uint8_t X;

  // Y index register
  uint8_t Y;

  // Stack pointer
  uint8_t SP;

  // Program counter
  uint16_t PC;

  // Status register
  union {

    // Struct bits
    //
    // S : Sign flag, Set if the result of an operation is negative
    // O : Overflow flag, Set when an arithmetic operation overflows
    // _ : Unused flag, Should always be set to 1
    // B : Break flag, Set when a software interrupt occurs
    // D : Decimal flag, Toggles decimal mode (0x00 - 0x99 mapped to 0 - 99)
    // I : Interrupt flag, Disables interrupts if set
    // Z : Zero flag, Set if the result of an operation is zero
    // C : Carry flag, Holds the carry out of the most significant bit in any arithmetic
    //     operation. In subtraction  however, this flag is cleared if a borrow is
    //     required and set to  1 if no borrow is required.
    //     Contains the shifted bit on shift operations
    struct {
      bool C : 1;
      bool Z : 1;
      bool I : 1;
      bool D : 1;
      bool B : 1;
      bool _ : 1;
      bool V : 1;
      bool S : 1;
    };
    uint8_t STATUS;
  };

  // Stores wether the last instruction was an illegal one
  // The CPU should just halt when it encounters an illegal
  // instruction
  bool illegal_opcode;

  // Interact with the stack
  void stack_push_byte(uint8_t value);
  void stack_push_word(uint16_t value);
  uint8_t stack_pop_byte();
  uint16_t stack_pop_word();

  // One byte immediate value
  //
  // e.g: LDA #$0x0A
  uint16_t addr_immediate();

  // Two byte address
  //
  // e.g: LDA $31F6
  uint16_t addr_absolute();

  // One byte address
  //
  // e.g: LDA $20
  uint16_t addr_absolute_zero();

  // No data, the operand is implied by the instruction
  // e.g: TAX
  uint16_t addr_implied();

  // Operand is stored inside accumulator
  //
  // e.g: ASL
  //      LSR
  //      ROL
  //      ROR
  uint16_t addr_accumulator();

  // Two byte address which is added to the X
  //
  // e.g: LDA $31F6, X
  uint16_t addr_x_indexed();

  // Two byte address which is added to the Y register
  //
  // e.g: LDA $31F6, Y
  uint16_t addr_y_indexed();

  // One byte address which is added to the X register
  //
  // e.g: LDA $20, X
  uint16_t addr_x_indexed_zero();

  // One byte address which is added to the Y register
  //
  // e.g: STX $20, Y
  uint16_t addr_y_indexed_zero();

  // Two byte address whose bytes are the new location
  // Note: this addressing modes only applies to the JMP instruction
  //
  // e.g: JMP ($215F)
  uint16_t addr_indirect();

  // One byte address which is added to the X register
  // The bytes at the calculated address are the operand
  //
  // e.g: LDA ($3E, X)
  uint16_t addr_pre_indexed_indirect();

  // One byte address whose contents are added to the Y register to form the
  // actual address at which the operand is stored
  //
  // e.g: LDA ($4C), Y
  uint16_t addr_post_indexed_indirect();

  // One byte relative offset
  //
  // e.g: BEQ $55
  uint16_t addr_relative();

  // CPU instructions
  //
  // Documentation was largely copied from: https://nesdev.com/6502.txt
  //
  // The following notation applies to this summary:
  //
  // A       Accumulator                  ^       Logical Exclusive Or
  // X, Y    Index Registers              fromS   Transfer from Stack
  // M       Memory                       toS     Transfer to Stack
  // S       Processor Status Register    ->      Transfer to
  // SP      Stack Pointer                <-      Transfer from
  // /       Change                       |       Logical OR
  // _       No Change                    PC      Program Counter
  // +       Add                          PCH     Program Counter High
  // &       Logical AND                  PCL     Program Counter Low
  // -       Subtract                     OPER    OPERAND
  // #       IMMEDIATE ADDRESSING MODE
  //

  // Handles illegal opcodes
  void op_illegal(uint16_t src);

  // ADC          Add memory to accumulator with carry          ADC
  //
  // Operation: A + M + C -> A, C                       N Z C I D V
  //                                                    / / / _ _ /
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   ADC #Oper           |    69   |    2    |
  // |  Zero Page     |   ADC Oper            |    65   |    2    |
  // |  Zero Page,X   |   ADC Oper,X          |    75   |    2    |
  // |  Absolute      |   ADC Oper            |    60   |    3    |
  // |  Absolute,X    |   ADC Oper,X          |    70   |    3    |
  // |  Absolute,Y    |   ADC Oper,Y          |    79   |    3    |
  // |  (Indirect,X)  |   ADC (Oper,X)        |    61   |    2    |
  // |  (Indirect),Y  |   ADC (Oper),Y        |    71   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_adc(uint16_t src);

  // AND          AND memory with accumulator                   AND
  //
  // Operation: A & M -> A                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   AND #Oper           |    29   |    2    |
  // |  Zero Page     |   AND Oper            |    25   |    2    |
  // |  Zero Page,X   |   AND Oper,X          |    35   |    2    |
  // |  Absolute      |   AND Oper            |    2D   |    3    |
  // |  Absolute,X    |   AND Oper,X          |    3D   |    3    |
  // |  Absolute,Y    |   AND Oper,Y          |    39   |    3    |
  // |  (Indirect,X)  |   AND (Oper,X)        |    21   |    2    |
  // |  (Indirect),Y  |   AND (Oper),Y        |    31   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_and(uint16_t src);

  // ASL          Shift left one bit (memory or accumulator)    ASL
  //
  // Operation: C <- M/A <- 0                           N Z C I D V
  //                                                    / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Accumulator   |   ASL A               |    0A   |    1    |
  // |  Zero Page     |   ASL Oper            |    06   |    2    |
  // |  Zero Page,X   |   ASL Oper,X          |    16   |    2    |
  // |  Absolute      |   ASL Oper            |    0E   |    3    |
  // |  Absolute, X   |   ASL Oper,X          |    1E   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_asl(uint16_t src);
  void op_asl_acc(uint16_t src);

  // BCC          Branch on carry clear                         BCC
  //
  // Operation: Branch on C = 0                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BCC Oper            |    90   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bcc(uint16_t src);

  // BCS          Branch on carry set                           BCS
  //
  // Operation: Branch on C = 1                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BCS Oper            |    B0   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bcs(uint16_t src);

  // BEQ          Branch on result zero                         BEQ
  //
  // Operation: Branch on Z = 1                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BEQ Oper            |    F0   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_beq(uint16_t src);

  // BIT          Test bits in memory with accumulator          BIT
  //
  // Operation: A & M, M7 -> N, M6 -> V                 N Z C I D V
  // If the result of A & M is zero then Z = 1,         / / _ _ _ /
  // otherwise Z = 0
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   BIT Oper            |    24   |    2    |
  // |  Absolute      |   BIT Oper            |    2C   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_bit(uint16_t src);

  // BMI          Branch on result minus                        BMI
  //
  // Operation: Branch on N = 1                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BMI Oper            |    30   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bmi(uint16_t src);

  // BNE          Branch on result not zero                     BNE
  //
  // Operation: Branch on Z = 0                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BMI Oper            |    D0   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bne(uint16_t src);

  // BPL          Branch on result plus                         BPL
  //
  // Operation: Branch on N = 0                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BPL Oper            |    10   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bpl(uint16_t src);

  // BRK          Force break                                   BRK
  //
  // Operation: Forced interrupt, toS(PC + 2), toS(S)   N Z C I D V
  //                                                    _ _ _ 1 _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   BRK                 |    00   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_brk(uint16_t src);

  // BVC          Branch on overflow clear                      BVC
  //
  // Operation: Branch on V = 0                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BVC Oper            |    50   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bvc(uint16_t src);

  // BVS          Branch on overflow set                        BVS
  //
  // Operation: Branch on V = 1                         N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Relative      |   BVS Oper            |    70   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_bvs(uint16_t src);

  // CLC          Clear carry flag                              CLC
  //
  // Operation: 0 -> C                                  N Z C I D V
  //                                                    _ _ 0 _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   CLC                 |    18   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_clc(uint16_t src);

  // CLD          Clear decimal mode                            CLD
  //
  // Operation: 0 -> D                                  N Z C I D V
  //                                                    _ _ _ _ 0 _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   CLD                 |    D8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_cld(uint16_t src);

  // CLI          Clear interrupt disable bit                   CLI
  //
  // Operation: 0 -> I                                  N Z C I D V
  //                                                    _ _ _ 0 _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   CLI                 |    58   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_cli(uint16_t src);

  // CLV          Clear overflow flag                           CLV
  //
  // Operation: 0 -> V                                  N Z C I D V
  //                                                    _ _ _ _ _ 0
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   CLV                 |    B8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_clv(uint16_t src);

  // CMP          Compare memory and accumulator                CMP
  //
  // Operation: A - M                                   N Z C I D V
  //                                                    / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   CMP #Oper           |    C9   |    2    |
  // |  Zero Page     |   CMP Oper            |    C5   |    2    |
  // |  Zero Page,X   |   CMP Oper,X          |    D5   |    2    |
  // |  Absolute      |   CMP Oper            |    CD   |    3    |
  // |  Absolute,X    |   CMP Oper,X          |    DD   |    3    |
  // |  Absolute,Y    |   CMP Oper,Y          |    D9   |    3    |
  // |  (Indirect,X)  |   CMP (Oper,X)        |    C1   |    2    |
  // |  (Indirect),Y  |   CMP (Oper),Y        |    D1   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_cmp(uint16_t src);

  // CPX          Compare memory and index X                    CPX
  //
  // Operation: X - M                                   N Z C I D V
  //                                                    / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   CPX #Oper           |    E0   |    2    |
  // |  Zero Page     |   CPX Oper            |    E4   |    2    |
  // |  Absolute      |   CPX Oper            |    EC   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_cpx(uint16_t src);

  // CPY          Compare memory and index Y                    CPY
  //
  // Operation: Y - M                                   N Z C I D V
  //                                                    / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   CPY #Oper           |    C0   |    2    |
  // |  Zero Page     |   CPY Oper            |    C4   |    2    |
  // |  Absolute      |   CPY Oper            |    CC   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_cpy(uint16_t src);

  // DEC          Decrement memory by one                       DEC
  //
  // Operation: M - 1 -> M                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   DEC Oper            |    C6   |    2    |
  // |  Zero Page,X   |   DEC Oper,X          |    D6   |    2    |
  // |  Absolute      |   DEC Oper            |    CE   |    3    |
  // |  Absolute,X    |   DEC Oper,X          |    DE   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_dec(uint16_t src);

  // DEX         Decrement index X by one                       DEX
  //
  // Operation: X - 1 -> X                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   DEX                 |    CA   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_dex(uint16_t src);

  // DEY          Decrement index Y by one                      DEY
  //
  // Operation: Y - 1 -> Y                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   DEY                 |    88   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_dey(uint16_t src);

  // EOR          XOR memory with accumulator                   XOR
  //
  // Operation: A ^ M -> A                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   EOR #Oper           |    49   |    2    |
  // |  Zero Page     |   EOR Oper            |    45   |    2    |
  // |  Zero Page,X   |   EOR Oper,X          |    55   |    2    |
  // |  Absolute      |   EOR Oper            |    40   |    3    |
  // |  Absolute,X    |   EOR Oper,X          |    50   |    3    |
  // |  Absolute,Y    |   EOR Oper,Y          |    59   |    3    |
  // |  (Indirect,X)  |   EOR (Oper,X)        |    41   |    2    |
  // |  (Indirect),Y  |   EOR (Oper),Y        |    51   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_eor(uint16_t src);

  // INC          Increment memory by one                       INC
  //
  // Operation: M + 1 -> M                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   INC Oper            |    E6   |    2    |
  // |  Zero Page,X   |   INC Oper,X          |    F6   |    2    |
  // |  Absolute      |   INC Oper            |    EE   |    3    |
  // |  Absolute,X    |   INC Oper,X          |    FE   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_inc(uint16_t src);

  // INX          Increment index X by one                      INX
  //
  // Operation: X + 1 -> X                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   INX                 |    E8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_inx(uint16_t src);

  // INY          Increment index Y by one                      INY
  //
  // Operation: Y + 1 -> Y                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   INY                 |    C8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_iny(uint16_t src);

  // JMP          Jump to new location                          JMP
  //
  // Operation: (PC + 1) -> PCL                         N Z C I D V
  //            (PC + 2) -> PCH                         _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Absolute      |   JMP Oper            |    4C   |    3    |
  // |  Indirect      |   JMP (Oper)          |    6C   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_jmp(uint16_t src);

  // JSR          Jump to new location saving return address    JSR
  //
  // Operation: toS(PC + 2), (PC + 1) -> PCL            N Z C I D V
  //                         (PC + 2) -> PCH            _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Absolute      |   JSR Oper            |    20   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_jsr(uint16_t src);

  // LDA          Load memory to accumulator                    LDA
  //
  // Operation: M -> A                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   LDA #Oper           |    A9   |    2    |
  // |  Zero Page     |   LDA Oper            |    A5   |    2    |
  // |  Zero Page,X   |   LDA Oper,X          |    B5   |    2    |
  // |  Absolute      |   LDA Oper            |    AD   |    3    |
  // |  Absolute,X    |   LDA Oper,X          |    BD   |    3    |
  // |  Absolute,Y    |   LDA Oper,Y          |    B9   |    3    |
  // |  (Indirect,X)  |   LDA (Oper,X)        |    A1   |    2    |
  // |  (Indirect),Y  |   LDA (Oper),Y        |    B1   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_lda(uint16_t src);

  // LDX          Load memory to index X                        LDX
  //
  // Operation: M -> X                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   LDX #Oper           |    A2   |    2    |
  // |  Zero Page     |   LDX Oper            |    A6   |    2    |
  // |  Zero Page,Y   |   LDX Oper,Y          |    B6   |    2    |
  // |  Absolute      |   LDX Oper            |    AE   |    3    |
  // |  Absolute,Y    |   LDX Oper,Y          |    BE   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_ldx(uint16_t src);

  // LDY          Load memory to index Y                        LDY
  //
  // Operation: M -> Y                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   LDY #Oper           |    A0   |    2    |
  // |  Zero Page     |   LDY Oper            |    A4   |    2    |
  // |  Zero Page,X   |   LDY Oper,X          |    B4   |    2    |
  // |  Absolute      |   LDY Oper            |    AC   |    3    |
  // |  Absolute,X    |   LDY Oper,X          |    BC   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_ldy(uint16_t src);

  // LSR          Shift right one bit (memory or accumulator)   LSR
  //
  // Operation: 0 -> M/A -> C                           N Z C I D V
  //                                                    0 / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Accumulator   |   LSR A               |    4A   |    1    |
  // |  Zero Page     |   LSR Oper            |    46   |    2    |
  // |  Zero Page,X   |   LSR Oper,X          |    56   |    2    |
  // |  Absolute      |   LSR Oper            |    4E   |    3    |
  // |  Absolute,X    |   LSR Oper,X          |    5E   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_lsr(uint16_t src);
  void op_lsr_acc(uint16_t src);

  // NOP          No operation                                  NOP
  //
  // Operation: No operation                            N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   NOP                 |    EA   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_nop(uint16_t src);

  // ORA          OR memory with accumulator                    ORA
  //
  // Operation: A | M -> A                              N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   ORA #Oper           |    09   |    2    |
  // |  Zero Page     |   ORA Oper            |    05   |    2    |
  // |  Zero Page,X   |   ORA Oper,X          |    15   |    2    |
  // |  Absolute      |   ORA Oper            |    0D   |    3    |
  // |  Absolute,X    |   ORA Oper,X          |    1D   |    3    |
  // |  Absolute,Y    |   ORA Oper,Y          |    19   |    3    |
  // |  (Indirect,X)  |   ORA (Oper,X)        |    01   |    2    |
  // |  (Indirect),Y  |   ORA (Oper),Y        |    11   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_ora(uint16_t src);

  // PHA          Push accumulator on stack                     PHA
  //
  // Operation: toS(A)                                  N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   PHA                 |    48   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_pha(uint16_t src);

  // PHP          Push processor status on stack                PHP
  //
  // Operation: toS(S)                                  N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   PHP                 |    08   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_php(uint16_t src);

  // PLA          Pull accumulator from stack                   PLA
  //
  // Operation: fromS -> A                              N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   PLA                 |    68   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_pla(uint16_t src);

  // PLP          Pull processor status from stack              PLP
  //
  // Operation: fromS -> S                              N Z C I D V
  //                                                    From stack
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   PLP                 |    28   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_plp(uint16_t src);

  // ROL          Rotate one bit left (memory or accumulator)   ROL
  //
  //              +------------------------------+
  //              |         M or A               |
  //              |   +-+-+-+-+-+-+-+-+    +-+   |
  // Operation:   +-< |7|6|5|4|3|2|1|0| <- |C| <-+      N Z C I D V
  //                  +-+-+-+-+-+-+-+-+    +-+          / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Accumulator   |   ROL A               |    2A   |    1    |
  // |  Zero Page     |   ROL Oper            |    26   |    2    |
  // |  Zero Page,X   |   ROL Oper,X          |    36   |    2    |
  // |  Absolute      |   ROL Oper            |    2E   |    3    |
  // |  Absolute,X    |   ROL Oper,X          |    3E   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_rol(uint16_t src);
  void op_rol_acc(uint16_t src);

  // ROR          Rotate one bit right (memory or accumulator)  ROR
  //
  //              +------------------------------+
  //              |                              |
  //              |   +-+    +-+-+-+-+-+-+-+-+   |
  // Operation:   +-> |C| -> |7|6|5|4|3|2|1|0| >-+      N Z C I D V
  //                  +-+    +-+-+-+-+-+-+-+-+          / / / _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Accumulator   |   ROR A               |    6A   |    1    |
  // |  Zero Page     |   ROR Oper            |    66   |    2    |
  // |  Zero Page,X   |   ROR Oper,X          |    76   |    2    |
  // |  Absolute      |   ROR Oper            |    6E   |    3    |
  // |  Absolute,X    |   ROR Oper,X          |    7E   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_ror(uint16_t src);
  void op_ror_acc(uint16_t src);

  // RTI          Return from interrupt                         RTI
  //
  // Operation: fromS(S), fromS(PC)                     N Z C I D V
  //                                                    From stack
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   RTI                 |    4D   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_rti(uint16_t src);

  // RTS          Return from subroutine                        RTS
  //
  // Operation: fromS(PC), PC + 1 -> PC                 N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   RTS                 |    60   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_rts(uint16_t src);

  // SBC          Subtract memory from accumulator with borrow  SBC
  //
  // Operation: A - M - C - > A                         N Z C I D V
  //                                                    / / / _ _ /
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Immediate     |   SBC #Oper           |    E9   |    2    |
  // |  Zero Page     |   SBC Oper            |    E5   |    2    |
  // |  Zero Page,X   |   SBC Oper,X          |    F5   |    2    |
  // |  Absolute      |   SBC Oper            |    ED   |    3    |
  // |  Absolute,X    |   SBC Oper,X          |    FD   |    3    |
  // |  Absolute,Y    |   SBC Oper,Y          |    F9   |    3    |
  // |  (Indirect,X)  |   SBC (Oper,X)        |    E1   |    2    |
  // |  (Indirect),Y  |   SBC (Oper),Y        |    F1   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_sbc(uint16_t src);

  // SEC          Set carry flag                                SEC
  //
  // Operation: 1 -> C                                  N Z C I D V
  //                                                    _ _ 1 _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   SEC                 |    38   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_sec(uint16_t src);

  // SED          Set decimal mode                              SED
  //
  // Operation: 1 -> D                                  N Z C I D V
  //                                                    _ _ _ _ 1 _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   SED                 |    F8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_sed(uint16_t src);

  // SEI          Set interrupt disable status                  SEI
  //
  // Operation: 1 -> I                                  N Z C I D V
  //                                                    _ _ _ 1 _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   SEI                 |    78   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_sei(uint16_t src);

  // STA          Store accumulator in memory                   STA
  //
  // Operation: A -> M                                  N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   STA Oper            |    85   |    2    |
  // |  Zero Page,X   |   STA Oper,X          |    95   |    2    |
  // |  Absolute      |   STA Oper            |    8D   |    3    |
  // |  Absolute,X    |   STA Oper,X          |    9D   |    3    |
  // |  Absolute,Y    |   STA Oper, Y         |    99   |    3    |
  // |  (Indirect,X)  |   STA (Oper,X)        |    81   |    2    |
  // |  (Indirect),Y  |   STA (Oper),Y        |    91   |    2    |
  // +----------------+-----------------------+---------+---------+
  void op_sta(uint16_t src);

  // STX          Store index X in memory                       STX
  //
  // Operation: X -> M                                  N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   STX Oper            |    86   |    2    |
  // |  Zero Page,Y   |   STX Oper,Y          |    96   |    2    |
  // |  Absolute      |   STX Oper            |    8E   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_stx(uint16_t src);

  // STY          Store index Y in memory                       STY
  //
  // Operation: Y -> M                                  N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Zero Page     |   STY Oper            |    84   |    2    |
  // |  Zero Page,X   |   STY Oper,X          |    94   |    2    |
  // |  Absolute      |   STY Oper            |    8C   |    3    |
  // +----------------+-----------------------+---------+---------+
  void op_sty(uint16_t src);

  // TAX          Transfer accumulator to index X               TAX
  //
  // Operation: A -> X                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TAX                 |    AA   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_tax(uint16_t src);

  // TAY          Transfer accumulator to index Y               TAY
  //
  // Operation: A -> Y                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TAY                 |    A8   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_tay(uint16_t src);

  // TSX          Transfer stack pointer to index X             TSX
  //
  // Operation: SP -> X                                 N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TSX                 |    BA   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_tsx(uint16_t src);

  // TXA          Transfer index X to accumulator               TXA
  //
  // Operation: X -> A                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TXA                 |    8A   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_txa(uint16_t src);

  // TXS          Transfer index X to stack pointer             TXS
  //
  // Operation: X -> SP                                 N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TXS                 |    9A   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_txs(uint16_t src);

  // TYA          Transfer index Y to accumulator               TYA
  //
  // Operation: Y -> A                                  N Z C I D V
  //                                                    / / _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   TYA                 |    98   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_tya(uint16_t src);

  // WAI          Wait for an interrupt to happen               WAI
  //
  // Operation: PC + 1 -> PC, Wait for interrupt        N Z C I D V
  //                                                    _ _ _ _ _ _
  //
  // +----------------+-----------------------+---------+---------+
  // | Addressing Mode| Assembly Language Form| OP CODE |No. Bytes|
  // +----------------+-----------------------+---------+---------+
  // |  Implied       |   WAI                 |    02   |    1    |
  // +----------------+-----------------------+---------+---------+
  void op_wai(uint16_t src);
};
}  // namespace M6502

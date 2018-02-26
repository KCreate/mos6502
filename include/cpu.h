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

#include <stdint>

#pragma once

namespace M6502 {

// General purpose bus
template <size_t BSIZE>
class Bus {
  BSIZE value;
};

// Virtual CPU for the MOS 6502
class CPU {

  // Accumulator register
  //
  // This is the only register that is able to perform math  operations
  uint8_t regA;

  // X Index register
  uint8_t regX;

  // Y index register
  uint8_t regX;

  // Stack pointer
  uint8_t regS;

  // Program counter
  uint16_t regPC;

  // Status register
  union {
    struct {

      // Sign flag: This is set if the result of an operation is negative,
      // cleared if positive
      bool s : 1;

      // Overflow flag: Set when an arithmetic operation produces a result too
      // large to be represented in a byte
      bool v : 1;

      // This bit is unused
      bool _ : 1;

      // This is set when a software interrupt  is executed.
      bool b : 1;

      // Decimal mode flag
      //
      // If set, enables the decimal mode (0x00-0x99 are 0-99)
      bool d : 1;

      // Interrupt flag
      //
      // Disables interrupts if the flag is set
      bool i : 1;

      // Zero flag
      //
      // Set if the result of an arithmetic or logical operation is zero.
      bool z : 1;

      // Carry flag
      //
      // Holds the carry out of the most significant bit in any arithmetic
      // operation. In subtraction operations however, this flag is cleared if a
      // borrow is required and set to 1 if no borrow is required.
      //
      // Contains the shifted bit on left and right shift operations
      bool c : 1;
    } status;
    uint8_t regSR;
  };

  // 64 kilobytes of memory
  //
  // The first 256 bytes (0x00-0xff) contain the hardware stack
  uint8_t memory[65536];

  // Memory access
  void mem_write(uint16_t addr, uint8_t value);
  uint8_t mem_read(uint16_t addr);

  // Immediate address mode
  //
  // A single byte after the opcode
  uint16_t addr_imm();

  // CPU instructions
  void exec();
  void op_adc(uint16_t pc);
  void op_and(uint16_t pc);
  void op_asl();
};
} // namespace M6502

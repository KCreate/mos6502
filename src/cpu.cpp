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

#include "cpu.h"

namespace M6502 {

void CPU::cycle() {
  uint8_t opcode;
  Instruction instruction;

  // Run instructions until we encounter an illegal one
  // In that case we just return and let the caller
  // decide what he wants to do
  while (!this->illegal_opcode) {
    opcode = this->bus->read_byte(this->PC++);
    instruction = this->dispatch_table[opcode];
    this->exec_instruction(instruction);
  }
}

void CPU::dump_state(std::ostream& out) {
  out << std::hex;

  out << "6502 Microcontroller at " << this << '\n';
  out << '\n';
  out << "Bus: " << this->bus << '\n';
  out << "Accumulator: " << static_cast<unsigned int>(this->A) << '\n';
  out << "Index X: " << static_cast<unsigned int>(this->X) << '\n';
  out << "Index Y: " << static_cast<unsigned int>(this->Y) << '\n';
  out << "Stack Pointer: " << static_cast<unsigned int>(this->SP) << '\n';
  out << "Program Counter: " << static_cast<unsigned int>(this->PC) << '\n';
  out << "Status Register: " << static_cast<unsigned int>(this->STATUS) << '\n';

  out << std::dec;
}

void CPU::interrupt_reset() {
  this->A = 0x00;
  this->X = 0x00;
  this->Y = 0x00;
  this->PC = this->bus->read_word(kVecRES);
  this->SP = 0xFD;
  this->STATUS = kMaskConstant;
  this->illegal_opcode = false;
}

void CPU::exec_instruction(Instruction) {
  std::cout << "executing an instruction" << '\n';
}
}  // namespace M6502

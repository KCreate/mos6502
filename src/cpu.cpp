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

#define DEFINE_OPCODE(HEXCODE, OPNAME, ADDRMODE) \
  instruction.addr = &CPU::addr_##ADDRMODE;      \
  instruction.code = &CPU::op_##OPNAME;          \
  this->dispatch_table[HEXCODE] = instruction

namespace M6502 {

CPU::CPU(Bus* b) : bus(b) {
  bus->attach_cpu(this);

  Instruction instruction;

  // Prefill dispatch table with illegal opcode handlers
  instruction.addr = &CPU::addr_implied;
  instruction.code = &CPU::op_illegal;
  for (int i = 0; i < 256; i++) {
    this->dispatch_table[i] = instruction;
  }

  // Initialize internal status fields
  this->illegal_opcode = false;
  this->shutdown = false;
  this->int_irq = false;
  this->int_brk = false;
  this->int_nmi = false;
  this->int_res = false;

  // Fill all valid opcodes
  //
  // Table taken from https://nesdev.com/6502.txt

  // 0x00 - 0x1F
  DEFINE_OPCODE(0x00, brk, implied);
  DEFINE_OPCODE(0x01, ora, pre_indexed_indirect);
  DEFINE_OPCODE(0x02, wai, implied);  // custom opcode
  DEFINE_OPCODE(0x05, ora, absolute_zero);
  DEFINE_OPCODE(0x06, asl, absolute_zero);
  DEFINE_OPCODE(0x08, php, implied);
  DEFINE_OPCODE(0x09, ora, immediate);
  DEFINE_OPCODE(0x0A, asl, accumulator);
  DEFINE_OPCODE(0x0D, ora, absolute);
  DEFINE_OPCODE(0x0E, asl, absolute);
  DEFINE_OPCODE(0x10, bpl, immediate);
  DEFINE_OPCODE(0x11, ora, post_indexed_indirect);
  DEFINE_OPCODE(0x15, ora, x_indexed_zero);
  DEFINE_OPCODE(0x16, asl, x_indexed_zero);
  DEFINE_OPCODE(0x18, clc, implied);
  DEFINE_OPCODE(0x19, ora, y_indexed);
  DEFINE_OPCODE(0x1D, ora, x_indexed);
  DEFINE_OPCODE(0x1E, asl, x_indexed);

  // 0x20 - 0x2F
  DEFINE_OPCODE(0x20, jsr, absolute);
  DEFINE_OPCODE(0x21, and, pre_indexed_indirect);
  DEFINE_OPCODE(0x24, bit, absolute_zero);
  DEFINE_OPCODE(0x25, and, absolute_zero);
  DEFINE_OPCODE(0x26, rol, absolute_zero);
  DEFINE_OPCODE(0x28, plp, implied);
  DEFINE_OPCODE(0x29, and, immediate);
  DEFINE_OPCODE(0x2A, rol, accumulator);
  DEFINE_OPCODE(0x2C, bit, absolute);
  DEFINE_OPCODE(0x2D, and, absolute);
  DEFINE_OPCODE(0x2E, rol, absolute);
  DEFINE_OPCODE(0x30, bmi, immediate);
  DEFINE_OPCODE(0x31, and, post_indexed_indirect);
  DEFINE_OPCODE(0x35, and, x_indexed_zero);
  DEFINE_OPCODE(0x36, rol, x_indexed_zero);
  DEFINE_OPCODE(0x38, sec, implied);
  DEFINE_OPCODE(0x39, and, y_indexed);
  DEFINE_OPCODE(0x3D, and, x_indexed);
  DEFINE_OPCODE(0x3E, rol, x_indexed);

  // 0x40 - 5F
  DEFINE_OPCODE(0x40, rti, implied);
  DEFINE_OPCODE(0x41, eor, pre_indexed_indirect);
  DEFINE_OPCODE(0x45, eor, absolute_zero);
  DEFINE_OPCODE(0x46, lsr, absolute_zero);
  DEFINE_OPCODE(0x48, pha, implied);
  DEFINE_OPCODE(0x49, eor, immediate);
  DEFINE_OPCODE(0x4A, lsr, accumulator);
  DEFINE_OPCODE(0x4C, jmp, absolute);
  DEFINE_OPCODE(0x4D, eor, absolute);
  DEFINE_OPCODE(0x4E, lsr, absolute);
  DEFINE_OPCODE(0x50, bvc, immediate);
  DEFINE_OPCODE(0x51, eor, post_indexed_indirect);
  DEFINE_OPCODE(0x55, eor, x_indexed_zero);
  DEFINE_OPCODE(0x56, lsr, x_indexed_zero);
  DEFINE_OPCODE(0x58, cli, implied);
  DEFINE_OPCODE(0x59, eor, y_indexed);
  DEFINE_OPCODE(0x50, eor, x_indexed);
  DEFINE_OPCODE(0x5E, lsr, x_indexed);

  // 0x60 - 7F
  DEFINE_OPCODE(0x60, rts, implied);
  DEFINE_OPCODE(0x61, adc, pre_indexed_indirect);
  DEFINE_OPCODE(0x65, adc, absolute_zero);
  DEFINE_OPCODE(0x66, ror, absolute_zero);
  DEFINE_OPCODE(0x68, pla, implied);
  DEFINE_OPCODE(0x69, adc, immediate);
  DEFINE_OPCODE(0x6A, ror, accumulator);
  DEFINE_OPCODE(0x6C, jmp, indirect);
  DEFINE_OPCODE(0x6D, adc, absolute);
  DEFINE_OPCODE(0x6E, ror, absolute);
  DEFINE_OPCODE(0x70, bvs, immediate);
  DEFINE_OPCODE(0x71, adc, post_indexed_indirect);
  DEFINE_OPCODE(0x75, adc, x_indexed_zero);
  DEFINE_OPCODE(0x76, ror, x_indexed_zero);
  DEFINE_OPCODE(0x78, sei, implied);
  DEFINE_OPCODE(0x79, adc, y_indexed);
  DEFINE_OPCODE(0x7D, adc, x_indexed);
  DEFINE_OPCODE(0x7E, ror, x_indexed);

  // 0x80 - 0x9F
  DEFINE_OPCODE(0x81, sta, pre_indexed_indirect);
  DEFINE_OPCODE(0x84, sty, absolute_zero);
  DEFINE_OPCODE(0x85, sta, absolute_zero);
  DEFINE_OPCODE(0x86, stx, absolute_zero);
  DEFINE_OPCODE(0x88, dey, implied);
  DEFINE_OPCODE(0x8A, txa, implied);
  DEFINE_OPCODE(0x8C, sty, absolute);
  DEFINE_OPCODE(0x8D, sta, absolute);
  DEFINE_OPCODE(0x8E, stx, absolute);
  DEFINE_OPCODE(0x90, bcc, immediate);
  DEFINE_OPCODE(0x91, sta, post_indexed_indirect);
  DEFINE_OPCODE(0x94, sty, x_indexed_zero);
  DEFINE_OPCODE(0x95, sta, x_indexed_zero);
  DEFINE_OPCODE(0x96, stx, y_indexed_zero);
  DEFINE_OPCODE(0x98, tya, implied);
  DEFINE_OPCODE(0x99, sta, y_indexed);
  DEFINE_OPCODE(0x9A, txs, implied);
  DEFINE_OPCODE(0x9D, sta, x_indexed);

  // 0xA0 - 0xBF
  DEFINE_OPCODE(0xA0, ldy, immediate);
  DEFINE_OPCODE(0xA1, lda, pre_indexed_indirect);
  DEFINE_OPCODE(0xA2, ldx, immediate);
  DEFINE_OPCODE(0xA4, ldy, absolute_zero);
  DEFINE_OPCODE(0xA5, lda, absolute_zero);
  DEFINE_OPCODE(0xA6, ldx, absolute_zero);
  DEFINE_OPCODE(0xA8, tay, implied);
  DEFINE_OPCODE(0xA9, lda, immediate);
  DEFINE_OPCODE(0xAA, tax, implied);
  DEFINE_OPCODE(0xAC, ldy, absolute);
  DEFINE_OPCODE(0xAD, lda, absolute);
  DEFINE_OPCODE(0xAE, ldx, absolute);
  DEFINE_OPCODE(0xB0, bcs, immediate);
  DEFINE_OPCODE(0xB1, lda, post_indexed_indirect);
  DEFINE_OPCODE(0xB4, ldy, x_indexed_zero);
  DEFINE_OPCODE(0xB5, lda, x_indexed_zero);
  DEFINE_OPCODE(0xB6, ldx, x_indexed_zero);
  DEFINE_OPCODE(0xB8, clv, implied);
  DEFINE_OPCODE(0xB9, lda, y_indexed);
  DEFINE_OPCODE(0xBA, tsx, implied);
  DEFINE_OPCODE(0xBC, ldy, x_indexed);
  DEFINE_OPCODE(0xBD, lda, x_indexed);
  DEFINE_OPCODE(0xBE, ldx, y_indexed);

  // 0xC0 - 0xDF
  DEFINE_OPCODE(0xC0, cpy, immediate);
  DEFINE_OPCODE(0xC1, cmp, pre_indexed_indirect);
  DEFINE_OPCODE(0xC4, cpy, absolute_zero);
  DEFINE_OPCODE(0xC5, cmp, absolute_zero);
  DEFINE_OPCODE(0xC6, dec, absolute_zero);
  DEFINE_OPCODE(0xC8, iny, implied);
  DEFINE_OPCODE(0xC9, cmp, immediate);
  DEFINE_OPCODE(0xCA, dex, implied);
  DEFINE_OPCODE(0xCC, cpy, absolute);
  DEFINE_OPCODE(0xCD, cmp, absolute);
  DEFINE_OPCODE(0xCE, dec, absolute);
  DEFINE_OPCODE(0xD0, bne, immediate);
  DEFINE_OPCODE(0xD1, cmp, post_indexed_indirect);
  DEFINE_OPCODE(0xD5, cmp, x_indexed_zero);
  DEFINE_OPCODE(0xD6, dec, x_indexed_zero);
  DEFINE_OPCODE(0xD8, cld, implied);
  DEFINE_OPCODE(0xD9, cmp, y_indexed);
  DEFINE_OPCODE(0xDD, cmp, x_indexed);
  DEFINE_OPCODE(0xDE, dec, x_indexed);

  // 0xE0 - 0xFF
  DEFINE_OPCODE(0xE0, cpx, immediate);
  DEFINE_OPCODE(0xE1, sbc, pre_indexed_indirect);
  DEFINE_OPCODE(0xE4, cpx, absolute_zero);
  DEFINE_OPCODE(0xE5, sbc, absolute_zero);
  DEFINE_OPCODE(0xE6, inc, absolute_zero);
  DEFINE_OPCODE(0xE8, inx, implied);
  DEFINE_OPCODE(0xE9, sbc, immediate);
  DEFINE_OPCODE(0xEA, nop, implied);
  DEFINE_OPCODE(0xEC, cpx, absolute);
  DEFINE_OPCODE(0xED, sbc, absolute);
  DEFINE_OPCODE(0xEE, inc, absolute);
  DEFINE_OPCODE(0xF0, beq, immediate);
  DEFINE_OPCODE(0xF1, sbc, post_indexed_indirect);
  DEFINE_OPCODE(0xF5, sbc, x_indexed_zero);
  DEFINE_OPCODE(0xF6, inc, x_indexed_zero);
  DEFINE_OPCODE(0xF8, sed, implied);
  DEFINE_OPCODE(0xF9, sbc, y_indexed);
  DEFINE_OPCODE(0xFD, sbc, x_indexed);
  DEFINE_OPCODE(0xFE, inc, x_indexed);

  this->handle_res();
}

void CPU::start() {
  // Run instructions until we encounter an illegal one
  // In that case we just return and let the caller
  // decide what he wants to do
  while (!this->shutdown && !this->illegal_opcode) {
    this->cycle();
  }
}

void CPU::cycle() {

  // Check if there was an interrupt
  if (!this->I) {
    if (this->int_irq) this->handle_irq();
    if (this->int_brk) this->handle_brk();
  }
  if (this->int_nmi) this->handle_nmi();
  if (this->int_res) this->handle_res();

  uint8_t opcode = this->bus->read_byte(this->PC++);
  Instruction instruction = this->dispatch_table[opcode];
  this->exec_instruction(instruction);
}

void CPU::handle_irq() {
  this->int_irq = false;
  this->stack_push_word(this->PC);
  this->stack_push_byte(this->STATUS);
  this->I = true;
  this->PC = this->bus->read_word(kVecIRQ);
}

void CPU::handle_brk() {
  this->int_brk = false;
  this->stack_push_word(this->PC);
  this->stack_push_byte(this->STATUS | kMaskBreak);
  this->I = true;
  this->PC = this->bus->read_word(kVecBRK);
}

void CPU::handle_nmi() {
  this->int_nmi = false;
  this->stack_push_word(this->PC);
  this->stack_push_byte(this->STATUS);
  this->I = true;
  this->PC = this->bus->read_word(kVecNMI);
}

void CPU::handle_res() {
  this->int_res = false;
  this->A = 0x00;
  this->X = 0x00;
  this->Y = 0x00;
  this->PC = this->bus->read_word(kVecRES);
  this->SP = kStackReset;
  this->STATUS = kMaskConstant;
  this->illegal_opcode = false;
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

void CPU::exec_instruction(Instruction instruction) {
  uint16_t src = (this->*instruction.addr)();
  (this->*instruction.code)(src);
}

uint16_t CPU::addr_immediate() {
  return this->PC++;
}

uint16_t CPU::addr_absolute() {
  uint16_t addr = this->PC;
  this->PC += 2;
  return this->bus->read_word(addr);
}

uint16_t CPU::addr_absolute_zero() {
  return this->bus->read_byte(this->PC++);
}

uint16_t CPU::addr_implied() {
  return 0;
}

uint16_t CPU::addr_accumulator() {
  return this->A;
}

uint16_t CPU::addr_x_indexed() {
  uint16_t addr = this->bus->read_word(this->PC);
  this->PC += 2;
  return addr + this->X;
}

uint16_t CPU::addr_y_indexed() {
  uint16_t addr = this->bus->read_word(this->PC);
  this->PC += 2;
  return addr + this->Y;
}

uint16_t CPU::addr_x_indexed_zero() {
  uint8_t addr = this->bus->read_byte(this->PC++);
  return addr + this->X;
}

uint16_t CPU::addr_y_indexed_zero() {
  uint8_t addr = this->bus->read_byte(this->PC++);
  return addr + this->Y;
}

uint16_t CPU::addr_indirect() {
  uint16_t addr = this->bus->read_word(this->PC++);
  return this->bus->read_word(addr);
}

uint16_t CPU::addr_pre_indexed_indirect() {
  uint8_t addr = this->bus->read_byte(this->PC++);

  // When adding the 1-byte address and the X-register, wrap around
  // addition is used - i.e. the sum is always a zero-page address.
  // e.g: FF + 2 = 0001 not 0101 as you might expect
  return this->bus->read_word((addr + this->X) & 0xFF);
}

uint16_t CPU::addr_post_indexed_indirect() {
  uint8_t addr = this->bus->read_byte(this->PC++);
  return this->bus->read_byte(addr) + this->Y;
}

void CPU::stack_push_byte(uint8_t value) {
  this->bus->write_byte(kStackBase + this->SP, value);
  if (this->SP == 0x00) {
    this->SP = kStackReset;
  } else {
    this->SP--;
  }
}

void CPU::stack_push_word(uint16_t value) {
  this->stack_push_byte((value >> 8) & 0xFF);
  this->stack_push_byte(value & 0xFF);
}

uint8_t CPU::stack_pop_byte() {
  if (this->SP == 0xFF) {
    this->SP = 0x00;
  } else {
    this->SP++;
  }
  return this->bus->read_byte(kStackBase + this->SP);
}

uint16_t CPU::stack_pop_word() {
  uint8_t lo = this->stack_pop_byte();
  uint8_t hi = this->stack_pop_byte();
  uint16_t result = lo | (hi << 8);
  return result;
}

void CPU::op_illegal(uint16_t) {
  this->illegal_opcode = true;
}

void CPU::op_adc(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  uint16_t tmp = operand + this->A + (this->C ? 1 : 0);
  this->Z = !(tmp & 0xFF);

  if (this->D) {
    // TODO: Implement and understand decimal addition
  } else {
    this->S = tmp & 0x80;
    this->V = !((this->A ^ operand) & 0x80) && ((this->A ^ tmp) & 0x80);
    this->C = tmp > 0xFF;
  }

  this->A = tmp & 0xFF;
}

void CPU::op_and(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  uint8_t res = operand & this->A;
  this->S = res & 0x80;
  this->Z = !res;
  this->A = res;
}

void CPU::op_asl(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  this->C = operand & 0x80;
  operand <<= 1;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->bus->write_byte(src, operand);
}

void CPU::op_asl_acc(uint16_t) {
  uint8_t operand = this->A;
  this->C = operand & 0x80;
  operand <<= 1;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_bcc(uint16_t src) {
  if (!this->C) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_bcs(uint16_t src) {
  if (this->C) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_beq(uint16_t src) {
  if (this->Z) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_bit(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  uint8_t res = operand & this->A;
  this->S = res & 0x80;
  this->V = res & 0x40;
  this->Z = !res;
}

void CPU::op_bmi(uint16_t src) {
  if (this->S) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_bne(uint16_t src) {
  if (!this->Z) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_bpl(uint16_t src) {
  if (!this->S) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_brk(uint16_t) {
  this->PC++;
  this->handle_brk();
}

void CPU::op_bvc(uint16_t src) {
  if (!this->V) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_bvs(uint16_t src) {
  if (this->V) {
    this->PC += this->bus->read_byte(src);
  }
}

void CPU::op_clc(uint16_t) {
  this->C = false;
}

void CPU::op_cld(uint16_t) {
  this->D = false;
}

void CPU::op_cli(uint16_t) {
  this->I = false;
}

void CPU::op_clv(uint16_t) {
  this->V = false;
}

void CPU::op_cmp(uint16_t src) {
  uint16_t result = this->A - this->bus->read_byte(src);
  this->C = result < 0x100;
  this->S = result & 0x80;
  this->Z = !(result & 0xFF);
}

void CPU::op_cpx(uint16_t src) {
  uint16_t result = this->X - this->bus->read_byte(src);
  this->C = result < 0x100;
  this->S = result & 0x80;
  this->Z = !(result & 0xFF);
}

void CPU::op_cpy(uint16_t src) {
  uint16_t result = this->Y - this->bus->read_byte(src);
  this->C = result < 0x100;
  this->S = result & 0x80;
  this->Z = !(result & 0xFF);
}

void CPU::op_dec(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  operand = (operand - 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->bus->write_byte(src, operand);
}

void CPU::op_dex(uint16_t) {
  uint8_t operand = this->X;
  operand = (operand - 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_dey(uint16_t) {
  uint8_t operand = this->Y;
  operand = (operand - 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->Y = operand;
}

void CPU::op_eor(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  operand = this->A ^ operand;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_inc(uint16_t) {
  uint8_t operand = this->X;
  operand = (operand + 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_inx(uint16_t) {
  uint8_t operand = this->X;
  operand = (operand + 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_iny(uint16_t) {
  uint8_t operand = this->X;
  operand = (operand + 1) % 256;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_jmp(uint16_t src) {
  this->PC = src;
}

void CPU::op_jsr(uint16_t src) {
  this->stack_push_word(this->PC - 1);
  this->PC = src;
}

void CPU::op_lda(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_ldx(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_ldy(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  this->S = operand & 0x80;
  this->Z = !operand;
  this->Y = operand;
}

void CPU::op_lsr(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  this->C = operand & 0x1;
  operand >>= 1;
  this->S = false;
  this->Z = !operand;
  this->bus->write_byte(src, operand);
}

void CPU::op_lsr_acc(uint16_t) {
  uint8_t operand = this->A;
  this->C = operand & 0x1;
  operand >>= 1;
  this->S = false;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_nop(uint16_t) {
  // do nothing
}

void CPU::op_ora(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  operand = this->A | operand;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_pha(uint16_t) {
  this->stack_push_byte(this->A);
}

void CPU::op_php(uint16_t) {
  this->stack_push_byte(this->STATUS | kMaskBreak);
}

void CPU::op_pla(uint16_t) {
  this->A = this->stack_pop_byte();
  this->S = this->A & 0x80;
  this->Z = !this->A;
}

void CPU::op_plp(uint16_t) {
  this->STATUS = this->stack_pop_byte();
  this->_ = true;
  this->B = false;
}

void CPU::op_rol(uint16_t src) {
  uint16_t operand = this->bus->read_byte(src);
  operand <<= 1;
  if (this->C) {
    operand |= 0x01;
  }
  this->C = operand > 0xFF;
  operand &= 0xFF;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->bus->write_byte(src, operand);
}

void CPU::op_rol_acc(uint16_t) {
  uint16_t operand = this->A;
  operand <<= 1;
  if (this->C) {
    operand |= 0x01;
  }
  this->C = operand > 0xFF;
  operand &= 0xFF;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_ror(uint16_t src) {
  uint16_t operand = this->bus->read_byte(src);
  if (this->C) {
    operand |= 0x100;
  }
  this->C = operand & 0x01;
  operand >>= 1;
  operand &= 0xFF;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->bus->write_byte(src, operand);
}

void CPU::op_ror_acc(uint16_t) {
  uint16_t operand = this->A;
  if (this->C) {
    operand |= 0x100;
  }
  this->C = operand & 0x01;
  operand >>= 1;
  operand &= 0xFF;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_rti(uint16_t) {
  this->STATUS = this->stack_pop_byte() | kMaskConstant;
  this->PC = this->stack_pop_word();
}

void CPU::op_rts(uint16_t) {
  this->PC = this->stack_pop_word() + 1;
}

void CPU::op_sbc(uint16_t src) {
  uint8_t operand = this->bus->read_byte(src);
  uint16_t tmp = this->A - operand - (this->C ? 0 : 1);
  this->S = tmp & 0x80;
  this->Z = !(tmp & 0xFF);
  this->V = ((this->A ^ tmp) & 0x80) && ((this->A ^ operand) & 0x80);

  if (this->D) {
    // TODO: Implement and understand decimal mode
  }

  this->C = tmp < 0x100;
  this->A = tmp & 0xFF;
}

void CPU::op_sec(uint16_t) {
  this->C = true;
}

void CPU::op_sed(uint16_t) {
  this->D = true;
}

void CPU::op_sei(uint16_t) {
  this->I = true;
}

void CPU::op_sta(uint16_t src) {
  this->bus->write_byte(src, this->A);
}

void CPU::op_stx(uint16_t src) {
  this->bus->write_byte(src, this->X);
}

void CPU::op_sty(uint16_t src) {
  this->bus->write_byte(src, this->Y);
}

void CPU::op_tax(uint16_t) {
  uint8_t operand = this->A;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_tay(uint16_t) {
  uint8_t operand = this->A;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->Y = operand;
}

void CPU::op_tsx(uint16_t) {
  uint8_t operand = this->SP;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->X = operand;
}

void CPU::op_txa(uint16_t) {
  uint8_t operand = this->X;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_txs(uint16_t) {
  this->SP = this->X;
}

void CPU::op_tya(uint16_t) {
  uint8_t operand = this->Y;
  this->S = operand & 0x80;
  this->Z = !operand;
  this->A = operand;
}

void CPU::op_wai(uint16_t) {
  this->PC++;

  // TODO: Implement this
}

}  // namespace M6502

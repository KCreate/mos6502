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

#include <chrono>
#include <iostream>
#include <thread>
#include <cstring>

#include "bus.h"
#include "cpu.h"
#include "iochip.h"
#include "rammodule.h"
#include "rommodule.h"

using namespace M6502;

int main() {
  using namespace std::chrono_literals;

  // Create the machine parts
  RAMModule<kSizeRAM> ram(kAddrRAM);
  IOChip io(kAddrIO);
  ROMModule<kSizeROM> rom(kAddrROM);

  // Create the bus and attach all the devices
  Bus bus;
  bus.attach_ram(&ram);
  bus.attach_io(&io);
  bus.attach_rom(&rom);

  // Set the reset vector
  rom.get_buffer()[kVecRES - kAddrROM] = 0x10;
  rom.get_buffer()[kVecRES - kAddrROM + 1] = 0x49;

  // Flash some code into the ROM for the CPU to execute
  //
  // Example used: vram_simple_copy.asm
  uint8_t code[] = {
                              //      .RST
    0xA9, 0x0A,               // 4910:  LDA #$0A      ; 1000 milliseconds
    0x8D, 0x06, 0x49,         // 4912:  STA 0x4906    ; ADDR_CLOCK1
                              //
    0xA9, 0xE0,               // 4915:  LDA #$E0      ; Set the brush color to red
    0x8D, 0x0C, 0x49,         // 4917:  STA 0x490C    ; ADDR_DRAW_ARG1
    0xA9, 0x03,               // 491A:  LDA #$03      ; BRUSH_SET_BODY
    0x8D, 0x0B, 0x49,         // 491C:  STA 0x490B    ; ADDR_DRAW_METHOD
                              //
    0xA9, 0x03,               // 491F:  LDA #$03      ; Set the outline color to blue
    0x8D, 0x0C, 0x49,         // 4921:  STA 0x490C    ; ADDR_DRAW_ARG1
    0xA9, 0x04,               // 4924:  LDA #$04      ; BRUSH_SET_OUTLINE
    0x8D, 0x0B, 0x49,         // 4926:  STA 0x490B    ; ADDR_DRAW_METHOD
                              //
                              //      .DRAW
    0x8E, 0x0C, 0x49,         // 4929:  STX 0x490C    ; x coordinate -> ADDR_DRAW_ARG1
    0xA9, 0x08,               // 492C:  LDA #$08      ; y coordinate -> ADDR_DRAW_ARG2
    0x8D, 0x0D, 0x49,         // 492E:  STA 0x490D
    0xA9, 0x10,               // 4931:  LDA #$10      ; width -> ADDR_DRAW_ARG3
    0x8D, 0x0E, 0x49,         // 4933:  STA 0x490E
    0xA9, 0x14,               // 4936:  LDA #$14      ; height -> ADDR_DRAW_ARG4
    0x8D, 0x0F, 0x49,         // 4938:  STA 0x490F
    0xA9, 0x00,               // 493B:  LDA #$00      ; DRAW_RECTANGLE -> ADDR_DRAW_METHOD
    0x8D, 0x0B, 0x49,         // 493D:  STA 0x490B
    0x4C, 0x29, 0x49,         // 4940:  JMP .DRAW
                              //
                              //      .IRQ
    0xE8,                     // 4943:  INX
    0x40,                     // 4944:  RTI
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  // Hook up IRQ interrupt handler
  rom.get_buffer()[kVecIRQ - kAddrROM] = 0x43;
  rom.get_buffer()[kVecIRQ - kAddrROM + 1] = 0x49;

  CPU cpu(&bus);
  bus.attach_cpu(&cpu);

  std::thread cpu_thread([&]() {
    cpu.dump_state(std::cout);
    cpu.start();
    std::cout << "cpu halted" << std::endl;
  });

  io.start();
  io.stop();
  cpu_thread.join();

  return 0;
}

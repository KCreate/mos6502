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
      // .def ADDR_DRAW_METHOD 0x490B
      // .def ADDR_DRAW_ARG1 0x490C
      // .def ADDR_DRAW_ARG2 0x490D
      // .def ADDR_DRAW_ARG3 0x490E
      // .def ADDR_DRAW_ARG4 0x490F
      // .def DRAW_RECTANGLE 0x00
      // .def BRUSH_SET_BODY 0x03
      // .def BRUSH_SET_OUTLINE 0x04
      // .def RED #$0xE0
      // .def GREEN #$0x1C
      // .RST
      //
      // ; set the brush color to red
      0xA9, 0xE0,        // 4910:  LDA RED
      0x8D, 0x0C, 0x49,  // 4912:  STA ADDR_DRAW_ARG1
      0xA9, 0x03,        // 4915:  LDA BRUSH_SET_BODY
      0x8D, 0x0B, 0x49,  // 4917:  STA ADDR_DRAW_METHOD
      0xA9, 0x1C,        // 491A:  LDA GREEN
      0x8D, 0x0C, 0x49,  // 491C:  STA ADDR_DRAW_ARG1
      0xA9, 0x04,        // 491F:  LDA BRUSH_SET_OUTLINE
      0x8D, 0x0B, 0x49,  // 4921:  STA ADDR_DRAW_METHOD
                         //
                         // ; initialize X axis counter
                         //
                         // .XRESET
      0xA2, 0x00,        // 4924:  LDX #$00
                         //
                         // ; draw the rectangle
      0x8E, 0x0C, 0x49,  // 4926:  STX ADDR_DRAW_ARG1
      0xA9, 0x08,        // 4929:  LDA #$08
      0x8D, 0x0D, 0x49,  // 492B:  STA ADDR_DRAW_ARG2
      0xA9, 0x10,        // 492E:  LDA #$10
      0x8D, 0x0E, 0x49,  // 4930:  STA ADDR_DRAW_ARG3
      0xA9, 0x14,        // 4933:  LDA #$14
      0x8D, 0x0F, 0x49,  // 4935:  STA ADDR_DRAW_ARG4
      0xA9, 0x00,        // 4938:  LDA DRAW_RECTANGLE
      0x8D, 0x0B, 0x49,  // 493A:  STA ADDR_DRAW_METHOD
      0xE8,              // 493D:  INX
                         // 493E:  CPX #$2F
                         // 4940:  BCS .XRESET
      0x4C, 0x26, 0x49   //   JMP .RST
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  CPU cpu(&bus);

  std::thread cpu_thread([&cpu, &io]() {
    cpu.start();
    io.stop();
  });

  io.start();
  cpu_thread.join();

  return 0;
}

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
                                // 4010:  .RST
    0xA9, 0xE0,                 //   lda #$E0
    0x8D, 0x0C, 0x49,           //   sta 0x490C
    0xA9, 0x03,                 //   lda #$03
    0x8D, 0x0B, 0x49,           //   sta 0x490B
    0xA9, 0x03,                 //   lda #$03
    0x8D, 0x0C, 0x49,           //   sta 0x490C
    0xA9, 0x04,                 //   lda 0x04
    0x8D, 0x0B, 0x49,           //   sta 0x490B
                                //
                                // 4924:  .DRAWBEGIN
    0xA9, 0x10,                 //   lda #$10
    0x8D, 0x0E, 0x49,           //   sta 0x490E
    0x8D, 0x0F, 0x49,           //   sta 0x490F
    0xA9, 0x00,                 //   lda #$00
                                // 492E:  .DRAWLOOP
    0x8E, 0x0C, 0x49,           //   stx 0x490C
    0x8C, 0x0D, 0x49,           //   sty 0x490D
    0x8D, 0x0B, 0x49,           //   sta 0x490B
    0x4C, 0x2E, 0x49,           //   jmp .DRAWLOOP
                                //
                                // 493A:  .IRQ
    0x48,                       //   pha
    0xAD, 0x03, 0x49,           //   lda 0x4903
    0xC9, 0x01,                 //   cmp #$01
    0xD0, 0x0B,                 //   bne .RESTORE_AND_EXIT
    0xAD, 0x04, 0x49,           //   lda 0x4904
    0xC9, 0x16,                 //   cmp #$16
    0xF0, 0x0E,                 //   beq .HANDLE_W_KEY
    0xC9, 0x00,                 //   cmp #$00
    0xF0, 0x0D,                 //   beq .HANDLE_A_KEY
    0xC9, 0x12,                 //   cmp #$12
    0xF0, 0x0C,                 //   beq .HANDLE_S_KEY
    0xC9, 0x03,                 //   cmp #$03
    0xF0, 0x0B,                 //   beq .HANDLE_D_KEY
                                //
                                // 4955:  .RESTORE_AND_EXIT
    0x68,                       //   pla
    0x40,                       //   rti
                                //
                                // 4957:  .HANDLE_W_KEY
    0x68,                       //   pla
    0x88,                       //   dey
    0x40,                       //   rti
                                //
                                // 495A:  .HANDLE_A_KEY
    0x68,                       //   pla
    0xCA,                       //   dex
    0x40,                       //   rti
                                //
                                // 495D:  .HANDLE_S_KEY
    0x68,                       //   pla
    0xC8,                       //   iny
    0x40,                       //   rti
                                //
                                // 4960:  .HANDLE_D_KEY
    0x68,                       //   pla
    0xE8,                       //   inx
    0x40                        //   rti
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  // Hook up IRQ interrupt handler
  rom.get_buffer()[kVecIRQ - kAddrROM] = 0x3A;
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

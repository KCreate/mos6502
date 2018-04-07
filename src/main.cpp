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
                          // 4910:  .RST
    0xA9, 0xC8,           //    lda #$C8
    0x8D, 0x08, 0x49,     //    sta 0x4908
    0xA9, 0x32,           //    lda #$C8
    0x8D, 0x06, 0x49,     //    sta 0x4906
                          //
                          // 491A:  .LOOP
    0xEA,                 //    nop
    0x4C, 0x1A, 0x49,     //    jmp .LOOP
                          //
                          // 491E:  .IRQ
    0xAD, 0x08, 0x49,     //    lda 0x4908
    0xAA,                 //    tax
    0x29, 0xCF,           //    and #$CF
    0x8D, 0x00, 0x00,     //    sta TMP
    0x8A,                 //    txa
    0x69, 0x10,           //    adc #$10
    0x29, 0x30,           //    and #$30
    0x6D, 0x00, 0x00,     //    adc TMP
    0x8D, 0x08, 0x49,     //    sta 0x4908
    0x40                  //    rti
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  // Hook up IRQ interrupt handler
  rom.get_buffer()[kVecIRQ - kAddrROM] = 0x1E;
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

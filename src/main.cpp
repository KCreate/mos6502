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

#include <iostream>
#include <thread>
#include <chrono>

#include "cpu.h"
#include "bus.h"
#include "rammodule.h"
#include "rommodule.h"
#include "iochip.h"

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
  rom.get_buffer()[kVecRES - kAddrROM] = 0x0C;
  rom.get_buffer()[kVecRES - kAddrROM + 1] = 0x48;

  // Flash some code into the ROM for the CPU to execute
  uint8_t code[] = {
    0xA2, 0x00,             // 480C: LDX #$00
                            //
    0xE0, 0x20,             // 480E: CPX #$20
    0xF0, 0x09,             // 4810: BEQ 481B
                            //
    0xA9, 0x0F,             // 4812: LDA #$0F, A
    0x9D, 0x00, 0x40,       // 4814: STA $800, X
    0xE8,                   // 4817: INX
    0x4C, 0x0E, 0x48,       // 4818: JMP $480E
                            //
    0xFF                    // 481B: illegal opcode
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  CPU cpu(&bus);
  cpu.start();

  std::this_thread::sleep_for(5s);

  return 0;
}

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
  rom.get_buffer()[kVecRES - kAddrROM] = 0x10;
  rom.get_buffer()[kVecRES - kAddrROM + 1] = 0x49;

  // Flash some code into the ROM for the CPU to execute
  uint8_t code[] = {
    0xEA,                   // 4910: NOP
    0x4C, 0x11, 0x49,       // 4911: JMP $4911
  };
  std::memcpy(rom.get_buffer(), code, sizeof(code));

  CPU cpu(&bus);

  std::thread cpu_thread([&cpu, &io]() {
    cpu.start();
    std::cout << cpu.PC << std::endl;
    io.stop();
  });

  io.start();
  std::cout << "we are here" << std::endl;
  cpu_thread.join();

  return 0;
}

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

#include <atomic>
#include <cstdint>

#include "busdevice.h"

#pragma once

namespace M6502 {

// Addresses on the bus of the attached devices
static constexpr uint16_t kAddrRAM = 0x0000;
static constexpr uint16_t kAddrIO = 0x4000;
static constexpr uint16_t kAddrROM = 0x4920;

// Sizes of different attached devices
static constexpr size_t kSizeRAM = 0x4000;
static constexpr size_t kSizeIO = 0x920;
static constexpr size_t kSizeROM = 0xB6E0;

// Forward declaration
class CPU;

// Abstraction of a bus attached to the 6502 micrcontroller
class Bus {
public:
  Bus() {
  }

  // Read access
  uint8_t read_byte(uint16_t address);
  uint16_t read_word(uint16_t address);

  // Write access
  void write_byte(uint16_t address, uint8_t value);
  void write_word(uint16_t address, uint16_t value);

  // Return the device which handles a given address
  BusDevice* resolve_address_to_device(uint16_t address);

  // Attach devices to the different parts of the bus
  void attach_cpu(CPU* cpu);
  void attach_ram(BusDevice* dev);
  void attach_io(BusDevice* dev);
  void attach_rom(BusDevice* dev);

  // Interrupts
  void int_irq();
  void int_nmi();
  void int_res();

private:
  // Attached devices
  CPU* cpu;
  BusDevice* RAM = nullptr;
  BusDevice* IO = nullptr;
  BusDevice* ROM = nullptr;
};
}  // namespace M6502

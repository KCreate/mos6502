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

#include "bus.h"

namespace M6502 {

uint8_t Bus::read_byte(uint16_t address) {
  BusDevice* dev = this->resolve_address_to_device(address);
  if (dev == nullptr)
    return 0;
  return dev->read(address - dev->mapped_address);
}

uint16_t Bus::read_word(uint16_t address) {
  BusDevice* dev = this->resolve_address_to_device(address);
  if (dev == nullptr)
    return 0;
  uint16_t result;
  result = dev->read(address - dev->mapped_address);
  result |= (dev->read(address - dev->mapped_address + 1) << 8);
  return result;
}

void Bus::write_byte(uint16_t address, uint8_t value) {
  BusDevice* dev = this->resolve_address_to_device(address);
  if (dev == nullptr)
    return;
  dev->write(address - dev->mapped_address, value);
}

void Bus::write_word(uint16_t address, uint16_t value) {
  BusDevice* dev = this->resolve_address_to_device(address);
  if (dev == nullptr)
    return;
  dev->write(address - dev->mapped_address, value & 0xFF);
  dev->write(address - dev->mapped_address + 1, (value >> 8) & 0xFF);
}

BusDevice* Bus::resolve_address_to_device(uint16_t address) {
  if (address < kAddrIO) return this->RAM;
  if (address >= kAddrROM) return this->ROM;
  return this->IO;
}

}  // namespace M6502

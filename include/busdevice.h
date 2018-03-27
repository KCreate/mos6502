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

#include <cstdint>
#include <functional>  // for std::function
#include <iostream>

#pragma once

namespace M6502 {

using BusRead = std::function<uint8_t(uint16_t)>;
using BusWrite = std::function<void(uint16_t, uint8_t)>;

class Bus;  // forward declaration

// Abstraction of a device attached to the bus
class BusDevice {
public:
  BusDevice(uint16_t maddr) : mapped_address(maddr) {
  }

  // Read and write access to the device
  virtual uint8_t read(uint16_t address) = 0;
  virtual void write(uint16_t address, uint8_t value) = 0;

  // The address at which this device was mapped into memory
  uint16_t mapped_address;
  Bus* bus;
};

// A device which can only be read from
class ReadOnlyDevice : public BusDevice {
public:
  using BusDevice::BusDevice;

  inline void write(uint16_t, uint8_t) {
    // do nothing
  }
};

// A device which can only be written to
class WriteOnlyDevice : public BusDevice {
public:
  using BusDevice::BusDevice;

  inline uint8_t read(uint16_t) {
    return 0;
  }
};
}  // namespace M6502

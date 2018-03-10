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

#include <SDL2/SDL.h>

#include "busdevice.h"

#pragma once

namespace M6502 {

// Display measurements
static constexpr size_t kVideoWidth = 64;
static constexpr size_t kVideoHeight = 32;
static constexpr size_t kVideoPixelsPerDot = 4;
static constexpr size_t kVideoWidthPixels = kVideoWidth * kVideoPixelsPerDot;
static constexpr size_t kVideoHeightPixels = kVideoHeight * kVideoPixelsPerDot;

// Control ports
//
// The IO chip provides several addresses, which when written to, perform various actions
// inside the chip. Via this mechanism you can update the display, update the audio channels or
// check for events.
static constexpr uint16_t kIOControlRender = 0x800;
static constexpr uint16_t kIOControlUpdateAudio = 0x801;
static constexpr uint16_t kIOControlCheckEvent = 0x802;
static constexpr uint16_t kIOControlToggleWindow = 0x803;

// The IO chip provides several events. The type and payload for these events are stored
// at specific memory locations inside the chip.
static constexpr uint8_t kIOEventKeydown = 0x00;
static constexpr uint8_t kIOEventKeyup = 0x01;
static constexpr uint8_t kIOEventMousemove = 0x02;
static constexpr uint8_t kIOEventMousedown = 0x03;
static constexpr uint8_t kIOEventMouseup = 0x04;

// The io chip responsible for rendering a 32x64 virtual display
//
// The io chip should also be able interrupt the CPU when certain events occur:
// - Mouse movement
// - Mouse down / up
// - Keypresses
class IOChip : public BusDevice {
public:
  IOChip(uint16_t maddr);
  ~IOChip();

  void init_screen();
  void write(uint16_t address, uint8_t value);
  uint8_t read(uint16_t address);

private:
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  // Memory buffer of the IO chip
  union {
    uint8_t buffer[0x80C];
    struct {

      // Display memory buffer
      uint8_t vram[0x800];

      // Control ports
      uint8_t render_display;
      uint8_t update_audio;
      uint8_t check_events;
      uint8_t toggle_window;

      // Event data
      uint8_t event_type;
      uint8_t keycode;
      uint8_t mouse_x;
      uint8_t mouse_y;

      // Audio channels
      uint8_t audio_1;
      uint8_t audio_2;
      uint8_t audio_3;
      uint8_t audio_4;
    };
  };
};

}  // namespace M6502

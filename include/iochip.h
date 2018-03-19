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

#include "busdevice.h"

#pragma once

namespace M6502 {

// IOChip
//
// The virtual display shows a 64x36 window, where each pixel is directly addressable
// in memory.
//
// Graphics mode:
//   In graphics mode, the display can either be used in RGB or HSL mode. Each pixel is 1 byte in size and can thus
//   display 256 different colors. Depending on the color encoding choosen, slightly different colors might be
//   reachable in each mode. The color encodings are specified as follows:
//
//   RGB-Pixel: rrrgggbb
//   HSL-Pixel: hhhhssll
//
//   The background and foreground color configuration bytes have no effect in this mode and are ignored.
//
// Text mode:
//   In text mode, each byte of VRAM stores a character to be rendered on the display. The character set used is
//   plain old ASCII. Character codes above 0x7f (bytes where bit 7 is set) are interpreted as 0x00 - 0x7f but are
//   rendered with the background and foreground colors swapped. Background and foreground colors of the display
//   can be configured at their respective control ports. The color encodings of these values are defined via the
//   respective flag in the control byte of the IO chip.
//
//   Character: 0 0000000
//              ^ ^
//              | |
//              | +- ANSCII Character code
//              +--- Swap background and foreground color
//
// Keyboard & Mouse access:
//   The IOChip listens for keyboard and mouse events. These events are passed to the CPU via the interrupt mechanism.
//   Interrupts for either keyboard or mouse events can be disabled via their respective flags in the control byte.
static constexpr size_t kIOVideoWidth = 64;
static constexpr size_t kIOVideoHeight = 36;
static constexpr size_t kIOVideoRedMask = 0xE0;
static constexpr size_t kIOVideoGreenMask = 0x1C;
static constexpr size_t kIOVideoBlueMask = 0x03;
static constexpr size_t kIOVideoHueMask = 0xF0;
static constexpr size_t kIOVideoSaturationMask = 0x0C;
static constexpr size_t kIOVideoLightnessMask = 0x03;

// Control ports
//
// These are the most important interfaces to the IO chip.
static constexpr uint16_t kIOControl = 0x900;
static constexpr uint16_t kIOTextModeBackgroundColor = 0x901;
static constexpr uint16_t kIOTextModeForegroundColor = 0x902;

// Event payload data
//
// Interrupts store their payload in these memory locations
static constexpr uint16_t kIOEventType = 0x903;
static constexpr uint16_t kIOKeyboardKeycode = 0x904;
static constexpr uint16_t kIOMouseXCoord = 0x904;
static constexpr uint16_t kIOMouseYCoord = 0x905;

// Hardware clocks
//
// The hardware clocks can interrupt the CPU at configurable intervals of time. When the first bit is cleared,
// the clock interrupts the CPU every 1/f seconds. If the first bit is set, the clocks interrupt the CPU
// every n seconds. If you change this value while an interval is still running, the active interval will
// not terminate, but will wait the previously configured amount of time and use the new timing afterwards.
//
// Each clock interrupt sets an event type in the event type memory location so the CPU knows which clock fired.
//
// If the clock is configured as 1/0 or 0 seconds then the clock is considered disabled and will not fire.
//
// Clock: 0 0000000
//        ^ ^
//        | |
//        | +- Fraction or amount of seconds
//        +--- Toggle between fraction or seconds
static constexpr uint16_t kIOClock1 = 0x906;
static constexpr uint16_t kIOClock2 = 0x907;

// Audio channels
//
// The audio system of the IO chip consists of four distinct audio channels which can each play a configurable
// sound.
//
// AudioChannel: 00 00 0000
//               ^  ^  ^
//               |  |  |
//               |  |  +- Pitch (Range of [0.2 - 2.2])
//               |  +---- Wave function (sine, square, saw, triangle)
//               +------- Volume (0%, 25%, 50%, 100%)
//
// Setting the volume to 0 will stop playing the audio, so no resources are wasted playing a mute sound.
static constexpr uint16_t kIOAudioChannel1 = 0x908;
static constexpr uint16_t kIOAudioChannel2 = 0x909;
static constexpr uint16_t kIOAudioChannel3 = 0x90A;
static constexpr uint16_t kIOAudioChannel4 = 0x90B;

// Reserved memory locations for future expansion
static constexpr uint16_t kIOReserved1 = 0x90C;
static constexpr uint16_t kIOReserved2 = 0x90D;
static constexpr uint16_t kIOReserved2 = 0x90E;
static constexpr uint16_t kIOReserved2 = 0x90F;

// Interrupt event codes
//
// When the IO chip interrupts the CPU and requests servicing, it puts an event type into the respective memory
// location. The Chip can then decide what to do with the data.
static constexpr uint8_t kIOEventKeydown = 0x00;
static constexpr uint8_t kIOEventKeyup = 0x01;
static constexpr uint8_t kIOEventMousemove = 0x02;
static constexpr uint8_t kIOEventMousedown = 0x03;
static constexpr uint8_t kIOEventMouseup = 0x04;
static constexpr uint8_t kIOEventClock1 = 0x05;
static constexpr uint8_t kIOEventClock2 = 0x06;

// Misc. IO control flags
//
// IO + 0x900: 0 0 0 0 0 0 0 0
//             ^ ^ ^ ^ ^ ^ ^ ^
//             | | | | | | | |
//             | | | | | | | +- Reserved for future expansion
//             | | | | | | +--- Enable / Disable the mouse
//             | | | | | +----- Enable / Disable the keyboard
//             | | | | +------- RGB / HSL color space
//             | | | +--------- Landscape / Portrait layout
//             | | +----------- Fullscreen or floating window
//             | +------------- Show or hide the window
//             +--------------- Graphics or text mode
static constexpr uint8_t kIOControlMode = 0x80;
static constexpr uint8_t kIOControlVisibility = 0x40;
static constexpr uint8_t kIOControlFullscreen = 0x20;
static constexpr uint8_t kIOControlOrientation = 0x10;
static constexpr uint8_t kIOControlColorMode = 0x08;
static constexpr uint8_t kIOControlKeyboardEnabled = 0x04;
static constexpr uint8_t kIOControlMouseEnabled = 0x02;

// 1 byte color value, encoding either an RGB or HSL value
struct ColorValue {
  uint8_t value;

  ColorValue(uint8_t v) : value(v) {
  }

  inline uint8_t get_rgb_r() {
    return (this->value | kIOVideoRedMask) >> 5;
  }
  inline uint8_t get_rgb_g() {
    return (this->value | kIOVideoGreenMask) >> 2;
  }
  inline uint8_t get_rgb_b() {
    return this->value | kIOVideoBlueMask;
  }
  inline uint8_t get_hsl_h() {
    return (this->value | kIOVideoHueMask) >> 4;
  }
  inline uint8_t get_hsl_s() {
    return (this->value | kIOVideoSaturationMask) >> 2;
  }
  inline uint8_t get_hsl_l() {
    return this->value | kIOVideoLightnessMask;
  }
};

class IOChip : public BusDevice {
public:
  IOChip(uint16_t maddr);
  ~IOChip();

  void write(uint16_t address, uint8_t value);
  uint8_t read(uint16_t address);

private:
  // Memory buffer of the IO chip
  union {
    uint8_t buffer[0x910] = {0};
    struct {
      // Display memory buffer
      uint8_t vram[0x900];

      uint8_t control;
      ColorValue background_color;
      ColorValue foreground_color;
      uint8_t event_type;
      union {
        struct {
          uint8_t keycode;
        } keyboard;
        struct {
          uint8_t x;
          uint8_t y;
        } mouse;
      };
      uint8_t clock1;
      uint8_t clock2;
      uint8_t audio_channel1;
      uint8_t audio_channel2;
      uint8_t audio_channel3;
      uint8_t audio_channel4;
      uint8_t reserved1;
      uint8_t reserved2;
      uint8_t reserved3;
      uint8_t reserved4;
    };
  };
};

}  // namespace M6502

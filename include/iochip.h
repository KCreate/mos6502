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

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "busdevice.h"

#pragma once

namespace M6502 {

// IOChip
//
// The virtual display shows a 64x36 window, where each pixel is directly addressable
// in memory.
//
// Graphics mode:
//   In graphics mode, the display can be driven via RGB values. Each pixel is 1 byte in size and can thus
//   display 256 different colors.
//
//   RGB-Pixel: rrrgggbb
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
static std::string kIOVideoTitle = "6502 Microcontroller";
static constexpr size_t kIOVideoWidth = 64;
static constexpr size_t kIOVideoHeight = 36;
static constexpr size_t kIOVideoScaleWidth = 5;
static constexpr size_t kIOVideoScaleHeight = 5;
static constexpr size_t kIOVideoModeWidth = kIOVideoWidth * kIOVideoScaleWidth;
static constexpr size_t kIOVideoModeHeight = kIOVideoHeight * kIOVideoScaleHeight;
static constexpr size_t kIOVRAMSize = kIOVideoWidth * kIOVideoHeight;
static constexpr size_t kIOVideoRedMask = 0xE0;
static constexpr size_t kIOVideoGreenMask = 0x1C;
static constexpr size_t kIOVideoBlueMask = 0x03;

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
// The hardware clocks can interrupt the CPU at configurable intervals of time. When the value stored in these
// memory locations is 0, the clocks are turned off. The minimum amount of time between clock pulses is
// 5ms * f, where f is the value stored inside memory. Reading from a clock will yield you the amount
// of 5ms intervals still left until the next clock pulse.
//
// Each clock interrupt sets an event type in the event type memory location so the CPU knows which clock fired.
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

// VRAM Access
//
// There are two methods of directly writing to VRAM. You either write directly to an address in the VRAM,
// (e.g. through some 16-bit addressing mode) or you utilize the various drawing functions the IOChip provides.
//
// Writing to kIODrawMethod will commit the action and execute the corresponding method. Values inside the argument
// bytes will not be altered and can be used for further drawing.
//
// Drawing functions supported by the IOChip:
//
// Geometric shapes
//  io_draw_rectangle(x, y, w, h)
//  io_draw_square(x, y, s)
//  io_draw_dot(x, y) - Uses the body color
//
// Color
//  io_brush_set_body(v)
//  io_brush_set_outline(v)
static constexpr uint16_t kIODrawMethod = 0x90B;
static constexpr uint16_t kIODrawArg1 = 0x90C;
static constexpr uint16_t kIODrawArg2 = 0x90D;
static constexpr uint16_t kIODrawArg3 = 0x90E;
static constexpr uint16_t kIODrawArg4 = 0x90F;

// Draw method codes
static constexpr uint8_t kIODrawRectangle = 0x00;
static constexpr uint8_t kIODrawSquare = 0x01;
static constexpr uint8_t kIODrawDot = 0x02;
static constexpr uint8_t kIOBrushSetBody = 0x03;
static constexpr uint8_t kIOBrushSetOutline = 0x04;

// Interrupt event codes
//
// When the IO chip interrupts the CPU and requests servicing, it puts an event type into the respective memory
// location. The Chip can then decide what to do with the data.
static constexpr uint8_t kIOEventUnspecified = 0x00;
static constexpr uint8_t kIOEventKeydown = 0x01;
static constexpr uint8_t kIOEventKeyup = 0x02;
static constexpr uint8_t kIOEventMousemove = 0x03;
static constexpr uint8_t kIOEventMousedown = 0x04;
static constexpr uint8_t kIOEventMouseup = 0x05;
static constexpr uint8_t kIOEventClock1 = 0x06;
static constexpr uint8_t kIOEventClock2 = 0x07;

// Misc. IO control flags
//
// IO + 0x900: 0 0 0 0 1 1 0 0
//             ^ ^ ^ ^ ^ ^ ^ ^
//             | | | | | | | |
//             | | | | | | | +- Reserved for future expansion
//             | | | | | | +--- Reserved for future expansion
//             | | | | | +----- Enable / Disable the mouse
//             | | | | +------- Enable / Disable the keyboard
//             | | | +--------- Landscape / Portrait layout
//             | | +----------- Floating or Fullscreen window
//             | +------------- Show or hide the window
//             +--------------- Graphics or text mode
static constexpr uint8_t kIOControlMode = 0x80;
static constexpr uint8_t kIOControlVisibility = 0x40;
static constexpr uint8_t kIOControlFullscreen = 0x20;
static constexpr uint8_t kIOControlOrientation = 0x10;
static constexpr uint8_t kIOControlKeyboardDisabled = 0x08;
static constexpr uint8_t kIOControlMouseDisabled = 0x04;

// 1 byte RGB value
struct ColorValue {
  uint8_t value;

  ColorValue() : value(0x00) {
  }

  ColorValue(uint8_t v) : value(v) {
  }

  inline uint8_t get_r() {
    return ((this->value & kIOVideoRedMask) >> 5) * 32;
  }
  inline uint8_t get_g() {
    return ((this->value & kIOVideoGreenMask) >> 2) * 32;
  }
  inline uint8_t get_b() {
    return (this->value & kIOVideoBlueMask) * 64;
  }
  inline sf::Color get_sfml_color() {
    return sf::Color(this->get_r(), this->get_g(), this->get_b());
  }
};

// Draw instruction telling the drawing thread what to do
struct DrawInstruction {
  uint8_t method_code;
  uint8_t arg1;
  uint8_t arg2;
  uint8_t arg3;
  uint8_t arg4;
};

class IOChip : public BusDevice {
public:
  IOChip(uint16_t maddr);
  ~IOChip();

  void start();
  void stop();
  void write(uint16_t address, uint8_t value);
  uint8_t read(uint16_t address);

private:
  // Starts an audio thread using a byte at a given offset
  void thread_audio(uint16_t audio_offset);

  // Starts a clock thread using a byte at a given offset
  void thread_clock(uint16_t clock_offset);

  // Stars a rendering thread using a sf::RenderWindow*
  void thread_render();

  // Drawing thread
  void thread_drawing();

  union {
    uint8_t memory[0x910];
    struct {
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
      uint8_t draw_method;
      uint8_t draw_arg1;
      uint8_t draw_arg2;
      uint8_t draw_arg3;
      uint8_t draw_arg4;
    };
  };

  // Thread synchronisation stuff
  sf::RenderWindow* main_window = nullptr;
  std::thread render_thread;
  std::thread drawing_thread;
  std::atomic<bool> shutdown;
  std::vector<std::thread> clock_threads;
  std::vector<std::thread> audio_threads;

  // Rendering configuration
  std::atomic<bool> text_mode;
  std::atomic<bool> window_hidden;
  std::atomic<bool> window_fullscreen;
  std::atomic<bool> window_portrait;
  std::atomic<bool> keyboard_disabled;
  std::atomic<bool> mouse_disabled;

  // Advanced rendering
  std::queue<DrawInstruction> draw_pipeline;
  std::mutex draw_mutex;
  std::shared_mutex draw_pipeline_mutex;
  std::condition_variable condition_draw;
  std::atomic<uint8_t> brush_body_color;
  std::atomic<uint8_t> brush_outline_color;

  // Advanced drawing methods
  void draw_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
  void draw_square(uint8_t x, uint8_t y, uint8_t s);
  void draw_dot(uint8_t x, uint8_t y);
};

}  // namespace M6502

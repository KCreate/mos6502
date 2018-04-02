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
#include <cstring>

#ifdef LINUX
#include <X11/Xlib.h>
#ifdef None
#  undef None
#  define X11None 0L
#  ifdef RevertToNone
#    undef RevertToNone
#    define RevertToNone (int)X11None
#  endif
#endif
#endif

#include "iochip.h"
#include "bus.h"

namespace M6502 {

IOChip::IOChip(uint16_t maddr) : BusDevice(maddr) {
  std::memset(this->vram, 0, kIOVRAMSize);
  this->control = kIOControlKeyboardDisabled | kIOControlMouseDisabled;

  this->background_color = ColorValue(0x00);
  this->foreground_color = ColorValue(0xFF);

  this->event_type = kIOEventUnspecified;
  this->mouse.x = 0x00;
  this->mouse.y = 0x00;

  this->clock1 = 0x00;
  this->clock2 = 0x00;

  this->audio_channel1 = 0x00;
  this->audio_channel2 = 0x00;
  this->audio_channel3 = 0x00;

  this->shutdown = false;
}

IOChip::~IOChip() {
  if (!this->shutdown) this->stop();
}

void IOChip::start() {
#ifdef LINUX
  XInitThreads();
#endif

  this->shutdown = false;

  // Start the audio, clock and rendering threads
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel1));
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel2));
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel3));
  this->clock_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock1));
  //this->clock_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock2));
  this->drawing_thread = std::thread(&IOChip::thread_drawing, this);

  // Create the window and the thread which handles all the drawing
  //
  // We separate the event loop and the drawing code because we don't want to
  // drop any frames just because we're waiting for events.
  //
  // SFML supports this natively and we don't have to write any new code to make
  // this work.
  //
  // See https://www.sfml-dev.org/tutorials/2.0/graphics-draw.php for a simple
  // tutorial on how drawing from other threads works.
  sf::VideoMode video_mode(kIOVideoModeWidth, kIOVideoModeHeight);
  this->main_window = new sf::RenderWindow(video_mode, kIOVideoTitle, sf::Style::Titlebar);
  this->render_thread = std::thread(&IOChip::thread_render, this);

  sf::Event event;
  while (!this->shutdown && this->main_window->isOpen()) {
    while (this->main_window->pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed: {
          this->main_window->close();
          this->shutdown = true;
          break;
        }
        case sf::Event::KeyPressed: {
          this->bus->int_irq();
          break;
        }
        default: {
          // TODO: Handle events
          break;
        }
      }
    }
  }
}

void IOChip::stop() {
  this->shutdown = true;
  this->render_thread.join();
  this->condition_draw.notify_one();
  this->drawing_thread.join();
  for (auto& t : this->clock_threads)
    t.join();
  for (auto& t : this->audio_threads)
    t.join();

  delete this->main_window;

  this->clock_threads.clear();
  this->audio_threads.clear();
  this->main_window = nullptr;
}

void IOChip::thread_audio(uint16_t) {
}

void IOChip::thread_clock(uint16_t source) {
  uint8_t clock_source = 0;
  while (!this->shutdown) {
    clock_source = this->memory[source];
    if (!clock_source) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      continue;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned int>(5) * clock_source));
    this->memory[kIOEventType] = source == kIOClock1 ? kIOEventClock1 : kIOEventClock2;
    this->bus->int_irq();
  }
}

void IOChip::thread_drawing() {
  while (!this->shutdown) {
    std::unique_lock<std::mutex> l(this->draw_mutex);
    this->condition_draw.wait(l, [&]() {
      std::shared_lock<std::shared_mutex> lk(this->draw_pipeline_mutex);
      return this->draw_pipeline.size() > 0 || this->shutdown;
    });

    // Check if we should shutdown
    if (this->shutdown) continue;

    // Fetch an instruction from the pipeline
    DrawInstruction instruction;
    {
      std::unique_lock<std::shared_mutex> lk(this->draw_pipeline_mutex);
      instruction = this->draw_pipeline.front();
      this->draw_pipeline.pop();
    }

    switch (instruction.method_code) {
      case kIODrawRectangle: {
        this->draw_rectangle(instruction.arg1, instruction.arg2, instruction.arg3, instruction.arg4);
        break;
      }
      case kIODrawSquare: {
        this->draw_square(instruction.arg1, instruction.arg2, instruction.arg3);
        break;
      }
      case kIODrawDot: {
        this->draw_dot(instruction.arg1, instruction.arg2);
        break;
      }
      case kIOBrushSetBody: {
        this->brush_body_color = instruction.arg1;
        break;
      }
      case kIOBrushSetOutline: {
        this->brush_outline_color = instruction.arg1;
        break;
      }
    }
  }
}

void IOChip::thread_render() {
  using namespace std::chrono_literals;

  while (!this->shutdown && this->main_window->isOpen()) {
    // Check the control bytes for the configuration of the display
    bool portrait_mode = this->control & kIOControlOrientation;
    bool window_hidden = this->control & kIOControlVisibility;
    // bool text_mode = this->control & kIOControlMode;

    // If the screen is hidden we sleep for some time until the window
    // is visible again.
    //
    // TODO: Could this be made more efficient using a lock?
    if (window_hidden) {
      std::this_thread::sleep_for(1s);
      continue;
    }

    // These are the dimensions of the window we are drawing to
    // and of the brush that is used to paint the pixels.
    uint32_t screen_width = portrait_mode ? kIOVideoHeight : kIOVideoWidth;
    uint32_t screen_height = portrait_mode ? kIOVideoWidth : kIOVideoHeight;
    uint32_t pixels_row = portrait_mode ? kIOVideoScaleHeight : kIOVideoScaleWidth;
    uint32_t pixels_column = portrait_mode ? kIOVideoScaleWidth : kIOVideoScaleHeight;

    // Iterate over every pixel in VRAM and draw it to the screen
    sf::RectangleShape brush(sf::Vector2f(pixels_row, pixels_column));
    for (uint8_t y = 0; y < screen_height; y++) {
      for (uint8_t x = 0; x < screen_width; x++) {
        // Fetch the color from VRAM
        uint8_t vram_raw = this->vram[x + y * screen_width];
        ColorValue vram_color(vram_raw);

        // Paint the corresponding section of the window
        brush.setFillColor(vram_color.get_sfml_color());
        brush.setPosition(sf::Vector2f(x * pixels_row, y * pixels_column));
        this->main_window->draw(brush);
      }
    }

    this->main_window->display();
  }
}

void IOChip::write(uint16_t address, uint8_t value) {
  this->memory[address] = value;

  // Some memory locations require further processing, these are handled here
  //
  // TODO: The render thread should pause if there are no new VRAM updates.
  //       This could be achieved via some new control flag which would enable
  //       or disable the render thread.
  switch (address) {
    case kIOControl: {
      this->text_mode = (value & kIOControlMode);
      this->window_hidden = (value & kIOControlVisibility);
      this->window_fullscreen = (value & kIOControlFullscreen);
      this->window_portrait = (value & kIOControlOrientation);
      this->keyboard_disabled = (value & kIOControlKeyboardDisabled);
      this->mouse_disabled = (value & kIOControlMouseDisabled);
      break;
    }
    case kIODrawMethod: {
      // Load the draw instruction and append to the pipeline
      uint8_t arg1 = this->memory[kIODrawArg1];
      uint8_t arg2 = this->memory[kIODrawArg2];
      uint8_t arg3 = this->memory[kIODrawArg3];
      uint8_t arg4 = this->memory[kIODrawArg4];

      {
        std::unique_lock<std::shared_mutex> lk(this->draw_pipeline_mutex);
        this->draw_pipeline.push({value, arg1, arg2, arg3, arg4});
      }

      // Notify the drawing thread that there is work to do
      this->condition_draw.notify_one();
      break;
    }
  }
}

uint8_t IOChip::read(uint16_t address) {
  std::cout << "read " << std::hex << address << std::dec << std::endl;
  return 0;
}

void IOChip::draw_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  bool portrait_mode = this->control & kIOControlOrientation;
  uint8_t screen_width = portrait_mode ? kIOVideoHeight : kIOVideoWidth;
  uint8_t screen_height = portrait_mode ? kIOVideoWidth : kIOVideoHeight;

  // Draw the rectangle
  for (uint8_t by = 0; by < h; by++) {
    if (by + y >= screen_height) continue;
    for (uint8_t bx = 0; bx < w; bx++) {
      uint8_t color = by == 0 || bx == 0 || by == h - 1 || bx == w - 1 ? this->brush_outline_color : brush_body_color;
      if (bx + x >= screen_width) continue;
      uint32_t offset = (bx + x) + (by + y) * screen_width;
      if (offset >= 0x900) {
        continue;
      }
      this->vram[offset] = color;
    }
  }
}

void IOChip::draw_square(uint8_t x, uint8_t y, uint8_t s) {
  bool portrait_mode = this->control & kIOControlOrientation;
  uint8_t screen_width = portrait_mode ? kIOVideoHeight : kIOVideoWidth;

  // Draw the rectangle
  for (uint8_t by = 0; by < s; by++) {
    for (uint8_t bx = 0; bx < s; bx++) {
      uint8_t color = by == 0 || bx == 0 || by == s - 1 || bx == s - 1 ? this->brush_outline_color : brush_body_color;
      uint32_t offset = (bx + x) + (by + y) * screen_width;
      if (offset >= 0x900)
        continue;
      this->vram[offset] = color;
    }
  }
}

void IOChip::draw_dot(uint8_t x, uint8_t y) {
  bool portrait_mode = this->control & kIOControlOrientation;
  uint8_t screen_width = portrait_mode ? kIOVideoHeight : kIOVideoWidth;
  this->vram[x + y * screen_width] = this->brush_body_color;
}

}  // namespace M6502

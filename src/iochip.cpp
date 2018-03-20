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

#include <cstring>

#include "iochip.h"

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
  this->audio_channel4 = 0x00;

  this->shutdown = false;
}

IOChip::~IOChip() {
  this->stop();
}

void IOChip::start() {
  this->shutdown = false;

  // Start the audio, clock and rendering threads
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel1));
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel2));
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel3));
  this->audio_threads.push_back(std::thread(&IOChip::thread_audio, this, kIOAudioChannel4));
  this->clock_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock1));
  this->clock_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock1));

  // Display event loop
  sf::RenderWindow window(sf::VideoMode(kIOVideoModeWidth, kIOVideoModeHeight), kIOVideoTitle, sf::Style::Titlebar);
  window.setFramerateLimit(30);
  this->render_threads.push_back(std::thread(&IOChip::thread_render, this, &window));

  sf::Event event;
  while (!this->shutdown && window.isOpen()) {
    while (window.waitEvent(event)) {
      std::cout << "got event: " << event.type << std::endl;
    }
  }

  this->stop();
}

void IOChip::stop() {
  this->shutdown = true;
  for (auto& t : this->clock_threads) {
    t.join();
  }
  for (auto& t : this->audio_threads) {
    t.join();
  }

  this->clock_threads.clear();
  this->audio_threads.clear();
}

void IOChip::thread_audio(uint16_t) {
}

void IOChip::thread_clock(uint16_t) {
}

void IOChip::thread_render(sf::RenderWindow* window) {
  while (!this->shutdown && window->isOpen()) {
    // Width and height of the brush.
    uint32_t brush_w = kIOVideoScaleWidth;
    uint32_t brush_h = kIOVideoScaleHeight;
    if (this->control & kIOControlOrientation) {
      brush_w = brush_h;
      brush_h = brush_w;
    }
    sf::RectangleShape brush(sf::Vector2f(brush_w, brush_h));

    // In portrait mode, we swap the width and height components
    if (this->control & kIOControlOrientation) {
      window->setSize(sf::Vector2u(brush_w * kIOVideoHeight, brush_h * kIOVideoWidth));
    } else {
      window->setSize(sf::Vector2u(brush_w * kIOVideoWidth, brush_h * kIOVideoHeight));
    }

    for (uint8_t y = 0; y < kIOVideoHeight; y++) {
      for (uint8_t x = 0; x < kIOVideoWidth; x++) {
        // In portrait mode, we have to swap the x/y coordinates
        // in order to draw correctly onto the portrait mode display.
        uint8_t brush_x = x;
        uint8_t brush_y = y;
        if (this->control & kIOControlOrientation) {
          brush_x = y;
          brush_y = x;
        }
        brush.setPosition(sf::Vector2f(brush_x * brush_w, brush_y * brush_h));

        // Check if we are in RGB or HSL colorspace
        ColorValue vram_byte = ColorValue(this->vram[brush_x + brush_y * brush_w]);
        if (this->control & kIOControlColorMode) {
          // HSL
          // TODO: Implement
        } else {
          // RGB
          brush.setFillColor(sf::Color(vram_byte.get_rgb_sfml_color()));
        }

        window->draw(brush);
      }
    }

    window->display();
  }
}

void IOChip::write(uint16_t address, uint8_t value) {
  std::cout << "write " << address << " <- " << static_cast<unsigned int>(value) << std::endl;
}

uint8_t IOChip::read(uint16_t address) {
  std::cout << "read " << std::hex << address << std::dec << std::endl;
  return 0;
}

}  // namespace M6502

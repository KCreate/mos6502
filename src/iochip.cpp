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
#include <cmath>

#ifdef LINUX
#include <X11/Xlib.h>
#ifdef None
#undef None
#define X11None 0L
#ifdef RevertToNone
#undef RevertToNone
#define RevertToNone (int)X11None
#endif
#endif
#endif

#include "bus.h"
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

  this->shutdown = false;
}

IOChip::~IOChip() {
  if (!this->shutdown)
    this->stop();
}

void IOChip::start() {
#ifdef LINUX
  XInitThreads();
#endif

  this->load_audio_buffers();

  this->shutdown = false;

  // Start the audio, clock and rendering threads
  this->worker_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock1));
  // this->worker_threads.push_back(std::thread(&IOChip::thread_clock, this, kIOClock2));
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
          uint8_t modifier_byte = 0x00;
          if (event.key.alt)
            modifier_byte |= kIOKeyboardModifierAlt;
          if (event.key.control)
            modifier_byte |= kIOKeyboardModifierControl;
          if (event.key.shift)
            modifier_byte |= kIOKeyboardModifierShift;
          if (event.key.system)
            modifier_byte |= kIOKeyboardModifierSystem;
          this->memory[kIOEventType] = kIOEventKeydown;
          this->memory[kIOKeyboardKeycode] = event.key.code;
          this->memory[kIOKeyboardModifiers] = modifier_byte;
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
  for (auto& t : this->worker_threads)
    t.join();

  delete this->main_window;

  this->worker_threads.clear();
  this->main_window = nullptr;
}

void IOChip::load_audio_buffers() {
  sf::Int16 raw[kIOAudioSamples];
  double x;

  // Generate sine wave data
  x = 0;
  for (uint32_t i = 0; i < kIOAudioSamples; i++) {
    raw[i] = kIOAudioAmplitude * sin(x * kIOAudioTwoPi);
    x += kIOAudioSampleIncrement;
  }
  this->audio_buffer_sine.loadFromSamples(raw, kIOAudioSamples, 1, kIOAudioSampleRate);

  // Generate square wave data
  x = 0;
  for (uint32_t i = 0; i < kIOAudioSamples; i++) {
    raw[i] = kIOAudioAmplitude * (sin(x * kIOAudioTwoPi) >= 0.0 ? 1 : 0.5);
    x += kIOAudioSampleIncrement;
  }
  this->audio_buffer_square.loadFromSamples(raw, kIOAudioSamples, 1, kIOAudioSampleRate);

  // TODO: Generate saw and triangle wave data
  //       Right now we just load these buffers with square wave data
  this->audio_buffer_saw.loadFromSamples(raw, kIOAudioSamples, 1, kIOAudioSampleRate);
  this->audio_buffer_triangle.loadFromSamples(raw, kIOAudioSamples, 1, kIOAudioSampleRate);
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
    if (this->shutdown)
      continue;

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
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(16));

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
    case kIOAudioChannel1:
    case kIOAudioChannel2:
    case kIOAudioChannel3: {
      this->update_audio(address, value);
      break;
    }
  }
}

uint8_t IOChip::read(uint16_t address) {
  return this->memory[address];
}

void IOChip::update_audio(uint16_t address, uint8_t value) {

  // Decode parameters
  uint8_t volume = (value & kIOAudioChannelVolume) >> 6;
  float volume_f = 0;
  uint8_t wave = (value & kIOAudioChannelWave) >> 4;
  float pitch = 0.2 + (static_cast<float>(value & kIOAudioChannelPitch) / 16) * 2;

  // sf::Sound::setVolume expects a value between 0 and 100
  if (volume == kIOAudioChannelVolume0) volume_f = 0;
  if (volume == kIOAudioChannelVolume25) volume_f = 25;
  if (volume == kIOAudioChannelVolume50) volume_f = 50;
  if (volume == kIOAudioChannelVolume100) volume_f = 100;

  // Get the source buffer for the sound
  sf::SoundBuffer* source_buffer = nullptr;
  if (wave == kIOAudioChannelWaveSine) source_buffer = &this->audio_buffer_sine;
  if (wave == kIOAudioChannelWaveSquare) source_buffer = &this->audio_buffer_square;
  if (wave == kIOAudioChannelWaveSaw) source_buffer = &this->audio_buffer_saw;
  if (wave == kIOAudioChannelWaveTriangle) source_buffer = &this->audio_buffer_triangle;

  // Get the target audio channel
  sf::Sound* target_channel = nullptr;
  if (address == kIOAudioChannel1) target_channel = &this->audio_sound1;
  if (address == kIOAudioChannel2) target_channel = &this->audio_sound2;
  if (address == kIOAudioChannel3) target_channel = &this->audio_sound3;

  std::cout << "pitch: " << pitch << std::endl;
  std::cout << "volume_f: " << volume_f << std::endl;
  std::cout << "wave function: " << static_cast<unsigned int>(wave) << std::endl;

  // Update sound parameters
  //target_channel->stop();
  target_channel->setPitch(pitch);
  target_channel->setVolume(volume_f);
  target_channel->setBuffer(*source_buffer);
  target_channel->setLoop(true);
  if (volume != 0) {
    std::cout << "playing sound" << std::endl;
    target_channel->play();
  } else {
    target_channel->pause();
  }
}

void IOChip::draw_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  bool portrait_mode = this->control & kIOControlOrientation;
  uint8_t screen_width = portrait_mode ? kIOVideoHeight : kIOVideoWidth;
  uint8_t screen_height = portrait_mode ? kIOVideoWidth : kIOVideoHeight;

  // Draw the rectangle
  for (uint8_t by = 0; by < h; by++) {
    if (by + y >= screen_height)
      continue;
    for (uint8_t bx = 0; bx < w; bx++) {
      uint8_t color = 0x00;
      if (by == 0 || bx == 0 || by == h - 1 || bx == w - 1) {
        color = this->brush_outline_color;
      } else {
        color = this->brush_body_color;
      }

      if (bx + x >= screen_width)
        continue;
      uint32_t offset = (bx + x) + (by + y) * screen_width;
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

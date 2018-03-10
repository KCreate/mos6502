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

#include "iochip.h"

namespace M6502 {

IOChip::IOChip(uint16_t maddr) : BusDevice(maddr) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    std::cout << "Disabling video subsystem" << std::endl;
    return;
  }

  this->window = SDL_CreateWindow("6502 Microcontroller", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    kVideoWidthPixels, kVideoHeightPixels, SDL_WINDOW_SHOWN);

  if (this->window == nullptr) {
    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    std::cout << "Disabling video subsystem" << std::endl;
    return;
  }

  this->renderer = SDL_CreateRenderer(this->window, -1, 0);

  if (this->renderer == nullptr) {
    SDL_DestroyWindow(this->window);
    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    std::cout << "Disabling video subsystem" << std::endl;
    return;
  }

  this->init_screen();
}

IOChip::~IOChip() {
  SDL_DestroyWindow(this->window);
  SDL_DestroyRenderer(this->renderer);
  SDL_Quit();
}

void IOChip::init_screen() {
  SDL_Surface* surface = SDL_GetWindowSurface(this->window);
  SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0, 0, 0));
  SDL_UpdateWindowSurface(this->window);
}

void IOChip::write(uint16_t address, uint8_t value) {
  std::cout << "write " << address << " <- " << static_cast<unsigned int>(value) << std::endl;
}

uint8_t IOChip::read(uint16_t address) {
  std::cout << "read " << address << std::endl;
  return 0;
}

}  // namespace M6502

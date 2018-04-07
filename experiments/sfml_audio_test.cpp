#include <SFML/Audio.hpp>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>

const unsigned SAMPLES = 1000;
const unsigned SAMPLE_RATE = 44100;
const unsigned AMPLITUDE = 30000;
const double TWO_PI = 6.28318;
const double increment = 0.01;

int main() {
  // The container of our raw data
  sf::Int16 raw[SAMPLES];

  // Create the data for the sine wave
  double x = 0;
  for (unsigned i = 0; i < SAMPLES; i++) {
    raw[i] = AMPLITUDE * (sin(x * TWO_PI) >= 0.0 ? 1 : 0.5);
    x += increment;
  }

  // Load the data into the sound buffer
  sf::SoundBuffer Buffer;
  Buffer.loadFromSamples(raw, SAMPLES, 1, SAMPLE_RATE);

  // Configure the sound and play
  sf::Sound Sound;
  Sound.setBuffer(Buffer);
  Sound.setLoop(true);
  Sound.setPitch(0.2);
  Sound.play();

  for (double p = 0.2; p < 1.4; p += 0.1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Sound.setPitch(p);
  }
}

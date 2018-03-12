#include <SFML/Audio.hpp>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>

const unsigned SAMPLES = 44100;
const unsigned SAMPLE_RATE = 44100;
const unsigned AMPLITUDE = 30000;
const double TWO_PI = 6.28318;
const double increment = 440.0 / 44100;

std::atomic<uint8_t> audio_channel_1 = 0;

void sound_handler() {
  // The container of our raw data
  sf::Int16 raw[SAMPLES];

  // Create the data for the sine wave
  double x = 0;
  for (unsigned i = 0; i < SAMPLES; i++) {
    raw[i] = AMPLITUDE * sin(x * TWO_PI);
    x += increment;
  }

  // Load the data into the sound buffer
  sf::SoundBuffer Buffer;
  Buffer.loadFromSamples(raw, SAMPLES, 1, SAMPLE_RATE);

  // Configure the sound and play
  sf::Sound Sound;
  Sound.setBuffer(Buffer);
  Sound.setLoop(true);
  Sound.setPitch(audio_channel_1);
  Sound.play();

  while (1) {
    sf::sleep(sf::milliseconds(10));
    Sound.setPitch(static_cast<float>(audio_channel_1) / 16 * 2 + 0.2);
  }
}

int main() {
  using namespace std::chrono_literals;

  std::thread audio_thread(sound_handler);

  while (true) {
    std::this_thread::sleep_for(0.1s);
    audio_channel_1 = (audio_channel_1 + 1) & 0x0F;
  }

  audio_thread.join();

  return 0;
}
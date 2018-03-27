#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>

using namespace std;

std::condition_variable cv;
std::mutex cv_m;

std::atomic<bool> shutdown;

void waits() {
  while (!shutdown) {
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "Waiting...\n";
    cv.wait(lk);
    std::cerr << "Activating...\n\n";
  }

  std::cerr << "Ended waits\n";
}

void signals() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cerr << "Notifying...\n";
  cv.notify_all();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cerr << "Notifying...\n";
  cv.notify_all();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cerr << "Notifying...\n";
  cv.notify_all();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cerr << "Notifying...\n";
  cv.notify_all();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  shutdown = true;
  std::cerr << "Shutting down...\n";
  cv.notify_all();
}

int main() {
  shutdown = false;

  std::thread t1(waits), t2(signals);
  t1.join();
  t2.join();
}

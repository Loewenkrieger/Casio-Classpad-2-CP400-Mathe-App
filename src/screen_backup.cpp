#include "screen_backup.hpp"
#include <cstring>

namespace mathe {
bool ScreenBackup::save(void* screen, Allocate allocate, Release release) {
  if (saved_ || !screen || !allocate || !release) return false;
  void* memory = allocate(bytes);
  if (!memory) return false;
  std::memcpy(memory, screen, bytes);
  saved_ = memory;
  screen_ = screen;
  release_ = release;
  return true;
}

bool ScreenBackup::restore() {
  if (!saved_) return false;
  std::memcpy(screen_, saved_, bytes);
  release_(saved_);
  saved_ = nullptr;
  screen_ = nullptr;
  release_ = nullptr;
  return true;
}
}

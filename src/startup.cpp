#include "screen_backup.hpp"
#include <sdk/calc/calc.h>
#include <sdk/os/lcd.h>
#include <cstdlib>

namespace {
constinit mathe::ScreenBackup originalScreen;
}

// Strong overrides of the SDK's weak hooks, including their priority-99
// constructor/destructor declarations from calc.h. Relocate SDK runs first.
extern "C" __attribute__((used, noinline, externally_visible)) void calcInit() {
  originalScreen.save(vram, std::malloc, std::free);
}

extern "C" __attribute__((used, noinline, externally_visible)) void calcExit() {
  if (originalScreen.restore()) LCD_Refresh();
}

namespace mathe {
bool screenReady() { return originalScreen.ready(); }
}

#pragma once
#include "catalog.hpp"

namespace mathe {
// Uses the task and selected field, never evaluates or changes entered values.
std::vector<std::string> inputHelp(const Operation& operation, int field, bool planar);
}

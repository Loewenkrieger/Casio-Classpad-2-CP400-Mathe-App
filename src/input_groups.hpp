#pragma once
#include "catalog.hpp"

namespace mathe {
struct InputGroup {
  std::string name;
  std::string title;
  std::vector<int> fields;
  bool vector = false;
  int tone = -1;
};

// Coordinate fields stay together. Plain scalar-only forms keep their list.
std::vector<InputGroup> inputGroups(const Operation& operation, bool planar);
int groupForField(const std::vector<InputGroup>& groups, int field);
}

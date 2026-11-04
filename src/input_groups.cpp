#include "input_groups.hpp"
#include <algorithm>

namespace mathe {
namespace {
bool coordinate(const std::string& label) {
  return label.size() > 2 && label[label.size()-2] == '.' &&
    (label.back() == 'x' || label.back() == 'y' || label.back() == 'z');
}

std::string groupTitle(const std::string& name, int number) {
  if (name[0] == 'n') return "Normale " + name;
  if (name[0] == 'M') return "Mittelpunkt " + name;
  if (name[0] >= 'A' && name[0] <= 'Z') return "Punkt " + name;
  return "Vektor " + std::to_string(number) + " (" + name + ")";
}

std::string scalarTitle(const std::string& name) {
  if (name == "k") return "Faktor k";
  if (name == "r" || name == "r1" || name == "r2") return "Radius " + name;
  if (name == "L") return "Laenge L";
  if (name == "d" || name == "d1" || name == "d2") return "Rechte Seite " + name;
  return name;
}
}

std::vector<InputGroup> inputGroups(const Operation& operation, bool planar) {
  if (std::none_of(operation.fields.begin(), operation.fields.end(),
      [](const Field& field) { return coordinate(field.label); })) return {};
  std::vector<InputGroup> groups;
  int ordinal = 0;
  for (std::size_t i = 0; i < operation.fields.size(); ++i) {
    const auto& label = operation.fields[i].label;
    if (!coordinate(label)) {
      groups.push_back({label, scalarTitle(label), {static_cast<int>(i)}, false, -1});
      continue;
    }
    if (planar && operation.planar && label.back() == 'z') continue;
    const std::string name = label.substr(0, label.size()-2);
    if (groups.empty() || !groups.back().vector || groups.back().name != name) {
      ++ordinal;
      groups.push_back({name, groupTitle(name, ordinal), {}, true, (ordinal-1)%4});
    }
    groups.back().fields.push_back(static_cast<int>(i));
  }
  return groups;
}

int groupForField(const std::vector<InputGroup>& groups, int field) {
  for (std::size_t i = 0; i < groups.size(); ++i)
    if (std::find(groups[i].fields.begin(), groups[i].fields.end(), field) != groups[i].fields.end())
      return static_cast<int>(i);
  return -1;
}
}

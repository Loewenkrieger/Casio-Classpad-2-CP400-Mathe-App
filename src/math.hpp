#pragma once
#include <array>
#include <string>
#include <vector>

namespace mathe {
constexpr double eps = 1e-10;
constexpr double pi = 3.14159265358979323846;
struct Vec {
  double x=0, y=0, z=0;
  double operator[](int i) const { return i==0?x:i==1?y:z; }
};
Vec operator+(Vec a, Vec b);
Vec operator-(Vec a, Vec b);
Vec operator*(Vec a, double k);
Vec operator/(Vec a, double k);
double dot(Vec a, Vec b);
Vec cross(Vec a, Vec b);
double norm(Vec a);
Vec unit(Vec a);
bool valid(Vec a);
bool parallel(Vec a, Vec b);
bool perpendicular(Vec a, Vec b);
double angle(Vec a, Vec b, bool acute=false);
std::string number(double x);
std::string vectorText(Vec a);
std::string equation(Vec n, double d);

struct Parsed { bool ok=false; double value=0; std::string error; };
Parsed parse(const std::string& text);

struct Linear {
  int rows=0, cols=0, rank=0;
  bool consistent=true;
  std::array<std::array<double,5>,4> a{};
  std::array<int,4> pivot{{-1,-1,-1,-1}};
  std::array<double,4> particular{};
  std::vector<std::array<double,4>> basis;
  std::vector<std::string> steps;
};
Linear solve(std::array<std::array<double,5>,4> a, int rows, int cols, bool trace=false);
struct Roots { bool all=false; std::vector<double> values; };
Roots quadratic(double a, double b, double c);
struct LinePair {
  enum Kind { Intersect, Skew, Parallel, Identical } kind=Skew;
  double s=0, t=0, distance=0;
  Vec p, q;
};
LinePair linePair(Vec a, Vec u, Vec b, Vec v);
}

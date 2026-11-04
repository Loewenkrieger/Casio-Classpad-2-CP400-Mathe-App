#pragma once
#include "math.hpp"
namespace mathe {
enum class Id {
 Add, Sub, Scale, Length, Normalize, Dot, Cross, Triple, Angle, Relation,
 Displacement, Midpoint, Divide, Collinear, Coplanar, TrianglePoint,
 LineThrough, PlaneThrough, PlaneParam, PlaneCoordinate,
 PointPoint, PointLine, PointPlane, LineLine, LinePlane, PlanePlane,
 ProjectVector, Triangle, Parallelogram, Tetrahedron,
 SpherePoint, SphereLine, SpherePlane, SphereSphere, SphereFour,
 ParamOrthogonal, ParamParallel, ParamLength, ParamPlane, ParamLine, ParamCoplanar,
 Lgs2, Lgs3, Lgs4, Combination, Independence
};
struct Field { std::string label; double initial=0; };
struct Operation {
 Id id; int category; std::string title, formula, method;
 std::vector<Field> fields;
 bool planar=false;
};
const std::vector<std::string>& categories();
const std::vector<Operation>& operations();
const Operation& operation(Id id);
struct Report { bool ok=true; std::vector<std::string> text; };
Report calculate(const Operation& op, const std::vector<double>& values);
std::vector<std::string> lesson(const Operation& op, bool example);
}

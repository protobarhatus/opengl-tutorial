#pragma once
#include "ComposedObject.h"

std::unique_ptr<Object> makePrizm(const std::vector<Vector<2>>& base, double height, const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makeBox(const Vector<3>& half_size, const Vector<3>& pos, const Quat& rot);
std::unique_ptr<Object> makeCone(double height, double rad, const Vector<3>& pos, const Quat& rot);
std::unique_ptr<Object> makePiramid(const std::vector<Vector<2>>& base, double height, const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makeCylinder(double height, double rad, const Vector<3>& position, const Quat& rotation);
std::unique_ptr<Object> makeSphere(double rad, const Vector<3>& position);
std::unique_ptr<Object> makePolyhedron(const std::vector<Vector<3>>& points, const std::vector<std::vector<int>>& edges, const Vector<3>& position, const Quat& rotation);



std::unique_ptr<Object> makeEmptyBoxWithWindowsExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makePolyhedronExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makePolyhedronWithoutCilinderExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makeSimpleIntersectionExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makeBoxCutExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makePolyhedronCutInHalfExample(const Vector<3>& position, const Quat& rotation);

std::unique_ptr<Object> makePlatesWithConeCutExample(const Vector<3>& position, const Quat& rotation);



template<typename T, typename ... Args> struct makeObject {  };

#define DECLARE_MAKE_FUNCTION(type) template<typename ... Args> struct makeObject<type, Args...> { static std::unique_ptr<Object> get(Args... args) { return make##type(args...);} }


DECLARE_MAKE_FUNCTION(Prizm);
DECLARE_MAKE_FUNCTION(Box);
DECLARE_MAKE_FUNCTION(Cone);
DECLARE_MAKE_FUNCTION(Piramid);
DECLARE_MAKE_FUNCTION(Cylinder);
DECLARE_MAKE_FUNCTION(Sphere);
DECLARE_MAKE_FUNCTION(Polyhedron);

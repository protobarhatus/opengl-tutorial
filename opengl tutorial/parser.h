#pragma once
#include "ComposedObject.h"
std::unique_ptr<Object> parse(const std::string& str);
std::string toStringScene(const Object& obj);
#pragma once
#include "ComposedObject.h"
SceneStruct parse(const std::string& str);
std::string toStringScene(const Object& obj, bool place_obj_ = true);
std::string toStringScene(const std::vector<std::unique_ptr<Object>>& obj);
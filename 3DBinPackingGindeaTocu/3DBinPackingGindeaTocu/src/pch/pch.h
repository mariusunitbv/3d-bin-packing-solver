#pragma once

// STL headers
#include <functional>
#include <execution>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>

#include <vector>
#include <string>
#include <stack>
#include <queue>

#include <unordered_map>
#include <map>

// External libraries
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <implot.h>

#include <raylib.h>
#include "../rl_imgui/rlImGui.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <nlohmann/json.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#endif

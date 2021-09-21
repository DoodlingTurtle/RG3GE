#pragma once 

namespace RG3GE::Global {

#define REGISTER_GLOBAL(t, n, v) extern t n;
#include "../engine_globals.hpp"
#include "./misc/global.hpp"

}

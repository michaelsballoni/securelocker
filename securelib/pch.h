#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "SHA256.h"
#include "Blowfish.h"
#include "crossguid/guid.hpp"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>

#include "Core.h"
#pragma comment(lib, "httplite")

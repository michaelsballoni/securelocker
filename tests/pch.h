#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // gethostbyname, etc.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include "Core.h"
#pragma comment(lib, "httplite")

#pragma comment(lib, "securelib")

#pragma once

#define JSON_SKIP_UNSUPPORTED_COMPILER_CHECK
#define JSON_SKIP_LIBRARY_VERSION_CHECK
#include "thirdparty\json.hpp"
using json = nlohmann::json;

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

inline void LuinuxAssert(bool assrt, std::string str)
{
    if (assrt)
        return;
    throw std::runtime_error(str);
}
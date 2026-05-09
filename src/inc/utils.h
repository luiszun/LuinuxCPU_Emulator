#pragma once
#include <iostream>
#include <sstream>
#include <vector>

std::string HexEscapedString(const std::vector<uint8_t> &bytes) {
  std::ostringstream out;

  out << std::hex << std::setfill('0');

  for (uint8_t byte : bytes) {
    out << "\\x" << std::setw(2) << static_cast<unsigned>(byte);
  }

  return out.str();
}
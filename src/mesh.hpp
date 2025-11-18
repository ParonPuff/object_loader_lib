#pragma once
#include <vector>
#include <cstdint>
#include <type.hpp>

struct Mesh{
  std::vector<matvec::vec3> positions;
  std::vector<matvec::vec3> normals;
  // std:vector<matvec::vec2> uv;
  std::vector<uint32_t> indices;
};

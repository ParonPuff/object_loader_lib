#pragma once
#include <cstdint>
#include <types.hpp>
#include <vector>

struct Mesh {
  std::vector<matvec::vec3> positions;
  std::vector<matvec::vec3> normals;
  std::vector<matvec::vec3> spherePoint;
  std::vector<uint32_t> indices;
};

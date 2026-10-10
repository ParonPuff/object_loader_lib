#pragma once
#include "mesh.hpp"
#include <string>

class ObjLoader {
public:
  static Mesh load(const std::string &file_path, bool normalize = true);
  static void computeSpherePoint(Mesh &mesh);
  static void computeNormals(Mesh &mesh);
  static void normalizeMesh(Mesh &mesh);
};

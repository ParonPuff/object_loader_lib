#pragma once
#include <string>
#include "mesh.hpp"

class ObjLoader {
  public:
    static Mesh load(const std::string& file_path, bool normalize = true);
    static void normalizeMesh(Mesh& mesh);
};

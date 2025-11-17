#include "obj_loader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Mesh ObjLoader::load(const std::string& path)
{
  Mesh mesh;
  
  std::ifstream file(path);
  if(!file.is_open())
    throw std::runtime_error("Could not open OBJ file: " + path);

  std::string line;
  while (std::getline(file,line)) {

    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if(prefix == "v") {
      float x,y,z;
      ss >> x >> y >> z;
      mesh.positions.emplace_back(x,y,z);
    }
    else if (prefix == "vn"){
      float x,y,z;
      ss >> x >> y >> z;
      mesh.normals.emplace_back(x,y,z);
    }
    else if (prefix == "vt"){
      float u,v;
      ss >> u >> v;
      //mesh.uvs.emplace_back(x,y,z);
      // TODO: handle uv's
    }
    else if (prefix == "f") {
      // TODO: handle faces
    }
  }

  return mesh;
}

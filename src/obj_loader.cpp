#include "../include/obj_loader.hpp"
#include "vec2.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>

Mesh ObjLoader::load(const std::string &path, bool normalize) {
  Mesh mesh;

  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Could not open OBJ file: " + path);

  std::string line;
  while (std::getline(file, line)) {

    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "v") {
      float x, y, z;
      ss >> x >> y >> z;
      mesh.positions.emplace_back(x, y, z);
    } else if (prefix == "vn") {
      float x, y, z;
      ss >> x >> y >> z;
      mesh.normals.emplace_back(x, y, z);
    } else if (prefix == "vt") {
      float u, v;
      ss >> u >> v;
      // mesh.uvs.emplace_back(x,y,z);
      //  TODO: handle uv's
    } else if (prefix == "f") {
      // read
      std::vector<uint32_t> verts;
      std::string token;
      while (ss >> token) {
        std::stringstream fs(token);

        std::string vStr;
        std::getline(fs, vStr, '/');
        verts.push_back(std::stoi(vStr) - 1);
      }

      // triangulate
      for (size_t i = 1; i + 1 < verts.size(); ++i) {
        mesh.indices.insert(mesh.indices.end(),
                            {verts[0], verts[i], verts[i + 1]});
      }
    }
  }
  if (mesh.normals.empty())
    ObjLoader::computeNormals(mesh);
  if (mesh.uv.empty())
    ObjLoader::computeUV(mesh);
  if (normalize)
    ObjLoader::normalizeMesh(mesh);

  return mesh;
}

void ObjLoader::computeUV(Mesh &mesh) {
  for (auto p : mesh.positions) {
    matvec::vec3 norm_p = p.normalized();

    float u = 0.5f + atan2(p.z, p.x) / (2.0f * M_PI);
    float v = 0.5f - asin(p.y) / M_PI;

    mesh.uv.emplace_back(matvec::vec2(u, v));
  }
}

void ObjLoader::computeNormals(Mesh &mesh) {
  for (auto p : mesh.positions) {
    mesh.normals.emplace_back(matvec::vec3()); // initialize normals to 0
  }

  for (int i = 0; i < mesh.indices.size(); i += 3) {
    matvec::vec3 v0_pos = mesh.positions[mesh.indices[i]];
    matvec::vec3 v1_pos = mesh.positions[mesh.indices[i + 1]];
    matvec::vec3 v2_pos = mesh.positions[mesh.indices[i + 2]];

    matvec::vec3 e1 = v1_pos - v0_pos;
    matvec::vec3 e2 = v2_pos - v0_pos;
    matvec::vec3 faceNormal = e1.cross(e2).normalized();

    mesh.normals[mesh.indices[i]] = mesh.normals[mesh.indices[i]] + faceNormal;
    mesh.normals[mesh.indices[i + 1]] =
        mesh.normals[mesh.indices[i + 1]] + faceNormal;
    mesh.normals[mesh.indices[i + 2]] =
        mesh.normals[mesh.indices[i + 2]] + faceNormal;
  }

  for (auto &n : mesh.normals)
    n = n.normalized();
}

void ObjLoader::normalizeMesh(Mesh &mesh) {
  if (mesh.positions.empty())
    return;

  // compute bounds
  matvec::vec3 min = mesh.positions[0];
  matvec::vec3 max = mesh.positions[0];

  for (const auto &p : mesh.positions) {
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    min.z = std::min(min.z, p.z);

    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    max.z = std::max(max.z, p.z);
  }

  // center
  matvec::vec3 center = (min + max) * 0.5f;
  for (auto &p : mesh.positions)
    p = p - center;

  // scale (unit cube)
  matvec::vec3 size = max - min;
  float largest = std::max({size.x, size.y, size.z});
  float scale = 1.0f / largest;

  for (auto &p : mesh.positions)
    p = p * scale;
}

#include "../include/obj_loader.hpp"
#include "vec2.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

Mesh ObjLoader::load(const std::string &path, bool normalize) {
  Mesh mesh;

  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Could not open OBJ file: " + path);

  std::string line;
  mesh.positions.clear();
  mesh.indices.clear();
  mesh.normals.clear();
  // mesh.uv.clear();
  mesh.spherePoint.clear();
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
        mesh.indices.insert(mesh.indices.end(), {verts[0], verts[i], verts[i + 1]});
      }
    }
  }
  mesh.normals.clear();
  //always recompute normals
  ObjLoader::computeNormals(mesh);
  if (normalize)
    ObjLoader::normalizeMesh(mesh);
  // if (mesh.uv.empty())
  if (mesh.spherePoint.empty())
    ObjLoader::computeSpherePoint(mesh);

  return mesh;
}

void ObjLoader::computeSpherePoint(Mesh &mesh) {
  float radius = 0;
  for (int i = 0; i < mesh.positions.size(); i++) {
    float distanceFromOrigin = mesh.positions[i].length();
    if (distanceFromOrigin > radius) {
      radius = distanceFromOrigin;
    }
  }
  radius += 10.0f;
  mesh.spherePoint.clear();

  for (int i = 0; i < mesh.positions.size(); i++) {
    // calculate the three coefficients of the distance equation
    float a, b, c;
    a = mesh.normals[i].dot(mesh.normals[i]);
    b = 2 * mesh.positions[i].dot(mesh.normals[i]);
    c = mesh.positions[i].dot(mesh.positions[i]) - radius * radius;

    float q, d1, d2, dPositive;
    q = 0.5f * (b + (b >= 0 ? 1 : -1) * sqrt(b * b - 4 * a * c));
    d1 = q / a;
    d2 = c / q;
    dPositive = d1 > 0 ? d1 : d2;
    assert(dPositive > 0);

    // calculate texture coordinates corresponding to pointOnSphere
    matvec::vec3 pointOnSphere = mesh.positions[i] + (mesh.normals[i] * dPositive);

    //rest is done in frag shader
    mesh.spherePoint.emplace_back(pointOnSphere);
  }
}

void ObjLoader::computeNormals(Mesh &mesh) {
  mesh.normals.assign(mesh.positions.size(), matvec::vec3(0.f, 0.f, 0.f));

  for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1],
         i2 = mesh.indices[i + 2];

    matvec::vec3 e1 = mesh.positions[i1] - mesh.positions[i0];
    matvec::vec3 e2 = mesh.positions[i2] - mesh.positions[i0];

    matvec::vec3 faceNormal = e1.cross(e2);

    mesh.normals[i0] = mesh.normals[i0] + faceNormal;
    mesh.normals[i1] = mesh.normals[i1] + faceNormal;
    mesh.normals[i2] = mesh.normals[i2] + faceNormal;
  }

  for (auto &n : mesh.normals) {
    float len = n.length();
    n = len > 1e-12f ? n * (1.0f / len) : matvec::vec3(0.f, 1.f, 0.f);
  }
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

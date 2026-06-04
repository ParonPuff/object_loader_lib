#include "../include/obj_loader.hpp"
#include "vec2.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

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
  for (int i = 0; i < mesh.positions.size(); i++) {
    matvec::vec3 V0 = mesh.positions[i];
    matvec::vec3 N  = mesh.normals[i].normalized();

    float r = 1.0f; // sphere radius (larger than object)

    // --- Second step: intersect normal ray with intermediate sphere ---
    // Ray: P = V0 + d*N
    // Sphere: P·P = r²  =>  ad² + bd + c = 0
    float a = N.dot(N);           // = 1 if N is normalized
    float b = 2.0f * V0.dot(N);
    float c = V0.dot(V0) - r * r;

    float discriminant = b * b - 4.0f * a * c;
    float d_plus;

    if (discriminant < 0.0f) {
      // Fallback: normalize position directly
      matvec::vec3 norm_p = V0.normalized();
      float u = 0.5f + atan2(norm_p.z, norm_p.x) / (2.0f * M_PI);
      float v = acos(-norm_p.y) / M_PI;
      mesh.uv.emplace_back(matvec::vec2(u, v));
      continue;
    }

    // Use the numerically stable quadratic solution from slide 27
    float q = -0.5f * (b + (b >= 0 ? 1.0f : -1.0f) * sqrt(discriminant));
    float d1 = q / a;
    float d2 = c / q;

    // Pick the positive root (in the direction of the normal)
    d_plus = (d1 > 0.0f) ? d1 : d2;

    // Intersection point P on the sphere
    matvec::vec3 P = V0 + N * d_plus;

    // Normalize P onto sphere surface (slide 28)
    matvec::vec3 Ps = P.normalized() * r;

    // --- Compute (s,t) from spherical coordinates (slide 29) ---
    float s = acosf(Ps.x / r) / M_PI;
    float t = atan2f(Ps.z, Ps.y) / M_PI + 0.5f;

    mesh.uv.emplace_back(matvec::vec2(s, t));
  }
  for (int i = 0; i < mesh.indices.size(); i += 3) {
    int i0 = mesh.indices[i];
    int i1 = mesh.indices[i + 1];
    int i2 = mesh.indices[i + 2];

    float u0 = mesh.uv[i0].x;
    float u1 = mesh.uv[i1].x;
    float u2 = mesh.uv[i2].x;

    // If any edge spans more than 0.5 in u, the triangle crosses the seam
    // Shift the "low" vertex up by 1 so all three are on the same side
    if (fabsf(u1 - u0) > 0.5f || fabsf(u2 - u0) > 0.5f || fabsf(u2 - u1) > 0.5f) {
      // Find the average u; vertices below 0.5 should be shifted to >1
      float avg = (u0 + u1 + u2) / 3.0f;

      // Duplicate affected vertices with corrected UVs rather than
      // modifying shared vertices (which would break other triangles)
      auto fixVertex = [&](int idx, float u) -> int {
        if ((avg > 0.5f && u < 0.5f) || (avg < 0.5f && u > 0.5f)) {
          // Create a new vertex copy with the shifted UV
          mesh.positions.push_back(mesh.positions[idx]);
          mesh.normals.push_back(mesh.normals[idx]);
          float newU = (u < 0.5f) ? u + 1.0f : u - 1.0f;
          mesh.uv.emplace_back(matvec::vec2(newU, mesh.uv[idx].y));
          return (int)mesh.positions.size() - 1;
        }
        return idx;
      };

      mesh.indices[i]     = fixVertex(i0, u0);
      mesh.indices[i + 1] = fixVertex(i1, u1);
      mesh.indices[i + 2] = fixVertex(i2, u2);
    }
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

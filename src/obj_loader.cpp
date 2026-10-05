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
        mesh.indices.insert(mesh.indices.end(),
                            {verts[0], verts[i], verts[i + 1]});
      }
    }
  }
  mesh.normals.clear();
  ObjLoader::computeNormals(mesh);
  if (normalize)
    ObjLoader::normalizeMesh(mesh);
  // if (mesh.uv.empty())
  if (mesh.spherePoint.empty())
    ObjLoader::computeUV(mesh);

  return mesh;
}
/* computeUV is accomplished by utilizing the two part texture mapping, first,
 * 1. compute a sphere enclosing the object
 * 2. compute intersection of object normal with spehere
 * 3. compute mapping from sphere to texture
 */

void ObjLoader::computeUV(Mesh &mesh) {
  // calculate the radius of the intermediate sphere as the distance furthest
  // from the origin among the points of the mesh
  // NOTE: this will result in the mesh intersecting with the sphere at atleast
  // one point since the sphere is not strictly larger than the mesh

  float radius = 0;
  for (int i = 0; i < mesh.positions.size(); i++) {
    float distanceFromOrigin = mesh.positions[i].length();
    if (distanceFromOrigin > radius) {
      radius = distanceFromOrigin;
    }
  }
  radius += 10.0f;
  // mesh.uv.clear();
  mesh.spherePoint.clear();

  std::cout << "radius: " << radius << "\n";
  for (int i = 0; i < mesh.positions.size(); i++) {
    // calculate the three coefficients of the distance equation
    float a, b, c;
    a = mesh.normals[i].dot(mesh.normals[i]);
    b = 2 * mesh.positions[i].dot(mesh.normals[i]);
    c = mesh.positions[i].dot(mesh.positions[i]) - radius * radius;
    /*
        float disc = std::max(0.f, b * b - 4 * a * c);
        float d = (-b + std::sqrt(disc)) / (2 * a); // c <= 0 so this root is >=
       0

        matvec::vec3 p = mesh.positions[i] + mesh.normals[i] * d;

        float s = std::atan2(p.z, p.x) / (2.f * M_PI) + 0.5f;            //
       longitude (u) float t = std::acos(std::clamp(p.y / radius, -1.f, 1.f)) /
       M_PI;  // latitude (v)

        std::cout << "--------------------------------------------------" <<
       "\n"; std::cout << "vx: " << mesh.positions[i].x << " vy: " <<
       mesh.positions[i].y
                  << " vz: " << mesh.positions[i].z << "\n";

        std::cout << "nx: " << mesh.normals[i].x << " ny: " << mesh.normals[i].y
                  << " vz: " << mesh.normals[i].z << "\n";
        std::cout << "px: " << p.x << " py: " << p.y << " pz: " << p.z << "\n";
        std::cout << "s: " << s << " t: " << t << "\n";
    */
    // solve f(d)
    float q, d1, d2, dPositive;
    q = 0.5f * (b + (b >= 0 ? 1 : -1) * sqrt(b * b - 4 * a * c));
    d1 = q / a;
    d2 = c / q;
    // NOTE: the point which intersects with the sphere should have a distance
    // of 0 hence why i treat 0 as positive
    dPositive = d1 > 0 ? d1 : d2;
    // if no root of the equation is positive something is wrong and i want to
    // know
    assert(dPositive > 0);

    // calculate texture coordinates corresponding to pointOnSphere
    matvec::vec3 pointOnSphere =
        mesh.positions[i] + (mesh.normals[i] * dPositive);

    float s = std::atan2(pointOnSphere.z, pointOnSphere.x) / (2.f * M_PI) +
              0.5f; // longitude (u)
    float t = std::acos(std::clamp(pointOnSphere.y / radius, -1.f, 1.f)) /
              M_PI; // latitude (v)

    // mesh.uv.emplace_back(matvec::vec2(s, t));
    mesh.spherePoint.emplace_back(pointOnSphere);
  }

  /*
  for (int i = 0; i < mesh.positions.size(); i++) {
    matvec::vec3 V0 = mesh.positions[i];
    matvec::vec3 N = mesh.normals[i].normalized();

    float r = 1.0f; // sphere radius (larger than object)

    // --- Second step: intersect normal ray with intermediate sphere ---
    // Ray: P = V0 + d*N
    // Sphere: P·P = r²  =>  ad² + bd + c = 0
    float a = N.dot(N); // = 1 if N is normalized
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

    // Longitude
    float u = atan2f(Ps.z, Ps.x) / (2.0f * M_PI) + 0.5f;

    // Latitude
    float v = 0.5f - asinf(Ps.y) / M_PI;

    // Wrap u into [0,1)
    u -= floorf(u);

    mesh.uv.emplace_back(matvec::vec2(u, v));
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
    if (fabsf(u1 - u0) > 0.5f || fabsf(u2 - u0) > 0.5f ||
        fabsf(u2 - u1) > 0.5f) {
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

      mesh.indices[i] = fixVertex(i0, u0);
      mesh.indices[i + 1] = fixVertex(i1, u1);
      mesh.indices[i + 2] = fixVertex(i2, u2);
    }
  }
  */
}

void ObjLoader::computeNormals(Mesh &mesh) {
  mesh.normals.assign(mesh.positions.size(), matvec::vec3(0.f, 0.f, 0.f));

  for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1],
         i2 = mesh.indices[i + 2];

    matvec::vec3 e1 = mesh.positions[i1] - mesh.positions[i0];
    matvec::vec3 e2 = mesh.positions[i2] - mesh.positions[i0];

    // Deliberately NOT normalized: length = 2 * area, so larger triangles
    // weigh more, and degenerate triangles contribute exactly zero.
    matvec::vec3 faceNormal = e1.cross(e2);

    mesh.normals[i0] = mesh.normals[i0] + faceNormal;
    mesh.normals[i1] = mesh.normals[i1] + faceNormal;
    mesh.normals[i2] = mesh.normals[i2] + faceNormal;
  }

  for (auto &n : mesh.normals) {
    float len = n.length();
    // Unreferenced vertex or perfectly cancelling normals: pick a fallback
    // instead of dividing by zero.
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

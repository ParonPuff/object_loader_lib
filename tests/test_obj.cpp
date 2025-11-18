#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <obj_loader.hpp>
#include <mesh.hpp>

#include <filesystem>

TEST_CASE("OBJ loader loads a simple triangle") {
  std::string path = std::filesystem::path(TEST_FIXTURES_DIR) / "triangle.obj";

  Mesh mesh = ObjLoader::load(path,false);

  REQUIRE(mesh.positions.size() == 3);
  REQUIRE(mesh.indices.size() == 3);

  REQUIRE(mesh.positions[0].x == 0.0f);
  REQUIRE(mesh.positions[1].x == 1.0f);
  REQUIRE(mesh.positions[2].y == 1.0f);

  REQUIRE(mesh.indices[0] == 0);
  REQUIRE(mesh.indices[1] == 1);
  REQUIRE(mesh.indices[2] == 2);
}

TEST_CASE("OBJ loader loads quad") {
  std::string path = std::filesystem::path(TEST_FIXTURES_DIR) / "quad.obj";

  Mesh mesh = ObjLoader::load(path, false);

  REQUIRE(mesh.positions.size() == 4);
  REQUIRE(mesh.indices.size() == 6);

  REQUIRE(mesh.positions[0].x == 0.0f);
  REQUIRE(mesh.positions[1].x == 1.0f);
  REQUIRE(mesh.positions[2].y == 1.0f);
  REQUIRE(mesh.positions[3].y == 1.0f);

  REQUIRE(mesh.indices[0] == 0);
  REQUIRE(mesh.indices[1] == 1);
  REQUIRE(mesh.indices[2] == 2);

  REQUIRE(mesh.indices[3] == 0);
  REQUIRE(mesh.indices[4] == 2);
  REQUIRE(mesh.indices[5] == 3);
}

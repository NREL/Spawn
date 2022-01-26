#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"
#include "../util/unzipped_file.hpp"
#include "paths.hpp"

#include <array>
#include <catch2/catch.hpp>

TEST_CASE("Test unzipped file")
{
  spawn::util::Temp_Directory td{};

  REQUIRE(spawn_fs::exists(testzip()));
  REQUIRE(spawn_fs::is_regular_file(testzip()));

  const auto out_file = td.dir() / "a_dir/a_file.data";
  spawn::util::Unzipped_File unzippedFile{testzip(), td.dir(), {"a_dir/a_file.data"}};

  REQUIRE(spawn_fs::exists(out_file));
  REQUIRE(spawn_fs::is_regular_file(out_file));

  std::ifstream ifs(out_file.string(), std::ios_base::binary);

  constexpr auto buffer_size = 256;
  std::array<char, buffer_size> data{};
  ifs.read(data.data(), data.size());

  REQUIRE(ifs.gcount() == 20);

  constexpr std::array<char, 20> expected_data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  CHECK(std::equal(std::begin(expected_data), std::end(expected_data), std::begin(data)));
}

TEST_CASE("Test missing zip file")
{
  spawn::util::Temp_Directory td{};

  const spawn_fs::path missing_file{"missing_file.zip"};

  REQUIRE(!spawn_fs::exists(missing_file));

  const auto out_file = td.dir() / "a_dir/a_file.data";

  REQUIRE_THROWS_WITH((spawn::util::Unzipped_File{missing_file, td.dir(), {"a_dir/a_file.data"}}),
                      Catch::Contains("Error opening zipfile") && Catch::Contains("missing_file.zip"));
}

TEST_CASE("Test missing file in zip")
{
  spawn::util::Temp_Directory td{};

  REQUIRE(spawn_fs::exists(testzip()));
  REQUIRE(spawn_fs::is_regular_file(testzip()));

  const auto out_file = td.dir() / "a_dir/b_file.data";
  REQUIRE_THROWS_WITH((spawn::util::Unzipped_File{testzip(), td.dir(), {"a_dir/b_file.data"}}),
                      Catch::Contains("Error opening file in zip") && Catch::Contains("a_dir/b_file.data"));
}

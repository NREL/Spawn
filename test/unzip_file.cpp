
#include "../util/temp_directory.hpp"
#include "../util/unzipped_file.hpp"
#include "testpaths.hpp"
#include <catch2/catch.hpp>
#include <array>

TEST_CASE("Test unzipped file")
{
  spawn::util::Temp_Directory td{};

  REQUIRE(boost::filesystem::exists(testzip()));
  REQUIRE(boost::filesystem::is_regular(testzip()));

  const auto out_file = td.dir() / "a_dir/a_file.data";
  spawn::util::Unzipped_File unzippedFile{testzip(), td.dir(), {"a_dir/a_file.data"}};

  REQUIRE(boost::filesystem::exists(out_file));
  REQUIRE(boost::filesystem::is_regular(out_file));

  std::ifstream ifs(out_file.string(), std::ios_base::binary);

  constexpr auto buffer_size = 256;
  std::array<char, buffer_size> data{};
  ifs.read(data.data(), data.size());

  REQUIRE(ifs.gcount() == 20);

  constexpr char expected_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  CHECK(std::equal(std::begin(expected_data), std::end(expected_data), std::begin(data)));
}

TEST_CASE("Test missing zip file")
{
  spawn::util::Temp_Directory td{};

  const boost::filesystem::path missing_file{"missing_file.zip"};

  REQUIRE(!boost::filesystem::exists(missing_file));

  const auto out_file = td.dir() / "a_dir/a_file.data";

  REQUIRE_THROWS_WITH((spawn::util::Unzipped_File{missing_file, td.dir(), {"a_dir/a_file.data"}}),
                      Catch::Contains("Error opening zipfile") && Catch::Contains("missing_file.zip"));
}

TEST_CASE("Test missing file in zip")
{
  spawn::util::Temp_Directory td{};

  REQUIRE(boost::filesystem::exists(testzip()));
  REQUIRE(boost::filesystem::is_regular(testzip()));

  const auto out_file = td.dir() / "a_dir/b_file.data";
  REQUIRE_THROWS_WITH((spawn::util::Unzipped_File{testzip(), td.dir(), {"a_dir/b_file.data"}}),
                      Catch::Contains("Error opening file in zip") && Catch::Contains("a_dir/b_file.data"));
}

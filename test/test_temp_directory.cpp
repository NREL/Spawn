#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Temp_Directory is created and destroyed")
{
  SECTION("Test empty directory")
  {
    fs::path temp_path{};

    SECTION("empty directory created")
    {
      spawn::util::Temp_Directory td;
      temp_path = td.dir();
      CHECK(fs::exists(temp_path));
      CHECK(fs::is_directory(temp_path));
    }

    SECTION("empty directory destroyed")
    {
      CHECK_FALSE(fs::exists(temp_path));
    }
  }

  SECTION("Test directory with files")
  {
    fs::path temp_path{};

    SECTION("empty directory created")
    {
      spawn::util::Temp_Directory td;
      temp_path = td.dir();
      CHECK(fs::exists(temp_path));
      CHECK(fs::is_directory(temp_path));

      const auto temp_file = td.dir() / "a_file.txt";

      SECTION("create file in temp directory")
      {
        std::ofstream ofs{temp_file.string()};
        ofs << "Hello World";
      }

      CHECK(fs::exists(temp_file));
      CHECK(fs::is_regular_file(temp_file));
    }

    SECTION("directory with files destroyed")
    {
      CHECK_FALSE(fs::exists(temp_path));
    }
  }
}

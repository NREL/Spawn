#include "../util/temp_directory.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Temp_Directory is created and destroyed")
{
  SECTION("Test empty directory")
  {
    boost::filesystem::path temp_path{};

    SECTION("empty directory created")
    {
      spawn::util::Temp_Directory td;
      temp_path = td.dir();
      CHECK(boost::filesystem::exists(temp_path));
      CHECK(boost::filesystem::is_directory(temp_path));
    }

    SECTION("empty directory destroyed")
    {
      CHECK_FALSE(boost::filesystem::exists(temp_path));
    }
  }

  SECTION("Test directory with files")
  {
    boost::filesystem::path temp_path{};

    SECTION("empty directory created")
    {
      spawn::util::Temp_Directory td;
      temp_path = td.dir();
      CHECK(boost::filesystem::exists(temp_path));
      CHECK(boost::filesystem::is_directory(temp_path));

      const auto temp_file = td.dir() / "a_file.txt";

      SECTION("create file in temp directory")
      {
        std::ofstream ofs{temp_file.string()};
        ofs << "Hello World";
      }

      CHECK(boost::filesystem::exists(temp_file));
      CHECK(boost::filesystem::is_regular(temp_file));
    }

    SECTION("directory with files destroyed")
    {
      CHECK_FALSE(boost::filesystem::exists(temp_path));
    }
  }
}
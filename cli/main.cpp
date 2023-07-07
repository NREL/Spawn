#include "cli/cli.hpp"
#include <spdlog/cfg/env.h>

void handle_eptr(std::exception_ptr eptr)
{
  try {
    if (eptr) {
      std::rethrow_exception(std::move(eptr));
    }
  } catch (const std::exception &e) {
    fmt::print("Spawn encountered an error:\n\"{}\"\n", e.what());
  }
}

int main(int argc, const char *argv[]) // NOLINT exception may escape from main
{
  spdlog::cfg::load_env_levels();

  std::exception_ptr eptr;

  try {
    spawn::cli::CLI cli;
    cli.parse(argc, argv);
  } catch (...) {
    eptr = std::current_exception();
  }

  if (eptr) {
    handle_eptr(eptr);
    return 1;
  } else {
    return 0;
  }
}

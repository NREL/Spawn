#include <errno.h>
#include <string.h>
#include <zip.h>

bool is_dir(const std::string& dir);

void walk_directory(const std::string& startdir, const std::string& inputdir, zip_t *zipper, bool no_compression);

void zip_directory(const std::string& inputdir, const std::string& output_filename, bool no_compression);


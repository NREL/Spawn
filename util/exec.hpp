#include <iostream>
#include <cstdarg>
#include <string>
#include <fstream>
#include <memory>
#include <cstdio>

namespace spawn {

inline std::string exec(const char* cmd) {
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

} // namespace spawn

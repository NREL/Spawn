#include "compare.hpp"

namespace spawn {

bool case_insensitive_compare(std::string s1, std::string s2) {
   //convert s1 and s2 into lower case strings
   transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
   transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
   return (s1.compare(s2) == 0);
}

}

#include <string>
#include <vector>

namespace spawn::open_modelica {

// Parse strings of the form:
// {"token1","token2", ...}
// which are commonly returned by OpenModelica
// return vector of string tokens
inline std::vector<std::string> parse_command_response(std::string_view response)
{
  std::string structured_text{response};

  // Removing the curly braces from the structured text
  structured_text = structured_text.substr(1, structured_text.length() - 2);

  // Tokenizing the structured text using comma as the delimiter
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string delimiter = ",";

  while ((pos = structured_text.find(delimiter)) != std::string::npos) {
    std::string token = structured_text.substr(0, pos);

    tokens.push_back(token);
    structured_text.erase(0, pos + delimiter.length());
  }

  // Adding the last token
  tokens.push_back(structured_text);

  // Remove leading and trailing '"'
  for (auto &token : tokens) {
    // Remove leading '"'
    if (!token.empty() && token.front() == '"') {
      token.erase(token.begin());
    }

    // Remove trailing '"'
    if (!token.empty() && token.back() == '"') {
      token.pop_back();
    }
  }

  return tokens;
}

} // namespace spawn::open_modelica

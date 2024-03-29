#include "idf_to_json.hpp"
#include <fstream>
#include <string>

using json = nlohmann::json;

namespace spawn {

json idf_to_json(const spawn_fs::path &idfpath)
{
  std::ifstream input_stream(idfpath.string(), std::ifstream::in);

  std::string input_file;
  std::string line;
  while (std::getline(input_stream, line)) {
    input_file.append(line + '\n');
  }

  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema);

  return parser.decode(input_file, schema);
}

void json_to_idf(const json &jsonidf, const spawn_fs::path &idfpath)
{
  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema);

  spawn_fs::create_directories(idfpath.parent_path());
  std::ofstream newidfstream(idfpath.string(), std::ofstream::out | std::ofstream::trunc);
  newidfstream << parser.encode(jsonidf, schema);
  newidfstream.close();
}

} // namespace spawn

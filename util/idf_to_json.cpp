#include "idf_to_json.hpp"
#include <string>
#include <fstream>

using json = nlohmann::json;

namespace spawn {

json idfToJSON(const std::filesystem::path & idfpath) {
  std::ifstream input_stream(idfpath.string(), std::ifstream::in);

  std::string input_file;
  std::string line;
  while (std::getline(input_stream, line)) {
    input_file.append(line + EnergyPlus::DataStringGlobals::NL);
  }

  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
  return parser.decode(input_file, schema);
}

void jsonToIdf(const json & jsonidf, const std::filesystem::path & idfpath) {
  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);

  std::ofstream newidfstream(idfpath.string(),  std::ofstream::out |  std::ofstream::trunc);
  newidfstream << parser.encode(jsonidf, schema);
  newidfstream.close();
}

} // namespace spawn


#ifndef COMPILERCHAIN_HPP_INCLUDED
#define COMPILERCHAIN_HPP_INCLUDED

#include <string>
#include <boost/filesystem.hpp>

namespace spawn {

int compileMO(
  const std::string & moInput,
  const boost::filesystem::path & outputDir,
  const boost::filesystem::path & mblPath,
  const boost::filesystem::path & jmodelicaHome
);

int compileC(const boost::filesystem::path & output_dir);

int modelicaToFMU(
  const std::string &moinput,
  const boost::filesystem::path & mblPath,
  const boost::filesystem::path & jmodelicaHome
);

} // namespace spawn

#endif // COMPILERCHAIN_HPP_INCLUDED


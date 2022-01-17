
#include <fstream>
#include <iostream>
#include <string>

#include <array>
#include <cassert>
#include <cstdio>
#include <zlib.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

int main(int argc, char *argv[])
{
  if (argc != 5) {
    return 1;
  }

  auto *infile = argv[1];
  auto *outfile = argv[2];
  auto *filenum = argv[3];
  auto *embeddedname = argv[4];

  int ret = 0;
  int flush = 0;
  unsigned have = 0;
  z_stream strm;
  std::array<unsigned char, CHUNK> in;
  std::array<unsigned char, CHUNK> out;

  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK) return 1;

  FILE *source = fopen(infile, "rb");
  if (source == NULL) {
    fputs("File error", stderr);
    return EXIT_FAILURE;
  }

  std::fstream outstream(outfile, std::fstream::out | std::fstream::trunc);

  // This is the compressed length in chars;
  unsigned length = 0;

  if (outstream.is_open()) {
    outstream << "static const uint8_t embedded_file_" << filenum << "[] = {";
    do {
      strm.avail_in = static_cast<uInt>(fread(in.data(), 1, CHUNK, source));
      if (ferror(source) != 0) {
        [[maybe_unused]] const auto result = deflateEnd(&strm);
        return EXIT_FAILURE;
      }
      flush = feof(source) != 0 ? Z_FINISH : Z_NO_FLUSH;
      strm.next_in = in.data();

      /* run deflate() on input until output buffer not full, finish
         compression if all of source has been read in */
      do {
        strm.avail_out = CHUNK;
        strm.next_out = out.data();
        ret = deflate(&strm, flush);   /* no bad return value */
        assert(ret != Z_STREAM_ERROR); /* state not clobbered */
        have = CHUNK - strm.avail_out;

        for (unsigned i = 0; i != have; ++i) {
          if (length != 0) {
            outstream << ",";
          }
          outstream << "0x" << std::hex << static_cast<int>(out[i]);
          ++length;
        }
      } while (strm.avail_out == 0);
      assert(strm.avail_in == 0); /* all input will be used */

      /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END); /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);

    outstream << "};";

    outstream << "\n";
    outstream << "static const char *embedded_file_name_" << filenum << " = \"" << embeddedname << "\";";
    outstream << "\n";
    outstream << "static const size_t embedded_file_len_" << filenum << " = " << std::dec << length << ";";
    outstream << std::endl;

    outstream.close();
    fclose(source);
  } else {
    std::cout << "Could not open '" << outfile << "' for writing" << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}

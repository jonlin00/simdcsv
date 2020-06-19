#include <Error.h>
#include <fmt/format.h>
#include <IO/Filepath.h>
#include <IO/CSVReader.h>

using namespace Wikinger;

class CSV_Clb {
public:
  void operator()(Error& err, uint32_t row, uint32_t col, CSVReader::Token& tk) {
#if WK_BUILD_DEBUG
    WK_DEBUG("CSV: {},{}: {}", row, col, tk.get<std::string_view>(err));
    
    mr = mr > row ? mr : row;
    mc = mc > col ? mc : col;
#endif
  }

private:
  uint32_t mr = 0;
  uint32_t mc = 0;
};

int main(int argc, char** argv) {
  Log::init();

  {
    Error err;

    CSVReader reader;
    //reader.open(err, "assets/worldcitiespop.csv");
    //reader.open(err, "assets/test.csv");
    //reader.setSep(';');

    reader.open(err, "C:/Source/Matlab/ODE/odesol2.csv");
    CSV_Clb clb;
    reader.read(err, clb);

    reader.close();

  }
  
  WK_INFO("Done!");
  return getchar();
  //return 0;
}
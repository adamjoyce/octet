// A basic csv file parser.
// std::getline() has no definition including octet::containers::string as a parameter type.
// For simplicity I have used std::string throughout.
//
// Author: Adam Joyce
// Version 1.05

// for std::ifstream
#include <fstream>
// for std::getline()
#include <string>
// for std::stringstream
#include <sstream>

namespace octet {
  class csv_parser {

  public:
    csv_parser() {

    }

    ~csv_parser() {

    }

    /// Parses a csv file and populates an array with the data.
    void vec4_file(std::string file_path, dynarray<vec4> &arr) {
      std::ifstream file_stream(file_path);

      // check the file opened ok
      if (!file_stream) {
        std::string err = "Error loading csv file";
        printf(err.c_str());
      }

      std::string line = "";

      while (file_stream.good()) {
        // load a row
        std::getline(file_stream, line, '\n');

        // delimit each row with a comma
        line = line + ',';
        std::stringstream line_stream(line);

        vec4 data;
        std::string value = "";

        while (line_stream.good()) {
          // load a cell value
          std::getline(line_stream, value, ',');

          // skip word references
          if (isdigit(value[0]) || value[0] == '-') {
            const float number = (float)atof(value.c_str());
            data[0] = number;

            float num;
            for (int i = 1; i < 4; i++) {
              // fetch the next value
              std::getline(line_stream, value, ',');

              // if the row does not use the entire vector
              if (value == "") {
                // pad the remaining space with 0's
                num = NULL;
              } else {
                num = (float)atof(value.c_str());
              }

              const float data_num = num;
              data[i] = data_num;
            }

            arr.push_back(data);
          }
        }
      }

      // check output for debugging
      /*for (int i = 0; i < arr.size(); i++) {
        for (int j = 0; j < 4; j++) {
          printf(" %f ", arr[i][j]);
        }
        printf("\n");
      }*/
    }
  };
}
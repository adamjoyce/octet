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

    dynarray<vec3> vec3_locations_file(std::string file_path) {
      std::ifstream file_stream(file_path);

      if (!file_stream) {
        std::string err = "Error loading csv file";
        printf(err.c_str());
      }

      dynarray<vec3> locations;
      std::string line = "";

      while (file_stream.good()) {
        std::getline(file_stream, line, '\n');

        // delimit each row with a comma
        line = line + ',';
        std::stringstream line_stream(line);

        float x, y, z;
        std::string value = "";

        while (line_stream.good()) {
          std::getline(line_stream, value, ',');

          if (isdigit(value[0]) || value[0] == '-') {
            // x coordinate
            x = (float)atof(value.c_str());
            printf(value.c_str());

            // y coordinate
            std::getline(line_stream, value, ',');
            y = (float)atof(value.c_str());
            printf(value.c_str());

            // z coordinate
            std::getline(line_stream, value, ',');
            z = (float)atof(value.c_str());
            printf(value.c_str());

            printf("\n");

            locations.push_back(vec4(x, y, z));
          }
        }
      }

      return locations;
    }

    dynarray<vec4> vec4_locations_file(std::string file_path) {
      std::ifstream file_stream(file_path);

      if (!file_stream) {
        std::string err = "Error loading csv file";
        printf(err.c_str());
      }

      dynarray<vec4> locations;
      std::string line = "";

      while (file_stream.good()) {
        std::getline(file_stream, line, '\n');

        // delimit each row with a comma
        line = line + ',';
        std::stringstream line_stream(line);

        float x, y, w, h;
        std::string value = "";

        while (line_stream.good()) {
          std::getline(line_stream, value, ',');

          if (isdigit(value[0]) || value[0] == '-') {
            // x coordinate
            x = (float)atof(value.c_str());
            printf(value.c_str());

            // y coordinate
            std::getline(line_stream, value, ',');
            y = (float)atof(value.c_str());
            printf(value.c_str());

            // width
            std::getline(line_stream, value, ',');
            w = (float)atof(value.c_str());
            printf(value.c_str());

            // height
            std::getline(line_stream, value, ',');
            h = (float)atof(value.c_str());
            printf(value.c_str());

            printf("\n");

            locations.push_back(vec4(x, y, w, h));
          }
        }
      }

      return locations;
    }
  };
}
// A simple CSV parser to read in the l-systems information.
// Author: Adam Joyce
// Version: 1.02

#include <fstream>      // for std::ifstream
#include <string>       // for std::getline()
#include <sstream>      // for std::stringstream

namespace octet {
  class l_system_parser : public resource {

    // For the l-system data.
    dynarray<char> variables;
    dynarray<char> constants;   // Not used for anything.
    dynarray<char> axiom;
    dynarray<std::string> previous_axioms;
    hash_map<char, std::string> rules;
    float angle_variation;
    unsigned int max_iterations;

  public:
    l_system_parser() {
    }

    ~l_system_parser() {
    }

    /// Read the l-system data in from a given CSV file.
    void read_data(const std::string &file_path) {

      // std::string for use with std::getline
      std::string line = "";
      std::string value = "";
      std::ifstream file_stream(file_path);

      if (!file_stream) {
        std::string error = "Error: Could not load CSV file";
        printf("%s", error);
      }

      // Variables.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');
      std::stringstream line_stream(line);

      while (line_stream.good()) {
        std::getline(line_stream, value, ',');

        if (value == "")
          break;

        variables.push_back(value[0]);
      }

      // Constants.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');

      line_stream.clear();
      line_stream.str(line);

      while (line_stream.good()) {
        std::getline(line_stream, value, ',');

        if (value == "")
          break;

        constants.push_back(value[0]);
      }

      // Axiom.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');

      line_stream.clear();
      line_stream.str(line);

      while (line_stream.good()) {
        std::getline(line_stream, value, ',');

        if (value == "")
          break;

        axiom.push_back(value[0]);
        previous_axioms.push_back(value);
      }

      // Rules.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');

      line_stream.clear();
      line_stream.str(line);

      while (line_stream.good()) {
        std::getline(line_stream, value, ',');

        if (value == "")
          break;

        char key = value[0];
        std::string temp = value.substr(3);

        rules[key] = temp;
      }

      // Angle variation.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');

      line_stream.clear();
      line_stream.str(line);

      std::getline(line_stream, value, ',');

      angle_variation = atof(value.c_str());

      // Maximum iterations.
      std::getline(file_stream, line, '\n');
      std::getline(file_stream, line, '\n');

      line_stream.clear();
      line_stream.str(line);

      std::getline(line_stream, value, ',');

      max_iterations = atof(value.c_str());

      // For debugging.
      /*for (unsigned int i = 0; i < variables.size(); ++i)
        printf(" %c ", variables[i]);

      printf("\n");

      for (unsigned int i = 0; i < constants.size(); ++i)
        printf(" %c ", constants[i]);

      printf("\n");

      for (unsigned int i = 0; i < axiom.size(); ++i)
        printf(" %c ", axiom[i]);

      printf("\n");

      for (unsigned int i = 0; i < variables.size(); ++i) {
        printf(" %c->%s ", variables[i], rules[variables[i]].c_str());
      }

      printf("\n");

      printf(" %f ", angle_variation);

      printf("\n");

      printf(" %i ", max_iterations);*/
     }

    /// Iterate to the next step of the L-System.
    void next_iteration() {
      dynarray<char> new_axiom;

      for (unsigned int i = 0; i < axiom.size(); ++i) {
        if (rules.contains(axiom[i])) {
          std::string temp = rules[axiom[i]];

          for (unsigned int j = 0; j < temp.size(); ++j) {
            new_axiom.push_back(temp[j]);
          }
        } else {
          new_axiom.push_back(axiom[i]);
        }
      }
      
      // Store current axiom for reverse iteration.
      std::string temp_axiom = "";
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        temp_axiom += axiom[i];
      }
      previous_axioms.push_back(temp_axiom);

      // Replace the axiom with the new axiom.
      axiom.resize(new_axiom.size());
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        axiom[i] = new_axiom[i];
      }

      // For debugging.
      /*for (unsigned int i = 0; i < axiom.size(); ++i) {
        printf("%c", axiom[i]);
      }
      printf("\n");*/
    }

    /// Iterate to the previous step of the L-System.
    void previous_iteration() {
      std::string new_axiom = previous_axioms.back();
      axiom.resize(new_axiom.size());
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        axiom[i] = new_axiom[i];
      }
      previous_axioms.pop_back();
    }

    dynarray<char> &get_axiom() {
      return axiom;
    }

    float &get_angle() {
      return angle_variation;
    }

    unsigned int &get_max_iterations() {
      return max_iterations;
    }

    void set_angle(float &angle) {
      angle_variation = angle;
    }

    void reset() {
      variables.reset();
      constants.reset();
      axiom.reset();
      rules.clear();
    }
  };
}
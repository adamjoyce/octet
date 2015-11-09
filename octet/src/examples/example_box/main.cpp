////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason and Adam Joyce 2012-2015
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// L Systems Assignment
//

#include "../../octet.h"
#include "example_box.h"

/// Create a app with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::example_box app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}
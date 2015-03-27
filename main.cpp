////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text overlay
//

#include "../../octet.h"
#include <fstream>
#include "Maths_L_Systems.h"

/// Create a box with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::Maths_L_Systems app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}



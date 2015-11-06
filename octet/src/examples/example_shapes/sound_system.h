// FMOD Sound System
// Some comments may be unnecessarily explicit for the author's own educational purposes.
//
// Author: Adam Joyce
// Version: 1.03 - Issue with 'bod' identifier in fmod.hpp and fmod.h - unsure of the exact issue
//
// API: http://www.fmod.org/documentation/#content/generated/lowlevel_api.html
// Reference: https://cuboidzone.wordpress.com/2013/07/26/tutorial-implementing-fmod/

#include "fmod.hpp"
#include "fmod_errors.h"

namespace octet {
  class sound_system {
    // pointer to the FMOD instance
    FMOD::System *system;

  public:
    sound_system() {
      if (FMOD::System_Create(&system) != FMOD_OK) {
        // print the functions's returned value here to determine which FMOD_RESULT error
        return;
      }

      // count the number of soundcard devices
      int driver_num = 0;
      system->getNumDrivers(&driver_num);

      if (driver_num == 0) {
        // no soundcard device present
        return;
      }

      // initialise the system object and sound device with 36 channels
      system->init(36, FMOD_INIT_NORMAL, nullptr);
    }

    ~sound_system() {

    }

    /// Create a sound.
    void create_sound(FMOD::Sound *sound, const char *file_path) {
      system->createSound(file_path, FMOD_DEFAULT, 0, &sound);
    }

    /// Play a sound.
    void play_sound(FMOD::Sound *sound, bool loop) {
      // should the sound loop?
      if (!loop) {
        sound->setMode(FMOD_LOOP_OFF);
      } else {
        sound->setMode(FMOD_LOOP_NORMAL);
        sound->setLoopCount(-1);
      }

      system->playSound(sound, nullptr, false, nullptr);
    }

    /// Release the sound.
    void release_sound(FMOD::Sound *sound) {
      sound->release();
    }
  };
}
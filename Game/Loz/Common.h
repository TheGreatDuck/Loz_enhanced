/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

// Allegro
//#include <allegro5/allegro5.h>
//#include <allegro5/allegro_windows.h>
#define ALLEGRO_BITMAP void
#define ALLEGRO_SHADER void
#define ALLEGRO_EVENT_QUEUE void
#define ALLEGRO_DISPLAY void
#define ALLEGRO_CONFIG void
#define ALLEGRO_VOICE void
#define ALLEGRO_MIXER void
#define ALLEGRO_SAMPLE_INSTANCE void
#define ALLEGRO_SAMPLE void
#define ALLEGRO_AUDIO_STREAM void

// C
#include <crtdbg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// C++
#include <cmath>
#include <memory>
#include <array>

// This project
#include "Global.h"
#include "Util.h"

#undef GetObject

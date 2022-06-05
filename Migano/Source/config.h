#pragma once
#define _REAL_TIME_ 0 // pitch shift in real time when playing the notes. plugin boots faster, but can make latency on many notes.

#define _PREPROCESS_ALL_ 0 // preprocess pitch shifting on audio clip when constructing ther synthesizer. plugin boots slower, but no latency playing notes.

#define _LOAD_ALL_ 1 // saving all pitch-shifted clips as a file. loading all clips from soundbank file. faster than preprocess option, no latency. but it would consume your physical memory space.

#include <string>

const float MAX_DURATION = 5.0f;
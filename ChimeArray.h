#ifndef CHIMEARRAY_H
#define CHIMEARRAY_H

#include "arduino.h"

typedef struct {
  unsigned short pitch;
  unsigned short duration;
} chime_struct;

const unsigned short CHIMEARRAY_PITCH_E3 = 165;
const unsigned short CHIMEARRAY_PITCH_B3 = 247;
const unsigned short CHIMEARRAY_PITCH_E4 = 330;
const unsigned short CHIMEARRAY_PITCH_FSHARP4 = 370;
const unsigned short CHIMEARRAY_PITCH_GSHARP4 = 415;
const unsigned short CHIMEARRAY_DURATION_QUARTER = 500;

class ChimeArray{
  public:
    ChimeArray();
    ~ChimeArray();

    void setDebugStream(Stream *pStream);
    void debug(String msg);

    void push(chime_struct *chime);
    bool shift(chime_struct *chime);
    bool isEmpty();

  private:
    Stream *debugStream;
    chime_struct *chimes;
    unsigned short count;
};

#endif

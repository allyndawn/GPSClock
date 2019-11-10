#include "ChimeArray.h"
#include "string.h"

ChimeArray::ChimeArray() {
  chimes = 0;
  count = 0;
  debugStream = 0;
}

ChimeArray::~ChimeArray() {
  delete[] chimes;
  count = 0;
}

void ChimeArray::setDebugStream(Stream *pStream) {
  debugStream = pStream;
}

void ChimeArray::debug(String msg) {
  if (debugStream) {
    debugStream->println(msg);
  }
}

void ChimeArray::push(chime_struct *chime) {
  count++;
  chime_struct *new_chimes = new chime_struct[count];
  unsigned short offset = sizeof(chime_struct)*(count - 1);

  if (count > 1) {
    memcpy(new_chimes, chimes, offset);
    delete[] chimes;
  }

  new_chimes[count - 1].pitch = chime->pitch;
  new_chimes[count - 1].duration = chime->duration;

  chimes = new_chimes;
}

bool ChimeArray::shift(chime_struct *chime) {
  if (count == 0) {
    return false;
  }

  chime->pitch = chimes[0].pitch;
  chime->duration = chimes[0].duration;
  count--;

  if (count > 0) {
    chime_struct *new_chimes = new chime_struct[count];
    memcpy(new_chimes, (void *)chimes + sizeof(chime_struct), sizeof(chime_struct) * count);
    delete[] chimes;
    chimes = new_chimes;
  } else {
    delete[] chimes;
  }

  return true;
}

bool ChimeArray::isEmpty() {
  return (count == 0);
}

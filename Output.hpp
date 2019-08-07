#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>

namespace manip {

enum class Erase: uint8_t { CursorToEnd, CursorToBegin, All};
std::ostream& operator<< (std::ostream& out, Erase eraseType);

class erase{
  int _rows;

public:
  erase(int rows);

  friend std::ostream& operator<< (std::ostream& out, erase manip);
};

enum class Mode: uint8_t {
  Normal, Bold, Underline, Blink, Reverse, NonDisplay
};

std::ostream& operator<< (std::ostream& out, Mode color);

enum class Color: uint8_t {
  Reset,
  Black=30, Red, Green, Yellow, Blue, Magenta, Cyan, White,
  BlackBg=40, RedBg, GreenBg, YellowBg, BlueBg, MagentaBg, CyanBg, WhiteBg
};

std::ostream& operator<< (std::ostream& out, Color color);

enum class Control: uint8_t {
  ScrollScreen, ScrollDown, ScrollUp, CursorDown, CursorSave, CursorRestore, CursorHide, CursorShow
};

std::ostream& operator<< (std::ostream& out, Control control);

class at{
  int _row, _column;

public:
  at(int row, int column);

  friend std::ostream& operator<< (std::ostream& out, const at& manip);
};

class rgb{
  uint8_t _red,_green,_blue;

public:
  rgb(uint8_t red, uint8_t green, uint8_t blue);

  friend std::ostream& operator<< (std::ostream& out, const rgb& manip);
};

}

using Output = std::ostream;

#endif

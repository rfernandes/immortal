#ifndef INPUT_H
#define INPUT_H

#include <iostream>

class Input{
  std::istream& _in;
  unsigned _next{0};
public:

  enum Key{
    Backspace = 127,

    Unknown = 257,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    CtrlUp,
    CtrlDown,
    CtrlLeft,
    CtrlRight,
    CtrlHome,
    CtrlEnd,
    CtrlM,
    Delete,
    Resize
  };

  explicit Input(std::istream &in);
  ~Input();

  unsigned get();
  unsigned put(unsigned int key);
};

#endif

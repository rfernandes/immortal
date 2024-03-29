#include "Output.hpp"

#include <type_traits>

using namespace std;

namespace manip {

std::ostream& operator<< (std::ostream& out, Erase eraseType) {
  return out << "\x1b[" << +static_cast<std::underlying_type_t<Color>>(eraseType) << 'K';
}

erase::erase::erase(int rows)
: _rows{rows}
{
}

std::ostream& operator<< (std::ostream& out, erase erase) {
  return out << "\x1b[" << +erase._rows << 'X';
}


std::ostream& operator<< (std::ostream& out, Mode mode) {
  return out << "\x1b[" << +static_cast<std::underlying_type_t<Color>>(mode) << 'm';
}

std::ostream& operator<< (std::ostream& out, Color color) {
  if (color == Color::Reset) {
    out << "\x1b[39;49m";
  } else {
    out << "\x1b[" << +static_cast<std::underlying_type_t<Color>>(color) << 'm';
  }
  return out;
}

std::ostream& operator<< (std::ostream& out, Control control) {
  switch (control){
    case Control::ScrollScreen:
      out << "\x1b[r";
      break;
    case Control::ScrollUp:
      out << "\x1b[S";
      break;
    case Control::ScrollDown:
      out << "\x1b[T";
      break;
    case Control::CursorDown:
      out << "\x1b[1A";
      break;
    case Control::CursorSave:
      out << "\x1b[s";
      break;
    case Control::CursorRestore:
      out << "\x1b[u";
      break;
    case Control::CursorHide:
      out << "\x1b[?25l";
      break;
    case Control::CursorShow:
      out << "\x1b[?25h";
      break;
    default:
      break;
  }
  return out;
}

at::at(int row, int column)
: _row{row}
, _column{column}
{}

std::ostream& operator<< (std::ostream& out, const at& at) {
  return out << "\x1b[" << +at._row << ';' << +at._column << 'H';
}

rgb::rgb(uint8_t red, uint8_t green, uint8_t blue)
: _red{red}
, _green{green}
, _blue{blue}
{
}

std::ostream& operator<< (std::ostream& out, const rgb& manip) {
  return out << "\x1b[38;2;" << +manip._red << ';' << +manip._green << ';' << +manip._blue << 'm';
}

}

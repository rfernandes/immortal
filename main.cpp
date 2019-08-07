#include "Input.hpp"
#include "Output.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <array>
#include <cmath>
#include <atomic>
#include <thread>
#include <chrono>

#include <signal.h>

using namespace std;
using namespace manip;

constexpr int tileSize=64;

class Window : public std::streambuf
{
public:
  Window(int row, int col, int width, int height, char background = ' ')
  : _row{row}
  , _col{col}
  , _width{width}
  , _height{height}
  , _background{background}
  {
    clear();
  }

//   std::streamsize xsputn(const char_type* buff, std::streamsize size) final {
//     streamsize remaining{size};
//     streamsize width{_width};
//     while (remaining)
//     {
//       int write = min(width, remaining);
//       std::cout << at(_row+_rowOffset, _col);
//       cout.write(buff, write);
//       remaining-=write;
//       buff+=write;
//       newLine();
//     }
//     return 0;
//   }

  int_type overflow(int_type type) final
  {
    if (type != traits_type::eof())
    {
      char ch = traits_type::to_char_type(type);
      switch (ch)
      {
        case '\0':
          break;
        case '\n':
          newLine();
          break;
        default: {
          cout << at(_row+_rowOffset, _col+_colOffset);
          cout.write(&ch, 1);
          nextChar();
        }
      }
    }
    return type;
  }

  void clear(){
    // Clear window
    for(int i=0; i < _height; ++i)
    {
      std::cout << at(i+_row, _col);
      std::fill_n(std::ostream_iterator<char>(std::cout), _width, _background);
    }
  }

private:

  void newLine(){
    (++_rowOffset) %= _height;
    _colOffset = 0;
  }

  void nextChar(){
    if (! ((++_colOffset) %= _width) )
    {
      newLine();
    }
  }

  int _row, _col, _width, _height;
  int _rowOffset{0}, _colOffset{0};
  char _background;
};

struct Point
{
  float x{.0},y{.0};

  explicit constexpr operator bool() const noexcept{
    return !isnan(x) && !isnan(y);
  }
};



struct Direction: public Point
{
  float angle{0.0};
};

bool operator == (const Point& lhs, const Point& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
bool operator != (const Point& lhs, const Point& rhs) noexcept {
  return !(lhs == rhs);
}


std::ostream& operator <<(std::ostream& out, const Point& point)
{
  return out << '(' << point.x << ',' << point.y << ')';
}


bool constexpr equals(float a, float b) {
    return std::fabs(a - b) < std::numeric_limits<float>::epsilon()*16;
}

struct Hit
{
  char tile;
  float distance;
};

const int mapWidth = 12;
const int mapHeight = 10;

class Map
{
    
public:

  Map(const Point& player)
  : _player{player}
  {
  }
    
  char getTile(const Point& p){
    auto tile = _map[mapWidth * std::round(p.y) + std::round(p.x)];
    return tile == ' ' ? '\0' : tile;
  }
    
private:

  friend std::ostream& operator << (std::ostream& out, const Map& map)
  {
    auto mapCopy = map._map;
    mapCopy[(static_cast<int>(map._player.y) * mapWidth) + static_cast<int>(map._player.x)] = 'P';
    return out.write(mapCopy.data(), mapCopy.size());
  }

  Window _commandWindow{2, 2, 10, 30};
  std::ostream _commandStream{&_commandWindow};

  const Point &_player;

  array<char, mapWidth*mapHeight + 1> _map{
"#==========#"
"|          #"
"|          #"
"|          |"
"|          B"
"|          |"
"|          A"
"|          /"
"|          #"
"#++++++++++#"
};

};

struct Line{
  Point a,b;
};

// Line slicer adapter
class Slice{
  const Line& _line;
  int _steps;
  float _step_x, _step_y;

public:

  Slice(const Line& line, int steps)
  : _line{line}
  , _steps{steps - 1}
  , _step_x{(_line.b.x - _line.a.x) / _steps}
  , _step_y{(_line.b.y - _line.a.y) / _steps}
  {}

  class Iterator{
    Point _point;
    float _step_x, _step_y;
    int _steps;

  public:
    Iterator(Point point, float step_x, float step_y, int steps)
    : _point{point}
    , _step_x{step_x}
    , _step_y{step_y}
    , _steps{steps}
    {}

    Iterator& operator++() {
        _point.x += _step_x;
        _point.y += _step_y;
        _steps--;
        return *this;
    }

    const Point& operator*() {
      return _point;
    }

    bool operator!=(const Iterator& other) const {
      return _step_x != other._step_x ||
        _step_y != other._step_y ||
        _steps != other._steps;
    }

    bool operator==(const Iterator& other) const {
      return !(*this != other);
    }

    using iterator_category = std::forward_iterator_tag;
    using value_type = Point;
    using difference_type = Point;
    using pointer = Point*;
    using reference = Point&;
  };

  Iterator begin(){
    return {_line.a, _step_x, _step_y, _steps};
  }

  Iterator end(){
    return {{}, _step_x, _step_y, -1}; // -1 is "past the range"
  }
};

// Line wall_north{{0,0},{12,0}};
// Line wall_south{{0,10},{12,10}};
// Line wall_west{{0,0},{0,10}};
Line wall_east{{11,1},{11,8}};


Line perpendicular(const Line& line)
{
  auto halfXDiff = (line.b.x - line.a.x) / 2;
  auto halfYDiff = (line.b.y - line.a.y) / 2;
  auto ax = line.b.x - halfYDiff;
  auto ay = line.b.y + halfXDiff;
  auto bx = line.b.x + halfYDiff;
  auto by = line.b.y - halfXDiff;

  return {{ax,ay},{bx,by}};
}

std::ostream& operator << (std::ostream& out, const Line& line){
  return out << '[' << line.a << ',' << line.b << ']';
}

Point constexpr intersect(const Line& lhs, const Line& rhs){
    // Line AB represented as a1x + b1y = c1
    float a1 = lhs.b.y - lhs.a.y;
    float b1 = lhs.a.x - lhs.b.x;
    float c1 = a1*(lhs.a.x) + b1*(lhs.a.y);

    // Line CD represented as a2x + b2y = c2
    float a2 = rhs.b.y - rhs.a.y;
    float b2 = rhs.a.x - rhs.b.x;
    float c2 = a2*(rhs.a.x)+ b2*(rhs.a.y);

    float determinant = a1*b2 - a2*b1;

    if (!determinant) {
      return {NAN, NAN};
    } else
    {
      float x = (b2*c1 - b1*c2) / determinant;
      float y = (a1*c2 - a2*c1) / determinant;
      return {x,y};
    }
}

bool constexpr contained(const Line& line, const Point& point)
{
    const auto x_min = std::min(line.a.x, line.b.x);
    const auto x_max = std::max(line.a.x, line.b.x);

    const auto y_min = std::min(line.a.y, line.b.y);
    const auto y_max = std::max(line.a.y, line.b.y);

    return !(x_min > point.x && point.x > x_max) &&
          !(y_min > point.y)  && !(point.y > y_max);
}

Point constexpr intersect_segment(const Line& lhs, const Line& rhs){
  const auto inter = intersect(lhs, rhs);
  return inter && contained(lhs, inter) && contained(rhs, inter) ? inter : Point{NAN, NAN};
}

float distance(const Point& lhs, const Point& rhs){
  return sqrt(pow(lhs.x - rhs.x, 2) + pow(lhs.y - rhs.y, 2));
}

Input in{std::cin};

int main(int argc, char **argv) {
  std::cout << Control::CursorHide;
  Direction player{5,5, 0};
  Map map{player};

  Window clear{1,1, 200, 50, '.'};

  signal(SIGWINCH, [](int /*signal*/){ in.put(Input::Resize); });

  bool exit{false};

  unsigned keystroke;
  Window mapWindow{33, 140, 12, 10};
  std::ostream mapStream(&mapWindow);

  const auto screenWidth = 158;
  const auto screenHeight = 30;
  Window viewPort{2,2, screenWidth, screenHeight, ' '};
  std::ostream viewStream(&viewPort);

  Window debugWindow{33, 2, 120, 12, ' '};
  std::ostream debugStream(&debugWindow);

  array<char, screenWidth*screenHeight> screenData;
  screenData.fill(' ');
//   auto screenMid = screenWidth / 2;

  const float movement = 0.25;

  while (!exit && (keystroke = in.get())) {

    switch (keystroke){
        case 'w': // Forward
        {
            Direction next = player;
            next.x += movement;
            if (! map.getTile(next) ){
                player=next;
            }
            break;
        }
        case 's': // Back
        {
            Direction next = player;
            next.x -= movement;
            if (! map.getTile(next) ){
                player=next;
            }
            break;
        }
        case 'd': // Strafe Left
        {
            Direction next = player;
            next.y += movement;
            if (! map.getTile(next) ){
                player=next;
            }
            break;
        }
        case 'a': // Strafe Right
        {
            Direction next = player;
            next.y -= movement;
            if (! map.getTile(next) ){
                player=next;
            }
            break;
        }
        case 'q': // Rotate Left
        {
            player.angle-= M_PI / 30;
            break;
        }
        case 'e': // Rotate Right
        {
            player.angle+= M_PI / 30;
            break;
        }
        case Input::Resize:
        {
//           debugStream << "resize ";
          break;
        }
        case 'p':
            cout << "Player: "<< player.x << ',' << player.y << '\n';
            break;
    }

    // Render;
    screenData.fill(' ');

    Point vanish{(player.x + (60 * std::cos(player.angle))), (player.y - (60 * std::sin(player.angle)))};
    Line ray{player, vanish};
    Line view = perpendicular(ray);

    int x = 0;
    int y = 1000;
    for (const auto& p : Slice(view, screenWidth))
    {
      Line columnRay{player, p};
      const auto inter = intersect_segment(columnRay, wall_east);
      if (inter)
      {
        auto dist =  distance(player, inter);
        auto viewHeight = screenHeight / (dist);
        const auto height = std::min(static_cast<int>(viewHeight), screenHeight/2);

        auto tile = map.getTile(inter);
        tile = tile ? tile : 'X';


        for(int i = 0; i < height; ++i)
        {
          screenData[(((screenHeight / 2 )-i) * screenWidth) + x] = tile;
        }
        if(tile == 'X')
        {
          debugStream << "pointz:" << std::round(inter.y) << " "<< std::round(inter.x) << "    \n";
        }

      } else {
        if (x < y){
          y = x;
//           debugStream << "fail ray:" << columnRay << " w:" << wall_east << " x:" << x << "    \n";
        }
      }
      ++x;
    }
    viewStream.write(screenData.data(), screenData.size());
    mapStream << map;

  }
    
  return 0;
}

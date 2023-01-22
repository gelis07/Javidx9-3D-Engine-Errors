#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <vector>
#include <array>
#include "ScreenCoord.h"

struct Color {
  float r, g, b, a;
};
struct Pos2D {
  float x;
  float y;
};

class Triangles{
    
    public:
      
      Triangles();
      std::vector<unsigned int> VertexArrays;
      std::vector<unsigned int> VertexBuffers;
      std::vector<int> ColorLocations;
      unsigned int Shader;
      std::vector<Color> Colors;
      int MVPLoc;
      void CreateTriangle(const std::array<Pos2D,3> &positions, Color color);
};
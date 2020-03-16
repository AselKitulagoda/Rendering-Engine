#ifndef WIREFRAME_HPP
#define WIREFRAME_HPP

#include "global.hpp"
#include "rasteriser.hpp"

using namespace std;
using namespace glm;

// Wireframe Stuff
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c);
void drawStroke(CanvasTriangle t, Colour c);
void drawWireframe(vector<ModelTriangle> tris);

void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c)
{
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  float numberOfValues = std::max(abs(dx), abs(dy));

  float xChange = dx/(numberOfValues);
  float yChange = dy/(numberOfValues);

  for(float i = 0; i < numberOfValues; i++)
  {
    float x = p1.x + (xChange * i);
    float y = p1.y + (yChange * i);
    uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
    window.setPixelColour(round(x), round(y), colour);
  }
}

void drawStroke(CanvasTriangle t, Colour c)
{
  drawLine(t.vertices[0], t.vertices[1], c);
  drawLine(t.vertices[1], t.vertices[2], c);
  drawLine(t.vertices[2], t.vertices[0], c);
}

void drawWireframe(vector <ModelTriangle> tris)
{
  window.clearPixels();
  for (size_t i=0; i<tris.size(); i++){
    CanvasTriangle new_tri = modelToCanvas(tris[i]);
    drawStroke(new_tri, tris[i].colour);
  }
}

#endif
#ifndef WIREFRAME_HPP
#define WIREFRAME_HPP

#include "global.hpp"

using namespace std;
using namespace glm;

// Wireframe Stuff
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c, float depthBuffer[WIDTH][HEIGHT]);
void drawStroke(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawWireframe(vector<ModelTriangle> triangles);

void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c, float depthBuffer[WIDTH][HEIGHT])
{
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  float dDepth = p2.depth - p1.depth;

  float numberOfValues = std::max(abs(dx), abs(dy));

  float xChange = dx/(numberOfValues);
  float yChange = dy/(numberOfValues);
  float depthChange = dDepth/(numberOfValues);

  for(float i = 0.0; i < numberOfValues; i++)
  {
    float x = p1.x + (xChange * i);
    float y = p1.y + (yChange * i);
    float depth = p1.depth + (depthChange * i);

    if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    {
      if(depth < depthBuffer[(int) x][(int) y])
      {
        depthBuffer[(int) x][(int) y] = depth;
        window.setPixelColour((int) x, (int) y, c.pack());
      }
    }
  }
}

void drawStroke(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT])
{
  drawLine(t.vertices[0], t.vertices[1], t.colour, depthBuffer);
  drawLine(t.vertices[1], t.vertices[2], t.colour, depthBuffer);
  drawLine(t.vertices[2], t.vertices[0], t.colour, depthBuffer);
}

void drawWireframe(vector<ModelTriangle> triangles) 
{
  window.clearPixels();
  
  float depthBuffer[WIDTH][HEIGHT];
  for(int i = 0; i < WIDTH; i++) 
  {
    for(int j = 0; j < HEIGHT; j++) 
    {
      depthBuffer[i][j] = (float) INFINITY;
    }
  }
  vector<ModelTriangle> filteredTriangles = backfaceCulling(triangles);

  if(cullingMode)
  {
    for(size_t i = 0; i < filteredTriangles.size(); i++)
    {
      CanvasTriangle projection = modelToCanvas(filteredTriangles.at(i));
      drawStroke(projection, depthBuffer);
    }
  }
  else 
  {
    for(size_t i = 0; i < triangles.size(); i++)
    {
      CanvasTriangle projection = modelToCanvas(triangles.at(i));
      drawStroke(projection, depthBuffer);
    }
  }
}

#endif
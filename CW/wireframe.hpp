#ifndef WIREFRAME_HPP
#define WIREFRAME_HPP

#include "global.hpp"

using namespace std;
using namespace glm;

// Wireframe Stuff
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c, float depthBuffer[WIDTH][HEIGHT]);
void drawStroke(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawWireframe(vector<ModelTriangle> triangles);

// Wu Line (Anti aliasing stuff)
float roundNumber(float x);
float fPart(float x);
float rfPart(float x);
void drawPixel(int x, int y, Colour c, float brightness);
void drawAALine(CanvasPoint p1, CanvasPoint p2, Colour c);
void drawAAStroke(CanvasTriangle t);

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
  // window.clearPixels();
  
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
    if(wuMode)
    {
      for(size_t i = 0; i < triangles.size(); i++)
      {
        CanvasTriangle projection = modelToCanvas(triangles.at(i));
        drawAAStroke(projection);
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
}

// Wu Lines

float roundNumber(float x) 
{
  return std::floor(x + 0.5);
}
 
float fPart(float x)
{
  return x - std::floor(x);
}

float rfPart(float x) 
{
  return 1 - fPart(x);
}

void drawPixel(int x, int y, Colour c, float brightness)
{
  Colour newColour = Colour(c.red, c.green, c.blue, brightness);
  if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    window.setPixelColour(x, y, newColour.packWithBrightness());
}

void drawAALine(CanvasPoint p1, CanvasPoint p2, Colour c)
{
  float x0 = p1.x; float y0 = p1.y;
  float x1 = p2.x; float y1 = p2.y;

  int steep = abs(y1 - y0) > abs(x1 - x0);

  // swap the co-ordinates if slope > 1 or we
  // draw backwards
  if(steep)
  {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if(x0 > x1)
  {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  //compute the slope
  float dx = x1 - x0;
  float dy = y1 - y0;
  float gradient = dy / dx;
  if(dx == 0.0)
    gradient = 1;

  int xend = round(x0);
  float yend = y0 + (gradient * (xend - x0));

  int xpxl1 = xend;
  int ypxl1 = std::floor(yend);

  if(steep)
  {
    drawPixel(ypxl1, xpxl1, c, rfPart(yend) * fPart(x0 + 0.5));
    drawPixel(ypxl1 + 1, xpxl1, c, fPart(yend) * fPart(x0 + 0.5));
  }
  else
  {
    drawPixel(xpxl1, ypxl1, c, rfPart(yend) * fPart(x0 + 0.5));
    drawPixel(xpxl1, ypxl1 + 1, c, fPart(yend) * fPart(x0 + 0.5));
  }
  
  float intersectY = yend + gradient;

  xend = round(x1);
  yend = y1 + (gradient * (xend - x1));

  int xpxl2 = xend;
  int ypxl2 = std::floor(yend);

  if(steep)
  {
    drawPixel(ypxl2, xpxl2, c, rfPart(yend) * fPart(x1 + 0.5));
    drawPixel(ypxl2 + 1, xpxl2, c, fPart(yend) * fPart(x1 + 0.5));
  }
  else
  {
    drawPixel(xpxl2, ypxl2, c, rfPart(yend) * fPart(x1 + 0.5));
    drawPixel(xpxl2, ypxl2 + 1, c, fPart(yend) * fPart(x1 + 0.5));
  }

  // main loop
  if(steep)
  {
    for(int x = xpxl1 + 1; x < xpxl2 - 1; x++)
    {
      drawPixel(std::floor(intersectY), x, c, rfPart(intersectY));
      drawPixel(std::floor(intersectY) + 1, x, c, fPart(intersectY));
      intersectY += gradient;
    }
  }
  else 
  {
    for(int x = xpxl1 + 1;x < xpxl2 - 1; x++)
    {
      drawPixel(x, std::floor(intersectY), c, rfPart(intersectY));
      drawPixel(x, std::floor(intersectY) + 1, c, fPart(intersectY));
      intersectY += gradient;
    }
  }
  
}

void drawAAStroke(CanvasTriangle t)
{
  drawAALine(t.vertices[0], t.vertices[1], t.colour);
  drawAALine(t.vertices[1], t.vertices[2], t.colour);
  drawAALine(t.vertices[2], t.vertices[0], t.colour);
}

#endif
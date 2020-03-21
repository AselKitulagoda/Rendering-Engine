#ifndef RASTERISER_HPP
#define RASTERISER_HPP

#include "global.hpp"
#include "wireframe.hpp"

using namespace std;
using namespace glm;

// Rasterising Stuff
void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawRasterised(vector<ModelTriangle> triangles);

void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT])
{
  CanvasPoint p1 = t.vertices[0];
  CanvasPoint p2 = t.vertices[1];
  CanvasPoint p3 = t.vertices[2];

  if(p1.y < p2.y){ std::swap(p1, p2); }
  if(p1.y < p3.y){ std::swap(p1, p3); }
  if(p2.y < p3.y){ std::swap(p2, p3); }

  CanvasPoint minPoint = p3;
  CanvasPoint midPoint = p2;
  CanvasPoint maxPoint = p1;

  float minMaxRatio = (maxPoint.x - minPoint.x) / (maxPoint.y - minPoint.y);
  float minMaxDepth = (maxPoint.depth - minPoint.depth) / (maxPoint.y - minPoint.y);

  float minMidRatio = (midPoint.x - minPoint.x) / (midPoint.y - minPoint.y);
  float minMidDepth = (midPoint.depth - minPoint.depth) / (midPoint.y - minPoint.y);

  float midMaxRatio = (maxPoint.x - midPoint.x) / (maxPoint.y - midPoint.y);
  float midMaxDepth = (maxPoint.depth - midPoint.depth) / (maxPoint.y - midPoint.y);

  CanvasPoint extraPoint;
  extraPoint.x = minPoint.x + minMaxRatio * (midPoint.y - minPoint.y);
  extraPoint.y = midPoint.y;
  extraPoint.depth = minPoint.depth + minMaxDepth * (midPoint.y - minPoint.y);

  // Top half interpolation
  for(float i = minPoint.y; i < midPoint.y; i++)
  {
    CanvasPoint from, to;

    from.x = minPoint.x + minMaxRatio * (i - minPoint.y);
    from.y = i;
    from.depth = minPoint.depth + minMaxDepth * (i - minPoint.y);

    to.x = minPoint.x + minMidRatio * (i - minPoint.y);
    to.y = i;
    to.depth = minPoint.depth + minMidDepth * (i - minPoint.y);

    drawLine(from, to, t.colour, depthBuffer);
  }

  // Bottom half interpolation
  for(float i = midPoint.y; i < maxPoint.y; i++)
  {
    CanvasPoint from, to;

    from.x = extraPoint.x + minMaxRatio * (i - midPoint.y);
    from.y = i;
    from.depth = extraPoint.depth + minMaxDepth * (i - midPoint.y);

    to.x = midPoint.x + midMaxRatio * (i - midPoint.y);
    to.y = i;
    to.depth = midPoint.depth + midMaxDepth * (i - midPoint.y);

    drawLine(from, to, t.colour, depthBuffer);
  }
}

void drawRasterised(vector<ModelTriangle> triangles) 
{
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
      drawFilled(projection, depthBuffer);
    }
  }
  else 
  {
    for(size_t i = 0; i < triangles.size(); i++)
    {
      CanvasTriangle projection = modelToCanvas(triangles.at(i));
      drawFilled(projection, depthBuffer);
    }
  }
}

#endif
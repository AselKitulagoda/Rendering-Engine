#ifndef RASTERISER_HPP
#define RASTERISER_HPP

#include "global.hpp"
#include "wireframe.hpp"

using namespace std;
using namespace glm;

// Rasterising Stuff
void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawRasterised(vector<ModelTriangle> triangles);

// Texturing Stuff
void drawTextureLine(CanvasPoint to, CanvasPoint from, vector<uint32_t> pixelColours);
void drawTextureMap(CanvasTriangle currentTri);

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

void drawTextureLine(CanvasPoint to, CanvasPoint from, vector<uint32_t> pixelColours)
{
  float dx = to.x - from.x;
  float dy = to.y - from.y;
  float numberOfValues = ceil(std::max(abs(dx), abs(dy)));

  vector<float> xs = interpolation(from.x, to.x, numberOfValues);
  vector<float> ys = interpolation(from.y, to.y, numberOfValues);
  
  TexturePoint numberOfTextureValues;
  numberOfTextureValues.x = to.texturePoint.x - from.texturePoint.x;
  numberOfTextureValues.y = to.texturePoint.y - from.texturePoint.y;

  for(float i = 0; i <= numberOfValues; i++)
  {
    TexturePoint tp;
    tp.x = from.texturePoint.x + (i * numberOfTextureValues.x/numberOfValues);
    tp.y = from.texturePoint.y + (i * numberOfTextureValues.y/numberOfValues);
    window.setPixelColour(xs[i], ys[i], pixelColours[round(tp.x) + round(tp.y) * 300]);
  }
}

void drawTextureMap(CanvasTriangle currentTri)
{ 
  CanvasPoint largest; largest.x = currentTri.vertices[0].x; largest.y = currentTri.vertices[0].y; largest.texturePoint = currentTri.vertices[0].texturePoint;
  CanvasPoint middle; middle.x = currentTri.vertices[1].x; middle.y = currentTri.vertices[1].y; middle.texturePoint = currentTri.vertices[1].texturePoint;
  CanvasPoint smallest; smallest.x = currentTri.vertices[2].x; smallest.y = currentTri.vertices[2].y; smallest.texturePoint = currentTri.vertices[2].texturePoint;
  
  if(largest.y < middle.y)
  {
    std::swap(largest, middle);
  }
  if(largest.y < smallest.y)
  {
    std::swap(largest, smallest);
  }
  if(middle.y < smallest.y)
  {
    std::swap(middle, smallest);
  }

  float ratio = (largest.y - middle.y)/(largest.y - smallest.y);
  CanvasPoint extraPoint;
  extraPoint.x = largest.x - ratio*(largest.x - smallest.x);
  extraPoint.y = largest.y - ratio*(largest.y - smallest.y);

  TexturePoint extraTex;
  extraTex.x = largest.texturePoint.x - ratio*(largest.texturePoint.x - smallest.texturePoint.x);
  extraTex.y = largest.texturePoint.y - ratio*(largest.texturePoint.y - smallest.texturePoint.y);
  
  extraPoint.texturePoint = extraTex;

  // Interpolation 
  int numberOfValuesTop = (largest.y - middle.y);
  int numberOfValuesBot = (middle.y - smallest.y);

  vector<CanvasPoint> largest_extraPoint = interpolate(largest, extraPoint, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> largest_middle = interpolate(largest, middle, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> smallest_extraPoint = interpolate(smallest, extraPoint, ceil(numberOfValuesBot)+1);
  vector<CanvasPoint> smallest_middle = interpolate(smallest, middle, ceil(numberOfValuesBot)+1);

  for(int i = 0; i <= numberOfValuesTop; i++)
  {
    drawTextureLine(largest_extraPoint[i], largest_middle[i], pixelColours);
  }

  for(int i = 0; i <= numberOfValuesBot+1; i++)
  {
    drawTextureLine(smallest_extraPoint[i], smallest_middle[i], pixelColours);
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
      
      if((projection.vertices[0].texturePoint.x) == -300.0f && (projection.vertices[0].texturePoint.y) == -300.0f && 
          (projection.vertices[1].texturePoint.x) == -300.0f && (projection.vertices[1].texturePoint.y) == -300.0f &&
          (projection.vertices[2].texturePoint.x) == -300.0f && (projection.vertices[2].texturePoint.y) == -300.0f)
      {
        drawFilled(projection, depthBuffer);
      }
      else
      {
        drawTextureMap(projection);
      }
    }
  }
  else 
  {
    for(size_t i = 0; i < triangles.size(); i++)
    {
      CanvasTriangle projection = modelToCanvas(triangles.at(i));
      
      if((projection.vertices[1].texturePoint.x) == -300.0f && (projection.vertices[1].texturePoint.y) == -300.0f)     
      {
        drawFilled(projection, depthBuffer);
      }
      else
      {
        drawTextureMap(projection);
      }
    }
  }
}

#endif
#ifndef RASTERISER_HPP
#define RASTERISER_HPP

#include "global.hpp"
#include "wireframe.hpp"

using namespace std;
using namespace glm;

// Rasterising Stuff
CanvasTriangle modelToCanvas(ModelTriangle t);
void computeDepth(CanvasTriangle t, double *depthBuffer);
void drawRasterised(vector<ModelTriangle> tris);

CanvasTriangle modelToCanvas(ModelTriangle modelTrig)
{
    float f = 3;
    CanvasTriangle canvasTrig = CanvasTriangle();
    canvasTrig.colour = modelTrig.colour;
    for(int i=0; i<3 ;i++) {
        float xdistance = modelTrig.vertices[i].x-cameraPos.x;
        float ydistance = modelTrig.vertices[i].y-cameraPos.y;
        float zdistance = modelTrig.vertices[i].z-cameraPos.z;
        vec3 cameraToVertex = vec3(xdistance, ydistance, zdistance);
        vec3 adjustedVector = cameraToVertex * cameraOrientation;
        float pScreen = f/-adjustedVector.z;
        // Scale up the x and y canvas coords to get a bigger image (rather than a big model loader scaling)
        float canvasScaling = 150;
        float xProj = (adjustedVector.x*pScreen*canvasScaling) + WIDTH/2;
        float yProj = (-adjustedVector.y*pScreen*canvasScaling) + HEIGHT/2;
        CanvasPoint p = CanvasPoint(xProj, yProj);
        p.depth = 1.0/-adjustedVector.z;
        canvasTrig.vertices[i] = p;
    }
    return canvasTrig;
}

void computeDepth(CanvasTriangle t, double *depthBuffer)
{
  CanvasPoint p1 = t.vertices[0];
  CanvasPoint p2 = t.vertices[1];
  CanvasPoint p3 = t.vertices[2];

  if(p1.y < p2.y){ std::swap(p1, p2); }
  if(p1.y < p3.y){ std::swap(p1, p3); }
  if(p2.y < p3.y){ std::swap(p2, p3); }

  float ratio = (p1.y - p2.y)/(p1.y - p3.y);
  CanvasPoint extraPoint;
  extraPoint.x = p1.x - ratio*(p1.x - p3.x);
  extraPoint.y = p1.y - ratio*(p1.y - p3.y);
  double depth = p1.depth - ratio*(p1.depth - p3.depth);
  extraPoint.depth = depth;

  // Interpolation
  int numberOfValuesTop = (p1.y - p2.y);
  int numberOfValuesBot = (p2.y - p3.y);

  // Interpolating between the z values
  vector<CanvasPoint> p1_extraPoint = interpolate(p1, extraPoint, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p1_p2 = interpolate(p1, p2, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p3_extraPoint = interpolate(p3, extraPoint, ceil(numberOfValuesBot)+1);
  vector<CanvasPoint> p3_p2 = interpolate(p3, p2, ceil(numberOfValuesBot)+1);

  for(size_t i = 0; i < p1_extraPoint.size(); i++)
  {
    vector<CanvasPoint> upper = interpolate(p1_extraPoint[i], p1_p2[i], abs(p1_extraPoint[i].x - p1_p2[i].x)+1);
    for(size_t j = 0; j < upper.size(); j++)
    {
      CanvasPoint check = upper[j];
      if((uint32_t) check.x >= 0 && (uint32_t) check.x < WIDTH && (uint32_t) check.y >= 0 && (uint32_t) check.y < HEIGHT)
      {
        if(check.depth > depthBuffer[((uint32_t)check.x + (uint32_t)check.y* WIDTH)])
        {
          depthBuffer[((uint32_t)check.x + (uint32_t)check.y* WIDTH)] = check.depth;
          Colour c = t.colour;
          uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
          window.setPixelColour((int)check.x, (int)check.y, colour);
        }
      }
    }
  }

  for(size_t i = 0; i < p3_extraPoint.size(); i++)
  {
    vector<CanvasPoint> lower = interpolate(p3_extraPoint[i], p3_p2[i], abs(p3_extraPoint[i].x - p3_p2[i].x)+1);
    for(size_t j = 0; j < lower.size(); j++)
    {
      CanvasPoint check = lower[j];
      if((uint32_t) check.x >= 0 && (uint32_t) check.x < WIDTH && (uint32_t) check.y >= 0 && (uint32_t) check.y < HEIGHT)
      {
        if(check.depth > depthBuffer[((uint32_t)check.x + (uint32_t)check.y* WIDTH)])
        {
          depthBuffer[((uint32_t)check.x + (uint32_t)check.y* WIDTH)] = check.depth;
          Colour c = t.colour;
          uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
          window.setPixelColour((int)check.x, (int)check.y, colour);
        }
      }
    }
  }
}

void drawRasterised(vector<ModelTriangle> tris)
{
  window.clearPixels();
  double *depthBuffer = (double*)malloc(sizeof(double) * WIDTH * HEIGHT);
  for(uint32_t y = 0; y < HEIGHT; y++)
  {
    for(uint32_t x = 0; x < WIDTH; x++)
    {
      depthBuffer[x+y*WIDTH] = -INFINITY;
    }
  }

  for(size_t t = 0; t < tris.size(); t++)
  {
    CanvasTriangle projection = modelToCanvas(tris[t]);
    computeDepth(projection, depthBuffer);
  }
}

#endif
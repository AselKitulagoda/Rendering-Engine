#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include "global.hpp"
#include "wireframe.hpp"
#include "rasteriser.hpp"

using namespace std;
using namespace glm;

// Raytracing Stuff
vec3 computeRayDirection(int x, int y);
RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles);
void drawRaytraced(vector<ModelTriangle> triangles);

// Lighting
float computeDotProduct(vec3 point, ModelTriangle t);
float calculateBrightness(int x, int y);

// Shadows
bool checkShadow(vec3 point, vector<ModelTriangle> triangles, int triangleIndex);

float computeDotProduct(vec3 point, ModelTriangle t)
{ 
  vec3 diff1 = t.vertices[1] - t.vertices[0];
  vec3 diff2 = t.vertices[2] - t.vertices[0];

  vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));
  vec3 pointToLight = glm::normalize(lightSource - point);

  float dotProduct = std::max(0.0f, (float) glm::dot(surfaceNormal, pointToLight));

  return dotProduct;
}

float calculateBrightness(vec3 point, ModelTriangle t)
{ 
  vec3 pointToLight = lightSource - point;
  float distance = glm::length(pointToLight);
  float dotProduct = computeDotProduct(point, t);

  float brightness = INTENSITY * dotProduct / (FRACTION_VAL * M_PI * distance * distance);

  if(brightness < (float) AMBIENCE)
  {
    brightness = (float) AMBIENCE;
  }
  if(brightness > 1.0f)
  {
    brightness = 1.0f;
  }

  return brightness;
}

vec3 computeRayDirection(int x, int y)
{
  vec3 rayDirection = glm::normalize((vec3((x - WIDTH/2), (-(y - HEIGHT/2)), FOCAL_LENGTH) - cameraPos) * cameraOrientation);
  return rayDirection;
}

bool checkShadow(vec3 point, vector<ModelTriangle> triangles, size_t triangleIndex) 
{
  vec3 rayShadow = glm::normalize(lightSource - point);
  float distanceFromLight = glm::length(rayShadow);

  bool isShadow = false;

  for(size_t i = 0; i < triangles.size(); i++)
  {
    ModelTriangle curr = triangles.at(i);
    vec3 e0 = curr.vertices[1] - curr.vertices[0];
    vec3 e1 = curr.vertices[2] - curr.vertices[0];
    vec3 SPVector = point - curr.vertices[0];
    mat3 DEMatrix(-rayShadow, e0, e1);

    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;

    if(inRange(u, 0.0, 1.0) && inRange(v, 0.0, 1.0) && (u+v <= 1.0) && (i != triangleIndex))
    {
      if(t < distanceFromLight && (abs(t - distanceFromLight) > (float) SHADOW_THRESH))
      {
        isShadow = true;
        break;
      }
    }
  }
  return isShadow;
}

RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles)
{ 
  RayTriangleIntersection result;
  result.distanceFromCamera = INFINITY;

  for(size_t i = 0; i < triangles.size(); i++)
  {
    ModelTriangle curr = triangles.at(i);
    vec3 e0 = curr.vertices[1] - curr.vertices[0];
    vec3 e1 = curr.vertices[2] - curr.vertices[0];
    vec3 SPVector = cameraPos - curr.vertices[0];
    mat3 DEMatrix(-rayDirection, e0, e1);

    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;

    if(inRange(u, 0.0, 1.0) && inRange(v, 0.0, 1.0) && (u+v <= 1.0))
    {
      if(t < result.distanceFromCamera)
      {
        vec3 point = curr.vertices[0] + (u * e0) + (v * e1);
        float brightness = calculateBrightness(point, curr);

        bool shadow = checkShadow(point, triangles, i);

        if(shadowMode)
        {
          if(shadow)
          {
            brightness = 0.15f;
          }
        }

        curr.colour = Colour(curr.colour.red * brightness, curr.colour.green * brightness, curr.colour.blue * brightness);

        result = RayTriangleIntersection(point, t, curr);
      }
    }
  }
  if(result.distanceFromCamera == INFINITY)
  {
    result.distanceFromCamera = -INFINITY;
  }
  return result;
}

void drawRaytraced(vector<ModelTriangle> triangles)
{ 
  window.clearPixels();
  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      vec3 ray = computeRayDirection(x, y);
      RayTriangleIntersection closestIntersect = getClosestIntersection(cameraPos, ray, triangles);
      if(closestIntersect.distanceFromCamera != -INFINITY)
      {
        window.setPixelColour(x, y, closestIntersect.intersectedTriangle.colour.pack());
      }
    }
  }
  cout << "RAYTRACING DONE" << endl;
}

#endif
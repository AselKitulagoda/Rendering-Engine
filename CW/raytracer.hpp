#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include "global.hpp"
#include "wireframe.hpp"
#include "rasteriser.hpp"

using namespace std;
using namespace glm;

// Raytracing Stuff
vec3 computeRayDirection(float x, float y);
RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles);
void drawRaytraced(vector<ModelTriangle> triangles);
Colour getAverageColour(vector<Colour> finalColours);
void drawRaytraceAntiAlias(vector<ModelTriangle> triangles);

// Lighting
float computeDotProduct(vec3 point, vec3 surfaceNormal, ModelTriangle t);
float calculateBrightness(vec3 point, ModelTriangle t, vec3 rayDirection, vector<ModelTriangle> triangles);

// Shadows
bool checkShadow(vec3 point, vec3 rayShadow, vector<ModelTriangle> alteredTriangles);

// Gouraud Shading
vector<float> calculateVertexBrightness(vector<ModelTriangle> triangles, ModelTriangle t, vec3 rayDirection);
float calculateAverageBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection);

// Reflection/Mirror
vec3 computeReflectedRay(vec3 incidentRay, ModelTriangle t);

float calculateBrightness(vec3 point, ModelTriangle t, vec3 rayDirection, vector<ModelTriangle> triangles)
{ 
  vec3 diff1 = t.vertices[1] - t.vertices[0];
  vec3 diff2 = t.vertices[2] - t.vertices[0];

  vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));

  vec3 pointToLight = lightSource - point;
  float distance = glm::length(pointToLight);

  float brightness = INTENSITY / (FRACTION_VAL * M_PI * distance * distance);

  float dotProduct = std::max(0.0f, (float) glm::dot(surfaceNormal, glm::normalize(pointToLight)));
  brightness *= pow(dotProduct, 1.0f);

  if(brightness < (float) AMBIENCE)
  {
    brightness = (float) AMBIENCE;
  }

  if(shadowMode)
  {
    vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
    bool shadow = checkShadow(point, pointToLight, alteredTriangles);
    if(shadow)
      brightness = 0.15f;
  } 

  if(brightness > 1.0f)
  {
    brightness = 1.0f;
  }

  return brightness;
}

vector<float> calculateVertexBrightness(vector<ModelTriangle> triangles, ModelTriangle t, vec3 rayDirection)
{
  vector<vec3> vertexNormals = getTriangleVertexNormals(t, triangleVertexNormals);
  vector<float> brightnessVals;
  for(int i = 0; i < 3; i++)
  {
    vec3 vertex = vertexNormals[i];
    vec3 vertexToLight = lightSource - t.vertices[i];
    float distance = glm::length(vertexToLight);

    float brightness = INTENSITY / (FRACTION_VAL * M_PI * distance * distance);

    float dotProduct = std::max(0.0f, (float) glm::dot(vertex, glm::normalize(vertexToLight)));
    brightness *= pow(dotProduct, 1.0f);

    // Specular Highlighting
    // vec3 flipped = -1.0f * rayDirection;
    // vec3 reflected = vertexToLight - (2.0f * vertex * glm::dot(vertexToLight, vertex));
    // float angle = std::max(0.0f, glm::dot(glm::normalize(flipped), glm::normalize(reflected)));
    // brightness += pow(angle, 1.0f);

    if(brightness < (float) AMBIENCE)
    {
      brightness = (float) AMBIENCE;
    }

    if(shadowMode)
    {
      vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
      bool shadow = checkShadow(t.vertices[i], vertexToLight, alteredTriangles);
      if(shadow)
        brightness = 0.15f;
    } 

    if(brightness > 1.0f)
    {
      brightness = 1.0f;
    }

    brightnessVals.push_back(brightness);
  }
  return brightnessVals;
}

float calculateAverageBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection)
{
  vector<float> brightnessVals = calculateVertexBrightness(triangles, t, rayDirection);
  float e0 = brightnessVals[1] - brightnessVals[0];
  float e1 = brightnessVals[2] - brightnessVals[0];

  float result = brightnessVals[0] + (u * e0) + (v * e1);

  return result;
}

vec3 computeRayDirection(float x, float y)
{
  vec3 rayDirection = glm::normalize((vec3((x - WIDTH/2), (-(y - HEIGHT/2)), FOCAL_LENGTH) - cameraPos) * cameraOrientation);
  return rayDirection;
}

bool checkShadow(vec3 point, vec3 rayShadow, vector<ModelTriangle> alteredTriangles) 
{
  float distanceFromLight = glm::length(rayShadow);
  
  bool isShadow = false;

  for(size_t i = 0; i < alteredTriangles.size(); i++)
  {
    ModelTriangle curr = alteredTriangles.at(i);
    vec3 e0 = curr.vertices[1] - curr.vertices[0];
    vec3 e1 = curr.vertices[2] - curr.vertices[0];
    vec3 SPVector = point - curr.vertices[0];
    mat3 DEMatrix(-glm::normalize(rayShadow), e0, e1);

    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;

    if(inRange(u, 0.0, 1.0) && inRange(v, 0.0, 1.0) && (u+v <= 1.0))
    {
      if(t < distanceFromLight && t > 0.0f)
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
        float brightness = calculateBrightness(point, curr, rayDirection, triangles);

        vec2 e0_tex = curr.texturepoints[1]*300.0f - curr.texturepoints[0]*300.0f;
        vec2 e1_tex = curr.texturepoints[2]*300.0f - curr.texturepoints[0]*300.0f;
        vec2 tex_point_final = curr.texturepoints[0]*300.0f + (u * e0_tex) + (v * e1_tex);

        uint32_t intersection_col = pixelColours[round(tex_point_final.x) + round(tex_point_final.y) * texWidth];
        
        Colour colour = Colour();
        vec3 newColour;

        if(curr.tag == "cornell")
        {
          newColour = vec3(curr.colour.red, curr.colour.green, curr.colour.blue);
          colour = Colour(newColour.x, newColour.y, newColour.z);
          colour.brightness = brightness;
        }
        else if(curr.tag == "hackspace")
        { 
          newColour = unpackColour(intersection_col);
          colour = Colour(newColour.x, newColour.y, newColour.z);
          colour.brightness = brightness;
        }
        else if(curr.tag == "sphere")
        {
          newColour = vec3(curr.colour.red, curr.colour.green, curr.colour.blue);
          brightness = calculateAverageBrightness(triangles, point, curr, u, v, rayDirection);
          colour = Colour(newColour.x, newColour.y, newColour.z);
          colour.brightness = brightness;
        }
        result = RayTriangleIntersection(point, t, curr, colour);
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
  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      vec3 ray = computeRayDirection((float) x, (float) y);
      RayTriangleIntersection closestIntersect = getClosestIntersection(cameraPos, ray, triangles);
      
      // Do reflection
      if(reflectiveMode)
      {
        if(closestIntersect.intersectedTriangle.colour.reflectivity > 0.0f)
        { 
          vector<ModelTriangle> reflectionTriangles = removeIntersectedTriangle(triangles, closestIntersect.intersectedTriangle);
          vec3 incidentRay = glm::normalize(closestIntersect.intersectionPoint - cameraPos);
          vec3 reflectedRay = computeReflectedRay(incidentRay, closestIntersect.intersectedTriangle);
          RayTriangleIntersection mirrorIntersect = getClosestIntersection(closestIntersect.intersectionPoint, reflectedRay, reflectionTriangles);
          if(mirrorIntersect.distanceFromCamera == -INFINITY) closestIntersect.colour = Colour(0, 0, 0);
          else closestIntersect.colour = mirrorIntersect.colour;
        }
      }

      if(closestIntersect.distanceFromCamera != -INFINITY)
      {
        window.setPixelColour(x, y, closestIntersect.colour.packWithBrightness());
      }
    }
  }
  cout << "RAYTRACING DONE" << endl;
}

Colour getAverageColour(vector<Colour> finalColours)
{
  Colour average = Colour(finalColours.at(0).red * 2, finalColours.at(0).green * 2, finalColours.at(0).blue * 2, finalColours.at(0).brightness * 2);
  for(size_t i = 1; i < finalColours.size(); i++)
  {
    int red = average.red + finalColours.at(i).red;
    int green = average.green + finalColours.at(i).green;
    int blue = average.blue + finalColours.at(i).blue;
    float brightness = average.brightness + finalColours.at(i).brightness;

    average = Colour(red, green, blue, brightness);
  }
  int denom = finalColours.size() + 2;
  average = Colour(average.red/denom, average.green/denom, average.blue/denom, average.brightness/denom);
  Colour toReturn = Colour(average.red, average.green, average.blue, average.brightness);
  return toReturn;
}

void drawRaytraceAntiAlias(vector<ModelTriangle> triangles)
{
  vector<vec2> quincunx;
  quincunx.push_back(vec2(0.0f, 0.0f));
  quincunx.push_back(vec2(0.5f, 0.0f));
  quincunx.push_back(vec2(-0.5f, 0.0f));
  quincunx.push_back(vec2(0.0f, 0.5f));
  quincunx.push_back(vec2(0.0f, -0.5f));

  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      vector<Colour> finalColours;

      for(size_t i = 0; i < quincunx.size(); i++)
      {
        vec3 ray = computeRayDirection(x + quincunx.at(i).x, y + quincunx.at(i).y);
        RayTriangleIntersection closestIntersect = getClosestIntersection(cameraPos, ray, triangles);
        if(closestIntersect.distanceFromCamera != -INFINITY)
        {
          finalColours.push_back(closestIntersect.colour);
        }
      }
      if(finalColours.size() > 0)
      {
        Colour c = getAverageColour(finalColours);
        window.setPixelColour(x, y, c.packWithBrightness());
      }
    }
  }
  cout << "RAYTRACE ANTI-ALIAS DONE" << endl;
}

// Reflection/Mirror stuff
vec3 computeReflectedRay(vec3 incidentRay, ModelTriangle t)
{
  vec3 diff1 = t.vertices[1] - t.vertices[0];
  vec3 diff2 = t.vertices[2] - t.vertices[0];

  vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));

  vec3 reflected = incidentRay - (2.0f * surfaceNormal * glm::dot(incidentRay, surfaceNormal));
  return reflected;
}

#endif
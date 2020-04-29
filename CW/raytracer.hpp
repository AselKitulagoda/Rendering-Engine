#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include "global.hpp"
#include "wireframe.hpp"
#include "rasteriser.hpp"

using namespace std;
using namespace glm;

// Raytracing Stuff
vec3 computeRayDirection(float x, float y);
vec3 calculateSurfaceNormal(ModelTriangle t);
RayTriangleIntersection getTriangleIntersection(vec3 viewPoint, vec3 rayDirection, ModelTriangle triangle);
RayTriangleIntersection getClosestIntersection(vec3 viewPoint, vec3 rayDirection, vector<ModelTriangle> triangles, RayTriangleIntersection* originalIntersection, int depth);
void drawRaytraced(vector<ModelTriangle> triangles);
Colour getAverageColour(vector<Colour> finalColours);
void drawRaytraceAntiAlias(vector<ModelTriangle> triangles);

// Reflection stuff
vec3 calculateReflectedRay(vec3 incidentRay, ModelTriangle t);

// Refraction stuff
vec3 calculateRefractedRay(vec3 rayDirection, vec3 surfaceNormal, float ior);

// Lighting
float computeDotProduct(vec3 point, vec3 surfaceNormal, ModelTriangle t);
float calculateBrightness(vec3 point, ModelTriangle t, vec3 rayDirection, vector<ModelTriangle> triangles);

// Shadows
bool checkHardShadow(vec3 point, vec3 rayShadow, vector<ModelTriangle> alteredTriangles);
float checkSoftShadow(vec3 point, vector<ModelTriangle> alteredTriangles);

// Smooth Shading
vector<float> calculateVertexBrightness(vector<ModelTriangle> triangles, ModelTriangle t, vec3 rayDirection);
float calculateGouraudBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection);
float calculatePhongBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection);

// Bump Mapping
float calculateBumpBrightness(vec3 point, ModelTriangle t, vec3 rayDirection, vector<ModelTriangle> triangles, vec3 triangleNormal);


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

  // Specular Highlighting
  if(reflectiveMode || metallicMode)
  {
    vec3 flipped = -1.0f * rayDirection;
    vec3 reflected = pointToLight - (2.0f * point * glm::dot(pointToLight, point));
    float angle = std::max(0.0f, glm::dot(glm::normalize(flipped), glm::normalize(reflected)));
    brightness += pow(angle, 10.0f);
  }

  if(brightness < (float) AMBIENCE)
  {
    brightness = (float) AMBIENCE;
  }

  if(hardShadowMode)
  {
    vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
    bool hardShadow = checkHardShadow(point, pointToLight, alteredTriangles);
    if(hardShadow)
      brightness = 0.15f;
  } 

  if(softShadowMode)
  {
    vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
    float penumbra = checkSoftShadow(point, alteredTriangles);
    brightness *= (1 - penumbra);
    if(brightness < 0.15f) brightness = 0.15f;
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
    vec3 surfaceNormal = calculateSurfaceNormal(t);
    vec3 reflected = calculateReflectedRay(rayDirection, t);
    float angle = std::max(0.0f, glm::dot(reflected, surfaceNormal));
    brightness += pow(angle, 10.0f);

    if(brightness < (float) AMBIENCE)
    {
      brightness = (float) AMBIENCE;
    }

    if(brightness > 1.0f)
    {
      brightness = 1.0f;
    }

    brightnessVals.push_back(brightness);
  }
  return brightnessVals;
}

float calculateGouraudBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection)
{
  vector<float> brightnessVals = calculateVertexBrightness(triangles, t, rayDirection);
  float b0 = brightnessVals[1] - brightnessVals[0];
  float b1 = brightnessVals[2] - brightnessVals[0];

  float result = brightnessVals[0] + (u * b0) + (v * b1);

  return result;
}

float calculatePhongBrightness(vector<ModelTriangle> triangles, vec3 point, ModelTriangle t, float u, float v, vec3 rayDirection)
{
  vector<vec3> vertexNormals = getTriangleVertexNormals(t, triangleVertexNormals);
  vec3 n0 = vertexNormals[1] - vertexNormals[0];
  vec3 n1 = vertexNormals[2] - vertexNormals[0];

  vec3 adjustedNormal = vertexNormals[0] + (u * n0) + (v * n1);

  vec3 pointToLight = lightSource - point;
  float distance = glm::length(pointToLight);

  float brightness = INTENSITY / (FRACTION_VAL * M_PI * distance * distance);

  float dotProduct = std::max(0.0f, (float) glm::dot(adjustedNormal, glm::normalize(pointToLight)));
  brightness *= pow(dotProduct, 1.0f);

  // Specular Highlighting
  vec3 reflected = calculateReflectedRay(rayDirection, t);
  float angle = std::max(0.0f, glm::dot(reflected, adjustedNormal));
  brightness += pow(angle, 10.0f);

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

float calculateBumpBrightness(vec3 point, ModelTriangle t, vec3 rayDirection, vector<ModelTriangle> triangles, vec3 triangleNormal)
{
  vec3 pointToLight = lightSource - point;
  float distance = glm::length(pointToLight);

  float brightness = INTENSITY / (FRACTION_VAL * M_PI * distance * distance);

  float dotProduct = std::max(0.0f, (float) glm::dot(triangleNormal, glm::normalize(pointToLight)));
  brightness *= pow(dotProduct, 1.0f);

  // Specular Highlighting
  vec3 reflected = calculateReflectedRay(rayDirection, t);
  float angle = std::max(0.0f, glm::dot(reflected, triangleNormal));
  brightness += pow(angle, 10.0f);

  if(brightness < (float) AMBIENCE)
  {
    brightness = (float) AMBIENCE;
  }

  if(hardShadowMode)
  {
    vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
    bool hardShadow = checkHardShadow(point, pointToLight, alteredTriangles);
    if(hardShadow)
      brightness = 0.15f;
  } 

  if(softShadowMode)
  {
    vector<ModelTriangle> alteredTriangles = removeIntersectedTriangle(triangles, t);
    float penumbra = checkSoftShadow(point, alteredTriangles);
    brightness *= (1 - penumbra);
    if(brightness < 0.15f) brightness = 0.15f;
  }

  if(brightness > 1.0f)
  {
    brightness = 1.0f;
  }
  return brightness;
}

vec3 computeRayDirection(float x, float y)
{
  vec3 rayDirection = glm::normalize((vec3((x - WIDTH/2), (-(y - HEIGHT/2)), FOCAL_LENGTH) - cameraPos) * cameraOrientation);
  return rayDirection;
}

bool checkHardShadow(vec3 point, vec3 rayShadow, vector<ModelTriangle> alteredTriangles) 
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

float checkSoftShadow(vec3 point, vector<ModelTriangle> alteredTriangles)
{
  float penumbra = 0.0f;
  int count = 0;
  for(size_t i = 0; i < lightSources.size(); i++)
  {
    vec3 rayShadow = lightSources.at(i) - point;
    bool shadow = checkHardShadow(point, rayShadow, alteredTriangles);
    if(shadow) count += 1;
  }
  penumbra = count / (float) lightSources.size();
  return penumbra;
}

vec3 calculateSurfaceNormal(ModelTriangle t)
{
  vec3 diff1 = t.vertices[1] - t.vertices[0];
  vec3 diff2 = t.vertices[2] - t.vertices[0];

  vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));

  return surfaceNormal;
}

vec3 calculateReflectedRay(vec3 incidentRay, ModelTriangle t)
{
  vec3 surfaceNormal = calculateSurfaceNormal(t);
  vec3 reflectedRay = incidentRay - 2.0f * (surfaceNormal * glm::dot(incidentRay, surfaceNormal));
  return glm::normalize(reflectedRay);
}

vec3 calculateMetalRay(vec3 incidentRay, ModelTriangle t)
{
  vec3 surfaceNormal = calculateSurfaceNormal(t);
  float roughness = generateRandomNum(-0.1f, 0.1f);
  vec3 reflectedRay = incidentRay - (2.0f + roughness) * (surfaceNormal * glm::dot(incidentRay, surfaceNormal));
  return glm::normalize(reflectedRay);
}

vec3 calculateRefractedRay(vec3 rayDirection, vec3 surfaceNormal, float ior)
{
  float cosi = glm::dot(rayDirection, surfaceNormal);

  // clamp cosi between -1 and 1
  if(cosi < -1) cosi = -1;
  else if(cosi > 1) cosi = 1;

  float etai = 1, etat = ior;

  vec3 n = surfaceNormal;

  if(cosi < 0)
  {
    cosi = -cosi;
  }
  else
  {
    std::swap(etai, etat);
    n = -surfaceNormal;
  }

  float eta = etai / etat;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  
  return k < 0 ? vec3(0, 0, 0) : eta * rayDirection + (eta * cosi - sqrtf(k)) * n;
}

float fresnel(vec3 rayDirection, vec3 surfaceNormal, float ior)
{
  float cosi = glm::dot(rayDirection, surfaceNormal);

  // clamp cosi between -1 and 1
  if(cosi < -1) cosi = -1;
  else if(cosi > 1) cosi = 1;

  float etai = 1;
  float etat = ior;

  if(cosi > 0) std::swap(etai, etat);

  float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
  float kr;
  if(sint >= 1) kr = 1;
  else
  {
    float cost = sqrtf(std::max(0.0f, 1 - sint * sint));
    cosi = fabsf(cosi);
    float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
    kr = (Rs * Rs + Rp * Rp) / 2;
  }
  return kr;
}

bool checkBackface(vec3 rayDirection, ModelTriangle t)
{
  vec3 surfaceNormal = calculateSurfaceNormal(t);
  float val = glm::dot(surfaceNormal, -rayDirection);

  if(val > 0.0f) return true;
  else return false;
}

RayTriangleIntersection getTriangleIntersection(vec3 viewPoint, vec3 rayDirection, ModelTriangle triangle)
{
  RayTriangleIntersection result;
  result.intersectionPoint = vec3(-1, -1, -1);
  result.distanceFromCamera = INFINITY;
  result.u = -1; result.v = -1;

  vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
  vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
  vec3 SPVector = viewPoint - triangle.vertices[0];
  mat3 DEMatrix(-rayDirection, e0, e1);

  vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

  float t = possibleSolution.x;
  float u = possibleSolution.y;
  float v = possibleSolution.z;

  if(inRange(u, 0.0, 1.0) && inRange(v, 0.0, 1.0) && (u + v <= 1.0))
  {
    vec3 point = triangle.vertices[0] + (u * e0) + (v * e1);
    result = RayTriangleIntersection(point, t, triangle, triangle.colour, u, v);
  }
  return result;
}

RayTriangleIntersection getClosestIntersection(vec3 viewPoint, vec3 rayDirection, vector<ModelTriangle> triangles, RayTriangleIntersection* originalIntersection, int depth)
{
  RayTriangleIntersection result;
  result.distanceFromCamera = INFINITY;
  result.colour = Colour(0, 0, 0);

  // Initialising the new colour
  Colour newColour;

  // Recurse only upto 5 times
  if(depth < 5)
  {
    // Set base distance to inf
    float minDist = INFINITY;

    #pragma omp parallel
    #pragma omp for
    // Loop through all model triangles, find the closest intersection
    for(size_t i = 0; i < triangles.size(); i++)
    {
      // Do culling only when reflective mode is off
      bool cull;
      if(!reflectiveMode) cull = checkBackface(rayDirection, triangles.at(i));

      // Backface culling and Bounding Box clipping
      if(triangles.at(i).boundingBoxVisible || triangles.at(i).refractive)
      {
        // Find the triangle intersection
        RayTriangleIntersection intersection = getTriangleIntersection(viewPoint, rayDirection, triangles.at(i));

        // Checking if its a valid intersection
        bool hit = (originalIntersection != nullptr && !compareModel(triangles.at(i), originalIntersection->intersectedTriangle)) || (originalIntersection == nullptr);

        // Update the minimum distance
        if(intersection.distanceFromCamera < minDist && hit)
        {
          result = intersection;
          minDist = intersection.distanceFromCamera;
        }
      }
    }

    // If triangle intersection is found, colour accordingly
    if(result.distanceFromCamera != INFINITY)
    {
      ModelTriangle curr = result.intersectedTriangle;
      vec3 point = result.intersectionPoint;

      // if texture is glass
      if(refractiveMode && curr.refractive)
      {
        // Do reflection
        vec3 incidentRay = rayDirection;
        vec3 reflectedRay = calculateReflectedRay(incidentRay, curr);

        // Find the closest intersection with reflected ray
        RayTriangleIntersection reflectionIntersect = getClosestIntersection(point, reflectedRay, triangles, &result, depth + 1);
        Colour r = reflectionIntersect.colour;
        vec3 reflectionColour = vec3(r.red, r.green, r.blue);

        // Do refraction
        vec3 surfaceNormal = calculateSurfaceNormal(curr);
        vec3 refractedRay = calculateRefractedRay(rayDirection, surfaceNormal, 1.5);

        // Find the closest intersection with refracted ray
        RayTriangleIntersection refractionIntersect = getClosestIntersection(point, refractedRay, triangles, &result, depth + 1);
        Colour c = refractionIntersect.colour;
        vec3 refractionColour = vec3(c.red, c.green, c.blue);

        // Doing fresnel
        float kr = fresnel(rayDirection, surfaceNormal, 1.5);
        vec3 combined = reflectionColour * kr + refractionColour * (1 - kr);

        if(refractedRay == vec3(0, 0, 0)) combined = reflectionColour;

        newColour = Colour(combined.x, combined.y, combined.z);
      }
      
      // if texture is mirror
      else if(reflectiveMode && curr.reflective)
      {
        vec3 incidentRay = rayDirection;
        vec3 reflectedRay = calculateReflectedRay(incidentRay, curr);

        // Find the closest intersection with reflected ray
        RayTriangleIntersection reflectionIntersect = getClosestIntersection(point, reflectedRay, triangles, &result, depth + 1);

        // Check if distance is not inf
        if(reflectionIntersect.distanceFromCamera == INFINITY) newColour = Colour(0, 0, 0);
        else newColour = reflectionIntersect.colour;
      }

      // if texture is metal
      else if(metallicMode && curr.metallic)
      {
        vec3 incidentRay = rayDirection;
        vec3 metalRay = calculateMetalRay(incidentRay, curr);
        
        // Find the closest intersection with reflected ray
        RayTriangleIntersection metalIntersect = getClosestIntersection(point, metalRay, triangles, &result, depth + 1);

        // Set the metal colour
        if(metalIntersect.distanceFromCamera == INFINITY) newColour = Colour(0, 0, 0);
        else
        {
          int mRed = (metalIntersect.colour.red * 0.3 + curr.colour.red * 0.7);
          int mGreen = (metalIntersect.colour.green * 0.3 + curr.colour.green * 0.7);
          int mBlue = (metalIntersect.colour.blue * 0.3 + curr.colour.blue * 0.7);

          newColour = Colour(mRed, mGreen, mBlue);
        }
      }
   
      // Perspective texture mapping on left wall
      else if(curr.tag == "checker")
      {
        // Find texture points of intersection point
        vec2 e0_tex = curr.texturepoints[1] * 738.f - curr.texturepoints[0] * 738.f;
        vec2 e1_tex = curr.texturepoints[2] * 738.f - curr.texturepoints[0] * 738.f;
        vec2 tex_point_final = curr.texturepoints[0] * 738.f + (result.u * e0_tex) + (result.v * e1_tex);

        // Get the respective pixel colour
        uint32_t intersectionCol = checkcols[round(tex_point_final.x) + round(tex_point_final.y) * 738];

        // Unpacking the intersection colour
        vec3 unpacked = unpackColour(intersectionCol);
        newColour = Colour(unpacked.x, unpacked.y, unpacked.z);
        newColour.brightness = calculateBrightness(point, curr, rayDirection, triangles);
      }

      // Perspective texture mapping on hackspace logo
      else if(curr.tag == "hackspace")
      {
        // Find texture points of intersection point
        vec2 e0_tex = curr.texturepoints[1] * 300.f - curr.texturepoints[0] * 300.f;
        vec2 e1_tex = curr.texturepoints[2] * 300.f - curr.texturepoints[0] * 300.f;
        vec2 tex_point_final = curr.texturepoints[0] * 300.f + (result.u * e0_tex) + (result.v * e1_tex);

        // Get the respective pixel colour
        uint32_t intersectionCol = 0;

        if(int(tex_point_final.x) <= 300 && int(tex_point_final.x) >= 0 && int(tex_point_final.y) >= 0 && int(tex_point_final.y) <= 300)
          intersectionCol = pixelColours[(int(tex_point_final.x) - 1) + (int(tex_point_final.y) - 1) * 300];

        // Unpacking the intersection colour
        vec3 unpacked = unpackColour(intersectionCol);
        newColour = Colour(unpacked.x, unpacked.y, unpacked.z);
        newColour.brightness = calculateBrightness(point, curr, rayDirection, triangles);
      }

      // Shading the Sphere
      else if(curr.tag == "sphere")
      {
        // Gouraud shading
        if(gouraudMode)
        {
          newColour = curr.colour;
          newColour.brightness = calculateGouraudBrightness(triangles, point, curr, result.u, result.v, rayDirection);
        }

        // Phong shading
        else if(phongMode)
        {
          newColour = curr.colour;
          newColour.brightness = calculatePhongBrightness(triangles, point, curr, result.u, result.v, rayDirection);
        }

        // Flat shading otherwise
        else
        {
          newColour = curr.colour;
          newColour.brightness = calculateBrightness(point, curr, rayDirection, triangles);
        }
      }

      // Bump mapping the right wall
      else if(curr.tag == "bump")
      {
        newColour = curr.colour;

        // Getting the bump points of the intersection point
        vec2 e0_bump = curr.texturepoints[1] * 949.f - curr.texturepoints[0] * 949.f;
        vec2 e1_bump = curr.texturepoints[2] * 949.f - curr.texturepoints[0] * 949.f;
        vec2 bump_point = curr.texturepoints[0] * 949.f + (result.u * e0_bump) + (result.v * e1_bump);

        // Checking if in range
        if(int(bump_point.x) >= 0 && int(bump_point.x) <= 949 && int(bump_point.y) >= 0 && int(bump_point.y) <= 949)
        {
          vec3 bump_normal = bumpNormals[int(bump_point.x) + int(bump_point.y) * 949];
          newColour.brightness = calculateBumpBrightness(point, curr, rayDirection, triangles, bump_normal);
        }
      }

      // Regular flat shading
      else
      {
        newColour = curr.colour;
        newColour.brightness = calculateBrightness(point, curr, rayDirection, triangles);
      }
    }
    result.colour = newColour;
  }
  return result;
}

void drawRaytraced(vector<ModelTriangle> triangles)
{
  #pragma omp parallel
  #pragma omp for
  for(int x = 0; x < WIDTH; x++)
  {
    for(int y = 0; y < HEIGHT; y++)
    {
      vec3 ray = computeRayDirection(float(x), float(y));

      // Find the closest intersection
      RayTriangleIntersection closestIntersection = getClosestIntersection(cameraPos, ray, triangles, nullptr, 1);

      // set pixel colour if distance != inf
      if(closestIntersection.distanceFromCamera < INFINITY)
      {
        window.setPixelColour(x, y, closestIntersection.colour.packWithBrightness());
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

  #pragma omp parallel
  #pragma omp for
  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      vector<Colour> finalColours;

      for(size_t i = 0; i < quincunx.size(); i++)
      {
        vec3 ray = computeRayDirection(x + quincunx.at(i).x, y + quincunx.at(i).y);
        RayTriangleIntersection closestIntersection = getClosestIntersection(cameraPos, ray, triangles, nullptr, 1);

        if(closestIntersection.distanceFromCamera < INFINITY)
        {
          finalColours.push_back(closestIntersection.colour);
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

#endif

#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <RayTriangleIntersection.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

#define WIDTH 640
#define HEIGHT 480
#define SCALE_FACTOR 0.3
#define FOCAL_LENGTH -500
#define INTENSITY 1
#define AMBIENCE 0.2
#define FRACTION_VAL 0.5
#define SHADOW_THRESH 0.1
#define MTLPATH "cornell-box.mtl"
#define OBJPATH "cornell-box.obj"

#define CAMERA_X 0
#define CAMERA_Y 0.5
#define CAMERA_Z 6

// OBJ Stuff
vector<ModelTriangle> readObj(float scale);
vector<Colour> readMaterial(string fname);

// Drawing Stuff
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c, float depthBuffer[WIDTH][HEIGHT]);
void drawStroke(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawWireframe(vector<ModelTriangle> triangles);
void drawRasterised(vector<ModelTriangle> triangles);

// Projection Stuff
CanvasTriangle modelToCanvas(ModelTriangle modelTrig);

// Camera Stuff
vec3 cameraTranslate(vec3 direction);
mat3 rotateX(double theta, mat3 cameraOrien);
mat3 rotateY(double theta, mat3 cameraOrien);
mat3 lookAt(vec3 point);
vec3 orbit(vec3 point, double orbitTheta);

// Raytracing Stuff
vec3 computeRayDirection(float x, float y);
RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles);
void drawRaytraced(vector<ModelTriangle> triangles);
Colour getAverageColour(vector<Colour> finalColours);
void drawRaytraceAntiAlias(vector<ModelTriangle> triangles);

// Lighting
float computeDotProduct(vec3 point, ModelTriangle t);
float calculateBrightness(int x, int y);

// Shadows
bool checkShadow(vec3 point, vector<ModelTriangle> triangles, int triangleIndex);

// Display and Event Stuff
void update();
void handleEvent(SDL_Event event);
void printVec3(string text, vec3 vector)
{
  cout << text << " = " << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << endl;
}

// Defining the Global Variables
int bool_flag = -1;
vec3 cameraPos(CAMERA_X, CAMERA_Y, CAMERA_Z);
mat3 cameraOrientation = mat3(1.0f);
vector<Colour> colours = readMaterial(MTLPATH);
vector<ModelTriangle> triangles = readObj(SCALE_FACTOR);
vec3 lightSource = vec3(-0.0315915, 1.20455, -0.6108);
int shadowMode = 0;

Colour getColourFromName(string mat, vector<Colour> colours)
{
  Colour result = Colour(0, 0, 0);
  for(size_t i = 0; i < colours.size(); i++)
  {
    if(mat == colours[i].name)
    {
      result.red = colours[i].red;
      result.blue = colours[i].blue;
      result.green = colours[i].green;
      break;
    }
  }
  return result;
}

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{
  SDL_Event event;

  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event))
    {
      handleEvent(event);
    }
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
  if (bool_flag == 0)
  {
    drawWireframe(triangles);
  }
  else if (bool_flag == 1)
  {
    drawRasterised(triangles);
  }
}

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

vector<Colour> readMaterial(string fname)
{
  ifstream fp;
  fp.open(fname);

  vector<Colour> colours;
  while(!fp.eof())
  {
    string comment, colourInfo, newline;
    getline(fp, comment);
    string *splitName = split(comment, ' ');

    getline(fp, colourInfo);
    string *splitColourInfo = split(colourInfo, ' ');

    int r = stof(splitColourInfo[1]) * 255;
    int g = stof(splitColourInfo[2]) * 255;
    int b = stof(splitColourInfo[3]) * 255;

    Colour c = Colour(splitName[1], r, g, b);

    getline(fp, newline);
    colours.push_back(c);
  }
  fp.close();
    // cout << "finished mat"<<endl;
  return colours;
}

vector<ModelTriangle> readObj(float scale)
{
  vector <ModelTriangle> tris;
  ifstream fp;
  vector<Colour> colours = readMaterial(MTLPATH);
  vector<vec3> vertic;
  fp.open(OBJPATH);

  if(fp.fail())
    cout << "fails" << endl;

  string newline;
  getline(fp,newline);
  getline(fp,newline);

  while(!fp.eof())
  {
    string light;
    string comment;
    string mat;
    getline(fp, comment);

    if (!comment.empty())
    {
      string *splitcomment = split(comment,' ');
      if (splitcomment[0] == "usemtl")
      {
        mat = splitcomment[1];
        while(true)
        {
          getline(fp,comment);
          if (!comment.empty())
          {
            string *splitcomment = split(comment,' ');
            if (splitcomment[0]=="v")
            {
              float x = stof(splitcomment[1]) * scale;
              float y = stof(splitcomment[2]) * scale;
              float z = stof(splitcomment[3]) * scale;
              vec3 verts = vec3(x,y,z);
              vertic.push_back(verts);
            }
            else
            {
              break;
            }
          }
        }
      }
    }
  }

  fp.clear();
  fp.seekg(0,ios::beg);
  if(fp.fail())
    cout << "fails" << endl;
  getline(fp,newline);
  getline(fp,newline);

  while(!fp.eof())
  {
    string comment_new;
    string mat;
    getline(fp,comment_new);
    if (!comment_new.empty())
    {
      string *splitcomment = split(comment_new,' ');
      if (splitcomment[0] == "usemtl")
      {
        mat = splitcomment[1];
        Colour tricolour = getColourFromName(mat,colours);
        tricolour.name = mat;

        while(true){
          getline(fp,comment_new);
          splitcomment = split(comment_new,' ');
          if (splitcomment[0] == "f" && !comment_new.empty()){break;}
        }

      bool not_reach = true;
        while (not_reach)
        {
          if (!comment_new.empty())
          {
            splitcomment = split(comment_new,' ');
            if (splitcomment[0]=="f")
            {
              int first_vert = stoi(splitcomment[1].substr(0, splitcomment[1].size()-1));
              int second_vert = stoi(splitcomment[2].substr(0, splitcomment[2].size()-1));
              int third_vert = stoi(splitcomment[3].substr(0, splitcomment[3].size()-1));

              tris.push_back(ModelTriangle(vertic[first_vert-1],vertic[second_vert-1],vertic[third_vert-1],tricolour));
            }
            else{break;}

          }
          if (!fp.eof()){
          getline(fp,comment_new);
          }
          else{not_reach=false;}

        }
      }
    }
  }
  fp.close();
  return tris;
}

CanvasTriangle modelToCanvas(ModelTriangle modelTrig)
{
    float f = -3;
    CanvasTriangle canvasTrig = CanvasTriangle();
    canvasTrig.colour = modelTrig.colour;
    for(int i=0; i<3 ;i++) {
        float xdistance = modelTrig.vertices[i].x-cameraPos.x;
        float ydistance = modelTrig.vertices[i].y-cameraPos.y;
        float zdistance = modelTrig.vertices[i].z-cameraPos.z;
        vec3 cameraToVertex = vec3(xdistance, ydistance, zdistance);
        vec3 adjustedVector = cameraOrientation * cameraToVertex;
        float pScreen = f/adjustedVector.z;
        // Scale up the x and y canvas coords to get a bigger image (rather than a big model loader scaling)
        float canvasScaling = 150;
        float xProj = (adjustedVector.x*pScreen*canvasScaling) + WIDTH/2;
        float yProj = (-adjustedVector.y*pScreen*canvasScaling) + HEIGHT/2;
        CanvasPoint p = CanvasPoint(xProj, yProj);
        p.depth = 1.0/adjustedVector.z;
        canvasTrig.vertices[i] = p;
    }
    return canvasTrig;
}

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

void drawWireframe(vector<ModelTriangle> triangles) 
{
  float depthBuffer[WIDTH][HEIGHT];
  for(int i = 0; i < WIDTH; i++) 
  {
    for(int j = 0; j < HEIGHT; j++) 
    {
      depthBuffer[i][j] = (float) INFINITY;
    }
  }

  for(size_t i = 0; i < triangles.size(); i++)
  {
    CanvasTriangle projection = modelToCanvas(triangles.at(i));
    drawStroke(projection, depthBuffer);
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

  for(size_t i = 0; i < triangles.size(); i++)
  {
    CanvasTriangle projection = modelToCanvas(triangles.at(i));
    drawFilled(projection, depthBuffer);
  }
}

bool inRange(float val, float v1, float v2)
{
  if(val >= v1 && val <= v2) return true;
  else return false;
}

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

vec3 computeRayDirection(float x, float y)
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

        curr.colour = Colour(curr.colour.red, curr.colour.green, curr.colour.blue, brightness);

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
      vec3 ray = computeRayDirection((float) x, (float) y);
      RayTriangleIntersection closestIntersect = getClosestIntersection(cameraPos, ray, triangles);
      if(closestIntersect.distanceFromCamera != -INFINITY)
      {
        Colour c = Colour(closestIntersect.intersectedTriangle.colour.red * closestIntersect.intersectedTriangle.colour.brightness,
                          closestIntersect.intersectedTriangle.colour.green * closestIntersect.intersectedTriangle.colour.brightness,
                          closestIntersect.intersectedTriangle.colour.blue * closestIntersect.intersectedTriangle.colour.brightness);
        window.setPixelColour(x, y, c.pack());
      }
    }
  }
  cout << "RAYTRACING DONE" << endl;
}

Colour getAverageColour(vector<Colour> finalColours)
{
  Colour average = finalColours.at(0);
  for(size_t i = 1; i < finalColours.size(); i++)
  {
    int red = (average.red + finalColours.at(i).red) / 2;
    int green = (average.green + finalColours.at(i).green) / 2;
    int blue = (average.blue + finalColours.at(i).blue) / 2;
    float brightness = (average.brightness + finalColours.at(i).brightness) / 2;

    average = Colour(red, green, blue, brightness);
  }
  Colour toReturn = Colour(average.red * average.brightness, average.green * average.brightness, average.blue * average.brightness);
  return toReturn;
}

void drawRaytraceAntiAlias(vector<ModelTriangle> triangles)
{
  window.clearPixels();

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
          finalColours.push_back(closestIntersect.intersectedTriangle.colour);
        }
      }
      if(finalColours.size() > 0)
      {
        Colour c = getAverageColour(finalColours);
        window.setPixelColour(x, y, c.pack());
      }
    }
  }
  cout << "RAYTRACE ANTI-ALIAS DONE" << endl;
}

mat3 rotateX(double theta, mat3 cameraOrien) 
{
  mat3 newCameraOrien;
  mat3 rotMatrix = mat3(1.0f);

  theta = radians(theta);

  rotMatrix[0] = vec3(1.0, 0.0, 0.0);
  rotMatrix[1] = vec3(0.0, cos(theta), -sin(theta));
  rotMatrix[2] = vec3(0.0, sin(theta), cos(theta));

  newCameraOrien = rotMatrix * cameraOrien;
  return newCameraOrien;
}

mat3 rotateY(double theta, mat3 cameraOrien) 
{
  mat3 newCameraOrien;
  mat3 rotMatrix = mat3(1.0f);

  theta = radians(theta);

  rotMatrix[0] = vec3(cos(theta), 0.0, sin(theta));
  rotMatrix[1] = vec3(0.0, 1.0, 0.0);
  rotMatrix[2] = vec3(-sin(theta), 0.0, cos(theta));

  newCameraOrien = rotMatrix * cameraOrien;
  return newCameraOrien;
}

void resetCameraStuff()
{
  cameraPos = vec3(CAMERA_X, CAMERA_Y, CAMERA_Z);
  cameraOrientation = mat3(1.0f);
  lightSource = vec3(-0.0315915, 1.20455, -0.6108);
}

mat3 lookAt(vec3 point)
{
  mat3 newCameraOrientation;

  double tiltTheta = degrees(atan(point.z / (point.y - CAMERA_Y)));
  if(tiltTheta >= 0)
  {
    tiltTheta -= 90;
  }
  newCameraOrientation = rotateX(tiltTheta, newCameraOrientation);

  double panTheta = degrees(atan((point.x - CAMERA_X) / point.z));
  if(point.z < 0)
  {
    panTheta += 180;
  } 
  newCameraOrientation = rotateY(panTheta, mat3(1.0f));

  return newCameraOrientation;
}

vec3 orbit(vec3 point, double orbitTheta)
{
  vec3 newCameraPos;
  orbitTheta = radians(orbitTheta);

  newCameraPos.x = (point.x * cos(orbitTheta)) + (point.z * sin(orbitTheta));
  newCameraPos.y = point.y;
  newCameraPos.z = (-point.x * sin(orbitTheta)) + (point.z * cos(orbitTheta));

  return newCameraPos;
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    window.clearPixels();
    if(event.key.keysym.sym == SDLK_LEFT) // camera x translate
    {
      cout << "TRANSLATE LEFT" << endl;
      cameraPos.x += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_RIGHT) // camera x translate
    {
      cout << "TRANSLATE RIGHT" << endl;
      cameraPos.x -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_UP) // camera y translate
    {
      cout << "TRANSLATE UP" << endl;
      cameraPos.y -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_DOWN) // camera y translate
    {
      cout << "TRANSLATE DOWN" << endl;
      cameraPos.y += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_n) // light x translate
    {
      cout << "LIGHT RIGHT" << endl;
      lightSource.x += 0.1;
      // printVec3("light position", lightSource);
    }
    else if(event.key.keysym.sym == SDLK_v) // light x translate
    {
      cout << "LIGHT LEFT" << endl;
      lightSource.x -= 0.1;
      // printVec3("light position",lightSource);
    }
    else if(event.key.keysym.sym == SDLK_b) // light y translate
    {
      cout << "LIGHT DOWN" << endl;
      lightSource.y -= 0.1;
      // printVec3("light position", lightSource);
    }
    else if(event.key.keysym.sym == SDLK_g) // light y translate
    {
      cout << "LIGHT UP" << endl;
      lightSource.y += 0.1;
      // printVec3("light position", lightSource);
    }
    else if(event.key.keysym.sym == SDLK_f) // light z translate
    {
      cout << "LIGHT FRONT" << endl;
      lightSource.z -= 0.1;
      // printVec3("light position", lightSource);
    }
    else if(event.key.keysym.sym == SDLK_h) // light z translate
    {
      cout << "LIGHT BACK" << endl;
      lightSource.z += 0.1;
      // printVec3("light position", lightSource);
    }
    else if(event.key.keysym.sym == SDLK_z) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_x) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_c) // clear screen
    {
      cout << "CLEAR SCREEN" << endl;
      window.clearPixels();
      bool_flag = -2;
    }
    else if(event.key.keysym.sym == SDLK_w) // camera rotate X
    {
      cout << "ROTATE X" << endl;
      cameraOrientation = rotateX(1.0, cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_s) // camera rotate X
    {
      cout << "ROTATE X OTHER" << endl;
      cameraOrientation = rotateX(-1.0, cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_d) // camera rotate Y
    {
      cout << "ROTATE Y" << endl;
      cameraOrientation = rotateY(1.0, cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_a) // camera rotate Y
    {
      cout << "ROTATE Y OTHER" << endl;
      cameraOrientation = rotateY(-1.0, cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_o) // toggle shadow mode
    {
      if(shadowMode == 1) shadowMode = 0;
      else shadowMode = 1;
      cout << "SHADOW MODE = " << shadowMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_r) // camera reset position
    {
      cout << "RESET CAMERA POS" << endl;
      resetCameraStuff();
    }
    else if(event.key.keysym.sym == SDLK_j) // wireframe
    {
      cout << "DRAWING WIREFRAME" << endl;
      bool_flag = 0;
    }
    else if(event.key.keysym.sym == SDLK_k) // rasterised
    {
      cout << "DRAWING RASTERISED" << endl;
      bool_flag = 1;
    }
    else if(event.key.keysym.sym == SDLK_l) // raytraced
    {
      cout << "DRAWING RAYTRACED" << endl;
      drawRaytraced(triangles);
    }
    else if(event.key.keysym.sym == SDLK_m) // raytraced anti alias
    {
      cout << "DRAWING RAYTRACED ANTI ALIAS" << endl;
      drawRaytraceAntiAlias(triangles);
    }
    else if(event.key.keysym.sym == SDLK_q) // orbit
    {
      cout << "ORBIT" << endl;
      cameraPos = orbit(cameraPos, 3.0);
      cameraOrientation = lookAt(cameraPos);
    }
    else if(event.key.keysym.sym == SDLK_e) // orbit
    {
      cout << "ORBIT" << endl;
      cameraPos = orbit(cameraPos, -3.0);
      cameraOrientation = lookAt(cameraPos);
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
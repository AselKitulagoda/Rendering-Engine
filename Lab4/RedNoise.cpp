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
#define PI 3.1415
#define FOCAL_RAYTRACE -500
#define INTENSITY 1
#define AMBIENCE 0.2
#define FRACTION_VAL 0.5
#define MTLPATH "cornell-box.mtl"
#define OBJPATH "cornell-box.obj"

// OBJ Stuff
vector<ModelTriangle> readObj(float scale);
vector<Colour> readMaterial(string fname);

// Interpolation Function
vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, int numberOfValues);

// Wireframe Stuff
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c);
void drawStroke(CanvasTriangle t, Colour c);
void drawWireframe(vector<ModelTriangle> tris);

// Rasterising Stuff
CanvasTriangle modelToCanvas(ModelTriangle t);
void computeDepth(CanvasTriangle t, double *depthBuffer);
void depthBuffer(vector<ModelTriangle> tris);

// Raytracing Stuff
vec3 computeRayDirection(int x, int y);
RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles);
void drawRaytraced(vector<ModelTriangle> triangles);

// Lighting
float computeDotProduct(vec3 point, ModelTriangle t);
float calculateBrightness(int x, int y);

// Display and Event Stuff
void update();
void handleEvent(SDL_Event event);
void printVec3(string text, vec3 vector)
{
  cout << text << " = " << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << endl;
}

// Defining the Global Variables
int bool_flag = -1;
vec3 cameraPos(0, 0, 6);
mat3 cameraOrientation = mat3(vec3(1, 0, 0),
                              vec3(0, 1, 0),
                              vec3(0, 0, 1));
vector<Colour> colours = readMaterial(MTLPATH);
vector<ModelTriangle> triangles = readObj(SCALE_FACTOR);
vec3 lightSource = vec3(-0.0315915, 1.20455, -1.0108);

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
      if (bool_flag == 0){
        drawWireframe(triangles);
      }
      else if (bool_flag == 1){
        depthBuffer(triangles);
      }
    }
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
}

vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, int numberOfValues)
{
  vector<CanvasPoint> vals;
  for(int i = 0; i <= numberOfValues; i++)
  {
    CanvasPoint p;
    p.x = from.x + (i * (to.x - from.x)/numberOfValues);
    p.y = from.y + (i * (to.y - from.y)/numberOfValues);
    p.texturePoint.x = from.texturePoint.x + (i * (to.texturePoint.x - from.texturePoint.x)/numberOfValues);
    p.texturePoint.y = from.texturePoint.y + (i * (to.texturePoint.y - from.texturePoint.y)/numberOfValues);
    double depth = from.depth + (i * (to.depth - from.depth)/numberOfValues);
    p.depth = depth;
    vals.push_back(p);
  }
  return vals;
}

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

void drawWireframe(vector <ModelTriangle> tris)
{
  window.clearPixels();
  for (size_t i=0; i<tris.size(); i++){
    CanvasTriangle new_tri = modelToCanvas(tris[i]);
    drawStroke(new_tri, tris[i].colour);
  }
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

void depthBuffer(vector<ModelTriangle> tris)
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
  float distance = glm::distance(point, lightSource);
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
  vec3 rayDirection = glm::normalize((vec3((x - WIDTH/2), (-(y - HEIGHT/2)), FOCAL_RAYTRACE) - cameraPos) * cameraOrientation);
  return rayDirection;
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
        float brightness = calculateBrightness(closestIntersect.intersectionPoint, closestIntersect.intersectedTriangle);
        Colour newColour = Colour(closestIntersect.intersectedTriangle.colour.red * brightness,
                                  closestIntersect.intersectedTriangle.colour.green * brightness,
                                  closestIntersect.intersectedTriangle.colour.blue * brightness);
        window.setPixelColour(x, y, newColour.pack());
      }
    }
  }
  cout << "RAYTRACING DONE" << endl;
}

void rotateX(float theta)
{
  mat3 rot(vec3(1.0, 0.0, 0.0),
                  vec3(0.0, cos(theta), -sin(theta)),
                  vec3(0.0, sin(theta), cos(theta)));
  cameraOrientation = cameraOrientation * rot;
}

void rotateY(float theta)
{
  mat3 rot(vec3(cos(theta), 0.0, sin(theta)),
                  vec3(0.0, 1.0, 0.0),
                  vec3(-sin(theta), 0.0, cos(theta)));
  cameraOrientation = cameraOrientation * rot;
  // cout << glm::to_string(cameraOrientation) << '\n';
}

void rotateZ(float theta)
{
  mat3 rot(vec3(cos(theta), -sin(theta), 0.0),
                  vec3(sin(theta), cos(theta), 0.0),
                  vec3(0.0, 0.0, 1.0));
  cameraOrientation = cameraOrientation * rot;
}

void lookAt(vec3 point)
{
  vec3 forward = glm::normalize(vec3(cameraPos - point));
  vec3 right = glm::normalize(glm::cross(vec3(0, 1, 0), forward));
  vec3 up = glm::normalize(glm::cross(forward, right));

  cameraOrientation = mat3(right, up, forward);
}

void resetCameraStuff()
{
  cameraPos = vec3(0, 0, 6);
  cameraOrientation = mat3(vec3(1, 0, 0),
                           vec3(0, 1, 0),
                           vec3(0, 0, 1));
  lightSource = vec3(-0.0315915, 1.20455, -1.0108);
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
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
      rotateX(0.01);
    }
    else if(event.key.keysym.sym == SDLK_s) // camera rotate X
    {
      cout << "ROTATE X OTHER" << endl;
      rotateX(-0.01);
    }
    else if(event.key.keysym.sym == SDLK_d) // camera rotate Y
    {
      cout << "ROTATE Y" << endl;
      rotateY(0.01);
    }
    else if(event.key.keysym.sym == SDLK_a) // camera rotate Y
    {
      cout << "ROTATE Y OTHER" << endl;
      rotateY(-0.01);
    }
    else if(event.key.keysym.sym == SDLK_b) // camera lookAt
    {
      cout << "LOOK AT" << endl;
      lookAt(vec3(-3.014011, 5.325313, -5.839967));
    }
    else if(event.key.keysym.sym == SDLK_r) // camera reset position
    {
      cout << "RESET CAMERA POS" << endl;
      resetCameraStuff();
    }
    else if(event.key.keysym.sym == SDLK_j) // wireframe
    {
      cout << "DRAWING WIREFRAME" << endl;
      drawWireframe(triangles);
      bool_flag = 0;
    }
    else if(event.key.keysym.sym == SDLK_k) // rasterised
    {
      cout << "DRAWING RASTERISED" << endl;
      depthBuffer(triangles);
      bool_flag = 1;
    }
    else if(event.key.keysym.sym == SDLK_l) // raytraced
    {
      cout << "DRAWING RAYTRACED" << endl;
      drawRaytraced(triangles);
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
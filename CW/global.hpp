#ifndef GLOBAL_HPP
#define GLOBAL_HPP

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
#define SCALE_FACTOR 0.005
#define PI 3.1415
#define FOCAL_RAYTRACE -500
#define INTENSITY 1
#define AMBIENCE 0.2
#define FRACTION_VAL 0.5
#define SHADOW_THRESH 0.1
#define MTLPATH "materials.mtl"
#define OBJPATH "logo.obj"
#define TEXPATH "texture.ppm"

vector<float> interpolation(float from, float to, int numberOfValues);
vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, int numberOfValues);
void printVec3(string text, vec3 vector);
bool inRange(float val, float v1, float v2);

// OBJ Stuff
vector<ModelTriangle> readObj(float scale);

// Texturing Stuff
vector<uint32_t> loadImage();
void drawTextureLine(CanvasPoint to, CanvasPoint from, vector<uint32_t> pixelColours);
void drawTextureMap();

// Defining the Global Variables
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
int bool_flag = -1;
vec3 cameraPos(0, 0, 6);
mat3 cameraOrientation = mat3(vec3(1, 0, 0),
                              vec3(0, 1, 0),
                              vec3(0, 0, 1));
vector<ModelTriangle> triangles = readObj(SCALE_FACTOR);
vector<TexturePoint> texpoints;
vector<CanvasTriangle> canvasTriangles;
vec3 lightSource = vec3(-0.0315915, 1.20455, -0.6108);
int shadowMode = 0;

vector<float> interpolation(float from, float to, int numberOfValues)
{
  vector<float> vals;
  for(int i = 0; i < numberOfValues + 1; i++)
  {
    float calc = from + (i * (to - from)/numberOfValues);
    vals.push_back(calc);
  }
  return vals;
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

void printVec3(string text, vec3 vector)
{
  cout << text << " = " << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << endl;
}

bool inRange(float val, float v1, float v2)
{
  if(val >= v1 && val <= v2) return true;
  else return false;
}

vector<ModelTriangle> readObj(float scale)
{
  vector <ModelTriangle> tris;
  ifstream fp;
  
  vector<vec3> vertic;
  fp.open(OBJPATH);

  if(fp.fail())
    cout << "fails" << endl;

  string newline;
  getline(fp,newline);
  getline(fp,newline);

  while(!fp.eof())
  {
    string comment;
    getline(fp, comment);

    if (!comment.empty())
    {
      while(true)
      {
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
        getline(fp,comment);
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
      bool not_reach = true;
        while (not_reach)
        {
          getline(fp,comment_new);
          string *splitcomment = split(comment_new,' ');
          if (!comment_new.empty())
          {
            if (splitcomment[0]=="f")
            {
              int first_vert = stoi(splitcomment[1].substr(0, splitcomment[1].find('/')));
              int second_vert = stoi(splitcomment[2].substr(0, splitcomment[2].find('/')));
              int third_vert = stoi(splitcomment[3].substr(0, splitcomment[3].find('/')));

              tris.push_back(ModelTriangle(vertic[first_vert-1],vertic[second_vert-1],vertic[third_vert-1],Colour(255,255,255)));
            }
            else if (splitcomment[0] == "vt"){
              float x_tex = stof(splitcomment[1]) * scale;
              float y_tex = stof(splitcomment[2]) * scale;
              texpoints.push_back(TexturePoint(x_tex, y_tex));
            }
            else{break;}

          }
          if (fp.eof())
            not_reach=false;
        }
    }
  }
  fp.close();
  return tris;
}

vector<uint32_t> loadImage()
{
  ifstream fp;
  fp.open(TEXPATH);

  string magicNum, comment, dimensions, byteSize;
  getline(fp, magicNum);
  getline(fp, comment);
  getline(fp, dimensions);
  getline(fp, byteSize);

  int whiteSpacePos = dimensions.find(" ");
  int newLinePos = dimensions.find('\n');

  int width = stoi(dimensions.substr(0, whiteSpacePos));
  int height = stoi(dimensions.substr(whiteSpacePos, newLinePos));

  vector<Colour> pixelVals;
  for(int i = 0; i < (width * height); i++)
  {
    Colour c;
    c.red = fp.get();
    c.green = fp.get();
    c.blue = fp.get();
    pixelVals.push_back(c);
  }

  vector<uint32_t> converted;
  for(size_t i = 0; i < pixelVals.size(); i++)
  { 
    Colour c = pixelVals[i];
    uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
    converted.push_back(colour);
  }

  return converted;
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

  for(float i = 0; i < numberOfValues; i++)
  {
    TexturePoint tp;
    tp.x = from.texturePoint.x + (i * numberOfTextureValues.x/numberOfValues);
    tp.y = from.texturePoint.y + (i * numberOfTextureValues.y/numberOfValues);
    cout << tp << endl;
    window.setPixelColour(xs[i], ys[i], pixelColours[round(tp.x) + round(tp.y) * WIDTH]);
  }
}

void drawTextureMap()
{ 
  vector<uint32_t> pixelColours = loadImage();

  CanvasPoint largest; largest.x = rand()%WIDTH; largest.y = rand()%HEIGHT; largest.texturePoint = TexturePoint(195, 5);
  CanvasPoint middle; middle.x = rand()%WIDTH; middle.y = rand()%HEIGHT; middle.texturePoint = TexturePoint(395, 380);
  CanvasPoint smallest; smallest.x = rand()%WIDTH; smallest.y = rand()%HEIGHT; smallest.texturePoint = TexturePoint(65, 330);
  
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

#endif
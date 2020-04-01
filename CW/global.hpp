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
#define SCALE_FACTOR 0.001
#define SCALE_CORNELL 0.3
#define SCALE_SPHERE 0.05
#define FOCAL_LENGTH -500
#define INTENSITY 1
#define AMBIENCE 0.2
#define FRACTION_VAL 0.5
#define SHADOW_THRESH 0.1
#define MTLPATH "materials.mtl"
#define OBJPATH "logo.obj"
#define TEXPATH "texture.ppm"

#define MTL_CORNELL "cornell-box.mtl"
#define OBJ_CORNELL "cornell-box.obj"

#define OBJ_SPHERE "sphere.obj"

#define CAMERA_X 0
#define CAMERA_Y 0.9
#define CAMERA_Z 2

#define SHADOW_SHIFT 0.1

vector<float> interpolation(float from, float to, int numberOfValues);
vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, int numberOfValues);
void printVec3(string text, vec3 vector);
bool inRange(float val, float v1, float v2);

// OBJ Stuff
vector<Colour> readMaterial(string fname);
vector<ModelTriangle> readObj(float scale);
Colour getColourFromName(string mat, vector<Colour> colours);
vector<ModelTriangle> readCornellBox(float scale);
vector<ModelTriangle> readSphere(float scale);

// Texturing Stuff
vector<uint32_t> loadImage();

// Saving PPM Image
void unpack(uint32_t col, std::ostream& fs);
void savePPM();

// 3D to 2D projection
CanvasTriangle modelToCanvas(ModelTriangle modelTrig);

// Backface Culling
vec3 getTriangleCentroid(ModelTriangle t);
vector<ModelTriangle> backfaceCulling(vector<ModelTriangle> triangles);

// Combining triangles
vector<ModelTriangle> combineTriangles(vector<ModelTriangle> triangles, vector<ModelTriangle> cornellTriangles);
vector<ModelTriangle> addSphereTriangles(vector<ModelTriangle> combined, vector<ModelTriangle> sphereTriangles);

// Gouraud Pre-processing
vector<pair<ModelTriangle, vector<vec3>>> updateVertexNormals(vector<ModelTriangle> triangles);
vector<vec3> getTriangleVertexNormals(ModelTriangle t, vector<pair<ModelTriangle, vector<vec3>>> triangleVertexNormals);

// Defining the Global Variables
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
int bool_flag = -1;
vec3 cameraPos(CAMERA_X, CAMERA_Y, CAMERA_Z);
mat3 cameraOrientation = mat3(1.0f);

// Cornell Box Material
vector<Colour> cornellColours = readMaterial(MTL_CORNELL);

// Loading Triangles
vector<ModelTriangle> triangles = readObj(SCALE_FACTOR);
vector<ModelTriangle> cornellTriangles = readCornellBox(SCALE_CORNELL);
vector<ModelTriangle> sphereTriangles = readSphere(SCALE_SPHERE);
vector<ModelTriangle> combinedTriangles = combineTriangles(triangles, cornellTriangles);
vector<ModelTriangle> allTriangles = addSphereTriangles(combinedTriangles, sphereTriangles);
vector<pair<ModelTriangle, vector<vec3>>> triangleVertexNormals;
vec3 unpackColour(uint32_t col);

vector<TexturePoint> texpoints;

vec3 lightSource = vec3(-0.0315915, 1.20455, -0.6108);
vector<vec3> lightSources;
int hardShadowMode = 0;
int softShadowMode = 0;
int gouraudMode = 0;
int phongMode = 0;
bool cullingMode = 0;
bool reflectiveMode = false;
vector<uint32_t> pixelColours = loadImage();
int texWidth;
int texHeight;

vector<ModelTriangle> combineTriangles(vector<ModelTriangle> triangles, vector<ModelTriangle> cornellTriangles)
{
  vector<ModelTriangle> combined;
  for(size_t i = 0; i < cornellTriangles.size(); i++)
  {
    combined.push_back(cornellTriangles.at(i));
  }

  for(size_t i = 0; i < triangles.size(); i++)
  {
    combined.push_back(triangles.at(i));
  }
  return combined;
}

vector<ModelTriangle> addSphereTriangles(vector<ModelTriangle> combined, vector<ModelTriangle> sphereTriangles)
{
  for(size_t i = 0; i < sphereTriangles.size(); i++)
  {
    combined.push_back(sphereTriangles.at(i));
  }
  return combined;
}

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
        p.texturePoint = TexturePoint(modelTrig.texturepoints[i].x*texWidth , modelTrig.texturepoints[i].y* texHeight);
        canvasTrig.vertices[i] = p;
    }
    return canvasTrig;
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

    Colour c = Colour(splitName[1], r, g, b, 0.0f);

    getline(fp, newline);
    colours.push_back(c);
  }
  fp.close();
    // cout << "finished mat"<<endl;
  return colours;
}

vector<ModelTriangle> readCornellBox(float scale)
{
  vector <ModelTriangle> tris;
  ifstream fp;
  vector<Colour> colours = readMaterial(MTL_CORNELL);
  vector<vec3> vertic;
  fp.open(OBJ_CORNELL);

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
        Colour tricolour = getColourFromName(mat,cornellColours);
        tricolour.name = mat;
        tricolour.reflectivity = 0.0f;

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
              ModelTriangle tri = ModelTriangle(vertic[first_vert-1],vertic[second_vert-1],vertic[third_vert-1],tricolour);
              tri.tag = "cornell";
              tris.push_back(tri);
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
            vec3 verts = vec3(x - 0.7, y - 0.1, z - 0.5);
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

  while(!fp.eof()){
    string comment_new;
      bool not_reach = true;
        while (not_reach)
        {
          getline(fp,comment_new);
          string *splitcomment = split(comment_new,' ');
          if (!comment_new.empty())
          {
            if (splitcomment[0] == "vt"){
                float x_tex = stof(splitcomment[1]);
                float y_tex = stof(splitcomment[2]);
                texpoints.push_back(TexturePoint(x_tex, y_tex));
              }
            else{break;}
          }
          if (fp.eof())
            not_reach=false;
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
              int first_tex_index = stoi(splitcomment[1].substr((splitcomment[1].find('/')+1),splitcomment[1].length()));
              int second_tex_index = stoi(splitcomment[2].substr(splitcomment[2].find('/')+1,splitcomment[2].length()));
              int third_tex_index = stoi(splitcomment[3].substr(splitcomment[3].find('/')+1,splitcomment[3].length()));
              
              vec2 first_tex_point = vec2(texpoints[first_tex_index-1].x,texpoints[first_tex_index-1].y);
              vec2 second_tex_point = vec2(texpoints[second_tex_index-1].x,texpoints[second_tex_index-1].y);
              vec2 third_tex_point = vec2(texpoints[third_tex_index-1].x,texpoints[third_tex_index-1].y);
              ModelTriangle tri = ModelTriangle(vertic[first_vert-1], vertic[second_vert-1], vertic[third_vert-1], Colour(255,255,255,0.0f), first_tex_point, second_tex_point, third_tex_point);
              tri.tag = "hackspace";
              tris.push_back(tri);
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

vector<ModelTriangle> readSphere(float scale)
{
  vector <ModelTriangle> tris;
  ifstream fp;
  
  vector<vec3> vertic;
  fp.open(OBJ_SPHERE);

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
            vec3 verts = vec3(x + 0.18, y + 0.55, z - 0.5);
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
              
              ModelTriangle tri = ModelTriangle(vertic[first_vert-1], vertic[second_vert-1], vertic[third_vert-1], Colour(255, 102, 0, 0.0f));
              tri.tag = "sphere";
              tris.push_back(tri);
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
  texWidth = width;
  int height = stoi(dimensions.substr(whiteSpacePos, newLinePos));
  texHeight = height;

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

void unpack(uint32_t col, std::ostream& fs)
{
  int red = (col >> 16) & 255;
  int green = (col >> 8) & 255;
  int blue = (col) & 255;

  fs << (u8) red;
  fs << (u8) green;
  fs << (u8) blue;
}

vec3 unpackColour(uint32_t col)
{
  int red = (col >> 16) & 255;
  int green = (col >> 8) & 255;
  int blue = (col) & 255;
  return vec3(red, green, blue);
} 

void saveToPPM()
{
  ofstream fs;
  fs.open("frame.ppm", std::ofstream::out | std::ofstream::trunc);
  fs << "P6\n";
  fs << to_string(WIDTH) << " " << to_string(HEIGHT) << "\n";
  fs << "255\n";

  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      unpack(window.getPixelColour(x,y), fs);
    }
  }
}

vec3 getTriangleCentroid(ModelTriangle t)
{
  vec3 v1 = t.vertices[0];
  vec3 v2 = t.vertices[1];
  vec3 v3 = t.vertices[2];

  vec3 result = vec3((v1.x + v2.x + v3.x)/3, (v1.y + v2.y + v3.y)/3, (v1.z + v2.z + v3.z)/3);
  return result;
}

vector<ModelTriangle> backfaceCulling(vector<ModelTriangle> triangles)
{ 
  vector<ModelTriangle> filtered;

  for(size_t i = 0; i < triangles.size(); i++)
  {
    ModelTriangle t = triangles.at(i);
    vec3 diff1 = t.vertices[1] - t.vertices[0];
    vec3 diff2 = t.vertices[2] - t.vertices[0];

    vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));
    vec3 centroid = getTriangleCentroid(t);
    vec3 vecToCamera = glm::normalize(cameraPos - centroid);

    float result = glm::dot(surfaceNormal, vecToCamera);
    if(result > 0.0f)
    {
      filtered.push_back(t);
    }
  }
  return filtered;
}

bool compareModel(ModelTriangle t1, ModelTriangle t2)
{
  if((t1.vertices[0] == t2.vertices[0]) && (t1.vertices[1] == t2.vertices[1]) && (t1.vertices[2] == t2.vertices[2]))
    return true;
  else
    return false;
}

vector<ModelTriangle> removeIntersectedTriangle(vector<ModelTriangle> triangles, ModelTriangle t)
{
  vector<ModelTriangle> result;

  for(size_t i = 0; i < triangles.size(); i++)
  {
    if(compareModel(t, triangles.at(i)) == false)
    {
      result.push_back(triangles.at(i));
    }
  }
  return result;
}

vector<pair<ModelTriangle, vector<vec3>>> updateVertexNormals(vector<ModelTriangle> triangles)
{
  vector<pair<ModelTriangle, vector<vec3>>> result;

  #pragma omp parallel
  #pragma omp for
  for(size_t i = 0; i < triangles.size(); i++)
  {
    vector<vec3> vertexNormals;
    for(int j = 0; j < 3; j++)
    {
      vec3 currVertex = triangles.at(i).vertices[j];
      vec3 average = vec3(0, 0, 0);
      int counter = 0;

      for(size_t k = 0; k < triangles.size(); k++)
      {
        for(int l = 0; l < 3; l++)
        {
          vec3 otherVertex = triangles.at(k).vertices[l];

          if(otherVertex == currVertex)
          {
            vec3 diff1 = triangles.at(k).vertices[1] - triangles.at(k).vertices[0];
            vec3 diff2 = triangles.at(k).vertices[2] - triangles.at(k).vertices[0];

            vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));

            average += surfaceNormal;
            counter += 1;
          }
        }
      }
      vec3 vNorm = average * (1 / (float) counter);
      vertexNormals.push_back(vNorm);
    }
    result.push_back(make_pair(triangles.at(i), vertexNormals));
  }
  return result;
}

vector<vec3> getTriangleVertexNormals(ModelTriangle t, vector<pair<ModelTriangle, vector<vec3>>> triangleVertexNormals)
{
  vector<vec3> result;
  for(size_t i = 0; i < triangleVertexNormals.size(); i++)
  {
    ModelTriangle curr = triangleVertexNormals.at(i).first;
    if(compareModel(t, curr))
    {
      result = triangleVertexNormals.at(i).second;
      break;
    }
  }
  return result;
}

#endif
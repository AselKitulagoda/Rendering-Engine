#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define WIDTH 500
#define HEIGHT 500
// #define MTLPATH "/home/asel/Documents/ComputerGraphics/Lab3/cornell-box.mtl"
// #define OBJPATH "/home/asel/Documents/ComputerGraphics/Lab3/cornell-box.obj"
#define MTLPATH "/home/ks17226/Documents/ComputerGraphics/Lab3/cornell-box.mtl"
#define OBJPATH "/home/ks17226/Documents/ComputerGraphics/Lab3/cornell-box.obj"
// #define MTLPATH "/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box.mtl"
// #define OBJPATH "/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box.obj"
#define CAMERA_Z 8

vector<ModelTriangle> readObj();
vector<Colour> readMaterial(string fname);
CanvasTriangle modelToCanvas(ModelTriangle t);
void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c);
void update();
void handleEvent(SDL_Event event);
void drawFilledWireframe(vector<ModelTriangle> tris);
void drawStroke(CanvasTriangle t, Colour c);
CanvasPoint getMinMaxX(CanvasTriangle t);
CanvasPoint getMinMaxY(CanvasTriangle t);
vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, int numberOfValues);
vector<float> interpolation(float from, float to, int numberOfValues);
float getZfromXY(uint32_t x, uint32_t y, vector<CanvasPoint> depthVals);
void computeDepth(vector<ModelTriangle> tris);

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
  vector <ModelTriangle> triangl;
  vector <Colour> colours;
  triangl = readObj();
  colours = readMaterial(MTLPATH);
  // drawFilledWireframe(triangl);
  computeDepth(triangl);

  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
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
    // double depth = from.depth + (i * (to.depth - from.depth)/numberOfValues);
    // p.depth = depth;
    vals.push_back(p);
  }
  return vals;
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
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

void drawFilled(CanvasTriangle f, Colour c)
{
  CanvasPoint p1 = f.vertices[0];
  CanvasPoint p2 = f.vertices[1];
  CanvasPoint p3 = f.vertices[2];

  if(p1.y < p2.y)
  {
    std::swap(p1, p2);
  }
  if(p1.y < p3.y)
  {
    std::swap(p1, p3);
  }
  if(p2.y < p3.y)
  {
    std::swap(p2, p3);
  }
  
  // p1 = top, p2 = mid, p3 = bot

  float ratio = (p1.y - p2.y)/(p1.y - p3.y);
  CanvasPoint extraPoint;
  extraPoint.x = p1.x - ratio*(p1.x - p3.x);
  extraPoint.y = p1.y - ratio*(p1.y - p3.y);

  // Interpolation 
  int numberOfValuesTop = (p1.y - p2.y);
  int numberOfValuesBot = (p2.y - p3.y);

  vector<CanvasPoint> p1_extraPoint = interpolate(p1, extraPoint, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p1_p2 = interpolate(p1, p2, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p3_extraPoint = interpolate(p3, extraPoint, ceil(numberOfValuesBot)+1);
  vector<CanvasPoint> p3_p2 = interpolate(p3, p2, ceil(numberOfValuesBot)+1);

  for(int i = 0; i <= numberOfValuesTop; i++)
  {
    drawLine(p1_extraPoint[i], p1_p2[i], c);
  }

  for(int i = 0; i <= numberOfValuesBot+1; i++)
  {
    drawLine(p3_extraPoint[i], p3_p2[i], c);
  }
  drawStroke(f, c);
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

vector<ModelTriangle> readObj()
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
              float x = stof(splitcomment[1]);
              float y = stof(splitcomment[2]);
              float z = stof(splitcomment[3]);
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
  // cout<<vertic.size()<<endl;
  // cout << "vertices pass done" << endl;

  fp.clear();
  fp.seekg(0,ios::beg);
  if(fp.fail())
    cout << "fails" << endl;
  // cout << "stream open" <<endl;
  getline(fp,newline);
  getline(fp,newline);

  while(!fp.eof())
  {
    // string light;
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

        while(true){
          getline(fp,comment_new);
          splitcomment = split(comment_new,' ');
          if (splitcomment[0] == "f" && !comment_new.empty()){break;}
        }

      bool not_reach = true;
        while (not_reach)
        {
          // cout << "this is a comment"<<comment_new << endl;
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
// cout << "finished Reading OBJ" << endl;
return tris;
}

CanvasTriangle modelToCanvas(ModelTriangle t)
{ 
  vec3 camera(0, 0, CAMERA_Z);
  float f=450; /* camera distance, some negative val */
  CanvasTriangle triangle;
  for(int i = 0; i < 3; i++)
  {
    vec3 pWorld = t.vertices[i];
    float xCamera = pWorld.x - camera.x;
    float yCamera = pWorld.y - camera.y;
    double zCamera = pWorld.z - camera.z;

    float pScreen = f/-zCamera;
    int xProj = std::floor(xCamera*pScreen) + WIDTH/2;
    int yProj = std::floor((1-yCamera)*pScreen) + HEIGHT/2;

    CanvasPoint p = CanvasPoint(xProj, yProj);
    p.depth = zCamera;
    triangle.vertices[i] = p;
  }
  return triangle;
}

void drawFilledWireframe(vector <ModelTriangle> tris){
  for (size_t i=0;i<tris.size();i++){
    // cout << i << endl;
    CanvasTriangle new_tri = modelToCanvas(tris[i]);
    drawFilled(new_tri, tris[i].colour);
  }
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

// void getDepthForExtraPoint(CanvasPoint top, CanvasPoint bottom, CanvasPoint extraPoint)
// {
//   vector<CanvasPoint> points = interpolate(top, bottom, ceil(bottom.y - top.y)+1);
//   for(size_t i = 0; i < points.size(); i++)
//   {
//     if(points.at(i).x == extraPoint.x && points.at(i).y == extraPoint.y)
//     {
//       extraPoint.depth = points.at(i).depth;
//     }
//   }
// }

void computeDepth(vector<ModelTriangle> tris)
{

  vector<float> depthBuffer;
  for(size_t i = 0; i < WIDTH * HEIGHT; i++)
  {
    depthBuffer.push_back(-std::numeric_limits<float>::infinity());
  }

  for(size_t t = 0; t < tris.size(); t++)
  // for(size_t t = 0; t < 1; t++)
  {
    CanvasTriangle projection = modelToCanvas(tris[t]);
    
    CanvasPoint p1 = projection.vertices[0];
    CanvasPoint p2 = projection.vertices[1];
    CanvasPoint p3 = projection.vertices[2];

    if(p1.y < p2.y){ std::swap(p1, p2); }
    if(p1.y < p3.y){ std::swap(p1, p3); }
    if(p2.y < p3.y){ std::swap(p2, p3); }

    float ratio = (p1.y - p2.y)/(p1.y - p3.y);
    CanvasPoint extraPoint;
    extraPoint.x = p1.x - ratio*(p1.x - p3.x);
    extraPoint.y = p1.y - ratio*(p1.y - p3.y);

    // Calculate Depth for extra point
    // getDepthForExtraPoint(p1, p3, extraPoint);
    extraPoint.depth = (p1.depth - p3.depth)/2;

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
      CanvasPoint from = p1_extraPoint.at(i);
      CanvasPoint to = p1_p2.at(i);
      int numberOfValues = round(to.x - from.x);

      for(int j = 0; j < numberOfValues; j++)
      {
        if((round(from.y) < HEIGHT) && round(from.y) >= 0 && round(from.x + j) >= 0 && round(from.x + j) < WIDTH)
        {
          if(numberOfValues == 1) { numberOfValues = 2; }

          cout << "from depth = " << from.depth << endl;
          cout << "to depth = " << to.depth << endl;

          float depth = -(from.depth + ((to.depth - from.depth) * j)/(numberOfValues - 1));
          cout << "depth = " << depth << endl;

          if(depth > depthBuffer.at(round((from.x + j)) + round(from.y) * WIDTH))
          {
            depthBuffer.at(round((from.x + j)) + round(from.y) * WIDTH) = depth;
            Colour c = tris[t].colour;
            uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
            window.setPixelColour(round(from.x + j), round(from.y), colour);
          }
        }
      }
    }

    for(size_t i = 0; i < p3_extraPoint.size(); i++)
    {
      CanvasPoint from = p3_extraPoint.at(i);
      CanvasPoint to = p3_p2.at(i);
      int numberOfValues = round(to.x - from.x);

      for(int j = 0; j < numberOfValues; j++)
      {
        if((round(from.y) < HEIGHT) && round(from.y) >= 0 && round(from.x + j) >= 0 && round(from.x + j) < WIDTH)
        {
          if(numberOfValues == 1) { numberOfValues = 2; }

          float depth = -1/(from.depth + ((to.depth - from.depth) * j/(numberOfValues - 1)));

          if(depth > depthBuffer.at((from.x + j) + from.y * WIDTH))
          {
            depthBuffer.at((from.x + j) + from.y * WIDTH) = depth;
            Colour c = tris[t].colour;
            uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
            window.setPixelColour(round(from.x + j), round(from.y), colour);
          }
        }
      }
    }
  }
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
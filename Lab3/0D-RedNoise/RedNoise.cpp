#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define WIDTH 320
#define HEIGHT 240

vector<ModelTriangle> readObj();
vector<Colour> readMaterial(string fname);
void update();
void handleEvent(SDL_Event event);
vector <ModelTriangle> tris;

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
  colours = readMaterial("/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box/cornell-box/cornell-box.mtl");
  
  cout << colours.size()<< endl;
  for (size_t t = 0;t<tris.size();t++){
    cout << tris[(int)t]<< endl ;
  }
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
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
  return colours;
}

vector<ModelTriangle> readObj()
{
  ifstream fp;
  vector<Colour> colours = readMaterial("/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box/cornell-box/cornell-box.mtl");
  vector<vec3> vertic;
  fp.open("/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box/cornell-box/cornell-box.obj");
    
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
  fp.close();

  ifstream fs;
  fs.open("/home/ak17520/Documents/ComputerGraphics/Lab3/cornell-box/cornell-box/cornell-box.obj");
  if(fs.fail())
    cout << "fails" << endl;

  getline(fs,newline);
  getline(fs,newline);

  while(!fs.eof())
  {
    string light;
    string comment;
    string mat;
    getline(fs,comment);
    if (!comment.empty())
    {
      string *splitcomment = split(comment,' ');
      if (splitcomment[0] == "usemtl")
      {
        mat = splitcomment[1];
        Colour tricolour = getColourFromName(mat,colours);

        while (true)
        {
          getline(fs,comment);
          if (!comment.empty())
          {
            string *splitcomment = split(comment,' ');
            if (splitcomment[0]=="f")
            {
              int first_vert = stoi(splitcomment[1].substr(0, splitcomment[1].size()-1));
              int second_vert = stoi(splitcomment[2].substr(0, splitcomment[2].size()-1));
              int third_vert = stoi(splitcomment[3].substr(0, splitcomment[3].size()-1));

              tris.push_back(ModelTriangle(vertic[first_vert-1],vertic[second_vert-1],vertic[third_vert-1],tricolour));
            }
          }
        }
      }
    }
  }
  fs.close();

return tris;
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
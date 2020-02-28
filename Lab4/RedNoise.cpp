#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

#define WIDTH 1000
#define HEIGHT 1000
#define SCALE_FACTOR 0.3
#define PI 3.1415
#define MTLPATH "cornell-box.mtl"
#define OBJPATH "cornell-box.obj"

vec3 cameraPos(0, 0, 6);
mat3 cameraOrientation = mat3(vec3(1, 0, 0),
                              vec3(0, 1, 0),
                              vec3(0, 0, 1));

vector<ModelTriangle> readObj(float scale);
vector<Colour> readMaterial(string fname);
RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles);
void update();
void handleEvent(SDL_Event event);

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

bool inRange(float val, float v1, float v2)
{
  if(val >= v1 && val <= v2) return true;
  else return false;
}

RayTriangleIntersection getClosestIntersection(vec3 cameraPos, vec3 rayDirection, vector<ModelTriangle> triangles)
{ 
  RayTriangleIntersection result;
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

    if()
    {
      if(inRange(u, 0.0, 1.0) && inRange(v, 0.0, 0.1) && (u+v <= 1.0))
      {
        
      }
    }
  }
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
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT)
    {
      cout << "TRANSLATE LEFT" << endl;
      cameraPos.x -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_RIGHT)
    {
      cout << "TRANSLATE RIGHT" << endl;
      cameraPos.x += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_UP)
    {
      cout << "TRANSLATE UP" << endl;
      cameraPos.y -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_DOWN)
    {
      cout << "TRANSLATE DOWN" << endl;
      cameraPos.y += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_c)
    {
      cout << "CLEAR SCREEN" << endl;
      window.clearPixels();
    }
    else if(event.key.keysym.sym == SDLK_w)
    {
      cout << "ROTATE X" << endl;
      rotateX(0.01);
    }
    else if(event.key.keysym.sym == SDLK_s)
    {
      cout << "ROTATE X OTHER" << endl;
      rotateX(-0.01);
    }
    else if(event.key.keysym.sym == SDLK_d)
    {
      cout << "ROTATE Y" << endl;
      rotateY(0.01);
    }
    else if(event.key.keysym.sym == SDLK_a)
    {
      cout << "ROTATE Y OTHER" << endl;
      rotateY(-0.01);
    }
    else if(event.key.keysym.sym == SDLK_l)
    {
      cout << "LOOK AT" << endl;
      lookAt(vec3(-3.014011, 5.325313, -5.839967));
    }
    else if(event.key.keysym.sym == SDLK_r)
    {
      cout << "RESET CAMERA POS" << endl;
      resetCameraStuff();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

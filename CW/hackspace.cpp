#include "global.hpp"
#include "wireframe.hpp"
#include "rasteriser.hpp"
#include "raytracer.hpp"

using namespace std;
using namespace glm;

// Camera Stuff
vec3 cameraTranslate(vec3 direction);
mat3 rotateX(double theta, mat3 cameraOrien);
mat3 rotateY(double theta, mat3 cameraOrien);
mat3 lookAt(vec3 point);
vec3 orbit(vec3 point, double orbitTheta);
void saveToPPM();

// Display and Event Stuff
void update();
vec3 unpack(uint32_t col);
void handleEvent(SDL_Event event);

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

vec3 unpack(uint32_t col){
  int red = ((int)((col)>>24))%255;
  int green = ((int)((col) >> 16))%255;
  int blue = ((int)((col) >> 8))%255;
  return vec3(red,green,blue);

    } 

void saveToPPM(){
  vector<uint32_t> drawWin;
  for (int x=0;x<WIDTH;x++){
    for (int y=0;y<HEIGHT;y++){
      drawWin.push_back(window.getPixelColour(x,y));
    }
  }
  ofstream fs;
  fs.open("frame.ppm", std::ofstream::out | std::ofstream::trunc);
  fs << "P6\n";
  // fs << "\n";
  fs << "640 480\n";
  fs << "255\n";
  vec3 col;
    for (int x=0;x<WIDTH* HEIGHT;x++){
      col = unpack(drawWin[x]);
      // fs << (u8)col.x<<(u8)col.y<<(u8)col.z;
      fs << (uin32) col.x;
      fs << (uint8_t) col.y;
      fs << (uint8_t) col.z;
}
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
    // window.clearPixels();
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
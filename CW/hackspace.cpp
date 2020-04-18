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

// Display and Event Stuff
void update();
vec3 unpack(uint32_t col);
void handleEvent(SDL_Event event);

int main(int argc, char* argv[])
{
  SDL_Event event;

  triangleVertexNormals = updateVertexNormals(allTriangles);
  
  // Updating the reflectivity
  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    if(allTriangles.at(i).colour.name == "Grey")
    {
      allTriangles.at(i).colour.reflectivity = 1.0f;
    }
  }

  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event))
    {
      handleEvent(event);
      if(event.type == SDL_KEYDOWN)
      {
        if (bool_flag == 0){
          window.clearPixels();
          drawWireframe(allTriangles);
        }
        else if (bool_flag == 1){
          window.clearPixels();
          drawRasterised(combinedTriangles);
        }
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

mat3 rotateZ(double theta, mat3 cameraOrien) 
{
  mat3 newCameraOrien;
  mat3 rotMatrix = mat3(1.0f);

  theta = radians(theta);

  rotMatrix[0] = vec3(cos(theta), -sin(theta), 0.0);
  rotMatrix[1] = vec3(sin(theta), cos(theta), 0.0);
  rotMatrix[2] = vec3(0.0, 0.0, 1.0);

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
      printVec3("camera Pos", cameraPos);
    }
    else if(event.key.keysym.sym == SDLK_DOWN) // camera y translate
    {
      cout << "TRANSLATE DOWN" << endl;
      cameraPos.y += 0.1;
      printVec3("camera Pos", cameraPos);
    }
    else if(event.key.keysym.sym == SDLK_z) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z += 0.1;
      printVec3("camera Pos", cameraPos);
    }
    else if(event.key.keysym.sym == SDLK_x) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z -= 0.1;
      printVec3("camera Pos", cameraPos);
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
    else if(event.key.keysym.sym == SDLK_f) // camera rotate Z
    {
      cout << "ROTATE Z" << endl;
      cameraOrientation = rotateZ(1.0, cameraOrientation);
      printMat3("camera orien", cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_g) // camera rotate Z
    {
      cout << "ROTATE Z OTHER" << endl;
      cameraOrientation = rotateZ(-1.0, cameraOrientation);
      printMat3("camera orien", cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_o) // toggle hard shadow mode
    {
      hardShadowMode = !hardShadowMode;
      cout << "HARD SHADOW MODE = " << hardShadowMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_r) // camera reset position
    {
      cout << "RESET CAMERA POS" << endl;
      resetCameraStuff();
      printVec3("camera Pos", cameraPos);
      printMat3("camera orien", cameraOrientation);
    }
    else if(event.key.keysym.sym == SDLK_j) // wireframe
    {
      cout << "DRAWING WIREFRAME" << endl;
      window.clearPixels();
      drawWireframe(allTriangles);
      bool_flag = 0;
    }
    else if(event.key.keysym.sym == SDLK_k) // rasterised
    {
      cout << "DRAWING RASTERISED" << endl;
      window.clearPixels();
      drawRasterised(combinedTriangles);
      bool_flag = 1;
    }
    else if(event.key.keysym.sym == SDLK_l) // raytraced
    {
      cout << "DRAWING RAYTRACED" << endl;
      window.clearPixels();
      drawRaytraced(allTriangles);
    }
    else if(event.key.keysym.sym == SDLK_m) // raytraced anti alias
    {
      cout << "DRAWING RAYTRACED ANTI ALIAS" << endl;
      window.clearPixels();
      drawRaytraceAntiAlias(allTriangles);
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
    else if(event.key.keysym.sym == SDLK_p) // save image
    {
      cout << "saved PPM, file num = " << filenum << endl;
      savePPM(filepath);
      filenum++;
      filepath = "fade_ray/" + std::to_string(filenum) + ".ppm";
    }
    else if(event.key.keysym.sym == SDLK_b) // backface culling mode
    {
      cullingMode = !cullingMode;
      cout << "CULLING MODE = " << cullingMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_1) // toggle soft shadow mode
    {
      softShadowMode = !softShadowMode;
      cout << "SOFT SHADOW MODE = " << softShadowMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_2) // Reflective mode
    {
      reflectiveMode = !reflectiveMode;
      cout << "REFLECTION MODE = " << reflectiveMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_3) // Gouraud mode
    {
      gouraudMode = !gouraudMode;
      cout << "GOURAUD MODE = " << gouraudMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_4) // Phong mode
    {
      phongMode = !phongMode;
      cout << "PHONG MODE = " << phongMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_5) // Wu Lines (Anti-aliasing)
    {
      wuMode = !wuMode;
      cout << "WU LINES MODE = " << wuMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_6) // Wu Lines (Anti-aliasing)
    {
      stepDiff += 6;
      cout << "STEP DIFF = " << stepDiff << endl;
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
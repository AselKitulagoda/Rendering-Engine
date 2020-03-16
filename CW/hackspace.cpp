#include "global.hpp"
#include "wireframe.hpp"
#include "rasteriser.hpp"
#include "raytracer.hpp"

using namespace std;
using namespace glm;

// Display and Event Stuff
void update();
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
      if (bool_flag == 0){
        drawWireframe(triangles);
      }
      else if (bool_flag == 1){
        drawRasterised(triangles);
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
  lightSource = vec3(-0.0315915, 1.20455, -0.6108);
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
      drawWireframe(triangles);
      bool_flag = 0;
    }
    else if(event.key.keysym.sym == SDLK_k) // rasterised
    {
      cout << "DRAWING RASTERISED" << endl;
      drawRasterised(triangles);
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
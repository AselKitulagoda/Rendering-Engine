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

void drawRedNoise();
void drawGradient();
void drawRainbow();
void update();
void handleEvent(SDL_Event event);
std::vector<float> interpolate(float from, float to, int numberOfValues);
std::vector<vec3> interpolateVec3(vec3 from, vec3 to, int numberOfValues);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{ 
  std::vector<vec3> vals = interpolateVec3(vec3(1, 4, 9.2), vec3(4, 1, 9.8), 4);
  for(int i = 0; i < (int)(vals.size()); i++)
  {
    cout << vals[i].x << " " << vals[i].y << " " << vals[i].z << endl;
  }
  SDL_Event event;
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    // drawRedNoise();
    // drawGradient();
    drawRainbow();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void drawRedNoise()
{
  window.clearPixels();
  for(int y=0; y<window.height ;y++) {
    for(int x=0; x<window.width ;x++) {
      float red = rand() % 255;
      float green = 0.0;
      float blue = 0.0;
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      window.setPixelColour(x, y, colour);
    }
  }
}

void drawGradient()
{ 
  vector<float> pixelVals = interpolate(0.0, 255.0, WIDTH);
  window.clearPixels();
  for(int y=0; y<window.height ;y++) {
    for(int x=0; x<window.width ;x++) {
      uint32_t colour = (255<<24) + (int(255 - pixelVals.at(x))<<16) + (int(255 - pixelVals.at(x))<<8) + int(255 - pixelVals.at(x));
      // uint32_t colour = pixelVals.at(y + x * HEIGHT);
      window.setPixelColour(x, y, colour);
    }
  }
}

void drawRainbow()
{
  vec3 red = vec3(255, 0, 0);
  vec3 green = vec3(0, 255, 0);
  vec3 blue = vec3(0, 0, 255);
  vec3 yellow = vec3(255, 255, 0);

  vector<vec3> redToYellow = interpolateVec3(red, yellow, HEIGHT);
  for(int y=0; y<window.height ;y++)
  {
    for(int x=0; x<window.width ;x++)
    {
      uint32_t colour = (255<<24) + (int(redToYellow[y].x)<<16) + (int(redToYellow[y].y)<<8) + int(redToYellow[y].z);
      window.setPixelColour(x, y, colour);
    }
  }

  vector<vec3> redToBlue = interpolateVec3(red, blue, WIDTH);
  for(int y=0; y<window.height ;y++)
  {
    for(int x=0; x<window.width ;x++)
    {
      uint32_t colour = (255<<24) + (int(redToBlue[x].x)<<16) + (int(redToBlue[x].y)<<8) + int(redToBlue[x].z);
      window.setPixelColour(x, y, colour);
    }
  }

  vector<vec3> blueToGreen = interpolateVec3(blue, green, HEIGHT);
  for(int y=0; y<window.height ;y++)
  {
    for(int x=0; x<window.width ;x++)
    {
      uint32_t colour = (255<<24) + (int(blueToGreen[y].x)<<16) + (int(blueToGreen[y].y)<<8) + int(blueToGreen[y].z);
      window.setPixelColour(x, y, colour);
    }
  }

  vector<vec3> greenToYellow = interpolateVec3(green, yellow, WIDTH); 
  for(int y=0; y<window.height ;y++)
  {
    for(int x=0; x<window.width ;x++)
    {
      uint32_t colour = (255<<24) + (int(greenToYellow[x].x)<<16) + (int(greenToYellow[x].y)<<8) + int(greenToYellow[x].z);
      window.setPixelColour(x, y, colour);
    }
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
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

std::vector<float> interpolate(float from, float to, int numberOfValues)
{
  std::vector<float> vals;
  float diff = ((to - from)/(numberOfValues - 1));
  float currValue = from;
  vals.push_back(from);
  for(int i = 0; i < numberOfValues - 1; i++)
  { 
    currValue += diff;
    vals.push_back(currValue);
  }
  return vals;
}

std::vector<vec3> interpolateVec3(vec3 from, vec3 to, int numberOfValues)
{
  std::vector<vec3> vals;
  float diff1 = (to.x - from.x)/(numberOfValues - 1);
  float diff2 = (to.y - from.y)/(numberOfValues - 1);
  float diff3 = (to.z - from.z)/(numberOfValues - 1);
  vec3 currValue = from;
  vals.push_back(from);
  for(int i = 0; i < numberOfValues - 1; i++)
  {
    currValue = vec3(currValue.x + diff1, currValue.y + diff2, currValue.z + diff3);
    vals.push_back(currValue);
  }
  return vals;
}
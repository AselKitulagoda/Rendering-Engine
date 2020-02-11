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

void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c);
void drawStroke(CanvasTriangle t, Colour c);
void drawFilled(CanvasTriangle f, Colour c);
void loadImage();

void update();
void handleEvent(SDL_Event event);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{ 
  SDL_Event event;
  loadImage();
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
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
  vector<CanvasPoint> points;
  points.push_back(f.vertices[0]);
  points.push_back(f.vertices[1]);
  points.push_back(f.vertices[2]);

  // Sorting the points wrt y coords
  for(int i = 0; i < 2; i++)
  {
    CanvasPoint p1 = points[i];
    CanvasPoint p2 = points[i+1];
    if(p1.y > p2.y)
    {
      points[i+1] = p1;
      points[i] = p2;
    }
  }

  CanvasPoint p1 = points[0];
  CanvasPoint p2 = points[1];
  if(p1.y > p2.y)
  {
    points[0] = p2;
    points[1] = p1;
  }

  CanvasPoint max, mid, min;
  max = points[0];
  mid = points[1];
  min = points[2];

  // Interpolating to find the extra point
  float dy = mid.y - max.y;
  float dxMax = min.x - max.x;
  float dyMax = min.y - max.y;
  float yChange = dy/dyMax;
  float dx = max.x + (yChange * dxMax);

  CanvasPoint extraPoint;
  extraPoint.x = dx; extraPoint.y = max.y + dy;

  // Filling the upper triangle row wise, left to right
  float numberOfRowsUp = mid.y - max.y;
  for(int i = 0; i < numberOfRowsUp + 1; i++)
  {
    float diff = i/numberOfRowsUp;
    float maxDiff1 = max.x - mid.x;
    float maxDiff2 = max.x - extraPoint.x;

    CanvasPoint start, end;
    start.x = round(max.x - (diff * maxDiff1)); start.y = max.y + i;
    end.x = round(max.x - (diff * maxDiff2)); end.y = max.y + i;

    drawLine(start, end, c);
  }

  // Filling the lower triangle row wise, left to right
  float numberOfRowsDown = min.y - mid.y;
  for(int i = numberOfRowsDown; i > 0; i--)
  {
    float diff = 1 - (i/numberOfRowsDown);
    float maxDiff1 = min.x - mid.x;
    float maxDiff2 = min.x - extraPoint.x;

    CanvasPoint start, end;
    start.x = round(min.x - (diff * maxDiff1)); start.y = extraPoint.y + i;
    end.x = round(min.x - (diff * maxDiff2)); end.y = extraPoint.y + i;

    drawLine(start, end, c);
  }
}

void loadImage()
{
  ifstream fp;
  fp.open("texture.ppm");

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

  for(int x = 0; x < width; x++)
  {
    for(int y = 0; y < height; y++)
    {
      window.setPixelColour(x, y, converted[x+y*width]);
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
    else if(event.key.keysym.sym == SDLK_u)
    {
      cout << "DRAWING TRIANGLE" << endl;
      CanvasPoint p1, p2, p3;
      p1.x = rand()%(WIDTH); p1.y = rand()%(HEIGHT); 
      p2.x = rand()%(WIDTH); p2.y = rand()%(HEIGHT);
      p3.x = rand()%(WIDTH); p3.y = rand()%(HEIGHT);

      CanvasTriangle t;
      t.vertices[0] = p1; t.vertices[1] = p2; t.vertices[2] = p3;

      Colour c;
      c.red = rand()%255; c.blue = rand()%255; c.green = rand()%255;

      drawStroke(t, c);
    }
    else if(event.key.keysym.sym == SDLK_f)
    {
      cout << "FILLING TRIANGLE" << endl;
      CanvasPoint p1, p2, p3;
      p1.x = rand()%(WIDTH); p1.y = rand()%(HEIGHT); 
      p2.x = rand()%(WIDTH); p2.y = rand()%(HEIGHT);
      p3.x = rand()%(WIDTH); p3.y = rand()%(HEIGHT);

      CanvasTriangle t;
      t.vertices[0] = p1; t.vertices[1] = p2; t.vertices[2] = p3;

      Colour c;
      c.red = rand()%255; c.blue = rand()%255; c.green = rand()%255;

      drawFilled(t, c);
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

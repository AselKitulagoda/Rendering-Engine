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

void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c);
void drawStroke(CanvasTriangle t, Colour c);
void drawFilled(CanvasTriangle f, Colour c);
vector<uint32_t> loadImage();
vector<CanvasPoint> interpolate(CanvasPoint p1, CanvasPoint p2, int numberOfValues);
void drawTextureLine(CanvasPoint p1, CanvasPoint p2, vector<uint32_t> pixelColours);
void drawTextureMap();

void update();
void handleEvent(SDL_Event event);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{ 
  SDL_Event event;
  // loadImage();
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();

    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

vector<CanvasPoint> interpolate(CanvasPoint p1, CanvasPoint p2, int numberOfValues)
{
  vector<CanvasPoint> vals;
  for(int i = 0; i < numberOfValues + 1; i++)
  {
    CanvasPoint p;
    p.x = p1.x + (i * (p2.x - p1.x)/numberOfValues);
    p.y = p1.y + (i * (p2.y - p1.y)/numberOfValues);
    p.texturePoint.x = p1.texturePoint.x + (i * (p2.texturePoint.x - p1.texturePoint.x)/numberOfValues);
    p.texturePoint.y = p1.texturePoint.y + (i * (p2.texturePoint.y - p1.texturePoint.y)/numberOfValues);
    vals.push_back(p);
  }
  return vals;
}

vector<float> interpolation(float f1, float f2, int numberOfValues)
{
  vector<float> vals;
  for(int i = 0; i < numberOfValues + 1; i++)
  {
    float calc = f1 + (i * (f2 - f1)/numberOfValues);
    vals.push_back(calc);
  }
  return vals;
}

void drawLine(CanvasPoint p1, CanvasPoint p2, Colour c)
{ 
  float dx = p1.x - p2.x;
  float dy = p1.y - p2.y;
  float numberOfValues = ceil(std::max(abs(dx), abs(dy)));

  vector<float> xs = interpolation(p1.x, p2.x, numberOfValues);
  vector<float> ys = interpolation(p1.y, p2.y, numberOfValues);

  for(float i = 0; i < numberOfValues; i++)
  {
    uint32_t colour = (255<<24) + (int(c.red)<<16) + (int(c.green)<<8) + int(c.blue);
    window.setPixelColour(xs[i], ys[i], colour);
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

  for(int i = 0; i <= numberOfValuesBot; i++)
  {
    drawLine(p3_extraPoint[i], p3_p2[i], c);
  }
  drawStroke(f, c);
}

vector<uint32_t> loadImage()
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
  return converted;
}

void drawTextureLine(CanvasPoint p1, CanvasPoint p2, vector<uint32_t> pixelColours)
{
  float dx = p1.x - p2.x;
  float dy = p1.y - p2.y;
  float numberOfValues = ceil(std::max(abs(dx), abs(dy)));

  vector<float> xs = interpolation(p1.x, p2.x, numberOfValues);
  vector<float> ys = interpolation(p1.y, p2.y, numberOfValues);

  TexturePoint numberOfTextureValues;
  numberOfTextureValues.x = p1.texturePoint.x - p2.texturePoint.x;
  numberOfTextureValues.y = p1.texturePoint.y - p2.texturePoint.y;

  for(float i = 0; i < numberOfValues; i++)
  {
    TexturePoint tp;
    tp.x = p2.texturePoint.x + (i * numberOfTextureValues.x/numberOfValues);
    tp.y = p2.texturePoint.y + (i * numberOfTextureValues.y/numberOfValues);
    cout << tp << endl;
    window.setPixelColour(xs[i], ys[i], pixelColours[round(tp.x) + round(tp.y) * WIDTH]);
  }
}

void drawTextureMap()
{ 
  vector<uint32_t> pixelColours = loadImage();

  CanvasPoint p1; p1.x = 160; p1.y = 10; p1.texturePoint = TexturePoint(195, 5);
  CanvasPoint p2; p2.x = 300; p2.y = 230; p2.texturePoint = TexturePoint(395, 380);
  CanvasPoint p3; p3.x = 10; p3.y = 150; p3.texturePoint = TexturePoint(65, 330);

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

  float ratio = (p1.y - p2.y)/(p1.y - p3.y);
  CanvasPoint extraPoint;
  extraPoint.x = p1.x - ratio*(p1.x - p3.x);
  extraPoint.y = p1.y - ratio*(p1.y - p3.y);

  TexturePoint extraTex;
  extraTex.x = p1.texturePoint.x - ratio*(p1.texturePoint.x - p3.texturePoint.x);
  extraTex.y = p1.texturePoint.y - ratio*(p1.texturePoint.y - p3.texturePoint.y);
  
  extraPoint.texturePoint = extraTex;

  // Interpolation 
  int numberOfValuesTop = (p1.y - p2.y);
  int numberOfValuesBot = (p2.y - p3.y);

  vector<CanvasPoint> p1_extraPoint = interpolate(p1, extraPoint, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p1_p2 = interpolate(p1, p2, ceil(numberOfValuesTop)+1);
  vector<CanvasPoint> p3_extraPoint = interpolate(p3, extraPoint, ceil(numberOfValuesBot)+1);
  vector<CanvasPoint> p3_p2 = interpolate(p3, p2, ceil(numberOfValuesBot)+1);

  for(int i = 0; i <= numberOfValuesTop; i++)
  {
    drawTextureLine(p1_extraPoint[i], p1_p2[i], pixelColours);
  }

  for(int i = 0; i <= numberOfValuesBot; i++)
  {
    drawTextureLine(p3_extraPoint[i], p3_p2[i], pixelColours);
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
    else if(event.key.keysym.sym == SDLK_m)
    { 
      cout << "DRAWING TEXTURE MAP TRIANGLE" << endl;
      drawTextureMap();
    }
    else if(event.key.keysym.sym == SDLK_c)
    { 
      cout << "CLEARING WINDOW" << endl;
      window.clearPixels();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

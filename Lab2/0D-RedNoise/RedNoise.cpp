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
void drawTextureMap();

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

vec2 getExtraTexturePoint(CanvasPoint max, CanvasPoint midd, CanvasPoint min, CanvasPoint extra, TexturePoint t1, TexturePoint t2, TexturePoint t3)
{ 

  // texture points
  vec2 tex1 = vec2(t1.x, t1.y);
  vec2 tex2 = vec2(t2.x, t2.y);
  vec2 tex3 = vec2(t3.x, t3.y);

  vector<vec2> points;
  points.push_back(tex1);
  points.push_back(tex2);
  points.push_back(tex3);

  // Sorting the points wrt y coords
  for(int i = 0; i < 2; i++)
  {
    vec2 p1 = points[i];
    vec2 p2 = points[i+1];
    if(p1.y > p2.y)
    {
      points[i+1] = p1;
      points[i] = p2;
    }
  }

  vec2 p1 = points[0];
  vec2 p2 = points[1];
  if(p1.y > p2.y)
  {
    points[0] = p2;
    points[1] = p1;
  }

  vec2 tex_top, tex_mid, tex_bot;
  tex_top = points[0];
  tex_mid = points[1];
  tex_bot = points[2];

  // canvas points
  vec2 top = vec2(max.x, max.y);
  vec2 mid = vec2(midd.x, midd.y);
  vec2 bot = vec2(min.x, min.y);
  vec2 ext = vec2(extra.x, extra.y);

  float a = ext.x - bot.x;
  float A = top.x - bot.x;
  float ratio_x = A/a;

  float b = ext.y - bot.y;
  float B = top.y - bot.y;
  float ratio_y = B/b;

  float tex_A = tex_top.x - tex_bot.x;
  float tex_ext_a = tex_A * ratio_x;

  float tex_B = tex_top.y - tex_bot.y;
  float tex_ext_b = tex_B * ratio_y;

  vec2 tex_extra = vec2(tex_bot.x + round(tex_ext_a), tex_bot.y + round(tex_ext_b));
  return tex_extra;
}

void drawTextureMap()
{ 
  CanvasPoint P1; P1.x = 160; P1.y = 10; P1.texturePoint = TexturePoint(195, 5);
  CanvasPoint P2; P2.x = 300; P2.y = 230; P2.texturePoint = TexturePoint(395, 380);
  CanvasPoint P3; P3.x = 10; P3.y = 150; P3.texturePoint = TexturePoint(65, 330);

  CanvasTriangle t;
  t.vertices[0] = P1; t.vertices[1] = P2; t.vertices[2] = P3;

  Colour c;
  c.red = rand()%255; c.blue = rand()%255; c.green = rand()%255;

  // drawStroke(t, c);
  CanvasTriangle x = CanvasTriangle(CanvasPoint(P1.texturePoint.x, P1.texturePoint.y), CanvasPoint(P2.texturePoint.x, P2.texturePoint.y), CanvasPoint(P3.texturePoint.x, P3.texturePoint.y));
  drawStroke(x, c);

  vector<CanvasPoint> points;
  points.push_back(t.vertices[0]);
  points.push_back(t.vertices[1]);
  points.push_back(t.vertices[2]);

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

  CanvasPoint top, mid, bot;
  top = points[0];
  mid = points[1];
  bot = points[2];

  // Interpolating to find the extra point
  float dy = mid.y - top.y;
  float dxMax = bot.x - top.x;
  float dyMax = bot.y - top.y;
  float yChange = dy/dyMax;
  float dx = top.x + (yChange * dxMax);

  CanvasPoint extraPoint;
  extraPoint.x = dx; extraPoint.y = top.y + dy;

  vec2 textureExtraPoint = getExtraTexturePoint(top, mid, bot, extraPoint, top.texturePoint, mid.texturePoint, bot.texturePoint);
  CanvasPoint test = CanvasPoint(textureExtraPoint.x, textureExtraPoint.y);
  drawLine(mid, test, c);
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
      drawTextureMap();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

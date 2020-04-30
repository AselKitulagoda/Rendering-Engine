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
void draw();
void update();
vec3 unpack(uint32_t col);
void handleEvent(SDL_Event event);

// Animation
void shiftVertices(vec3 direction, float amount);
vec3 getMinY();
void flattenVertices(float amount);
void spin();
void jump(float amount);
void bounce();
void squash(float amount);
void jumpAndSquash();
void fadeIn();

int main(int argc, char* argv[])
{
  SDL_Event event;
  
  // Updating triangle indices
  allTriangles = updateTriangleIndices(allTriangles);

  // Updating the triangle vertex normals
  triangleVertexNormals = updateVertexNormals(allTriangles);

  // Updating the triangle textures
  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    if(allTriangles.at(i).colour.name == "Green")
    {
      allTriangles.at(i).reflective = true;
    }
    if(allTriangles.at(i).colour.name == "Red")
    {
      allTriangles.at(i).refractive = true;
    }
    if(allTriangles.at(i).colour.name == "Yellow")
    {
      allTriangles.at(i).metallic = true;
    }
  }

  // Separating into objects
  allObjects = initialiseObjects(allTriangles);

  // Updating the bounding boxes
  allObjects = updateBoundingBox();

  draw();

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

void draw()
{
  // Clear screen before drawing
  window.clearPixels();

  // Update the Bounding box visibilities of the objects
  allObjects = updateObjectVisibility();

  // Update the triangles visibilities
  allTriangles = updateTriangleVisibility();

  // Check drawing mode and render
  if(drawMode == 0) drawWireframe(allTriangles);
  else if(drawMode == 1) drawRasterised(allTriangles);
  else if(drawMode == 2) drawRaytraced(allTriangles);
  else if(drawMode == 3) drawRaytraceAntiAlias(allTriangles);
}

void shiftVertices(vec3 direction, float amount)
{ 
  vec3 shift = amount * direction;
  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    for(int j = 0; j < 3; j++)
    {
      vec3 vert = allTriangles.at(i).vertices[j];
      vert += shift;
      allTriangles.at(i).vertices[j] = vert;
    }
  }
}

void spin()
{
  for(int i = 0; i < 120; i++)
  {
    cameraPos = orbit(cameraPos, 3.0);
    cameraOrientation = lookAt(cameraPos);
    draw();
    window.renderFrame();
    cout << "saved PPM, file num = " << filenum << endl;
    savePPM(filepath);
    filenum++;
    filepath = "raytracer_frames/" + std::to_string(filenum) + ".ppm";
  }
}

void moveVertices(float amt)
{
  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    if(allTriangles.at(i).tag == "sphere" || allTriangles.at(i).tag == "hackspace")
    {
      for(int j = 0; j < 3; j++)
      {
        allTriangles.at(i).vertices[j] += amt;
      }
    }
  }
}

void jump(float amount)
{
  float u = sqrt(2 * G * amount); // v = 0, u^2 = 2as
  float time = 2 * (u / G); // t = 2u/g
  for(float t = 0; t < time; t += 0.06f)
  {
    float s = ((u * t) + (t * t * -G / 2)); // s = ut + at*2/2
    shiftVertices(vec3(0, 1.f, 0), s);
    draw();
    window.renderFrame();
    cout << "saved PPM, file num = " << filenum << endl;
    savePPM(filepath);
    filenum++;
    filepath = "test_frames/" + std::to_string(filenum) + ".ppm";
    shiftVertices(vec3(0, -1.f, 0), s);
  }
}

void bounce()
{
  for(float a = 5; a >= 0; a -= 0.5)
  {
    jump(a);
  }
}

vec3 getMinY()
{ // 0,1,2 is the averaged vertices of the scene 3 is the lowest y in that process
  vec3 avg = vec3(0, 0, 0);
  float lowestPoint = INFINITY;

  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    for(int j = 0; j < 3; j++)
    {
      avg += allTriangles.at(i).vertices[j];
      if(allTriangles.at(i).vertices[j].y < lowestPoint)
        lowestPoint = allTriangles.at(i).vertices[j].y;
    }
  }
  avg = avg / (3 * (float) allTriangles.size());
  vec3 result = vec3(avg.x, lowestPoint, avg.z);
  return result;
}

void flattenVertices(float amount)
{
  vec3 pointOfRef = getMinY();

  for(size_t i = 0; i < allTriangles.size(); i++)
  {
    for(int j = 0; j < 3; j++)
    {
      vec3 dir = allTriangles.at(i).vertices[j] - pointOfRef;
      vec3 adjustedVert = allTriangles.at(i).vertices[j] + (amount * dir);
      adjustedVert.y = allTriangles.at(i).vertices[j].y - (amount * dir.y);
      allTriangles.at(i).vertices[j] = adjustedVert;
    }
  }
}

void squash(float amount)
{
  vector<ModelTriangle> originalTriangles = allTriangles;
  for(float t = 0; t < 30; t++)
  {
    float a = (2 * amount) / 900;
    float b = a * 30;
    float squashAmount = (b * t) - (a * t * t);
    flattenVertices(squashAmount);
    draw();
    window.renderFrame();
    // cout << "saved PPM, file num = " << filenum << endl;
    // savePPM(filepath);
    // filenum++;
    // filepath = "test_frames/" + std::to_string(filenum) + ".ppm";
    // allTriangles = originalTriangles;
  }
}

void fadeIn()
{
  vector<vec2> points;
  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      vec3 col = unpackColour(window.getPixelColour(x, y));
      if(col != vec3(0, 0, 0))
      {
        points.push_back(vec2(x, y));
      }
    }
  }

  float yDiff = points[(int) points.size() - 1].y - points[0].y;
  int stepTracker = 0;

  for(int step = 0; step <= yDiff; step += 5)
  { 
    stepTracker = step;
    // counter += 1;
    // if(yDiff - step < 12)
    // {
    //   hardShadowMode = false;
    //   softShadowMode = true;
    //   gouraudMode = false;
    //   phongMode = true;
    // }

    draw();
    for(int y = points[0].y; y <= points[(int) points.size() - 1].y - step; y++)
    {
      for(int x = points[0].x; x <= points[(int) points.size() - 1].x; x++)
      {
        Colour c = Colour(0, 0, 0);
        window.setPixelColour(x, y, c.pack());
      }
    }
    window.renderFrame();
    // cout << "saved PPM, file num = " << filenum << endl;
    // savePPM(filepath);
    // filenum++;
    // filepath = "test_frames/" + std::to_string(filenum) + ".ppm";
  }
  if(stepTracker != yDiff)
  {
    draw();
  }
}

void jumpAndSquash()
{
  jump(3);
  cout << "FIRST PART DONE!" << endl;
  // jump(2);
  // jump(1);
  // draw(); window.renderFrame();
  // cout << "saved PPM, file num = " << filenum << endl;
  // savePPM(filepath);
  // filenum++;
  // filepath = "wireframe_frames/" + std::to_string(filenum) + ".ppm";
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
      draw();
    }
    else if(event.key.keysym.sym == SDLK_RIGHT) // camera x translate
    {
      cout << "TRANSLATE RIGHT" << endl;
      cameraPos.x -= 0.1;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_UP) // camera y translate
    {
      cout << "TRANSLATE UP" << endl;
      // cameraPos.y -= 0.1;
      cameraPos.y -= 0.05;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_DOWN) // camera y translate
    {
      cout << "TRANSLATE DOWN" << endl;
      // cameraPos.y += 0.1;
      cameraPos.y += 0.05;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_z) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z += 0.1;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_x) // camera z translate
    {
      cout << "TRANSLATE Z" << endl;
      cameraPos.z -= 0.1;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_c) // clear screen
    {
      cout << "CLEAR SCREEN" << endl;
      window.clearPixels();
      drawMode = -1;
    }
    else if(event.key.keysym.sym == SDLK_w) // camera rotate X
    {
      cout << "ROTATE X" << endl;
      cameraOrientation = rotateX(1.0, cameraOrientation);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_s) // camera rotate X
    {
      cout << "ROTATE X OTHER" << endl;
      cameraOrientation = rotateX(-1.0, cameraOrientation);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_d) // camera rotate Y
    {
      cout << "ROTATE Y" << endl;
      cameraOrientation = rotateY(1.0, cameraOrientation);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_a) // camera rotate Y
    {
      cout << "ROTATE Y OTHER" << endl;
      cameraOrientation = rotateY(-1.0, cameraOrientation);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_f) // camera rotate Z
    {
      cout << "ROTATE Z" << endl;
      cameraOrientation = rotateZ(1.0, cameraOrientation);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_g) // camera rotate Z
    {
      cout << "ROTATE Z OTHER" << endl;
      cameraOrientation = rotateZ(-1.0, cameraOrientation);
      draw();
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
      draw();
    }
    else if(event.key.keysym.sym == SDLK_j) // wireframe
    {
      cout << "DRAWING WIREFRAME" << endl;
      window.clearPixels();
      drawMode = 0;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_k) // rasterised
    {
      cout << "DRAWING RASTERISED" << endl;
      window.clearPixels();
      drawMode = 1;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_l) // raytraced
    {
      cout << "DRAWING RAYTRACED" << endl;
      window.clearPixels();
      drawMode = 2;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_m) // raytraced anti alias
    {
      cout << "DRAWING RAYTRACED ANTI ALIAS" << endl;
      window.clearPixels();
      drawMode = 3;
      draw();
    }
    else if(event.key.keysym.sym == SDLK_q) // orbit
    {
      cout << "SPIN" << endl;
      spin();
    }
    else if(event.key.keysym.sym == SDLK_e) // orbit
    {
      cout << "ORBIT" << endl;
      cameraPos = orbit(cameraPos, -1.0);
      cameraOrientation = lookAt(cameraPos);
      draw();
    }
    else if(event.key.keysym.sym == SDLK_p) // save image
    {
      cout << "saved PPM, file num = " << filenum << endl;
      // savePPM(filepath);
      // filenum++;
      // filepath = "test_frames/" + std::to_string(filenum) + ".ppm";
    }
    else if(event.key.keysym.sym == SDLK_b) // Metallic Mode
    {
      metallicMode = !metallicMode;
      cout << "METALLIC MODE = " << metallicMode << endl;
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
      draw();
    }
    else if(event.key.keysym.sym == SDLK_6) // Refraction mode
    {
      refractiveMode = !refractiveMode;
      cout << "REFRACTION MODE = " << refractiveMode << endl;
    }
    else if(event.key.keysym.sym == SDLK_7) // Jump mode
    {
      cout << "BOUNCE" << endl;
      bounce();
    }
    else if(event.key.keysym.sym == SDLK_8) // Squash mode
    {
      cout << "JUMP AND SQUASH" << endl;
      jumpAndSquash();
    }
    else if(event.key.keysym.sym == SDLK_9) // Fade in
    {
      cout << "FADE IN" << endl;
      fadeIn();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

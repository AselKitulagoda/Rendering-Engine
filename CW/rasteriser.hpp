#ifndef RASTERISER_HPP
#define RASTERISER_HPP

#include "global.hpp"
#include "wireframe.hpp"

using namespace std;
using namespace glm;

int TempTexHeight;
int TempTexWidth;

// Rasterising Stuff
void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT]);
void drawRasterised(vector<ModelTriangle> triangles);

// Texturing Stuff
void drawTextureLine(CanvasPoint to, CanvasPoint from, vector<uint32_t> pixelColours,CanvasPoint closest,CanvasPoint furthest,float depthBuffer[WIDTH][HEIGHT],int TexSize);
void drawTextureMap(CanvasTriangle currentTri,float depthBuffer[WIDTH][HEIGHT],vector<uint32_t> pixelColours,int TexSize);
void drawFilled(CanvasTriangle t, float depthBuffer[WIDTH][HEIGHT])
{
  CanvasPoint p1 = t.vertices[0];
  CanvasPoint p2 = t.vertices[1];
  CanvasPoint p3 = t.vertices[2];

  if(p1.y < p2.y){ std::swap(p1, p2); }
  if(p1.y < p3.y){ std::swap(p1, p3); }
  if(p2.y < p3.y){ std::swap(p2, p3); }

  CanvasPoint minPoint = p3;
  CanvasPoint midPoint = p2;
  CanvasPoint maxPoint = p1;

  float minMaxRatio = (maxPoint.x - minPoint.x) / (maxPoint.y - minPoint.y);
  float minMaxDepth = (maxPoint.depth - minPoint.depth) / (maxPoint.y - minPoint.y);

  float minMidRatio = (midPoint.x - minPoint.x) / (midPoint.y - minPoint.y);
  float minMidDepth = (midPoint.depth - minPoint.depth) / (midPoint.y - minPoint.y);

  float midMaxRatio = (maxPoint.x - midPoint.x) / (maxPoint.y - midPoint.y);
  float midMaxDepth = (maxPoint.depth - midPoint.depth) / (maxPoint.y - midPoint.y);

  CanvasPoint extraPoint;
  extraPoint.x = minPoint.x + minMaxRatio * (midPoint.y - minPoint.y);
  extraPoint.y = midPoint.y;
  extraPoint.depth = minPoint.depth + minMaxDepth * (midPoint.y - minPoint.y);





  // Top half interpolation
  for(float i = minPoint.y; i < midPoint.y; i++)
  {
    CanvasPoint from, to;

    from.x = minPoint.x + minMaxRatio * (i - minPoint.y);
    from.y = i;
    from.depth = minPoint.depth + minMaxDepth * (i - minPoint.y);

    to.x = minPoint.x + minMidRatio * (i - minPoint.y);
    to.y = i;
    to.depth = minPoint.depth + minMidDepth * (i - minPoint.y);

    drawLine(from, to, t.colour, depthBuffer);
  }

  // Bottom half interpolation
  for(float i = midPoint.y; i < maxPoint.y; i++)
  {
    CanvasPoint from, to;

    from.x = extraPoint.x + minMaxRatio * (i - midPoint.y);
    from.y = i;
    from.depth = extraPoint.depth + minMaxDepth * (i - midPoint.y);

    to.x = midPoint.x + midMaxRatio * (i - midPoint.y);
    to.y = i;
    to.depth = midPoint.depth + midMaxDepth * (i - midPoint.y);

    drawLine(from, to, t.colour, depthBuffer);
  }
}

void drawTextureLine(CanvasPoint to, CanvasPoint from, vector<uint32_t> pixelColours,CanvasPoint closest,CanvasPoint furthest,float depthBuffer[WIDTH][HEIGHT],int TexSize)
{
  // Find number of values we need in x-direction
  float dx = to.x - from.x;
  // Find number of values we need in y-direction 
  float dy = to.y - from.y;
  // Calculate how many values we need we need according to depth 
  float dDepth = to.depth - from.depth;

  // We need equal number of values in to and from so we take the max of whichevers biggest
  float numberOfValues = std::max(abs(dx), abs(dy));

  float Z0 = 1/furthest.depth;
  float Z1 = 1/closest.depth;
  float C0 = furthest.texturePoint.y;
  float C1 = closest.texturePoint.y;
  // cout << 1/to.depth << endl;
  
  //value to interpolate across to get delta we need in x,y and depth
  float xChange = dx/(numberOfValues);
  float yChange = dy/(numberOfValues);
  float depthChange = dDepth/(numberOfValues); 

  // vector<float> xs = interpolation(from.x, to.x, numberOfValues);
  // vector<float> ys = interpolation(from.y, to.y, numberOfValues);
  // float depthChange = dDepth/(numberOfValues);

  TexturePoint numberOfTextureValues;
  // How many values do we need in texture line in x and y
  numberOfTextureValues.x = to.texturePoint.x - from.texturePoint.x;
  numberOfTextureValues.y = to.texturePoint.y - from.texturePoint.y;
  //cout << to.depth << endl;

  for(float i = 0; i <= numberOfValues; i++)
  {
    TexturePoint tp;
    //interpolate across Canvas Triangle not necessarily in a straight line
    float x = from.x + (xChange * i);
    float y = from.y + (yChange * i);
    // interpolate depth
    // float depth = from.depth + (depthChange * i);
    float oneOverZ = from.depth + i*depthChange;
    float z = 1/oneOverZ;
    float q = (i * numberOfTextureValues.y/numberOfValues);
    float c = ((((C0/Z0)*(1-q))+((C1/Z1)*q)) / (((1/Z0)*(1-q))+((1/Z1)*q)));
    // tp.x = c + (i * numberOfTextureValues.x/numberOfValues);
    if (abs(z)<0 || abs(z)>100){
      cout<<z<<endl;
    }

    //Interpolate across Texture triangle in exact same way
    tp.y = (from.texturePoint.y+ (i * numberOfTextureValues.y/numberOfValues))*z*TexSize;
    tp.x = (from.texturePoint.x+ (i * numberOfTextureValues.x/numberOfValues))*z*TexSize;
    ;
    // tp.y = c;
    // tp.x = (((C0_x/Z0)*(1-i))+((C1_x/Z1)*i)) / (((1/Z0)*(1-i))+((1/Z1)*i));
    // float depth = from.depth + (depthChange * i);

    //Check if x,y and texture point co-ords are within limits
    if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && tp.x > 0 && tp.x < TexSize && tp.y > 0 && tp.y < TexSize)
    {
      // if(depth < depthBuffer[(int) xs[i]][(int) ys[i]])
      // {
      //   depthBuffer[(int) xs[i]][(int) ys[i]] = depth;
      // cout << "x is: " << (int)x << "y is: " << (int)y<< endl;
        // cout << round(tp.x*z) + round(tp.y*z) * 300 <<  "z is " << z << endl;
        // cout << tp.y << endl;

        //set
        window.setPixelColour((int)x, (int)y, pixelColours[round(tp.x) + round(tp.y) * TexSize]);
      // }
  }
}
}

void drawTextureMap(CanvasTriangle currentTri,float depthBuffer[WIDTH][HEIGHT],vector<uint32_t> pixelColours,int TexSize)
{ 
  // CanvasPoint largest; largest.x = currentTri.vertices[0].x; largest.y = currentTri.vertices[0].y; largest.texturePoint = currentTri.vertices[0].texturePoint;
  // CanvasPoint middle; middle.x = currentTri.vertices[1].x; middle.y = currentTri.vertices[1].y; middle.texturePoint = currentTri.vertices[1].texturePoint;
  // CanvasPoint smallest; smallest.x = currentTri.vertices[2].x; smallest.y = currentTri.vertices[2].y; smallest.texturePoint = currentTri.vertices[2].texturePoint;
    // cout <<  << endl;

// Sorts each vertex in the Canvas Triangle to find out which point has the highest,middle and smallest points according to y to find extra point
  CanvasPoint largest = currentTri.vertices[0];
  CanvasPoint middle = currentTri.vertices[1];
  CanvasPoint smallest = currentTri.vertices[2];


  CanvasPoint persFurthest;
  CanvasPoint persClosest;


  // sorting vertices to find largest , middle and smallest points
  if(largest.y < middle.y)
  {
    std::swap(largest, middle);
  }
  if(largest.y < smallest.y)
  {
    std::swap(largest, smallest);
  }
  if(middle.y < smallest.y)
  {
    std::swap(middle, smallest);
  }

vector <pair<float,CanvasPoint>> depthList = {{largest.depth,largest},{middle.depth,middle},{smallest.depth,smallest}};
std::sort(depthList.begin(),depthList.end(),[](const std::pair<float,CanvasPoint> &left,const std::pair<float,CanvasPoint> &right){
  return left.first<right.first;
});
persFurthest = depthList.at(2).second;
persClosest = depthList.at(0).second;


// finds the proportion of how far down the middle point is relative to vertical between largest point and the smallest point
 float ratio = (largest.y - middle.y)/(largest.y - smallest.y);
  CanvasPoint extraPoint;
  // this ratio is the same for the x and y and we use this ratio to find the position of the extraPoint relative to Canvas 
  //minus here is because we are moving up/left for the lowest point according to ratio for x and y
  extraPoint.x = largest.x - ratio*(largest.x - smallest.x);
  extraPoint.y = largest.y - ratio*(largest.y - smallest.y);
  // DEPTH MATCHES THIS
  // extraPoint.depth = minPoint.depth + minMaxDepth * (midPoint.y - minPoint.y);
  // float minMaxDepth = (maxPoint.depth - minPoint.depth) / (maxPoint.y - minPoint.y);
  // DEPTH NEEDS TO BE THIS

  float minMaxDepth = (largest.depth-smallest.depth)/(largest.y-smallest.y);
  extraPoint.depth = smallest.depth + minMaxDepth * (middle.y-smallest.y);
  // we initialise a texture point
  TexturePoint extraTex;
  // this same ratio applies to the texture triangle in the same way so we find the extra point in the texture triangle
  extraTex.x = largest.texturePoint.x - ratio*(largest.texturePoint.x - smallest.texturePoint.x);
  extraTex.y = largest.texturePoint.y - ratio*(largest.texturePoint.y - smallest.texturePoint.y);
  
  extraPoint.texturePoint = extraTex;

  // Interpolation 

  // We seperately interpolate the top half and bottom half 
  // numberofValuesTop is bottom half of the triangles from extraPoint to largestPoint
  int numberOfValuesTop = (largest.y - middle.y);
  // numberofValuesBot is top half of the triangles from smallestPoint to extraPoint
  int numberOfValuesBot = (middle.y - smallest.y);

  // First interpolate bottom half of the triangle from the largest point to extra point
  vector<CanvasPoint> largest_extraPoint = interpolate(largest, extraPoint, ceil(numberOfValuesTop)+1);
  // Now interpolate bottom half of the triangle from the largest point to middle point 
  vector<CanvasPoint> largest_middle = interpolate(largest, middle, ceil(numberOfValuesTop)+1);
  // Now interpolate top half of the triangle from from smallest to extra Point
  vector<CanvasPoint> smallest_extraPoint = interpolate(smallest, extraPoint, ceil(numberOfValuesBot)+1);
  //Now interpolate top half of the triangle from smallest Point middle Point
  vector<CanvasPoint> smallest_middle = interpolate(smallest, middle, ceil(numberOfValuesBot)+1);

  for(int i = 0; i <= numberOfValuesTop; i++)
  {
    // DrawLine(from,to) in this case
    //Draw line on bottom half of the triangle from largestPoint_to_middle_point interpolaton to largest_to_extra_point interpolation
    drawTextureLine(largest_extraPoint[i], largest_middle[i], pixelColours,persFurthest,persClosest,depthBuffer,TexSize);
  }

  for(int i = 0; i <= numberOfValuesBot+1; i++)
  {
    //Draw line on top half of triangle from smallestPoint_to_extra_point interpolation to smallest_to_middle_point interpolation  
    drawTextureLine(smallest_extraPoint[i], smallest_middle[i], pixelColours,persFurthest,persClosest,depthBuffer,TexSize);
  } 
}
void drawRasterised(vector<ModelTriangle> triangles) 
{

//initialise depth buffer
  float depthBuffer[WIDTH][HEIGHT];
  for(int i = 0; i < WIDTH; i++) 
  {
    for(int j = 0; j < HEIGHT; j++) 
    {
      //initialise all values to infinity
      depthBuffer[i][j] = (float) INFINITY;
    }
  }

  vector<ModelTriangle> filteredTriangles = backfaceCulling(triangles);

  if(cullingMode)
  {
    for(size_t i = 0; i < filteredTriangles.size(); i++)
    {
      //convert all model triangles to a canvas triangle
      CanvasTriangle projection = modelToCanvas(filteredTriangles.at(i));
      
      if(filteredTriangles.at(i).tag == "cornell")
      {
        //calls Draw Filled for all Cornell box canvas triangles, takes in depth buffer 
        drawFilled(projection, depthBuffer);
      }
      else
      {
        // calls Draw Texture map on hackspace canvas triangles
      if (triangles.at(i).tag == "checker"){
          drawTextureMap(projection,depthBuffer,checkcols,738);
        }
       if (triangles.at(i).tag == "hackspace"){
          drawTextureMap(projection,depthBuffer,pixelColours,300);
        }
    }
  }
  }
  else 
  {
    for(size_t i = 0; i < triangles.size(); i++)
    {
      CanvasTriangle projection = modelToCanvas(triangles.at(i));
      
      if(triangles.at(i).tag == "cornell")     
      {
        drawFilled(projection, depthBuffer);
      }
      else
      {
      if (triangles.at(i).tag == "checker"){
          drawTextureMap(projection,depthBuffer,checkcols,738);
        }
      if (triangles.at(i).tag == "hackspace"){
          TempTexWidth=300;
          TempTexHeight=300;
          drawTextureMap(projection,depthBuffer,pixelColours,300);
        }
      }
    }
  }
}


#endif
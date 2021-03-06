#include <glm/glm.hpp>
#include "Colour.h"
#include <string>

class ModelTriangle
{
  public:
    glm::vec3 vertices[3];
    Colour colour;
    glm::vec2 texturepoints[3];
    std::string tag;
    bool boundingBoxVisible;
    int triangleIndex;

    // properties
    bool refractive;
    bool reflective;
    bool metallic;

    ModelTriangle()
    {
    }

    ModelTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Colour trigColour)
    {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      texturepoints[0] = glm::vec2(-1,-1);
      texturepoints[1] = glm::vec2(-1,-1);
      texturepoints[2] = glm::vec2(-1,-1);
      colour = trigColour;
      tag = "";
      boundingBoxVisible = false;
      refractive = false;
      reflective = false;
      metallic = false;
    }

    ModelTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Colour trigColour, glm::vec2 t0, glm::vec2 t1, glm::vec2 t2)
    {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      texturepoints[0] = t0;
      texturepoints[1] = t1;
      texturepoints[2] = t2;
      colour = trigColour;
      tag = "";
      boundingBoxVisible = false;
      refractive = false;
      reflective = false;
      metallic = false;
    }

};

std::ostream& operator<<(std::ostream& os, const ModelTriangle& triangle)
{
    os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")" << std::endl;
    os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")" << std::endl;
    os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")" << std::endl;
    os << std::endl;
    return os;
}

#include <glm/glm.hpp>
#include "Colour.h"
#include <string>

class ModelTriangle
{
  public:
    glm::vec3 vertices[3];
    Colour colour;
    glm::vec2 texturepoints[3];
    glm::vec3 vertexNormals[3];

    ModelTriangle()
    {
    }

    glm::vec3 initialiseVertexNormal()
    {
      glm::vec3 diff1 = vertices[1] - vertices[0];
      glm::vec3 diff2 = vertices[2] - vertices[0];

      glm::vec3 surfaceNormal = glm::normalize(glm::cross(diff1, diff2));

      return surfaceNormal;
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
      vertexNormals[0] = initialiseVertexNormal();
      vertexNormals[1] = initialiseVertexNormal();
      vertexNormals[2] = initialiseVertexNormal();
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
      vertexNormals[0] = initialiseVertexNormal();
      vertexNormals[1] = initialiseVertexNormal();
      vertexNormals[2] = initialiseVertexNormal();
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

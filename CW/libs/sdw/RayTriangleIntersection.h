#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

class RayTriangleIntersection
{
  public:
    glm::vec3 intersectionPoint;
    float distanceFromCamera;
    ModelTriangle intersectedTriangle;
    Colour colour;
    float u;
    float v;

    RayTriangleIntersection()
    {
    }

    RayTriangleIntersection(glm::vec3 point, float distance, ModelTriangle triangle)
    {
        intersectionPoint = point;
        distanceFromCamera = distance;
        intersectedTriangle = triangle;
        colour = Colour(255, 255, 255);
        u = -1;
        v = -1;
    }

    RayTriangleIntersection(glm::vec3 point, float distance, ModelTriangle triangle, Colour c)
    {
        intersectionPoint = point;
        distanceFromCamera = distance;
        intersectedTriangle = triangle;
        colour = c;
        u = -1;
        v = -1;
    }

    RayTriangleIntersection(glm::vec3 point, float distance, ModelTriangle triangle, Colour c, float uVal, float vVal)
    {
        intersectionPoint = point;
        distanceFromCamera = distance;
        intersectedTriangle = triangle;
        colour = c;
        u = uVal;
        v = vVal;
    }
};

std::ostream& operator<<(std::ostream& os, const RayTriangleIntersection& intersection)
{
    os << "Intersection is at " << glm::to_string(intersection.intersectionPoint) << " on triangle " << intersection.intersectedTriangle << " at a distance of " << intersection.distanceFromCamera << std::endl;
    return os;
}
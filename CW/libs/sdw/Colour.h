#include <iostream>

class Colour
{
  public:
    std::string name;
    int red;
    int green;
    int blue;
    float brightness;
    float reflectivity;
    float refractivity;

    Colour()
    {
    }

    Colour(int r, int g, int b)
    {
      name = "";
      red = r;
      green = g;
      blue = b;
      brightness = 1.0f;
      reflectivity = 0.0f;
      refractivity = 0.0f;
    }

    Colour(std::string n, int r, int g, int b)
    {
      name = n;
      red = r;
      green = g;
      blue = b;
      brightness = 1.0f;
      reflectivity = 0.0f;
      refractivity = 0.0f;
    }

    Colour(int r, int g, int b, float bright)
    {
      name = "";
      red = r;
      green = g;
      blue = b;
      brightness = bright;
      reflectivity = 0.0f;
      refractivity = 0.0f;
    }

    uint32_t pack()
    {
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      return colour;
    }

    uint32_t packWithBrightness()
    {
      uint32_t colour = (255<<24) + (int(red * brightness)<<16) + (int(green * brightness)<<8) + int(blue * brightness);
      return colour; 
    }
};

std::ostream& operator<<(std::ostream& os, const Colour& colour)
{
    os << colour.name << " [" << colour.red << ", " << colour.green << ", " << colour.blue << "]" << std::endl;
    return os;
}


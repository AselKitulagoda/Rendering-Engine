class Object
{
    public:
        std::string name;
        std::vector<ModelTriangle> triangles;
        bool visible;
        std::vector<glm::vec3> boxVertices;

        Object()
        {   
            name = "";
            visible = false;
        }

        Object(std::string tag, std::vector<ModelTriangle> modelTriangles)
        {
            name = tag;
            triangles = modelTriangles;
            visible = false;
        }
};
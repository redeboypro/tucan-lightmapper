#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include "Lightmapper.h"
#include "tucan-lightmapper.h"

int main()
{
    std::string modelFileName;
    std::string textureFileName;
    std::string width;
    std::string height;

    std::cin >> modelFileName;
    std::cin >> textureFileName;
    std::cin >> width;
    std::cin >> height;

	auto mesh = loadObj(modelFileName);
	Lightmapper* lightmapper = new Lightmapper(mesh, std::stoi(width), std::stoi(height), 0.25, 0, Vector3D::Normalize(Vector3D(2, -5, -3)));
    
    auto start = std::chrono::high_resolution_clock::now();

    lightmapper->CalculateDiffuse();
    lightmapper->CastShadows();

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<D64> duration = end - start;
    std::cout << duration.count() << " seconds" << std::endl;

	lightmapper->Encode(textureFileName);
	delete lightmapper;
}

std::vector<Triangle> loadObj(const std::string& fileName)
{
	std::vector<Triangle> triangles;

	std::ifstream stream(fileName);
	if (!stream.is_open())
		std::cerr << "Can't load OBJ file!" << std::endl;

	std::vector<Vector3D> vertices;
	std::vector<Vector3D> normals;
	std::vector<TexCoord> texCoordinates;

    std::string line;

    while (std::getline(stream, line)) 
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v")
        {
            Vector3D vertex;
            iss >> vertex.X >> vertex.Y >> vertex.Z;
            vertices.push_back(vertex);
        }
        else if (type == "vt") 
        {
            TexCoord uv;
            iss >> uv.U >> uv.V;
            texCoordinates.push_back(uv);
        }
        else if (type == "vn")
        {
            Vector3D normal;
            iss >> normal.X >> normal.Y >> normal.Z;
            normals.push_back(normal);
        }
        else if (type == "f") 
        {
            int vIndices[3];
            int tIndices[3];
            int nIndices[3];

            for (int i = 0; i < 3; ++i) 
            {
                std::string data;
                iss >> data;
                std::istringstream dataStream(data);

                dataStream >> vIndices[i];
                vIndices[i]--;

                if (data.find("/") != std::string::npos) {
                    char slash;
                    dataStream >> slash >> tIndices[i] >> slash >> nIndices[i];
                    tIndices[i]--;
                    nIndices[i]--;
                }
            }

            triangles.push_back(Triangle(
                Vertex(vertices[vIndices[0]], normals[nIndices[0]], texCoordinates[tIndices[0]]),
                Vertex(vertices[vIndices[1]], normals[nIndices[1]], texCoordinates[tIndices[1]]),
                Vertex(vertices[vIndices[2]], normals[nIndices[2]], texCoordinates[tIndices[2]])));
        }
    }

	return triangles;
}
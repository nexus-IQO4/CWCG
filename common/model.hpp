#pragma once

#include <vector>
#include <string>
#include <stdio.h>
#include <cstring>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <common/texture.hpp> // Needs the global loadTexture function

// Texture struct to hold texture data
struct Texture
{
    unsigned int id;
    std::string type;
};

class Model
{
public:
    // Model data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<Texture>   textures;

    // Public material properties
    float ka = 0.1f, kd = 0.9f, ks = 0.4f, Ns = 20.0f;

    // Constructor that loads a model from a file path
    Model(const char* path)
    {
        // Load object data from file
        bool res = loadObj(path, vertices, uvs, normals);

        // If loaded successfully, set up OpenGL buffers
        if (res) {
            setupBuffers();
        }
    }

    // Renders the model
    void draw(unsigned int& shaderID)
    {
        // Bind all textures
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string name = textures[i].type;
            // Set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shaderID, (name + "Map").c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // Draw the model's triangles
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
        glBindVertexArray(0);

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }

    // Adds a texture to the model
    void addTexture(const char* path, const std::string type)
    {
        Texture texture;
        texture.id = ::loadTexture(path); // Use the global loadTexture function
        if (texture.id == 0) {
            printf("Model failed to add texture at path: %s\n", path);
            return;
        }
        texture.type = type;
        textures.push_back(texture);
    }

    // Frees buffer memory
    void deleteBuffers()
    {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &uvBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteVertexArrays(1, &VAO);
    }

private:
    // Buffer IDs
    unsigned int VAO;
    unsigned int vertexBuffer;
    unsigned int uvBuffer;
    unsigned int normalBuffer;

    // Method to set up OpenGL buffers
    void setupBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Vertex Buffer
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // UV Buffer
        glGenBuffers(1, &uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Normal Buffer
        glGenBuffers(1, &normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindVertexArray(0);
    }

    // Method to load an .obj file, styled like the template
    bool loadObj(const char* path,
        std::vector<glm::vec3>& outVertices,
        std::vector<glm::vec2>& outUVs,
        std::vector<glm::vec3>& outNormals)
    {
        printf("Loading file %s\n", path);

        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> tempVertices;
        std::vector<glm::vec2> tempUVs;
        std::vector<glm::vec3> tempNormals;

        FILE* file = fopen(path, "r");
        if (file == NULL)
        {
            printf("Impossible to open the file. Check paths and directories.\n");
            getchar(); // Pauses console like the template
            return false;
        }

        while (true)
        {
            char lineHeader[128];
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF) break;

            if (strcmp(lineHeader, "v") == 0) {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                tempVertices.push_back(vertex);
            }
            else if (strcmp(lineHeader, "vt") == 0) {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                tempUVs.push_back(uv);
            }
            else if (strcmp(lineHeader, "vn") == 0) {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                tempNormals.push_back(normal);
            }
            else if (strcmp(lineHeader, "f") == 0) {
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                    &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                    &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                    &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

                if (matches != 9) {
                    printf("File can't be read by this simple parser. Try exporting with triangulated faces and normals.\n");
                    fclose(file);
                    return false;
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
                uvIndices.push_back(uvIndex[1]);
                uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            else {
                char commentBuffer[1000];
                fgets(commentBuffer, 1000, file);
            }
        }

        for (unsigned int i = 0; i < vertexIndices.size(); i++) {
            unsigned int vertexIndex = vertexIndices[i];
            unsigned int uvIndex = uvIndices[i];
            unsigned int normalIndex = normalIndices[i];

            glm::vec3 vertex = tempVertices[vertexIndex - 1];
            glm::vec2 uv = tempUVs[uvIndex - 1];
            glm::vec3 normal = tempNormals[normalIndex - 1];

            outVertices.push_back(vertex);
            outUVs.push_back(uv);
            outNormals.push_back(normal);
        }

        fclose(file);
        return true;
    }
};
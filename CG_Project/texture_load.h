#pragma once

#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

// to upload images(PNG or JPG) as textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

GLuint bgVAO = 0, bgVBO = 0;

GLuint cubeVAO = 0, cubeVBO = 0;       // cube for boss
// cube vertex 
float cube[8][3] =
{
    {0.15f, 0, -0.15f}, {-0.15f, 0, -0.15f}, {-0.15f, 0, 0.15f}, {0.15f, 0, 0.15f},
    {0.15f, 0.3f, -0.15f}, {-0.15f, 0.3f, -0.15f}, {-0.15f, 0.3f, 0.15f}, {0.15f, 0.3f, 0.15f}
};
int faces[6][4] = {
    {0, 1, 2, 3}, // 아래면
    {4, 7, 6, 5}, // 윗면
    {1, 5, 6, 2}, // 뒷면
    {0, 3, 7, 4}, // 앞면
    {0, 4, 5, 1}, // 왼쪽
    {3, 2, 6, 7}  // 오른쪽
};

int cubeVertexCount = 0;

void pushVertex(std::vector<GLfloat>& vtx, const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv)
{
    vtx.push_back(p.x); vtx.push_back(p.y); vtx.push_back(p.z);   // position
    vtx.push_back(n.x); vtx.push_back(n.y); vtx.push_back(n.z);   // normal
    vtx.push_back(uv.x); vtx.push_back(uv.y);   // texcoord
}

void InitCube()
{
    std::vector<GLfloat> vertices;

    for (int i = 0; i < 6; i++)
    {
        int v0 = faces[i][0], v1 = faces[i][1], v2 = faces[i][2], v3 = faces[i][3];

        glm::vec3 p0(cube[v0][0], cube[v0][1], cube[v0][2]);
        glm::vec3 p1(cube[v1][0], cube[v1][1], cube[v1][2]);
        glm::vec3 p2(cube[v2][0], cube[v2][1], cube[v2][2]);
        glm::vec3 p3(cube[v3][0], cube[v3][1], cube[v3][2]);

        // 면 노말
        glm::vec3 n = glm::normalize(glm::cross(p2 - p0, p1 - p0));

        // 간단히: 한 면의 4개 꼭짓점 uv
        glm::vec2 uv0(0.0f, 0.0f);
        glm::vec2 uv1(1.0f, 0.0f);
        glm::vec2 uv2(1.0f, 1.0f);
        glm::vec2 uv3(0.0f, 1.0f);

        // 삼각형 1: v0, v1, v2
        pushVertex(vertices, p0, n, uv0);
        pushVertex(vertices, p1, n, uv1);
        pushVertex(vertices, p2, n, uv2);
        // 삼각형 2: v0, v2, v3
        pushVertex(vertices, p0, n, uv0);
        pushVertex(vertices, p2, n, uv2);
        pushVertex(vertices, p3, n, uv3);
    }

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GLfloat) * vertices.size(),
        vertices.data(),
        GL_STATIC_DRAW);

    // position : location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal : location = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texcoord : location = 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    cubeVertexCount = static_cast<int>(vertices.size() / 8);
}

void InitBackgroundQuad()
{
    float quadVertices[] = {
        // positions   // texcoords
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    GLuint EBO;
    glGenVertexArrays(1, &bgVAO);
    glGenBuffers(1, &bgVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(bgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // texcoord
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// 텍스처
GLuint LoadTexture(const char* filename)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data)
    {
        std::cout << "Failed to load: " << filename << "\n";
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // 필터링 & 래핑
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;

    glTexImage2D(GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return texID;   // 텍스처 ID를 리턴
}
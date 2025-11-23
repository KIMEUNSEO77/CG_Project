#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Mesh
{
    GLuint vao = 0, vbo = 0;
    GLsizei count = 0; // number of vertices
};

// interleaved = [px,py,pz, nx,ny,nz] * N
static void SubdivideToUnitSphere(std::vector<float>& inter, int levels, float radius = 1.0f)
{
    auto toSphere = [&](const glm::vec3& p) {
        glm::vec3 n = glm::normalize(p);
        return radius * n;
        };

    for (int lv = 0; lv < levels; ++lv) {
        std::vector<float> next;
        next.reserve(inter.size() * 4); // 대략 4배로 늘어남

        for (size_t i = 0; i + 17 < inter.size(); i += 18) {
            // 한 삼각형의 3개 정점 (pos만 쓰고 normal은 새로 계산)
            glm::vec3 p0(inter[i + 0], inter[i + 1], inter[i + 2]);
            glm::vec3 p1(inter[i + 6], inter[i + 7], inter[i + 8]);
            glm::vec3 p2(inter[i + 12], inter[i + 13], inter[i + 14]);

            // 원 꼭짓점도 정확히 구 표면으로 투영(반지름 보정)
            p0 = toSphere(p0);
            p1 = toSphere(p1);
            p2 = toSphere(p2);

            // 변 중점
            glm::vec3 m01 = toSphere((p0 + p1) * 0.5f);
            glm::vec3 m12 = toSphere((p1 + p2) * 0.5f);
            glm::vec3 m20 = toSphere((p2 + p0) * 0.5f);

            auto pushTri = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                glm::vec3 na = glm::normalize(a);
                glm::vec3 nb = glm::normalize(b);
                glm::vec3 nc = glm::normalize(c);
                // a
                next.push_back(a.x); next.push_back(a.y); next.push_back(a.z);
                next.push_back(na.x); next.push_back(na.y); next.push_back(na.z);
                // b
                next.push_back(b.x); next.push_back(b.y); next.push_back(b.z);
                next.push_back(nb.x); next.push_back(nb.y); next.push_back(nb.z);
                // c
                next.push_back(c.x); next.push_back(c.y); next.push_back(c.z);
                next.push_back(nc.x); next.push_back(nc.y); next.push_back(nc.z);
                };

            // 한 삼각형 → 4개
            pushTri(p0, m01, m20);
            pushTri(m01, p1, m12);
            pushTri(m20, m12, p2);
            pushTri(m01, m12, m20);
        }

        inter.swap(next);
    }
}

static bool LoadOBJ_PosNorm_Interleaved(const char* path, Mesh& out)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[OBJ] failed to open: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    // interleaved: px,py,pz, nx,ny,nz (per vertex)
    std::vector<float> interleaved;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string tag; iss >> tag;
        if (tag == "v") {
            glm::vec3 p; iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn") {
            glm::vec3 n; iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "f") {
            // Expect "a//a b//b c//c"
            for (int i = 0; i < 3; ++i) {
                std::string tok; iss >> tok;
                // split by "//"
                size_t p = tok.find("//");
                if (p == std::string::npos) {
                    std::cerr << "[OBJ] face format must be v//vn\n";
                    return false;
                }
                int vi = std::stoi(tok.substr(0, p));
                int ni = std::stoi(tok.substr(p + 2));
                // OBJ is 1-based
                glm::vec3 P = positions[vi - 1];
                glm::vec3 N = normals[ni - 1];
                interleaved.push_back(P.x); interleaved.push_back(P.y); interleaved.push_back(P.z);
                interleaved.push_back(N.x); interleaved.push_back(N.y); interleaved.push_back(N.z);
            }
        }
    }

    for (size_t i = 0; i < interleaved.size(); i += 6) {
        glm::vec3 P(interleaved[i + 0], interleaved[i + 1], interleaved[i + 2]);
        glm::vec3 N = glm::normalize(P);        // 구 표면 법선
        interleaved[i + 3] = N.x;
        interleaved[i + 4] = N.y;
        interleaved[i + 5] = N.z;
    }

    SubdivideToUnitSphere(interleaved, /*levels=*/2, /*radius=*/0.4f);

    // GL buffer
    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);

    glBindVertexArray(out.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    out.count = static_cast<GLsizei>(interleaved.size() / 6);
    return true;
}

// 일반 OBJ 파일을 로드하는 함수 (구가 아닌 모델용)
static bool LoadOBJ_General(const char* path, Mesh& out)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[OBJ] failed to open: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    // interleaved: px,py,pz, nx,ny,nz (per vertex)
    std::vector<float> interleaved;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string tag; iss >> tag;
        
        if (tag == "v") {
            glm::vec3 p; iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn") {
            glm::vec3 n; iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "f") {
            // Expect "a//a b//b c//c" format
            for (int i = 0; i < 3; ++i) {
                std::string tok; iss >> tok;
                
                // split by "//"
                size_t p = tok.find("//");
                if (p == std::string::npos) {
                    std::cerr << "[OBJ] face format must be v//vn\n";
                    return false;
                }
                
                int vi = std::stoi(tok.substr(0, p));
                int ni = std::stoi(tok.substr(p + 2));
                
                // OBJ is 1-based
                glm::vec3 P = positions[vi - 1];
                glm::vec3 N = normals[ni - 1];
                
                interleaved.push_back(P.x); interleaved.push_back(P.y); interleaved.push_back(P.z);
                interleaved.push_back(N.x); interleaved.push_back(N.y); interleaved.push_back(N.z);
            }
        }
    }

    if (interleaved.empty()) {
        std::cerr << "[OBJ] no geometry found in: " << path << "\n";
        return false;
    }

    // GL buffer 생성
    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);

    glBindVertexArray(out.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    
    // layout(location=1) vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    out.count = static_cast<GLsizei>(interleaved.size() / 6);
    
    std::cout << "[OBJ] Loaded " << path << " with " << out.count << " vertices\n";
    return true;
}
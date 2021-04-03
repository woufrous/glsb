#pragma once

#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "mesh.h"

struct CCS {
    glm::vec3 e_x;
    glm::vec3 e_y;
    glm::vec3 e_z;
};

struct Camera {
    glm::mat4 get_view_matrix() const noexcept {
        return glm::lookAt(pos, tgt, glm::vec3(0.f, 0.f, 1.f));
    }

    glm::mat4 get_proj_matrix() const noexcept {
        return glm::perspective(glm::radians(fov), aspect, clip_dist.first, clip_dist.second);
    }

    CCS local_ccs() const noexcept {
        auto ccs = CCS{};
        ccs.e_z = glm::normalize(tgt-pos);
        ccs.e_y = glm::normalize(glm::cross(ccs.e_z, glm::vec3(0.f, 0.f, 1.f)));
        ccs.e_x = glm::normalize(glm::cross(ccs.e_y, ccs.e_z));
        return ccs;
    }

    glm::vec3 dir() const noexcept {
        return glm::normalize(pos-tgt);
    }

    glm::vec3 pos;
    glm::vec3 tgt;
    std::pair<float, float> clip_dist;
    float fov;
    float aspect;
};

struct DiffuseLight {
    glm::vec3 dir;
    glm::vec3 color;
    float intensity;
};

struct Scene {
    Camera cam;
    struct AmbientLight {
        glm::vec3 color;
        float intensity;
    } ambient;
    DiffuseLight diffuse;
};

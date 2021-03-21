#pragma once

#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "mesh.h"

struct Camera {
    glm::mat4 get_vp_matrix() const noexcept {
        auto view = glm::lookAt(pos, tgt, glm::vec3(0.f, 0.f, 1.f));
        auto proj = glm::perspective(glm::radians(fov), aspect, clip_dist.first, clip_dist.second);
        return proj * view;
    }

    glm::vec3 pos;
    glm::vec3 tgt;
    std::pair<float, float> clip_dist;
    float fov;
    float aspect;
};

struct Light {
    glm::vec3 pos;
    glm::vec3 color;
    float intensity;
};

struct Scene {
    Camera cam;
    Light light;
    std::vector<Mesh> meshes;
};

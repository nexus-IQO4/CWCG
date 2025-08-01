#pragma once
#include <glm/glm.hpp>

namespace mymaths {
    // Vector functions
    float length(const glm::vec3& v);
    glm::vec3 normalize(const glm::vec3& v);
    float dot(const glm::vec3& v1, const glm::vec3& v2);
    glm::vec3 cross(const glm::vec3& v1, const glm::vec3& v2);

    // Matrix functions
    // A custom implementation of the lookAt matrix
    glm::mat4 lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& worldUp);
}
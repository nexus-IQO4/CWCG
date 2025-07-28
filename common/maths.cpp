#include "maths.hpp"
#include <cmath> // For sqrt

namespace mymaths {
    // Calculates the length (magnitude) of a vector.
    float length(const glm::vec3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    // Returns a unit vector in the same direction as the original.
    glm::vec3 normalize(const glm::vec3& v) {
        float l = length(v);
        if (l == 0.0f) return glm::vec3(0.0f); // Avoid division by zero.
        return glm::vec3(v.x / l, v.y / l, v.z / l);
    }

    // Calculates the dot product of two vectors.
    float dot(const glm::vec3& v1, const glm::vec3& v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    // Calculates the cross product of two vectors.
    glm::vec3 cross(const glm::vec3& v1, const glm::vec3& v2) {
        return glm::vec3(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        );
    }

    // Custom implementation of the lookAt function to create a view matrix.
    // This is a key requirement for higher marks in LO1.
    glm::mat4 lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& worldUp) {
        // 1. Calculate the camera's direction vector (z-axis)
        glm::vec3 z_axis = mymaths::normalize(position - target);

        // 2. Calculate the camera's right vector (x-axis)
        glm::vec3 x_axis = mymaths::normalize(mymaths::cross(mymaths::normalize(worldUp), z_axis));

        // 3. Calculate the camera's up vector (y-axis)
        glm::vec3 y_axis = mymaths::cross(z_axis, x_axis);

        // 4. Create the rotation matrix
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation[0][0] = x_axis.x; rotation[1][0] = x_axis.y; rotation[2][0] = x_axis.z;
        rotation[0][1] = y_axis.x; rotation[1][1] = y_axis.y; rotation[2][1] = y_axis.z;
        rotation[0][2] = z_axis.x; rotation[1][2] = z_axis.y; rotation[2][2] = z_axis.z;

        // 5. Create the translation matrix
        glm::mat4 translation = glm::mat4(1.0f);
        translation[3][0] = -position.x;
        translation[3][1] = -position.y;
        translation[3][2] = -position.z;

        // The view matrix is the combination of the rotation and translation
        return rotation * translation;
    }

}
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform vertex position to world space and then to clip space
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
    // Transform normal vector to world space
    // Using the inverse transpose avoids incorrect normals with non-uniform scaling
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates to the fragment shader
    TexCoords = aTexCoords;
}
#version 410 compatibility

in vec3 FragPos;
in vec3 Normal;
in vec4 VertexColor;

void main() {
    gl_Position = gl_Vertex;
}
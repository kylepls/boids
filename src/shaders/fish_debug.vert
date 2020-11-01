#version 440 compatibility

in vec3 n;
in vec2 t;
in vec4 c;

out vec3 i_vertex_in;
out vec3 i_normal_in;
out vec2 i_tex_in;
out vec4 i_color;

void main() {
    gl_Position = gl_Vertex;

    i_vertex_in = gl_Vertex.xyz;
    i_normal_in = n;
    i_tex_in = t;
    i_color = c;
}
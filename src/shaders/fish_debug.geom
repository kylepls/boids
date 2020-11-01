#version 440 compatibility

layout (triangles) in;
layout (line_strip, max_vertices=10) out;

in vec3 i_vertex_in[];
in vec3 i_normal_in[];
in vec2 i_tex_in[];
in vec4 i_color[];

uniform vec3 ViewPos;
uniform vec3 position;
uniform vec3 velocity;
uniform int show_normals;
uniform int animate_fish;

uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;

uniform float time;

out vec2 tex_in;
out vec4 color;

void main() {
    mat4 matProj = gl_ModelViewProjectionMatrix;
    tex_in = vec2(0.5);
    color = vec4(1, 0, 1, 1);

    for (int i = 0; i < 3; i++) {
        float z = gl_in[i].gl_Position.z;

        float scale = 0.5;
        float freq = 5;
        float size = clamp(1/(z+4), 0, 1);
        vec4 add = vec4(sin(time/1000 * freq + z)*scale * size, 0, 0, 0);
        if (animate_fish == 0) {
            add = vec4(0);
        }

        gl_Position = matProj * (add + gl_in[i].gl_Position);
        vec4 worldPos = ModelMatrix * (add + gl_in[i].gl_Position);
        color = mix(vec4(1, 0, 1, 1), vec4(1, 1, 0, 1), distance(worldPos.xyz, ViewPos)/20);
        EmitVertex();
    }
    EndPrimitive();

    if (show_normals == 1) {
        vec4 P0 = gl_in[0].gl_Position;
        vec4 P1 = gl_in[1].gl_Position;
        vec4 P2 = gl_in[2].gl_Position;

        vec4 middle = (P0 + P1 + P2)/3;
        vec4 worldPos = ModelMatrix * middle;
        float m = clamp(distance(worldPos.xyz, ViewPos)/20, 0, 1);

        color = vec4(1, 1, 0, 1);
        color = mix(color, vec4(color.rgb, 0), m);
        gl_Position = matProj * middle;
        EmitVertex();

        gl_Position = matProj * (middle + normalize(vec4(i_normal_in[0], 0)) * 0.1);
        EmitVertex();
        EndPrimitive();
    }
}

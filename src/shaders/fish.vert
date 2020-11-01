#version 440 compatibility

uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform float time;

uniform int animate_fish;

layout(location=0) in vec3 vertex_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec2 tex_in;

out struct Data {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoord;
} Vert;


void main() {
    vec4 world = ModelMatrix * vec4(vertex_in, 1);
    Vert.FragPos = world.xyz;
    Vert.Normal = normalize(gl_NormalMatrix * normal_in.xyz);
    Vert.texCoord = tex_in;

    float z = vertex_in.z;
    float scale = 0.5;
    float freq = 5;
    float size = clamp(1/(z+4), 0, 1);
    vec4 add = vec4(sin(time/1000 * freq + z)*scale * size, 0, 0, 0);
    if (animate_fish == 0) {
        add = vec4(0);
    }

    gl_Position = gl_ModelViewProjectionMatrix * (vec4(vertex_in, 1) + add);
}
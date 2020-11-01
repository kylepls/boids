#version 440

uniform vec3 ViewPos;
uniform vec3 LightPos;

uniform float fogDensity;
uniform vec4 fogColor;
uniform float fogMax;
uniform float fogMin;
uniform int show_fog;

const float ambientMultiplier = 0.7;
const float specularMultiplier = 0.5;
const vec3 lightColor = vec3(1, 1, 1);

uniform sampler2D tex;

in struct Data {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoord;
} Vert;

out vec4 FragColor;

float fog_factor() {
    float d = distance(ViewPos, Vert.FragPos);
    float linearFog = 1 - (fogMax - d)/ (fogMax - fogMin);
    float expFog = 1/exp(d * fogDensity);
    float expFog2 = 1/exp(pow(d*fogDensity, 2));
    return clamp(linearFog + expFog + expFog2, 0, 1);
}

vec4 apply_fog(vec4 in_color) {
    if (show_fog == 1) {
        return mix(in_color, fogColor, fog_factor());
    } else {
        return in_color;
    }
}

void main() {
    vec4 VertexColor = texture2D(tex, Vert.texCoord);
    vec3 ambient = ambientMultiplier * lightColor * VertexColor.rgb;

    vec3 lightDir = normalize(LightPos - Vert.FragPos);
    float diff = max(dot(Vert.Normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(ViewPos - Vert.FragPos);
    vec3 reflectDir = reflect(-lightDir, Vert.Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
    vec3 specular = specularMultiplier * spec * lightColor;

    vec4 lighted = vec4((ambient + diffuse + specular) * VertexColor.rgb, 1);
    FragColor = apply_fog(lighted);
}

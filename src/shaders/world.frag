#version 440

uniform vec3 ViewPos;
uniform vec3 LightPos;

uniform float fogDensity;
uniform vec4 fogColor;
uniform float fogMax;
uniform float fogMin;

uniform sampler2D caustic;
uniform float caustic_intensity;
uniform float caustic_scale;

uniform int light_demo;
uniform int caustic_demo;
uniform int show_caustic;
uniform int show_fog;

const float ambientMultiplier = 0.7;
const float specularMultiplier = 0.5;
const vec3 lightColor = vec3(1, 1, 1);

in vec3 FragPos;
in vec3 Normal;
in vec4 VertexColor;
in float occlusion;

out vec4 FragColor;

float caustic_multiplier() {
    return max(dot(normalize(Normal), normalize(LightPos-FragPos)), 0) * occlusion;
}

float fog_factor() {
    float d = distance(ViewPos, FragPos);
    float linearFog = 1 - (fogMax - d)/ (fogMax - fogMin);
    float expFog = 1/exp(d * fogDensity);
    float expFog2 = 1/exp(pow(d*fogDensity, 2));
    return linearFog + expFog + expFog2;
}

vec4 apply_fog(vec4 in_color) {
    if (show_fog == 1) {
        return mix(in_color, fogColor, fog_factor());
    } else {
        return in_color;
    }
}

void main() {
    vec3 ambient = ambientMultiplier * lightColor * VertexColor.rgb;

    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
    vec3 specular = specularMultiplier * spec * lightColor;

    vec4 lighted = vec4((ambient + diffuse + specular) * occlusion * VertexColor.rgb, 1);
    vec4 caustic_value = texture2D(caustic, FragPos.xz*caustic_scale)*caustic_intensity*caustic_multiplier();

    if (show_caustic == 0) {
        caustic_value = vec4(0);
    }

    FragColor = apply_fog(lighted+caustic_value);

    if (caustic_demo == 1) {
        FragColor = texture2D(caustic, FragPos.xz*caustic_scale);
    }

    if (light_demo == 1) {
        FragColor = mix(vec4(0, 0, 0, 1), vec4(1, 1, 1, 1), occlusion);
    }
}

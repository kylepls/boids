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

const float ambientMultiplier = 0.7;
const float specularMultiplier = 0.5;
const vec3 lightColor = vec3(1, 1, 1);

in vec3 FragPos;
in vec3 Normal;
in vec4 VertexColor;
in float light;

out vec4 FragColor;

float caustic_multiplier() {
    return max(dot(Normal, LightPos-FragPos), 0);
    //    return 1;
}

float fog_factor() {
    float d = distance(ViewPos, FragPos);
    float linearFog = 1 - (fogMax - d)/ (fogMax - fogMin);
    float expFog = 1/exp(d * fogDensity);
    float expFog2 = 1/exp(pow(d*fogDensity, 2));
    return linearFog + expFog + expFog2;
}

vec4 apply_fog(vec4 in_color) {
    return mix(in_color, fogColor, fog_factor());
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

    vec4 lighted = vec4((ambient + diffuse + specular) * VertexColor.rgb, 1);

    FragColor = apply_fog(lighted);
    //        FragColor = mix(vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), caustic_multiplier());
    //    FragColor = vec4(light, 1, 0, 1);
}

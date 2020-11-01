#version 410
#extension GL_EXT_gpu_shader4 : enable

layout (points) in;
layout (triangle_strip, max_vertices=15) out;

out vec3 FragPos;
out vec3 Normal;
out vec4 VertexColor;
out float occlusion;

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

uniform int calculate_occlusion;

uniform int octaves;
uniform float iso_level;
uniform float persistence;
uniform float lacunarity;
uniform float weight_multiplier;
uniform float noiseOffset;

// This is really dumb, 100 = max octaves
uniform float octave_offsets[100];

uniform float noise_scale;
uniform isampler2D marching_lookup_table;
uniform float world_scale;

uniform vec3 LightPos;

//	Simplex 3D Noise
//	by Ian McEwan, Ashima Arts
vec4 permute(vec4 x){ return mod(((x*34.0)+1.0)*x, 289.0); }
vec4 taylorInvSqrt(vec4 r){ return 1.79284291400159 - 0.85373472095314 * r; }

float noise(vec3 v){
    const vec2 C = vec2(1.0/6.0, 1.0/3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 =   v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0);
    vec4 p = permute(permute(permute(
    i.z + vec4(0.0, i1.z, i2.z, 1.0))
    + i.y + vec4(0.0, i1.y, i2.y, 1.0))
    + i.x + vec4(0.0, i1.x, i2.x, 1.0)
    );

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0;// N=7
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z *ns.z);//  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);// mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

float offset_value(int i) {
    //    return texelFetch(octave_offsets, i, 0).r;
    return octave_offsets[i];
}

// Fractional Brownian Motion implementation inspired from
// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu
float density(vec4 P) {
    if (P.y < 1) {
        return 0.0;
    }

    float computeNoise = 0;
    vec4 localOffset = vec4(0, 20, 0, 0);

    float frequency = noise_scale / 100;
    float amplitude = 1;
    float weight = 1;
    for (int j = 0; j < octaves; j++) {
        float o = offset_value(j);
        vec4 N = P * frequency + o + noiseOffset + localOffset;
        float n = noise(N.xyz);
        float v = 1 - abs(n);
        v = v * v;
        v *= weight;
        weight = max(min(v * weight_multiplier, 1), 0);
        computeNoise += v * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    float finalVal = -(20 - P.y) + computeNoise * weight_multiplier + mod(P.y, 5);

    return finalVal;
}

vec4 color(vec4 P) {
    vec3 color = vec3(0, 0, 0);
    vec3 offsets = vec3(10, 20, -442);

    float amplitude = 1;
    float frequency = 0.025;
    vec3 weights = vec3(1, 1, 1);

    for (int j = 0; j < octaves/4; j++) {
        vec3 n = vec3(
        noise(P.xyz*frequency+offsets[0]+noiseOffset),
        noise(P.xyz*frequency+offsets[1]+noiseOffset),
        noise(P.xyz*frequency+offsets[2]+noiseOffset)
        ) + 0.5;
        vec3 v = 1-abs(n);

        v = pow(v, vec3(2, 2, 2))*weights;

        weights = max(min(v * weight_multiplier, vec3(1, 1, 1)), vec3(0, 0, 0));
        amplitude *= persistence;
        frequency *= lacunarity;

        color += v * amplitude;
    }
    return vec4(color, 1.0);
}

const int cornerIndexAFromEdge[12] = int[](0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3);
const int cornerIndexBFromEdge[12] = int[](1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7);

const vec3 deltas[8] = vec3[](
vec3(0, 0, 0),
vec3(1, 0, 0),
vec3(1, 0, 1),
vec3(0, 0, 1),
vec3(0, 1, 0),
vec3(1, 1, 0),
vec3(1, 1, 1),
vec3(0, 1, 1)
);

#define RAYS 32
// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu
const vec4[RAYS] occlusion_rays = vec4[](
// 32 rays with a nice poisson distribution on a sphere:
vec4(0.286582, 0.257763, -0.922729, 1),
vec4(-0.171812, -0.888079, 0.426375, 1),
vec4(0.440764, -0.502089, -0.744066, 1),
vec4(-0.841007, -0.428818, -0.329882, 1),
vec4(-0.380213, -0.588038, -0.713898, 1),
vec4(-0.055393, -0.207160, -0.976738, 1),
vec4(-0.901510, -0.077811, 0.425706, 1),
vec4(-0.974593, 0.123830, -0.186643, 1),
vec4(0.208042, -0.524280, 0.825741, 1),
vec4(0.258429, -0.898570, -0.354663, 1),
vec4(-0.262118, 0.574475, -0.775418, 1),
vec4(0.735212, 0.551820, 0.393646, 1),
vec4(0.828700, -0.523923, -0.196877, 1),
vec4(0.788742, 0.005727, -0.614698, 1),
vec4(-0.696885, 0.649338, -0.304486, 1),
vec4(-0.625313, 0.082413, -0.776010, 1),
vec4(0.358696, 0.928723, 0.093864, 1),
vec4(0.188264, 0.628978, 0.754283, 1),
vec4(-0.495193, 0.294596, 0.817311, 1),
vec4(0.818889, 0.508670, -0.265851, 1),
vec4(0.027189, 0.057757, 0.997960, 1),
vec4(-0.188421, 0.961802, -0.198582, 1),
vec4(0.995439, 0.019982, 0.093282, 1),
vec4(-0.315254, -0.925345, -0.210596, 1),
vec4(0.411992, -0.877706, 0.244733, 1),
vec4(0.625857, 0.080059, 0.775818, 1),
vec4(-0.243839, 0.866185, 0.436194, 1),
vec4(-0.725464, -0.643645, 0.243768, 1),
vec4(0.766785, -0.430702, 0.475959, 1),
vec4(-0.446376, -0.391664, 0.804580, 1),
vec4(-0.761557, 0.562508, 0.321895, 1),
vec4(0.344460, 0.753223, -0.560359, 1)

//vec4(0.080521, 0.975000, 0.207102, 1),
//vec4(-0.340748, 0.925000, -0.168124, 1),
//vec4(0.464828, 0.875000, -0.135314, 1),
//vec4(-0.293405, 0.825000, 0.483000, 1),
//vec4(-0.122912, 0.775000, -0.619893, 1),
//vec4(0.555134, 0.725000, 0.407678, 1),
//vec4(-0.733503, 0.675000, 0.079678, 1),
//vec4(0.515298, 0.625000, -0.586381, 1),
//vec4(0.016906, 0.575000, 0.817979, 1),
//vec4(-0.587756, 0.525000, -0.615563, 1),
//vec4(0.878019, 0.475000, 0.058803, 1),
//vec4(-0.706828, 0.425000, 0.565481, 1),
//vec4(0.142574, 0.375000, -0.915995, 1),
//vec4(0.523971, 0.325000, 0.787292, 1),
//vec4(-0.933440, 0.275000, -0.230356, 1),
//vec4(0.855229, 0.225000, -0.466860, 1),
//vec4(-0.318563, 0.175000, 0.931608, 1),
//vec4(-0.397433, 0.125000, -0.909077, 1),
//vec4(0.911724, 0.075000, 0.403899, 1),
//vec4(-0.947480, 0.025000, 0.318836, 1)
);

int triTableValue(int i, int j){
    return texelFetch2D(marching_lookup_table, ivec2(j, i), 0).a;
}

vec4 lerp(vec4 v1, vec4 v2) {
    float t = (iso_level-v1.w)/(v2.w-v1.w);
    return vec4(mix(v1, v2, t).xyz, 1);
}

vec4 getCubePoint(vec4 origin, vec3 offset) {
    vec3 P = origin.xyz + (offset*world_scale);
    return vec4(P, density(vec4(P, 1)));
}

void main() {
    mat4 matProj = ProjectionMatrix * ModelViewMatrix;
    vec4 point = gl_in[0].gl_Position;

    // Marching cubes modified from http://paulbourke.net/geometry/polygonise/
    // Also referenced: https://github.com/SebLague/Marching-Cubes
    vec4 cubeCorners[8];
    for (int i = 0; i < 8; i++) {
        vec3 P = (deltas[i] * world_scale) + point.xyz;
        cubeCorners[i] = vec4(P, density(vec4(P, 1)));
    }

    int cubeIndex = 0;
    if (cubeCorners[0].w < iso_level) cubeIndex |= 1;
    if (cubeCorners[1].w < iso_level) cubeIndex |= 2;
    if (cubeCorners[2].w < iso_level) cubeIndex |= 4;
    if (cubeCorners[3].w < iso_level) cubeIndex |= 8;
    if (cubeCorners[4].w < iso_level) cubeIndex |= 16;
    if (cubeCorners[5].w < iso_level) cubeIndex |= 32;
    if (cubeCorners[6].w < iso_level) cubeIndex |= 64;
    if (cubeCorners[7].w < iso_level) cubeIndex |= 128;

    for (int i = 0; triTableValue(cubeIndex, i) != -1; i += 3) {
        int a0 = cornerIndexAFromEdge[triTableValue(cubeIndex, i)];
        int b0 = cornerIndexBFromEdge[triTableValue(cubeIndex, i)];

        int a1 = cornerIndexAFromEdge[triTableValue(cubeIndex, i+1)];
        int b1 = cornerIndexBFromEdge[triTableValue(cubeIndex, i+1)];

        int a2 = cornerIndexAFromEdge[triTableValue(cubeIndex, i+2)];
        int b2 = cornerIndexBFromEdge[triTableValue(cubeIndex, i+2)];

        vec4 v1 = lerp(cubeCorners[a0], cubeCorners[b0]);
        vec4 v2 = lerp(cubeCorners[a1], cubeCorners[b1]);
        vec4 v3 = lerp(cubeCorners[a2], cubeCorners[b2]);

        vec3 outDir = normalize(cross(v2.xyz-v1.xyz, v3.xyz-v1.xyz));

        if (calculate_occlusion == 1) {
            vec4 middle = (v1+v2+v3)/3;

            int samples = 5;
            float sample_scale = 0.4;
            int blocked = 0;
            for (int r_i = 0; r_i< RAYS; r_i++) {
                for (int s = 1; s<samples+1; s++) {
                    vec4 r = s * sample_scale * occlusion_rays[r_i] + middle;
                    if (density(r) < iso_level) {
                        blocked++;
                        break;
                    }
                }
            }
            occlusion = float(blocked)/32;
        } else {
            occlusion = 1;
        }

        VertexColor = color(v1);
        gl_Position = matProj * v1;
        FragPos = v1.xyz;
        Normal = outDir;
        EmitVertex();

        VertexColor = color(v2);
        gl_Position = matProj * v2;
        FragPos = v2.xyz;
        Normal = outDir;
        EmitVertex();

        VertexColor = color(v3);
        gl_Position = matProj * v3;
        FragPos = v3.xyz;
        Normal = outDir;
        EmitVertex();

        EndPrimitive();
    }
}

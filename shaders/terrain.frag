#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
} fs_in;

out vec4 FragColor;

uniform vec3 viewPos;

// PBR-текстуры (альбедо, нормали, roughness, AO)
uniform sampler2D texAlbedo;
uniform sampler2D texNormal;
uniform sampler2D texRoughness;
uniform sampler2D texAO;

// параметры направленного света (Солнце)
uniform vec3 sun_direction;
uniform vec3 sun_color;        // интенсивность/цвет света
uniform float  sun_ambient;    // фоновая составляющая

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz нормал распределения
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float denom  = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// Schlick-GGX geometry term
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r*r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// приближение Fresnel (Schlick)
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    // 1 семплинг карт
    vec3 albedo    = pow(texture(texAlbedo, fs_in.TexCoord).rgb, vec3(2.2)); // sRGB->linear
    vec3 normMap   = texture(texNormal, fs_in.TexCoord).rgb * 2.0 - 1.0;
    float roughness= texture(texRoughness, fs_in.TexCoord).r;
    float ao       = texture(texAO, fs_in.TexCoord).r;

    // 2 TBN → мировая нормаль
    vec3 N = normalize(fs_in.TBN * normMap);

    // 3 векторы
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 L = normalize(-sun_direction);
    vec3 H = normalize(V + L);

    // 4 PBR компоненты
    // базовый F0 для диэлектриков ≈ 0.04
    vec3 F0 = vec3(0.04);
    vec3 F  = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator    = D * G * F;
    float denom       = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denom;

    // диффузная часть Ламберта
    vec3 kD = vec3(1.0) - F;
    vec3 diffuse = kD * albedo / PI;

    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = sun_color;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // 5 ambient occlusion + фон
    vec3 ambient = sun_ambient * albedo * ao;

    vec3 color = ambient + Lo;

    // gamma-correction back to sRGB
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}

#version 330 core
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight sun;
uniform vec3 viewPos;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;
    float bias = 0.005;
    for(int x=-1; x<=1; ++x)
        for(int y=-1; y<=1; ++y) {
            float pcf = texture(shadowMap,
                projCoords.xy + vec2(x,y)*texelSize).r;
            shadow += (projCoords.z - bias > pcf) ? 1.0 : 0.0;
        }
    shadow /= 9.0;
    return shadow;
}

void main() {
    // смешение текстур по высоте
    float height = fs_in.FragPos.y;
    float h1 = 20.0, h2 = 40.0, blend = 5.0;
    float gW = clamp(1.0 - smoothstep(h1-blend,h1+blend,height),0,1);
    float sW = clamp(smoothstep(h2-blend,h2+blend,height),0,1);
    float rW = clamp(1.0 - gW - sW,0,1);
    vec2 uv = fs_in.TexCoord;
    vec3 grass = texture(grassTex, uv).rgb;
    vec3 rock  = texture(rockTex,  uv).rgb;
    vec3 snow  = texture(snowTex,  uv).rgb;
    vec3 baseColor = gW*grass + rW*rock + sW*snow;

    // Blinn-Phong
    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(-sun.direction);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 H = normalize(L + V);
    float diff = max(dot(N,L),0.0);
    float spec = pow(max(dot(N,H),0.0), 32.0);
    vec3 ambient  = sun.ambient  * baseColor;
    vec3 diffuse  = sun.diffuse  * diff * baseColor;
    vec3 specular = sun.specular * spec * vec3(1.0);

    // тени
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = ambient + (1.0 - shadow)*(diffuse + specular);

    FragColor = vec4(lighting, 1.0);
}

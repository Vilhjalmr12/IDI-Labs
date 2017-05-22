#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;
uniform vec3 colFocus;
uniform vec3 posFocus;

// Valors per als components que necessitem dels focus de llum
vec3 llumAmbient = vec3(0.2, 0.2, 0.2);

out vec3 matAmb;
out vec3 matDiff;
out vec3 matSpec;
out float matShin;
out mat4 viewMatrix;
out vec3 fcolor;
out vec4 vertSCO;
out vec3 normalSCO;


void main() {
    //Donem valor a les variables que enviem al fragment shader
    matAmb = matamb;
    matDiff = matdiff;
    matSpec = matspec;
    matShin = matshin;
    viewMatrix = view;
    fcolor = colFocus;
    
    vertSCO = (view * TG * vec4(vertex, 1.0));
    mat3 norMatrix = inverse (transpose (mat3 (view * TG)));
    normalSCO = normalize(norMatrix * normal);
    gl_Position = proj * vertSCO;
}

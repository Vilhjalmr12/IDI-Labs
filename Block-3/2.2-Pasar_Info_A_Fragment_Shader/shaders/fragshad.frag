#version 330 core

in vec3 fcolor;
in vec4 vertSCO;
in vec3 normalSCO;
in vec3 matSpec;
in vec3 matDiff;
in vec3 matAmb;
in float matShin;

out vec4 FragColor;

uniform vec3 colFocus;
uniform vec3 posFocus;
uniform bool ilumType; //True = SCO (luz de camara) False = SCA (luz de escena)
vec3 llumAmbient = vec3(0.2, 0.2, 0.2);


vec3 Lambert (vec3 NormSCO, vec3 L) {
    // S'assumeix que els vectors que es reben com a paràmetres estan normalitzats
    // Inicialitzem color a component ambient
    vec3 colRes = llumAmbient * matAmb;

    // Afegim component difusa, si n'hi ha
    if (dot (L, NormSCO) > 0)
      colRes = colRes + fcolor * matDiff * dot (L, NormSCO);
    return (colRes);
}

vec3 Phong (vec3 NormSCO, vec3 L, vec4 vertSCO) {
    // Els vectors estan normalitzats
    // Inicialitzem color a Lambert
    vec3 colRes = Lambert (NormSCO, L);

    // Calculem R i V
    if (dot(NormSCO,L) < 0)
      return colRes;  // no hi ha component especular

    vec3 R = reflect(-L, NormSCO); // equival a: normalize (2.0*dot(NormSCO,L)*NormSCO - L);
    vec3 V = normalize(-vertSCO.xyz);

    if ((dot(R, V) < 0) || (matShin == 0))
      return colRes;  // no hi ha component especular

    // Afegim la component especular
    float shine = pow(max(0.0, dot(R, V)), matShin);
    return (colRes + matSpec * fcolor * shine);
}


void main() {
        vec4 auxPosFocus = vec4(posFocus, 1.0);
        vec3 L = normalize(auxPosFocus.xyz - vertSCO.xyz);
        vec3 color = Phong(normalSCO, L, vertSCO);
        FragColor = vec4(color, 1);
}

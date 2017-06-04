#version 330 core

in vec3 fcolor;
out vec4 FragColor;

uniform bool isStriped;

void main() {
    if (isStriped) {
        if ((int(gl_FragCoord.y/10)%2) == 0)
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        if ((int(gl_FragCoord.y/10)%2) == 1)
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        FragColor = vec4(fcolor,1);
    }
}

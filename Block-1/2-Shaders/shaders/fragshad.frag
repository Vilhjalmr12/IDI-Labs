#version 330 core

out vec4 FragColor;

void main() {
    //FragColor = vec4(0, 0, 0, 1);
    
    //Pinta el triangulo por sectores
    if (gl_FragCoord.x < 357.0 && gl_FragCoord.y < 357.0)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    if (gl_FragCoord.x < 357.0 && gl_FragCoord.y > 357.0)
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    if (gl_FragCoord.x > 357.0 && gl_FragCoord.y < 357.0)
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    if (gl_FragCoord.x > 357.0 && gl_FragCoord.y > 357.0)
        FragColor = vec4(1.0, 0.0, 1.0, 1.0);

        
    //Las franjas que cumplan esto, quedan descartadas
    if((int(gl_FragCoord.y) % 12) <= 6) {
        discard;
    }
        
}


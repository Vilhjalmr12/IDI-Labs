#version 330 core

in vec3 vertex;

void main()  {
    //Modifica el tamaño multiplicando cada vertex por 0.7
    gl_Position = vec4 (vertex*0.7, 1.0);
}

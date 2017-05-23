#include "MyGLWidget.h"
#include <iostream>

//_________________________METODOS DE Qt________________________
//______________________________________________________________
MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent) {
  setFocusPolicy(Qt::ClickFocus);  // per rebre esdeveniments de teclat
  scale = 1.0f;
}

MyGLWidget::~MyGLWidget () {
  if (program != NULL)
    delete program;
}

//Punto de inicio
void MyGLWidget::initializeGL () {
    // Cal inicialitzar l'ús de les funcions d'OpenGL
    initializeOpenGLFunctions();
    glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
    glEnable(GL_DEPTH_TEST);
    carregaShaders(); //Carga los shaders de disco e importa algunos de sus atributos
    createBuffers(); //Carga los modelos externos de disco en variables Model
    iniCamera(); //Inicializa la posicion y orientacion del observador (camara)
}

//Qt llama a este metodo cada que hay que refrescar la ventana
void MyGLWidget::paintGL () {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Borramos el frame-buffer
    //LLamamos a los metodos que pintan todos aquellos elementos que queremos ver en pantalla
    //pintaHomer();
    pintaTerra();
    pintaPatricio1();
    pintaPatricio2();
    glBindVertexArray (0);
}

//Qt lo llama cada vez que redimensionamos la ventana
void MyGLWidget::resizeGL (int w, int h) {
    //Modificamos el ra y el FOV del modelo para que no se deforme al redimensionar la ventana
    //Version para proyeccion perspectiva
    glViewport(0, 0, w, h); //El vp coge los valores de la ventana Qt
    ra = float(w) / float(h); //Modifica ra del window segun ra del vp
    if (ra < 1.0) {FOV = 2.0*atan(tan(FOVini/2.0)/ra);} //Recalculamos FOV cuando vp es mas alto que ancho
    projectTransform(); //Volvemos a calcular la perspectiveMatrix

    //Version con la esfera para evitar deformaciones (para proyeccion ortogonal)
    /*raV = float(w)/float(h);
    ra = raV;
    if(raV > 1.0) {
        left = -radiScnTotal * raV;
        right = radiScnTotal * raV;
        bottom = - radiScnTotal;
        top = radiScnTotal;
        projectTransform();
    }
    if(raV < 1.0){
        FOV = 2.0*atan(tan(FOV)/raV);
        left = -radiScnTotal;
        right = radiScnTotal;
        top = radiScnTotal/raV;
        bottom = -radiScnTotal/raV;
        projectTransform();
    }*/
}

//____________________CREACION DE LOS BUFFERS___________________
//______________________________________________________________
void MyGLWidget::createBuffers () {
    //Insertamos en sus respectivos VAOs cada uno de los elementos que queremos pintar, tanto los elementos declarados aqui como los modelos cargados de disco
    /*homer.load("models/HomerProves.obj");
    calculaEsferaContenidoraHomerProves();
    carregarModelHomer();*/
    carregaTerra();
    patricio.load("models/Patricio.obj"); //Cargamos un modelo de disco
    calculaEsferaContenidoraPatricio();
    //Estas tres variables sirven para calcular los movimientos del raton, les damos un valor inicial por defecto para que no sean indefinidos
    oldXRot = PminPa.x;
    oldYRot = PminPa.y;
    oldYZoom = PminPa.y;
    carregarModelPatricio();
    calculaEsferaTotal(); //Esfera que contiene todo el modelo entero
    glBindVertexArray(0);
}

//_______________________CARGAR LOS SHADERS_____________________
//______________________________________________________________
void MyGLWidget::carregaShaders() {
    // Creem els shaders per al fragment shader i el vertex shader
    QOpenGLShader fs (QOpenGLShader::Fragment, this);
    QOpenGLShader vs (QOpenGLShader::Vertex, this);
    // Carreguem el codi dels fitxers i els compilem
    fs.compileSourceFile("shaders/fragshad.frag");
    vs.compileSourceFile("shaders/vertshad.vert");
    // Creem el program
    program = new QOpenGLShaderProgram(this);
    // Li afegim els shaders corresponents
    program->addShader(&fs);
    program->addShader(&vs);
    // Linkem el program
    program->link();
    // Indiquem que aquest és el program que volem usar
    program->bind();

    //Variables que cogemos del shader para darles valor
    // Obtenim identificador per a l'atribut “vertex” del vertex shader
    vertexLoc = glGetAttribLocation (program->programId(), "vertex");
    // Obtenim identificador per a l'atribut “color” del vertex shader
    colorLoc = glGetAttribLocation (program->programId(), "color");
    // Uniform locations
    transLoc = glGetUniformLocation(program->programId(), "TG");
    //Demanem un uniform location per al uniform de la matriu
    projLoc = glGetUniformLocation(program->programId(), "proj");
    viewLoc = glGetUniformLocation(program->programId(), "view");
}

//______________MATRICES QUE MODIFICAN EL MODELO________________
//______________________________________________________________
//Inicializamos todas las variables de camara que usan la view y la projection
void MyGLWidget::iniCamera() {
    float dist = radiScnTotal*2.0; //distancia Observador -> esfera que contiene la escena
    //FOV = (float)M_PI/2.0f;
    FOV = 2 * asin(radiScnTotal/dist); //Angulo de obertura (vertical) de la camara
    FOVini = FOV; //Valor inicial del FOV para poder redimensionar la ventana correctamente
    //VRP = glm::vec3(0,0,0);
    VRP = PcentreTotal; //Punto al que apunta la camara
    OBS = VRP + dist*glm::vec3(0.0, 0.0, 1.0); //Posicion de la camara
    UP = glm::vec3(0,1,0);
    ra = 1.0F;
    zNear = radiScnTotal;
    zFar = dist + radiScnTotal;
    modelTransformTerra();
    projectTransform();
    viewTransform();
}

void MyGLWidget::projectTransform() {
    // glm::perspective (FOV en radians, ra window, znear, zfar)

    //Vista perspectiva
    glm::mat4 Proj = glm::perspective(FOV, ra, zNear, zFar);
    //Vista Ortogonal
    //Sirven las dos versiones de debajo
    //Deberia servir tambien pero no -> glm::mat4 Proj = glm::ortho(-radiScnTotal, radiScnTotal, -radiScnTotal, radiScnTotal, zNear, zFar);
    //glm::mat4 Proj = glm::ortho(left, right,bottom, top, zNear, zFar);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform() {
    /*glm::perspective (OBS, VRP, UP)*/

    //Perspectiva normal
    /*glm::mat4 View = glm::lookAt(OBS, VRP, UP);*/

    //Perspectiva con angulos de euler
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -(radiScnTotal*2.0)));
    view = glm::translate(view, -VRP);
    view= glm::rotate(view, rotaX, glm::vec3(0,1,0));
    view = glm::rotate(view, rotaY, glm::vec3(1,0,0));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
}

void MyGLWidget::keyPressEvent(QKeyEvent* event)
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_S: { // escalar a més gran
        scale += 0.05;
        break;
    } case Qt::Key_D: { // escalar a més petit
        scale -= 0.05;
        break;
    } case Qt::Key_R: { // Rotar
        rotacio -= M_PI/24.0; //Radians
        break;
    } case Qt::Key_Z: { // zoom in
        FOV -= (float)M_PI/180;
        projectTransform();
        break;
    } case Qt::Key_X: { // zoom out
        FOV += (float)M_PI/180;
        projectTransform();
        break;
    } default: event->ignore(); break;
  }
    update();
}

void MyGLWidget::mousePressEvent (QMouseEvent *e) {
  oldXRot = e->x();
  oldYRot = e->y();
  oldYZoom = e->y();
  //Mira si se pulsa el boton izquierda del raton y ningun modificador de teclado
  if (e->buttons() == Qt::LeftButton &&
      !(e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier))) {
    isDoing = ROTATE;
  }
  //Mira si se pulsa el boton derecho del raton
  if (e->buttons() & Qt::RightButton &&
      !(e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier))) {
    isDoing = ZOOM;
  }
}

void MyGLWidget::mouseReleaseEvent(QMouseEvent *){
    isDoing = NONE;
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e) {
    makeCurrent();
    if (isDoing == ROTATE) {
        rotaX += (e->x() - oldXRot) * M_PI / 180.0;
        rotaY += (e->y() - oldYRot) * M_PI / 180.0;
        viewTransform();
        oldXRot = e->x();
        oldYRot = e->y();
    } else if (isDoing == ZOOM) {
        if (e->y() > oldYZoom) {
            FOV += (float)M_PI/180;
        } else if (e->y() < oldYZoom){
            FOV -= (float)M_PI/180;
        }
        oldYZoom = e->y();
        projectTransform();
    }
    update();
}

//____________________________HOMER__________________________
void MyGLWidget::carregarModelHomer() {
    glGenVertexArrays(1, &VAOhomer);
    glBindVertexArray(VAOhomer);
    glGenBuffers(1, &VBOhomerVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOhomerVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*homer.faces().size()*3*3, homer.VBO_vertices(), GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);
    glGenBuffers(1, &VBOhomerMaterial);
    glBindBuffer(GL_ARRAY_BUFFER, VBOhomerMaterial);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*homer.faces().size()*3*3, homer.VBO_matdiff(), GL_STATIC_DRAW);

    // Activem l'atribut colorLoc
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(colorLoc);
}

void MyGLWidget::pintaHomer(){
    glBindVertexArray (VAOhomer);
    glDrawArrays(GL_TRIANGLES, 0, homer.faces().size()*3);
}

void MyGLWidget::calculaEsferaContenidoraHomerProves() {
    //En este caso se puede harcodear porque sabemos el tamaño del suelo
    //y el enunciado nos dice que la altura es 2
    PminHP = glm::vec3(-1.0, -1.0, -1.0);
    PmaxHP = glm::vec3(1.0, 1.0, 1.0);
    PcentreHP = glm::vec3((PmaxHP.x + PminHP.x)/2, (PmaxHP.y + PminHP.y)/2, (PmaxHP.z + PminHP.z)/2);
    //Para saber el radio se calsula la distancia del centro a Pmin o Pmax
    //Distancia dos puntos: d = sqrt((x2-x1)² + (y2-y1)²)
    //En este caso hacemos la distancia del centro a Pmax.
    float coordx = pow(PmaxHP.x - PcentreHP.x, 2);
    float coordy = pow(PmaxHP.y - PcentreHP.y, 2);
    float coordz = pow(PmaxHP.z - PcentreHP.z, 2);
    radiScnHP = sqrt(coordx + coordy + coordz);
}

//____________________________TERRA__________________________
void MyGLWidget::carregaTerra(){
    glm::vec3 Vertexs[4];
    Vertexs[0] = glm::vec3(-2.0, 0.0, -2.0);
    Vertexs[1] = glm::vec3(-2.0, 0.0, 2.0);
    Vertexs[2] = glm::vec3(2.0, 0.0, -2.0);
    Vertexs[3] = glm::vec3(2.0, 0.0, 2.0);

    glGenVertexArrays(1, &VAOterra);
    glBindVertexArray(VAOterra);

    glGenBuffers(1, &VBOterraVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOterraVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertexs), Vertexs, GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    glm::vec3 Colors[4];
    Colors[0] = glm::vec3(1, 0.81, 0);
    Colors[1] = glm::vec3(0, 0.4, 0.66);
    Colors[2] = glm::vec3(0, 0.4, 0.66);
    Colors[3] = glm::vec3(1, 0.81, 0);

    glGenBuffers(1, &VBOterraMaterial);
    glBindBuffer(GL_ARRAY_BUFFER, VBOterraMaterial);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);

    // Activem l'atribut colorLoc
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(colorLoc);
}

void MyGLWidget::pintaTerra(){
    glBindVertexArray (VAOterra);
    modelTransformTerra();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void MyGLWidget::modelTransformTerra () {
    // Matriu de transformació de model
    glm::mat4 transform (1.0f);
    transform = glm::scale(transform, glm::vec3(scale));
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

void MyGLWidget::calculaEsferaTotal() {
    PminTotal.x = -2.0;
    PminTotal.y = 0.0;
    PminTotal.z = -2.0;
    PmaxTotal.x = 2.0;
    PmaxTotal.y = 1.0;
    PmaxTotal.z = 2.0;
    PcentreTotal = glm::vec3((PmaxTotal.x + PminTotal.x)/2, (PmaxTotal.y + PminTotal.y)/2, (PmaxTotal.z + PminTotal.z)/2);
    float coordx = pow(PmaxTotal.x - PcentreTotal.x, 2);
    float coordy = pow(PmaxTotal.y - PcentreTotal.y, 2);
    float coordz = pow(PmaxTotal.z - PcentreTotal.z, 2);
    radiScnTotal = sqrt(coordx + coordy + coordz);
}

//____________________________PATRICIO1__________________________
void MyGLWidget::carregarModelPatricio() {
    glGenVertexArrays(1, &VAOpatricio);
    glBindVertexArray(VAOpatricio);
    glGenBuffers(1, &VBOpatricioVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOpatricioVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patricio.faces().size()*3*3, patricio.VBO_vertices(), GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);
    glGenBuffers(1, &VBOpatricioMaterial);
    glBindBuffer(GL_ARRAY_BUFFER, VBOpatricioMaterial);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patricio.faces().size()*3*3, patricio.VBO_matdiff(), GL_STATIC_DRAW);

    // Activem l'atribut colorLoc
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(colorLoc);
}

void MyGLWidget::pintaPatricio1(){
    glBindVertexArray (VAOpatricio);
    modelTransformPatricio1();
    glDrawArrays(GL_TRIANGLES, 0, patricio.faces().size()*3);
}

void MyGLWidget::modelTransformPatricio1() {
    // Matriu de transformació de model
    //Fem que només s'apliqui al model carregat i no al terra
    glm::mat4 transform (1.0f);
    glm::vec3 position(1, 0.5, 1); //Centre de la base a (1, 0, 1)
    transform = glm::translate(transform, position);
    float escala = 1.0/(PmaxPa.y - PminPa.y); //Alçada 1
    transform = glm::scale(transform, glm::vec3(escala, escala, escala));
    transform = glm::rotate(transform, rotacio, glm::vec3(0,1,0)); //Rotacio dinamica del model
    transform = glm::translate(transform, -PcentrePa);
    //Traslladat fins a l'origen de coordenades
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

void MyGLWidget::calculaEsferaContenidoraPatricio() {
    //En este caso se puede harcodear porque sabemos el tamaño del suelo
    //y el enunciado nos dice que la altura es 2
    //Igualamos los vectores a los primeros valores para poder hacer las comparaciones
    PminPa.x = patricio.vertices()[0];
    PminPa.y = patricio.vertices()[1];
    PminPa.z = patricio.vertices()[2];
    PmaxPa.x = patricio.vertices()[0];
    PmaxPa.y = patricio.vertices()[1];
    PmaxPa.z = patricio.vertices()[2];
    //Iteramos sobre el modelo y no sobre el VAO o el VBO porque en el
    //modelo cada vertice aparece siempre un unica vez.
    int numCoords = patricio.vertices().size();
    for (int i=0; i<numCoords; i+=3) {
        float auxX = patricio.vertices()[i];
        float auxY = patricio.vertices()[i+1];
        float auxZ = patricio.vertices()[i+2];
        PminPa.x = std::min(PminPa.x, auxX);
        PmaxPa.x = std::max(PmaxPa.x, auxX);
        PminPa.y = std::min(PminPa.y, auxY);
        PmaxPa.y = std::max(PmaxPa.y, auxY);
        PminPa.z = std::min(PminPa.z, auxZ);
        PmaxPa.z = std::max(PmaxPa.z, auxZ);
    }
    PcentrePa = glm::vec3((PmaxPa.x + PminPa.x)/2, (PmaxPa.y + PminPa.y)/2, (PmaxPa.z + PminPa.z)/2);
    //Para saber el radio se calsula la distancia del centro a Pmin o Pmax
    //Distancia dos puntos: d = sqrt((x2-x1)² + (y2-y1)²)
    //En este caso hacemos la distancia del centro a Pmax.
    float coordx = pow(PmaxPa.x - PcentrePa.x, 2);
    float coordy = pow(PmaxPa.y - PcentrePa.y, 2);
    float coordz = pow(PmaxPa.z - PcentrePa.z, 2);
    radiScnPa = sqrt(coordx + coordy + coordz);
}

//____________________________PATRICIO2__________________________
void MyGLWidget::pintaPatricio2(){
    glBindVertexArray (VAOpatricio);
    modelTransformPatricio2();
    glDrawArrays(GL_TRIANGLES, 0, patricio.faces().size()*3);
}

void MyGLWidget::modelTransformPatricio2() {
    // Matriu de transformació de model
    //Fem que només s'apliqui al model carregat i no al terra
    glm::mat4 transform(1.0f);
    glm::vec3 position(-1, 0.5, -1); //Centre de la base a (-1, 0, -1)
    transform = glm::translate(transform, position);
    float escala = 1.0/(PmaxPa.y - PminPa.y); //Alçada 1
    transform = glm::scale(transform, glm::vec3(escala, escala, escala));
    transform = glm::rotate(transform, rotacio, glm::vec3(0,1,0)); //Rotacio dinamica del model
    transform = glm::rotate(transform, (float)M_PI, glm::vec3(0,1,0)); //Rotacio fixa del model
    transform = glm::translate(transform, -PcentrePa);
    //Traslladat fins a l'origen de coordenades
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}
//_______________________________________________________________
//_______________________________________________________________

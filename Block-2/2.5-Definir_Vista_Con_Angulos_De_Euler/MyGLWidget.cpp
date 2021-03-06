#include "MyGLWidget.h"
#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
  scale = 1.0f;
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)

//________________________ADDED CODE____________________________
//______________________________________________________________
    glEnable(GL_DEPTH_TEST);
    carregaShaders();
    createBuffers();
    iniCamera();
//______________________________________________________________
//______________________________________________________________
}

void MyGLWidget::paintGL ()
{
//________________________ADDED CODE____________________________
//______________________________________________________________
    // Esborrem el frame-buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Carreguem la transformació de model
    modelTransform();
    /*pintaHomer();
    modelTransformTerra();
    pintaTerra();*/
    pintaPatricio();
    glBindVertexArray (0);
//______________________________________________________________
//______________________________________________________________
}

void MyGLWidget::modelTransform ()
{
//________________________ADDED CODE____________________________
//______________________________________________________________
    // Matriu de transformació de model
    //Fem que només s'apliqui al model carregat i no al terra
    glm::mat4 transform (1.0f);
    transform = glm::scale(transform, glm::vec3(scale));
    transform = glm::rotate(transform, rotacio, glm::vec3(0,1,0));
    transform = glm::translate(transform, -PcentrePa);
    //Traslladat fins a l'origen de coordenades
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
//_____________________________________________________________
//______________________________________________________________
}

void MyGLWidget::resizeGL (int w, int h)
{
//________________________ADDED CODE____________________________
//______________________________________________________________
    glViewport(0, 0, w, h); //El vp coge los valores de la ventana Qt
    ra = float(w) / float(h); //Modifica ra del window segun ra del vp
    if (ra < 1.0) {FOV = 2.0*atan(tan((float)M_PI/4)/ra);}
    //Recalculamos FOV cuando vp es mas alto que ancho
    projectTransform(); //Volvemos a calcular la perspectiveMatrix
//______________________________________________________________
//______________________________________________________________
}

void MyGLWidget::keyPressEvent(QKeyEvent* event)
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_S: { // escalar a més gran
      scale += 0.05;
      break;
    }
    case Qt::Key_D: { // escalar a més petit
      scale -= 0.05;
      break;
    }
    case Qt::Key_R: { // escalar a més petit
        rotacio -= M_PI/24.0; //Radians
        break;
    }
    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::createBuffers ()
{
//________________________ADDED CODE____________________________
//______________________________________________________________
    //Canviem els VBOs de la caseta per l'VBO carregat amb el model HomerProves
    /*homer.load("models/HomerProves.obj");
    calculaEsferaContenidoraHomerProves();
    carregarModelHomer();
    carregaTerra();*/
    //Canviem els VBOs de la caseta per l'VBO carregat amb el model Patricio
    patricio.load("models/Patricio.obj");
    calculaEsferaContenidoraPatricio();
    carregarModelPatricio();
    glBindVertexArray(0);
//______________________________________________________________
//______________________________________________________________
}

void MyGLWidget::carregaShaders()
{
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

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  // Obtenim identificador per a l'atribut “color” del vertex shader
  colorLoc = glGetAttribLocation (program->programId(), "color");
  // Uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");


//__________________________ADDED CODE____________________________
//________________________________________________________________

    //Demanem un uniform location per al uniform de la matriu
    projLoc = glGetUniformLocation(program->programId(), "proj");
    viewLoc = glGetUniformLocation(program->programId(), "view");
}

void MyGLWidget::iniCamera() {
    float dist = radiScnPa*2.0;
    //FOV = (float)M_PI/2.0f;
    FOV = 2 * asin(radiScnPa/dist);
    VRP = glm::vec3(0,0,0);
    OBS = VRP + dist*glm::vec3(0.0, 0.0, 1.0);
    UP = glm::vec3(0,1,0);
    ra = 1.0F;
    zNear = radiScnPa;
    zFar = dist + radiScnPa;
    modelTransform();
    projectTransform();
    viewTransform();
}

void MyGLWidget::projectTransform() {
    // glm::perspective (FOV en radians, ra window, znear, zfar)
    glm::mat4 Proj = glm::perspective(FOV, ra, zNear, zFar);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform() {
    /*glm::perspective (OBS, VRP, UP)*/
    //Perspectiva normal
    /*glm::mat4 View = glm::lookAt(OBS, VRP, UP);*/
    //Perspectiva con angulos de euler
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -(radiScnPa*2.0)));
    /*view= glm::rotate(view,rotaX,glm::vec3(0,1,0));
    view = glm::rotate(view,rotaY,glm::vec3(1,0,0));*/
    view = glm::translate(view, -VRP);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
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
    Vertexs[0] = glm::vec3(-1.0, -1.0, -1.0);
    Vertexs[1] = glm::vec3(-1.0, -1.0, 1.0);
    Vertexs[2] = glm::vec3(1.0, -1.0, -1.0);
    Vertexs[3] = glm::vec3(1.0, -1.0, 1.0);


    glGenVertexArrays(1, &VAOterra);
    glBindVertexArray(VAOterra);

    glGenBuffers(1, &VBOterraVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOterraVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertexs), Vertexs, GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    glm::vec3 Colors[4];
    Colors[0] = glm::vec3(1, 0, 0);
    Colors[1] = glm::vec3(0, 1, 0);
    Colors[2] = glm::vec3(0, 0, 1);
    Colors[3] = glm::vec3(1, 0, 1);

    glGenBuffers(1, &VBOterraMaterial);
    glBindBuffer(GL_ARRAY_BUFFER, VBOterraMaterial);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);

    // Activem l'atribut colorLoc
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(colorLoc);
}

void MyGLWidget::pintaTerra(){
    glBindVertexArray (VAOterra);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void MyGLWidget::modelTransformTerra ()
{
    // Matriu de transformació de model
    glm::mat4 transform (1.0f);
    transform = glm::scale(transform, glm::vec3(scale));
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

//____________________________PATRICIO__________________________
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

void MyGLWidget::pintaPatricio(){
    glBindVertexArray (VAOpatricio);
    glDrawArrays(GL_TRIANGLES, 0, patricio.faces().size()*3);
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
//_______________________________________________________________
//_______________________________________________________________

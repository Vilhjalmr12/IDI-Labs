#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
  xClick = yClick = 0;
  angleY = 0.0;
  angleRot = 0;
  perspectiva = true;
  isDoing = NONE;
  radiEsc = sqrt(12);
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL () {
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  

  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  glEnable(GL_DEPTH_TEST);
  carregaShaders();
  createBuffers();
  projectTransform ();
  viewTransform ();
  iniCamera();
}

void MyGLWidget::paintGL () 
{
  // Esborrem el frame-buffer i el depth-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniform1i(isVacaLoc, false);
  // Activem el VAO per a pintar el terra 
  glBindVertexArray (VAO_Terra);
  modelTransformTerra ();
  // pintem
  glDrawArrays(GL_TRIANGLES, 0, 12);

  // Activem el VAO per a pintar el Patricio
  glBindVertexArray (VAO_Patr);
  modelTransformPatricio ();
  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, patr.faces().size()*3);

  /*glBindVertexArray(VAO_Patr);
  modelTransformPatricio2();
  glDrawArrays(GL_TRIANGLES, 0, patr.faces().size()*3);*/

  glUniform1i(isVacaLoc, true); //Le dice al shader si pintamos la vaca o no
  glBindVertexArray (VAO_Vaca);
  modelTransformVaca();
  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, vaca.faces().size()*3);
  
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h) 
{
    //Modificamos el ra y el FOV del modelo para que no se deforme al redimensionar la ventana
    //Version para proyeccion perspectiva
    glViewport(0, 0, w, h); //El vp coge los valores de la ventana Qt
    ra = float(w) / float(h); //Modifica ra del window segun ra del vp
    if (ra < 1.0) {FOV = 2.0*atan(tan(FOVini/2.0)/ra);} //Recalculamos FOV cuando vp es mas alto que ancho
    projectTransform(); //Volvemos a calcular la perspectiveMatrix
}

void MyGLWidget::createBuffers ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  patr.load("./models/Patricio.obj");
  calculaCapsaModel ();
  vaca.load("./models/cow.obj");
  calculaCapsaModelVaca ();

  // Calculem la capsa contenidora del model
  calculaCapsaModel ();
  oldXRot = 0;
  oldYRot = 0;
  oldYZoom = 0;
  
  // Creació del Vertex Array Object del Patricio
  glGenVertexArrays(1, &VAO_Patr);
  glBindVertexArray(VAO_Patr);
  // Creació dels buffers del model patr
  // Buffer de posicions
  glGenBuffers(1, &VBO_PatrPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_vertices(), GL_STATIC_DRAW);
  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);
  // Buffer de normals
  glGenBuffers(1, &VBO_PatrNorm);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrNorm);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_normals(), GL_STATIC_DRAW);
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);
  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glGenBuffers(1, &VBO_PatrMatamb);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatamb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matamb(), GL_STATIC_DRAW);
  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);
  // Buffer de component difusa
  glGenBuffers(1, &VBO_PatrMatdiff);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatdiff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matdiff(), GL_STATIC_DRAW);
  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);
  // Buffer de component especular
  glGenBuffers(1, &VBO_PatrMatspec);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatspec);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matspec(), GL_STATIC_DRAW);
  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);
  // Buffer de component shininness
  glGenBuffers(1, &VBO_PatrMatshin);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatshin);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3, patr.VBO_matshin(), GL_STATIC_DRAW);
  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  // Creació del Vertex Array Object de la vaca
  glGenVertexArrays(1, &VAO_Vaca);
  glBindVertexArray(VAO_Vaca);
  // Creació dels buffers del model patr
  // Buffer de posicions
  glGenBuffers(1, &VBO_VacaPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_vertices(), GL_STATIC_DRAW);
  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);
  // Buffer de normals
  glGenBuffers(1, &VBO_VacaNorm);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaNorm);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_normals(), GL_STATIC_DRAW);
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);
  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glGenBuffers(1, &VBO_VacaMatamb);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaMatamb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matamb(), GL_STATIC_DRAW);
  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);
  // Buffer de component difusa
  glGenBuffers(1, &VBO_VacaMatdiff);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaMatdiff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matdiff(), GL_STATIC_DRAW);
  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);
  // Buffer de component especular
  glGenBuffers(1, &VBO_VacaMatspec);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaMatspec);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matspec(), GL_STATIC_DRAW);
  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);
  // Buffer de component shininness
  glGenBuffers(1, &VBO_VacaMatshin);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_VacaMatshin);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3, vaca.VBO_matshin(), GL_STATIC_DRAW);
  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  // Dades del terra
  // VBO amb la posició dels vèrtexs
  glm::vec3 posterra[12] = {
	glm::vec3(-2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(-2.0, 1.0, -2.0),
	glm::vec3(-2.0, 1.0, -2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(2.0, 1.0, -2.0)
  }; 

  // VBO amb la normal de cada vèrtex
  glm::vec3 norm1 (0,1,0);
  glm::vec3 norm2 (0,0,1);
  glm::vec3 normterra[12] = {
	norm1, norm1, norm1, norm1, norm1, norm1, // la normal (0,1,0) per als primers dos triangles
	norm2, norm2, norm2, norm2, norm2, norm2  // la normal (0,0,1) per als dos últims triangles
  };

  // Definim el material del terra
  glm::vec3 amb(0.2,0,0.2);
  glm::vec3 diff(0.8,0,0.8);
  glm::vec3 spec(0,0,0);
  float shin = 100;

  // Fem que aquest material afecti a tots els vèrtexs per igual
  glm::vec3 matambterra[12] = {
	amb, amb, amb, amb, amb, amb, amb, amb, amb, amb, amb, amb
  };
  glm::vec3 matdiffterra[12] = {
	diff, diff, diff, diff, diff, diff, diff, diff, diff, diff, diff, diff
  };
  glm::vec3 matspecterra[12] = {
	spec, spec, spec, spec, spec, spec, spec, spec, spec, spec, spec, spec
  };
  float matshinterra[12] = {
	shin, shin, shin, shin, shin, shin, shin, shin, shin, shin, shin, shin
  };

// Creació del Vertex Array Object del terra
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  glGenBuffers(1, &VBO_TerraPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posterra), posterra, GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBO_TerraNorm);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraNorm);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normterra), normterra, GL_STATIC_DRAW);

  // Activem l'atribut normalLoc
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glGenBuffers(1, &VBO_TerraMatamb);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatamb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matambterra), matambterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glGenBuffers(1, &VBO_TerraMatdiff);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatdiff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matdiffterra), matdiffterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glGenBuffers(1, &VBO_TerraMatspec);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatspec);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matspecterra), matspecterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glGenBuffers(1, &VBO_TerraMatshin);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatshin);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matshinterra), matshinterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
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
  // Obtenim identificador per a l'atribut “normal” del vertex shader
  normalLoc = glGetAttribLocation (program->programId(), "normal");
  // Obtenim identificador per a l'atribut “matamb” del vertex shader
  matambLoc = glGetAttribLocation (program->programId(), "matamb");
  // Obtenim identificador per a l'atribut “matdiff” del vertex shader
  matdiffLoc = glGetAttribLocation (program->programId(), "matdiff");
  // Obtenim identificador per a l'atribut “matspec” del vertex shader
  matspecLoc = glGetAttribLocation (program->programId(), "matspec");
  // Obtenim identificador per a l'atribut “matshin” del vertex shader
  matshinLoc = glGetAttribLocation (program->programId(), "matshin");

  // Demanem identificadors per als uniforms del vertex shader
  transLoc = glGetUniformLocation (program->programId(), "TG");
  projLoc = glGetUniformLocation (program->programId(), "proj");
  viewLoc = glGetUniformLocation (program->programId(), "view");
  isVacaLoc = glGetUniformLocation (program->programId(), "isVaca");
}

void MyGLWidget::modelTransformPatricio ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  TG = glm::rotate(TG, angleRot, glm::vec3(0, 1, 0)); //Rotacio dinamica teclat
  TG = glm::translate(TG, glm::vec3(1, -0.10, 0));
  TG = glm::scale(TG, glm::vec3(escala, escala, escala));
  TG = glm::rotate(TG, (float)M_PI/2, glm::vec3(0, 1, 0)); //Rotacio fixa del model
  TG = glm::translate(TG, -centrePatr);
  
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformVaca()
{
    glm::mat4 TG(1.f);  // Matriu de transformació
    TG = glm::rotate(TG, angleRot, glm::vec3(0, 1, 0)); //Rotacio dinamica teclat
    TG = glm::translate(TG, glm::vec3(1, -0.53, 0));
    TG = glm::scale(TG, glm::vec3(escalaVaca, escalaVaca, escalaVaca));
    TG = glm::rotate(TG, -(float)M_PI/2, glm::vec3(1,0,0)); //Rotacio fixa del model
    TG = glm::translate(TG, -centreVaca);

    glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformPatricio2() {
  glm::mat4 TG(1.f);  // Matriu de transformació
  TG = glm::translate(TG, glm::vec3(0, alturaPat, 0));
  TG = glm::scale(TG, glm::vec3(escala, escala, escala));
  TG= glm::rotate(TG, (float)M_PI, glm::vec3(0,0,1)); //Rotacio fixa del model
  TG = glm::translate(TG, -centrePatr);

  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformTerra ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::projectTransform ()
{
  glm::mat4 Proj;  // Matriu de projecció
  if (perspectiva)
    Proj = glm::perspective(FOV, ra, zNear, zFar);
  else
    Proj = glm::ortho(-radiEsc, radiEsc, -radiEsc, radiEsc, radiEsc, 3.0f*radiEsc);

  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform () {
    glm::mat4 View = glm::lookAt(OBS, VRP, UP);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &View[0][0]);

    /*glm::mat4 View;  // Matriu de posició i orientació
    View = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2*radiEsc));
    View = glm::rotate(View, rotaY, glm::vec3(1, 0, 0));
    View = glm::rotate(View, rotaX, glm::vec3(0, 1, 0));
    View = glm::translate(View, glm::vec3(0, (-alturaPat/2), 0));
    glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);*/


  //Perspectiva con angulos de euler
  /*glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -(radiEsc*2.0)));
  view = glm::translate(view, -VRP);
  view= glm::rotate(view, rotaX, glm::vec3(0,1,0));
  view = glm::rotate(view, rotaY, glm::vec3(1,0,0));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);*/

}

void MyGLWidget::calculaCapsaModel ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = patr.vertices()[0];
  miny = maxy = patr.vertices()[1];
  minz = maxz = patr.vertices()[2];
  for (unsigned int i = 3; i < patr.vertices().size(); i+=3)
  {
    if (patr.vertices()[i+0] < minx)
      minx = patr.vertices()[i+0];
    if (patr.vertices()[i+0] > maxx)
      maxx = patr.vertices()[i+0];
    if (patr.vertices()[i+1] < miny)
      miny = patr.vertices()[i+1];
    if (patr.vertices()[i+1] > maxy)
      maxy = patr.vertices()[i+1];
    if (patr.vertices()[i+2] < minz)
      minz = patr.vertices()[i+2];
    if (patr.vertices()[i+2] > maxz)
      maxz = patr.vertices()[i+2];
  }
  escala = 0.25/(maxy-miny);
  centrePatr[0] = (minx+maxx)/2.0; centrePatr[1] = (miny+maxy)/2.0; centrePatr[2] = (minz+maxz)/2.0;

  float coordx = pow(maxx - centrePatr[0], 2);
  float coordy = pow(maxy - centrePatr[1], 2);
  float coordz = pow(maxz - centrePatr[2], 2);
  radiPat = sqrt(coordx + coordy + coordz);
  alturaPat = (maxy - miny) * escala;
}

void MyGLWidget::calculaCapsaModelVaca ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = vaca.vertices()[0];
  miny = maxy = vaca.vertices()[1];
  minz = maxz = vaca.vertices()[2];
  for (unsigned int i = 3; i < vaca.vertices().size(); i+=3)
  {
    if (vaca.vertices()[i+0] < minx)
      minx = vaca.vertices()[i+0];
    if (vaca.vertices()[i+0] > maxx)
      maxx = vaca.vertices()[i+0];
    if (vaca.vertices()[i+1] < miny)
      miny = vaca.vertices()[i+1];
    if (vaca.vertices()[i+1] > maxy)
      maxy = vaca.vertices()[i+1];
    if (vaca.vertices()[i+2] < minz)
      minz = vaca.vertices()[i+2];
    if (vaca.vertices()[i+2] > maxz)
      maxz = vaca.vertices()[i+2];
  }
  escalaVaca = 0.5/(maxy-miny);
  centreVaca[0] = (minx+maxx)/2.0; centreVaca[1] = (miny+maxy)/2.0; centreVaca[2] = (minz+maxz)/2.0;

  float coordx = pow(maxx - centreVaca[0], 2);
  float coordy = pow(maxy - centreVaca[1], 2);
  float coordz = pow(maxz - centreVaca[2], 2);
  radiVaca = sqrt(coordx + coordy + coordz);
  alturaVaca = (maxy - miny) * escalaVaca;
}

void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_O: { // canvia òptica entre perspectiva i axonomètrica
        perspectiva = !perspectiva;
        projectTransform ();
        break;
    }
    case Qt::Key_R: { // canvia òptica entre perspectiva i axonomètrica
        float aux = angleRot + M_PI/3.0;
        VRP = glm::vec3(sin(aux), VRP[1], cos(aux));
        angleRot -= M_PI/6.0; //Radians

        viewTransform();
        break;
    }
    default: event->ignore(); break;
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
        //Ya que esta ahora la opcion de hacer zoom con el slider quito la opcion del raton
        if (e->y() > oldYZoom) {
            float currentVal = FOV*180/M_PI;
            if (currentVal < 178) {
                FOV += (float)M_PI/180;
            }
        } else if (e->y() < oldYZoom){
            float currentVal = FOV*180/M_PI;
            if (currentVal > 1) {
                FOV -= (float)M_PI/180;
            }
        }
        oldYZoom = e->y();
        emit updateZoom(180.0 - (FOV * (180/M_PI)));
        projectTransform();
    }
    update();
}

void MyGLWidget::iniCamera() {
    float dist = radiEsc*2.0; //distancia Observador -> esfera que contiene la escena
    //FOV = (float)M_PI/2.0f;
    //FOV = 2 * asin(radiEsc/dist); //Angulo de obertura (vertical) de la camara
    FOV = M_PI/3.0;
    FOVini = FOV; //Valor inicial del FOV para poder redimensionar la ventana correctamente
    emit updateZoom(180.0 - (FOV * (180/M_PI))); //Para que el slider del zoom no empiece en 0
    VRP = glm::vec3(1.0, -0.65, 0.0);
    //VRP = glm::vec3(0, 0, 0);
    //VRP = centrePatr; //Punto al que apunta la camara

    //OBS = VRP + dist*glm::vec3(0, 0, 1.0); //Posicion de la camara
    OBS = glm::vec3(-1.0, 1.0, -1.0);
    UP = glm::vec3(0, 1, 0);
    ra = 1.0F;
    zNear = 0.01;
    zFar = 6;
    modelTransformTerra();
    projectTransform();
    viewTransform();
}

//Public Slots
void MyGLWidget::changeZoom(int newZoom) {
  makeCurrent();
  FOV = (float)(180 - newZoom) * M_PI/180;
  projectTransform();
  update();
}



#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//____________________CODIGO PROPIO_______________________________
#include "model.h"
//________________________________________________________________

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core 
{
  Q_OBJECT

  public:
    MyGLWidget (QWidget *parent=0);
    ~MyGLWidget ();

  protected:
    // initializeGL - Aqui incluim les inicialitzacions del contexte grafic.
    virtual void initializeGL ( );
    // paintGL - Mètode cridat cada cop que cal refrescar la finestra.
    // Tot el que es dibuixa es dibuixa aqui.
    virtual void paintGL ( );
    // resizeGL - És cridat quan canvia la mida del widget
    virtual void resizeGL (int width, int height);
    // keyPressEvent - Es cridat quan es prem una tecla
    virtual void keyPressEvent (QKeyEvent *event);

  private:
    void createBuffers ();
    void carregaShaders ();
    void modelTransform ();

    // attribute locations
    GLuint vertexLoc, colorLoc;
    // uniform locations
    GLuint transLoc;
    // VAO i VBO names
    GLuint VAO_Casa, VBO_CasaPos, VBO_CasaCol;
    // Program
    QOpenGLShaderProgram *program;
    // Internal vars
    float scale;
    glm::vec3 pos;
    
    
    //__________________________ADDED CODE__________________________
    //______________________________________________________________
    void projectTransform();
    void viewTransform();
    void iniCamera();
    GLuint projLoc;
    GLuint viewLoc;
    glm::vec3 OBS;
    glm::vec3 VRP;
    glm::vec3 UP;
    float FOV, ra, zNear, zFar;
    float rotacio;
    
    
    void carregarModelHomer();
    void pintaHomer();
    Model homer;
    GLuint VAOhomer, VBOhomerVertex, VBOhomerMaterial;
    
    void pintaTerra();
    void carregaTerra();
    void modelTransformTerra();
    GLuint VAOterra, VBOterraVertex, VBOterraMaterial;
    void calculaEsferaContenidoraHomerProves();
    glm::vec3 PminHP, PmaxHP, PcentreHP;
    float radiScnHP;
    
    void carregarModelPatricio();
    void pintaPatricio();
    void calculaEsferaContenidoraPatricio();
    Model patricio;
    GLuint VAOpatricio, VBOpatricioVertex, VBOpatricioMaterial;
    glm::vec3 PminPa, PmaxPa, PcentrePa;
    float radiScnPa;
    //______________________________________________________________
    //______________________________________________________________
};


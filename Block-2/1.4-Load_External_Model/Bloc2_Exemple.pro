TEMPLATE    = app
QT         += opengl 

INCLUDEPATH +=  /usr/include/glm

FORMS += MyForm.ui

HEADERS += MyForm.h MyGLWidget.h

SOURCES += main.cpp MyForm.cpp \
        MyGLWidget.cpp 

 
 
 

INCLUDEPATH +=  /home2/users/alumnes/1174613/dades/LabIDI/Model

SOURCES +=  /home2/users/alumnes/1174613/dades/LabIDI/Model/model.cpp

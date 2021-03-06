QT += widgets
requires(qtConfig(filedialog))

HEADERS =   \
        ../../../src/fresco/fr_mainwindow.h \
        ../../../src/fresco/fr_mainwindow.h \
        ../../../src/fresco/fr_mdichild.h \
        ../../../src/fresco/fr_colorswatch.h \
        ../../../src/fresco/fr_toolbar.h

SOURCES =   \
        ../../../src/fresco/fr_mainapp.cpp      \
        ../../../src/fresco/fr_mainwindow.cpp   \
        ../../../src/fresco/fr_mdichild.cpp     \
        ../../../src/fresco/fr_colorswatch.cpp  \
        ../../../src/fresco/fr_toolbar.cpp

SOURCES += \
        ../../../src/cove/barn/cv_include.cpp   \
        ../../../src/cove/barn/cv_aid.cpp       \
        ../../../src/cove/barn/cv_cmdexec.cpp   \
        ../../../src/cove/barn/cv_typerep.cpp   \
        ../../../src/cove/barn/cv_trace.cpp     \
        ../../../src/cove/flux/cv_spritz.cpp    \

RESOURCES = ../../../src/fresco/fr_mdi.qrc


INCLUDEPATH = ../../../src/

# install
#target.path = ../
#INSTALLS += target

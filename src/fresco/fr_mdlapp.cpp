// fr_mdlapp.cpp ________________________________________________________________________________________________________ 

#include	"fresco/tenor/fr_include.h"
#include	"fresco/fr_mainwindow.h"

//_____________________________________________________________________________________________________________________________

int main( int argc, char *argv[])
{
	Cv_Logger::DoInit( "log.txt");
	CV_FNTRACE(())

    Q_INIT_RESOURCE(mdi);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("MDI Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt MDI Example");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;
    const QStringList posArgs = parser.positionalArguments();
    for (const QString &fileName : posArgs)
        mainWin.openFile(fileName);
    mainWin.show();
    return app.exec();
}

//_____________________________________________________________________________________________________________________________

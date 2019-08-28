// fr_mainapp.cpp ________________________________________________________________________________________________________

#include	"fresco/tenor/fr_include.h"
#include	"fresco/fr_mainwindow.h"

//_____________________________________________________________________________________________________________________________

static void usage()
{
    qWarning() << "Usage: mainwindow [-SizeHint<color> <width>x<height>] ...";
    exit(1);
}

//_____________________________________________________________________________________________________________________________

enum ParseCommandLineArgumentsResult {
    CommandLineArgumentsOk,
    CommandLineArgumentsError,
    HelpRequested
};

//_____________________________________________________________________________________________________________________________

static ParseCommandLineArgumentsResult
    parseCustomSizeHints(const QStringList &arguments, MainWindow::CustomSizeHintMap *result)
{
    result->clear();
    const int argumentCount = arguments.size();
    for (int i = 1; i < argumentCount; ++i) {
        const QString &arg = arguments.at(i);
        if (arg.startsWith(QLatin1String("-SizeHint"))) {
            const QString name = arg.mid(9);
            if (name.isEmpty())
                return CommandLineArgumentsError;
            if (++i == argumentCount)
                return CommandLineArgumentsError;
            const QString sizeStr = arguments.at(i);
            const int idx = sizeStr.indexOf(QLatin1Char('x'));
            if (idx == -1)
                return CommandLineArgumentsError;
            bool ok;
            const int w = sizeStr.leftRef(idx).toInt(&ok);
            if (!ok)
                return CommandLineArgumentsError;
            const int h = sizeStr.midRef(idx + 1).toInt(&ok);
            if (!ok)
                return CommandLineArgumentsError;
            result->insert(name, QSize(w, h));
        } else if (arg == QLatin1String("-h") || arg == QLatin1String("--help")) {
            return HelpRequested;
        } else {
            return CommandLineArgumentsError;
        }
    }

    return CommandLineArgumentsOk;
}

//_____________________________________________________________________________________________________________________________

int main( int argc, char *argv[])
{
	Cv_Logger::DoInit( "log.txt");
	CV_FNTRACE(())

    Q_INIT_RESOURCE(mdi);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    MainWindow::CustomSizeHintMap customSizeHints;
    switch (parseCustomSizeHints(QCoreApplication::arguments(), &customSizeHints)) {
    case CommandLineArgumentsOk:
        break;
    case CommandLineArgumentsError:
        usage();
        return -1;
    case HelpRequested:
        usage();
        return 0;
    }
    QCoreApplication::setApplicationName("MDI Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt MDI Example");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow      mainWin(customSizeHints);
    mainWin.resize(800, 600);
    const QStringList posArgs = parser.positionalArguments();
    for (const QString &fileName : posArgs)
        mainWin.openFile(fileName);
    mainWin.show();
    return app.exec();
}

//_____________________________________________________________________________________________________________________________

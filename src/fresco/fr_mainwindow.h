// fr_mainwindow.h _____________________________________________________________________________________________________________
#pragma once

#include <QMainWindow>

//_____________________________________________________________________________________________________________________________

class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
QT_END_NAMESPACE

//_____________________________________________________________________________________________________________________________

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<QString, QSize>    CustomSizeHintMap;

public:
    explicit MainWindow(const CustomSizeHintMap &customSizeHints, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);


    bool                    openFile(const QString &fileName);

protected:
    void                    closeEvent(QCloseEvent *event) override;

private slots:
    void                    actionTriggered(QAction *action);
    void                    saveLayout();
    void                    loadLayout();
    void                    setDockOptions();

    void                    createDockWidget();
    void                    destroyDockWidget(QAction *action);
    void                    newFile();
    void                    open();
    void                    save();
    void                    saveAs();
    void                    updateRecentFileActions();
    void                    openRecentFile();

#ifndef QT_NO_CLIPBOARD
    void                    cut();
    void                    copy();
    void                    paste();
#endif

    void                    about();
    void                    updateMenus();
    void                    updateWindowMenu();
    MdiChild                *createMdiChild();
    void                    switchLayoutDirection();

private:
    enum { MaxRecentFiles = 5 };

    void                    createActions();
    void                    createStatusBar();
    void                    readSettings();
    void                    writeSettings();
    bool                    loadFile(const QString &fileName);
    static bool             hasRecentFiles();
    void                    prependToRecentFiles(const QString &fileName);
    void                    setRecentFilesVisible(bool visible);
    MdiChild                *activeMdiChild() const;
    QMdiSubWindow           *findMdiChild(const QString &fileName) const;

    QMdiArea                *mdiArea;

    QMenu                   *windowMenu;
    QAction                 *newAct;
    QAction                 *saveAct;
    QAction                 *saveAsAct;
    QAction                 *recentFileActs[MaxRecentFiles];
    QAction                 *recentFileSeparator;
    QAction                 *recentFileSubMenuAct;

#ifndef QT_NO_CLIPBOARD
    QAction                 *cutAct;
    QAction                 *copyAct;
    QAction                 *pasteAct;
#endif

    QAction                 *closeAct;
    QAction                 *closeAllAct;
    QAction                 *tileAct;
    QAction                 *cascadeAct;
    QAction                 *nextAct;
    QAction                 *previousAct;
    QAction                 *windowMenuSeparatorAct;


    QList<QToolBar*>        toolBars;
    QMenu                   *dockWidgetMenu;
    QMenu                   *mainWindowMenu;
    QList<QDockWidget *>    extraDockWidgets;
    QMenu                   *destroyDockWidgetMenu;

};

//_____________________________________________________________________________________________________________________________


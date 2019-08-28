// fr_mainwindow.h _____________________________________________________________________________________________________________
#pragma once

#include <QMainWindow>

//_____________________________________________________________________________________________________________________________

class MdiChild;
class ToolBar;
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

    //QMdiArea                *mdiArea;

    QMenu                   *windowMenu;
    QAction                 *m_NewAct;
    QAction                 *m_OpenAct;
    QAction                 *m_SaveAct;
    QAction                 *m_SaveAsAct;
    QAction                 *recentFileActs[MaxRecentFiles];
    QAction                 *recentFileSeparator;
    QAction                 *recentFileSubMenuAct;

#ifndef QT_NO_CLIPBOARD
    QAction                 *m_CutAct;
    QAction                 *m_CopyAct;
    QAction                 *m_PasteAct;
#endif

    QAction                 *closeAct;
    QAction                 *closeAllAct;
    QAction                 *tileAct;
    QAction                 *cascadeAct;
    QAction                 *nextAct;
    QAction                 *previousAct;
    QAction                 *windowMenuSeparatorAct;

	void					setupToolBar();
    void					setupMenuBar();
    void					setupDockWidgets(const CustomSizeHintMap &customSizeHints);

    QList<ToolBar*>			m_ToolBars;
    QMenu                   *m_DockWidgetMenu;

    QMenu                   *m_LayoutMenu;
    QMenu                   *m_DockMenu;
    QList<QDockWidget *>    extraDockWidgets;
    QMenu                   *destroyDockWidgetMenu;

};

//_____________________________________________________________________________________________________________________________


// fr_mdlapp.cpp ________________________________________________________________________________________________________ 

#include	"fresco/tenor/fr_include.h" 
#include	"fresco/fr_mainwindow.h"
#include	"fresco/fr_mdichild.h"
#include	"fresco/fr_toolbar.h"

//_____________________________________________________________________________________________________________________________

static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
static inline QString fileKey() { return QStringLiteral("file"); }

//_____________________________________________________________________________________________________________________________

MainWindow::MainWindow(const CustomSizeHintMap &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), mdiArea( new QMdiArea)
{
	CV_FNTRACE(())

    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, &QMdiArea::subWindowActivated,  this, &MainWindow::updateMenus);

    createActions();
    createStatusBar();
    updateMenus();

    readSettings();

    setObjectName("MainWindow");
    setWindowTitle(tr("Fresco"));
    //setUnifiedTitleAndToolBarOnMac(true); 

     setupToolBar();
	setupMenuBar();
     //setupDockWidgets(customSizeHints);

    statusBar()->showMessage(tr("Status Bar"));
}

//_____________________________________________________________________________________________________________________________

void MainWindow::closeEvent(QCloseEvent *event)
{
	CV_FNTRACE(())

    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

//_____________________________________________________________________________________________________________________________

void MainWindow::actionTriggered(QAction *action)
{

}

//_____________________________________________________________________________________________________________________________

void MainWindow::setupToolBar()
{
    ToolBar            *fileToolBar = new ToolBar( QString::fromUtf8("File"), this);
	
    fileToolBar->addAction( m_NewAct);
    fileToolBar->addAction( m_OpenAct);
    fileToolBar->addAction(m_SaveAct);

	m_ToolBars.append( fileToolBar);
	addToolBar( fileToolBar);

	
    ToolBar				*editToolBar = new ToolBar( QString::fromUtf8("Edit"), this); 
    editToolBar->addAction(m_CutAct);
    editToolBar->addAction(m_CopyAct);
    editToolBar->addAction(m_PasteAct);
	m_ToolBars.append( editToolBar);
	addToolBar( editToolBar);

#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    for (int i = 0; i < 3; ++i) {
        ToolBar *tb = new ToolBar(QString::fromLatin1("Tool Bar %1").arg(i + 1), this);
		tb->CreateActions();
        m_ToolBars.append(tb);
        addToolBar(tb);
    }
}

//_____________________________________________________________________________________________________________________________

void MainWindow::setupMenuBar()
{
    m_LayoutMenu = menuBar()->addMenu(tr("&Layout"));

    m_LayoutMenu->addAction(tr("Save layout..."), this, &MainWindow::saveLayout);
    m_LayoutMenu->addAction(tr("Load layout..."), this, &MainWindow::loadLayout);
    m_LayoutMenu->addAction(tr("Switch layout direction"),this, &MainWindow::switchLayoutDirection);

    m_LayoutMenu->addSeparator();  
	m_DockMenu  = menuBar()->addMenu(tr("&Dock"));
    QAction *action = m_DockMenu->addAction(tr("Animated docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AnimatedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = m_DockMenu->addAction(tr("Allow nested docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowNestedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = m_DockMenu->addAction(tr("Allow tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowTabbedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = m_DockMenu->addAction(tr("Force tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & ForceTabbedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = m_DockMenu->addAction(tr("Vertical tabs"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & VerticalTabs);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = m_DockMenu->addAction(tr("Grouped dragging"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & GroupedDragging);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    QMenu *toolBarMenu = menuBar()->addMenu(tr("Tool bars"));
    for (int i = 0; i < m_ToolBars.count(); ++i)
	{
		QMenu	*menu = m_ToolBars.at(i)->toolbarMenu();
		if ( menu)
        toolBarMenu->addMenu( menu);
	}

    m_DockWidgetMenu = menuBar()->addMenu(tr("&Widgets"));

    QMenu *aboutMenu = menuBar()->addMenu(tr("About"));
    QAction *aboutAct = aboutMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = aboutMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

//_____________________________________________________________________________________________________________________________

void MainWindow::saveLayout()
{ 
    QString			fileName = QFileDialog::getSaveFileName(this, tr("Save layout"));
    if (fileName.isEmpty())
        return;
    QFile			file( fileName);
    if ( !file.open( QFile::WriteOnly)) {
        QString			msg = tr("Failed to open %1\n%2").arg( QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning( this, tr( "Error"), msg);
        return;
    }

    QByteArray		geo_data = saveGeometry();
    QByteArray		layout_data = saveState();

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok)
        ok = file.write(geo_data) == geo_data.size();
    if (ok)
        ok = file.write(layout_data) == layout_data.size();

    if (!ok) {
        QString msg = tr( "Error writing to %1\n%2").arg( QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}
//_____________________________________________________________________________________________________________________________

void MainWindow::loadLayout()
{
    QString		fileName = QFileDialog::getOpenFileName(this, tr("Load layout"));
    if (fileName.isEmpty())
        return;
    QFile		file(fileName);
    if (!file.open(QFile::ReadOnly)) 
	{
        QString msg = tr("Failed to open %1\n%2").arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    uchar			geo_size;
    QByteArray		geo_data;
    QByteArray		layout_data;

    bool			ok = file.getChar((char*)&geo_size);
    if (ok) 
	{
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }
    if (ok) 
	{
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok)
        ok = restoreGeometry(geo_data);
    if (ok)
        ok = restoreState(layout_data);

    if (!ok) 
	{
        QString		msg = tr("Error reading %1").arg(QDir::toNativeSeparators(fileName));
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

//_____________________________________________________________________________________________________________________________

void MainWindow::setDockOptions( void)
{ 
    DockOptions opts;
    QList<QAction*> actions = m_DockMenu->actions();

    if (actions.at(0)->isChecked())
        opts |= AnimatedDocks;
    if (actions.at(1)->isChecked())
        opts |= AllowNestedDocks;
    if (actions.at(2)->isChecked())
        opts |= AllowTabbedDocks;
    if (actions.at(3)->isChecked())
        opts |= ForceTabbedDocks;
    if (actions.at(4)->isChecked())
        opts |= VerticalTabs;
    if (actions.at(5)->isChecked())
        opts |= GroupedDragging;

    QMainWindow::setDockOptions(opts);
} 

//_____________________________________________________________________________________________________________________________

void MainWindow::createDockWidget()
{

}

//_____________________________________________________________________________________________________________________________

void MainWindow::destroyDockWidget(QAction *action)
{

}

//_____________________________________________________________________________________________________________________________

void MainWindow::newFile()
{
	CV_FNTRACE(())

    MdiChild *child = createMdiChild();
    child->newFile();
    child->show();
}
//_____________________________________________________________________________________________________________________________

void MainWindow::open()
{
	CV_FNTRACE(())

    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        openFile(fileName);
}

//_____________________________________________________________________________________________________________________________

bool MainWindow::openFile(const QString &fileName)
{
	CV_FNTRACE(())

    if (QMdiSubWindow *existing = findMdiChild(fileName)) {
        mdiArea->setActiveSubWindow(existing);
        return true;
    }
    const bool succeeded = loadFile(fileName);
    if (succeeded)
        statusBar()->showMessage(tr("File loaded"), 2000);
    return succeeded;
}

//_____________________________________________________________________________________________________________________________

bool MainWindow::loadFile(const QString &fileName)
{
	CV_FNTRACE(())

    MdiChild *child = createMdiChild();
    const bool succeeded = child->loadFile(fileName);
    if (succeeded)
        child->show();
    else
        child->close();
    MainWindow::prependToRecentFiles(fileName);
    return succeeded;
}

//_____________________________________________________________________________________________________________________________

static QStringList readRecentFiles(QSettings &settings)
{
	CV_FNTRACE(())

    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}
//_____________________________________________________________________________________________________________________________

static void writeRecentFiles(const QStringList &files, QSettings &settings)
{
	CV_FNTRACE(())

    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}
//_____________________________________________________________________________________________________________________________

bool MainWindow::hasRecentFiles()
{
	CV_FNTRACE(())

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

//_____________________________________________________________________________________________________________________________

void MainWindow::prependToRecentFiles(const QString &fileName)
{
	CV_FNTRACE(())

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);

    setRecentFilesVisible(!recentFiles.isEmpty());
}

//_____________________________________________________________________________________________________________________________

void MainWindow::setRecentFilesVisible(bool visible)
{
	CV_FNTRACE(())

    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}
//_____________________________________________________________________________________________________________________________

void MainWindow::updateRecentFileActions()
{
	CV_FNTRACE(())

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for ( ; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for ( ; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);
}
//_____________________________________________________________________________________________________________________________

void MainWindow::openRecentFile()
{
	CV_FNTRACE(())

    if (const QAction *action = qobject_cast<const QAction *>(sender()))
        openFile(action->data().toString());
}

//_____________________________________________________________________________________________________________________________

void MainWindow::save()
{
	CV_FNTRACE(())

    if (activeMdiChild() && activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

//_____________________________________________________________________________________________________________________________

void MainWindow::saveAs()
{
	CV_FNTRACE(())

    MdiChild *child = activeMdiChild();
    if (child && child->saveAs()) {
        statusBar()->showMessage(tr("File saved"), 2000);
        MainWindow::prependToRecentFiles(child->currentFile());
    }
}

//_____________________________________________________________________________________________________________________________

#ifndef QT_NO_CLIPBOARD
void MainWindow::cut()
{
	CV_FNTRACE(())

    if (activeMdiChild())
        activeMdiChild()->cut();
}

//_____________________________________________________________________________________________________________________________

void MainWindow::copy()
{
	CV_FNTRACE(())

    if (activeMdiChild())
        activeMdiChild()->copy();
}

//_____________________________________________________________________________________________________________________________

void MainWindow::paste()
{
	CV_FNTRACE(())

    if (activeMdiChild())
        activeMdiChild()->paste();
}
#endif

//_____________________________________________________________________________________________________________________________

void MainWindow::about()
{
	CV_FNTRACE(())

   QMessageBox::about(this, tr("About MDI"),
            tr("The <b>MDI</b> example demonstrates how to write multiple "
               "document interface applications using Qt."));
}

//_____________________________________________________________________________________________________________________________

void MainWindow::updateMenus()
{
	CV_FNTRACE(())

    bool hasMdiChild = (activeMdiChild() != nullptr);
    m_SaveAct->setEnabled(hasMdiChild);
    m_SaveAsAct->setEnabled(hasMdiChild);
#ifndef QT_NO_CLIPBOARD
    m_PasteAct->setEnabled(hasMdiChild);
#endif
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    windowMenuSeparatorAct->setVisible(hasMdiChild);

#ifndef QT_NO_CLIPBOARD
    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor().hasSelection());
    m_CutAct->setEnabled(hasSelection);
    m_CopyAct->setEnabled(hasSelection);
#endif
}

//_____________________________________________________________________________________________________________________________

void MainWindow::updateWindowMenu()
{
	CV_FNTRACE(())

    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(windowMenuSeparatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        QMdiSubWindow *mdiSubWindow = windows.at(i);
        MdiChild *child = qobject_cast<MdiChild *>(mdiSubWindow->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action = windowMenu->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
            mdiArea->setActiveSubWindow(mdiSubWindow);
        });
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
    }
}

//_____________________________________________________________________________________________________________________________

MdiChild *MainWindow::createMdiChild()
{
	CV_FNTRACE(())

    MdiChild	*child = new MdiChild;
    mdiArea->addSubWindow(child);

#ifndef QT_NO_CLIPBOARD
    connect(child, &QTextEdit::copyAvailable, m_CutAct, &QAction::setEnabled);
    connect(child, &QTextEdit::copyAvailable, m_CopyAct, &QAction::setEnabled);
#endif

    return child;
}

 

//_____________________________________________________________________________________________________________________________

void MainWindow::createActions()
{
	CV_FNTRACE(())

    QMenu               *fileMenu = menuBar()->addMenu(tr("&File"));

    const QIcon         newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    m_NewAct = new QAction(newIcon, tr("&New"), this);
    m_NewAct->setShortcuts(QKeySequence::New);
    m_NewAct->setStatusTip(tr("Create a new file"));
    connect( m_NewAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction( m_NewAct);

    const QIcon         openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    m_OpenAct = new QAction( openIcon, tr("&Open..."), this);
    m_OpenAct->setShortcuts( QKeySequence::Open);
    m_OpenAct->setStatusTip(tr("Open an existing file"));
    connect( m_OpenAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction( m_OpenAct);

    const QIcon         saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    m_SaveAct = new QAction(saveIcon, tr("&Save"), this);
    m_SaveAct->setShortcuts(QKeySequence::Save);
    m_SaveAct->setStatusTip(tr("Save the document to disk"));
    connect(m_SaveAct, &QAction::triggered, this, &MainWindow::save);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    m_SaveAsAct = new QAction(saveAsIcon, tr("Save &As..."), this);
    m_SaveAsAct->setShortcuts(QKeySequence::SaveAs);
    m_SaveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(m_SaveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(m_SaveAsAct);

    fileMenu->addSeparator();

    QMenu               *recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    fileMenu->addAction(tr("Switch layout direction"), this, &MainWindow::switchLayoutDirection);

    fileMenu->addSeparator();

//! [0]
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);
//! [0]

#ifndef QT_NO_CLIPBOARD
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    m_CutAct = new QAction(cutIcon, tr("Cu&t"), this);
    m_CutAct->setShortcuts(QKeySequence::Cut);
    m_CutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(m_CutAct, &QAction::triggered, this, &MainWindow::cut);
    editMenu->addAction(m_CutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    m_CopyAct = new QAction(copyIcon, tr("&Copy"), this);
    m_CopyAct->setShortcuts(QKeySequence::Copy);
    m_CopyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(m_CopyAct, &QAction::triggered, this, &MainWindow::copy);
    editMenu->addAction(m_CopyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    m_PasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    m_PasteAct->setShortcuts(QKeySequence::Paste);
    m_PasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(m_PasteAct, &QAction::triggered, this, &MainWindow::paste);
    editMenu->addAction(m_PasteAct);
#endif

    windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, &QAction::triggered,
            mdiArea, &QMdiArea::closeActiveSubWindow);

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, &QAction::triggered, mdiArea, &QMdiArea::closeAllSubWindows);

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, &QAction::triggered, mdiArea, &QMdiArea::tileSubWindows);

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, &QAction::triggered, mdiArea, &QMdiArea::cascadeSubWindows);

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, &QAction::triggered, mdiArea, &QMdiArea::activateNextSubWindow);

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, &QAction::triggered, mdiArea, &QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();

    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

//_____________________________________________________________________________________________________________________________

void MainWindow::createStatusBar()
{
	CV_FNTRACE(())

    statusBar()->showMessage(tr("Ready"));
}

//_____________________________________________________________________________________________________________________________

void MainWindow::readSettings()
{
	CV_FNTRACE(())

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

//_____________________________________________________________________________________________________________________________

void MainWindow::writeSettings()
{
	CV_FNTRACE(())

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

MdiChild *MainWindow::activeMdiChild() const
{
	CV_FNTRACE(())

    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return nullptr;
}

//_____________________________________________________________________________________________________________________________

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName) const
{
	CV_FNTRACE(())

    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    const QList<QMdiSubWindow *> subWindows = mdiArea->subWindowList();
    for (QMdiSubWindow *window : subWindows) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return nullptr;
}

//_____________________________________________________________________________________________________________________________

void MainWindow::switchLayoutDirection()
{
	CV_FNTRACE(())

    if (layoutDirection() == Qt::LeftToRight)
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
}

//_____________________________________________________________________________________________________________________________

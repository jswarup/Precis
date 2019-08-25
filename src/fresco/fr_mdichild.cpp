// fr_mdlchild.cpp ________________________________________________________________________________________________________ 

#include	"fresco/tenor/fr_include.h"  
#include	"fresco/fr_mdichild.h"

//_____________________________________________________________________________________________________________________________

MdiChild::MdiChild()
{
	CV_FNTRACE(())

    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
}

//_____________________________________________________________________________________________________________________________

void MdiChild::newFile()
{
	CV_FNTRACE(())

    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("document%1.txt").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");

    connect(document(), &QTextDocument::contentsChanged,
            this, &MdiChild::documentWasModified);
}

//_____________________________________________________________________________________________________________________________

bool MdiChild::loadFile(const QString &fileName)
{
	CV_FNTRACE(())

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QGuiApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    connect(document(), &QTextDocument::contentsChanged,
            this, &MdiChild::documentWasModified);

    return true;
}

//_____________________________________________________________________________________________________________________________

bool MdiChild::save()
{
	CV_FNTRACE(())

    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

//_____________________________________________________________________________________________________________________________

bool MdiChild::saveAs()
{
	CV_FNTRACE(())

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

//_____________________________________________________________________________________________________________________________

bool MdiChild::saveFile(const QString &fileName)
{
	CV_FNTRACE(())

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QGuiApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}

//_____________________________________________________________________________________________________________________________

QString MdiChild::userFriendlyCurrentFile()
{
	CV_FNTRACE(())

    return strippedName(curFile);
}

//_____________________________________________________________________________________________________________________________

void MdiChild::closeEvent(QCloseEvent *event)
{
	CV_FNTRACE(())

    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

//_____________________________________________________________________________________________________________________________

void MdiChild::documentWasModified()
{
	CV_FNTRACE(())

    setWindowModified(document()->isModified());
}

//_____________________________________________________________________________________________________________________________

bool MdiChild::maybeSave()
{
	CV_FNTRACE(())

    if (!document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, tr("MDI"),
                                   tr("'%1' has been modified.\n"
                                      "Do you want to save your changes?")
                                   .arg(userFriendlyCurrentFile()),
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

void MdiChild::setCurrentFile(const QString &fileName)
{
	CV_FNTRACE(())

    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

//_____________________________________________________________________________________________________________________________

QString MdiChild::strippedName(const QString &fullFileName)
{
	CV_FNTRACE(())

    return QFileInfo(fullFileName).fileName();
}

//_____________________________________________________________________________________________________________________________

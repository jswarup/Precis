// fr_toolbar.h _____________________________________________________________________________________________________________
#pragma once

#include    "fresco/tenor/fr_include.h"

//_____________________________________________________________________________________________________________________________

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

//_____________________________________________________________________________________________________________________________

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit	ToolBar(const QString &title, QWidget *parent);

	void			CreateActions( void);

    QMenu			*toolbarMenu() const { return m_Menu; }

private slots:
    void			order();
    void			randomize();
    void			addSpinBox();
    void			removeSpinBox();

    void			changeMovable(bool movable);

    void			allowLeft(bool a);
    void			allowRight(bool a);
    void			allowTop(bool a);
    void			allowBottom(bool a);

    void			placeLeft(bool p);
    void			placeRight(bool p);
    void			placeTop(bool p);
    void			placeBottom(bool p);

    void			updateMenu();
    void			insertToolBarBreak();

private:
    void			allow(Qt::ToolBarArea area, bool allow);
    void			place(Qt::ToolBarArea area, bool place);

    QSpinBox		*spinbox;
    QAction			*spinboxAction;

    QMenu			*m_Menu;
    QAction			*orderAction;
    QAction			*randomizeAction;
    QAction			*addSpinBoxAction;
    QAction			*removeSpinBoxAction;

    QAction			*movableAction;

    QActionGroup	*allowedAreasActions;
    QAction			*allowLeftAction;
    QAction			*allowRightAction;
    QAction			*allowTopAction;
    QAction			*allowBottomAction;

    QActionGroup	*areaActions;
    QAction			*leftAction;
    QAction			*rightAction;
    QAction			*topAction;
    QAction			*bottomAction;
}; 

//_____________________________________________________________________________________________________________________________


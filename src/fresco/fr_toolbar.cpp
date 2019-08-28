// fr_toolbar.cpp ________________________________________________________________________________________________________

#include	"fresco/tenor/fr_include.h"
#include	"fresco/fr_toolbar.h"

//_____________________________________________________________________________________________________________________________

void render_qt_text(QPainter *painter, int w, int h, const QColor &color)
{
    QPainterPath path;
    path.moveTo(-0.083695, 0.283849);
    path.cubicTo(-0.049581, 0.349613, -0.012720, 0.397969, 0.026886, 0.428917);
    path.cubicTo(0.066493, 0.459865, 0.111593, 0.477595, 0.162186, 0.482108);
    path.lineTo(0.162186, 0.500000);
    path.cubicTo(0.115929, 0.498066, 0.066565, 0.487669, 0.014094, 0.468810);
    path.cubicTo(-0.038378, 0.449952, -0.088103, 0.423839, -0.135082, 0.390474);
    path.cubicTo(-0.182061, 0.357108, -0.222608, 0.321567, -0.256722, 0.283849);
    path.cubicTo(-0.304712, 0.262250, -0.342874, 0.239362, -0.371206, 0.215184);
    path.cubicTo(-0.411969, 0.179078, -0.443625, 0.134671, -0.466175, 0.081963);
    path.cubicTo(-0.488725, 0.029255, -0.500000, -0.033043, -0.500000, -0.104932);
    path.cubicTo(-0.500000, -0.218407, -0.467042, -0.312621, -0.401127, -0.387573);
    path.cubicTo(-0.335212, -0.462524, -0.255421, -0.500000, -0.161752, -0.500000);
    path.cubicTo(-0.072998, -0.500000, 0.003903, -0.462444, 0.068951, -0.387331);
    path.cubicTo(0.133998, -0.312218, 0.166522, -0.217440, 0.166522, -0.102998);
    path.cubicTo(0.166522, -0.010155, 0.143394, 0.071325, 0.097138, 0.141441);
    path.cubicTo(0.050882, 0.211557, -0.009396, 0.259026, -0.083695, 0.283849);
    path.moveTo(-0.167823, -0.456963);
    path.cubicTo(-0.228823, -0.456963, -0.277826, -0.432624, -0.314831, -0.383946);
    path.cubicTo(-0.361665, -0.323340, -0.385082, -0.230335, -0.385082, -0.104932);
    path.cubicTo(-0.385082, 0.017569, -0.361376, 0.112025, -0.313964, 0.178433);
    path.cubicTo(-0.277248, 0.229368, -0.228534, 0.254836, -0.167823, 0.254836);
    path.cubicTo(-0.105088, 0.254836, -0.054496, 0.229368, -0.016045, 0.178433);
    path.cubicTo(0.029055, 0.117827, 0.051605, 0.028691, 0.051605, -0.088975);
    path.cubicTo(0.051605, -0.179562, 0.039318, -0.255803, 0.014744, -0.317698);
    path.cubicTo(-0.004337, -0.365409, -0.029705, -0.400548, -0.061362, -0.423114);
    path.cubicTo(-0.093018, -0.445680, -0.128505, -0.456963, -0.167823, -0.456963);
    path.moveTo(0.379011, -0.404739);
    path.lineTo(0.379011, -0.236460);
    path.lineTo(0.486123, -0.236460);
    path.lineTo(0.486123, -0.197292);
    path.lineTo(0.379011, -0.197292);
    path.lineTo(0.379011, 0.134913);
    path.cubicTo(0.379011, 0.168117, 0.383276, 0.190442, 0.391804, 0.201886);
    path.cubicTo(0.400332, 0.213330, 0.411246, 0.219052, 0.424545, 0.219052);
    path.cubicTo(0.435531, 0.219052, 0.446227, 0.215264, 0.456635, 0.207689);
    path.cubicTo(0.467042, 0.200113, 0.474993, 0.188910, 0.480486, 0.174081);
    path.lineTo(0.500000, 0.174081);
    path.cubicTo(0.488436, 0.210509, 0.471957, 0.237911, 0.450564, 0.256286);
    path.cubicTo(0.429170, 0.274662, 0.407054, 0.283849, 0.384215, 0.283849);
    path.cubicTo(0.368893, 0.283849, 0.353859, 0.279094, 0.339115, 0.269584);
    path.cubicTo(0.324371, 0.260074, 0.313530, 0.246534, 0.306592, 0.228965);
    path.cubicTo(0.299653, 0.211396, 0.296184, 0.184075, 0.296184, 0.147002);
    path.lineTo(0.296184, -0.197292);
    path.lineTo(0.223330, -0.197292);
    path.lineTo(0.223330, -0.215667);
    path.cubicTo(0.241833, -0.224049, 0.260697, -0.237992, 0.279922, -0.257495);
    path.cubicTo(0.299147, -0.276999, 0.316276, -0.300129, 0.331310, -0.326886);
    path.cubicTo(0.338826, -0.341070, 0.349523, -0.367021, 0.363400, -0.404739);
    path.lineTo(0.379011, -0.404739);
    path.moveTo(-0.535993, 0.275629);

    painter->translate(w / 2, h / 2);
    double scale = qMin(w, h) * 8 / 10.0;
    painter->scale(scale, scale);

    painter->setRenderHint(QPainter::Antialiasing);

    painter->save();
    painter->translate(.1, .1);
    painter->fillPath(path, QColor(0, 0, 0, 63));
    painter->restore();

    painter->setBrush(color);
    painter->setPen(QPen(Qt::black, 0.02, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
    painter->drawPath(path);
}

//_____________________________________________________________________________________________________________________________

static QPixmap genIcon(const QSize &iconSize, const QString &, const QColor &color, qreal pixelRatio)
{
    int w = qRound(iconSize.width() * pixelRatio);
    int h = qRound(iconSize.height() * pixelRatio);

    QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);

    QPainter p(&image);
	 
    render_qt_text(&p, w, h, color);

    QPixmap pm = QPixmap::fromImage(image, Qt::DiffuseDither | Qt::DiffuseAlphaDither);
    pm.setDevicePixelRatio(pixelRatio);
    return pm;
}

static QPixmap genIcon(const QSize &iconSize, int number, const QColor &color, qreal pixelRatio)
{ return genIcon(iconSize, QString::number(number), color, pixelRatio); }

ToolBar::ToolBar(const QString &title, QWidget *parent)
    : QToolBar(parent)
    , spinbox(nullptr)
    , spinboxAction(nullptr)
{
    setWindowTitle(title);
    setObjectName(title);

    setIconSize(QSize(32, 32));

    qreal dpr = devicePixelRatioF();
    menu = new QMenu("One", this);
    menu->setIcon(genIcon(iconSize(), 1, Qt::black, dpr));
    menu->addAction(genIcon(iconSize(), "A", Qt::blue, dpr), "A");
    menu->addAction(genIcon(iconSize(), "B", Qt::blue, dpr), "B");
    menu->addAction(genIcon(iconSize(), "C", Qt::blue, dpr), "C");
    addAction(menu->menuAction());

    QAction *two = addAction(genIcon(iconSize(), 2, Qt::white, dpr), "Two");
    QFont boldFont;
    boldFont.setBold(true);
    two->setFont(boldFont);

    addAction(genIcon(iconSize(), 3, Qt::red, dpr), "Three");
    addAction(genIcon(iconSize(), 4, Qt::green, dpr), "Four");
    addAction(genIcon(iconSize(), 5, Qt::blue, dpr), "Five");
    addAction(genIcon(iconSize(), 6, Qt::yellow, dpr), "Six");
    orderAction = new QAction(this);
    orderAction->setText(tr("Order Items in Tool Bar"));
    connect(orderAction, &QAction::triggered, this, &ToolBar::order);

    randomizeAction = new QAction(this);
    randomizeAction->setText(tr("Randomize Items in Tool Bar"));
    connect(randomizeAction, &QAction::triggered, this, &ToolBar::randomize);

    addSpinBoxAction = new QAction(this);
    addSpinBoxAction->setText(tr("Add Spin Box"));
    connect(addSpinBoxAction, &QAction::triggered, this, &ToolBar::addSpinBox);

    removeSpinBoxAction = new QAction(this);
    removeSpinBoxAction->setText(tr("Remove Spin Box"));
    removeSpinBoxAction->setEnabled(false);
    connect(removeSpinBoxAction, &QAction::triggered, this, &ToolBar::removeSpinBox);

    movableAction = new QAction(tr("Movable"), this);
    movableAction->setCheckable(true);
    connect(movableAction, &QAction::triggered, this, &ToolBar::changeMovable);

    allowedAreasActions = new QActionGroup(this);
    allowedAreasActions->setExclusive(false);

    allowLeftAction = new QAction(tr("Allow on Left"), this);
    allowLeftAction->setCheckable(true);
    connect(allowLeftAction, &QAction::triggered, this, &ToolBar::allowLeft);

    allowRightAction = new QAction(tr("Allow on Right"), this);
    allowRightAction->setCheckable(true);
    connect(allowRightAction, &QAction::triggered, this, &ToolBar::allowRight);

    allowTopAction = new QAction(tr("Allow on Top"), this);
    allowTopAction->setCheckable(true);
    connect(allowTopAction, &QAction::triggered, this, &ToolBar::allowTop);

    allowBottomAction = new QAction(tr("Allow on Bottom"), this);
    allowBottomAction->setCheckable(true);
    connect(allowBottomAction, &QAction::triggered, this, &ToolBar::allowBottom);

    allowedAreasActions->addAction(allowLeftAction);
    allowedAreasActions->addAction(allowRightAction);
    allowedAreasActions->addAction(allowTopAction);
    allowedAreasActions->addAction(allowBottomAction);

    areaActions = new QActionGroup(this);
    areaActions->setExclusive(true);

    leftAction = new QAction(tr("Place on Left") , this);
    leftAction->setCheckable(true);
    connect(leftAction, &QAction::triggered, this, &ToolBar::placeLeft);

    rightAction = new QAction(tr("Place on Right") , this);
    rightAction->setCheckable(true);
    connect(rightAction, &QAction::triggered, this, &ToolBar::placeRight);

    topAction = new QAction(tr("Place on Top") , this);
    topAction->setCheckable(true);
    connect(topAction, &QAction::triggered, this, &ToolBar::placeTop);

    bottomAction = new QAction(tr("Place on Bottom") , this);
    bottomAction->setCheckable(true);
    connect(bottomAction, &QAction::triggered, this, &ToolBar::placeBottom);

    areaActions->addAction(leftAction);
    areaActions->addAction(rightAction);
    areaActions->addAction(topAction);
    areaActions->addAction(bottomAction);

    connect(movableAction, &QAction::triggered, areaActions, &QActionGroup::setEnabled);

    connect(movableAction, &QAction::triggered, allowedAreasActions, &QActionGroup::setEnabled);

    menu = new QMenu(title, this);
    menu->addAction(toggleViewAction());
    menu->addSeparator();
    menu->addAction(orderAction);
    menu->addAction(randomizeAction);
    menu->addSeparator();
    menu->addAction(addSpinBoxAction);
    menu->addAction(removeSpinBoxAction);
    menu->addSeparator();
    menu->addAction(movableAction);
    menu->addSeparator();
    menu->addActions(allowedAreasActions->actions());
    menu->addSeparator();
    menu->addActions(areaActions->actions());
    menu->addSeparator();
    menu->addAction(tr("Insert break"), this, &ToolBar::insertToolBarBreak);

    connect(menu, &QMenu::aboutToShow, this, &ToolBar::updateMenu);

    randomize();
}

void ToolBar::updateMenu()
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
    Q_ASSERT(mainWindow);

    const Qt::ToolBarArea area = mainWindow->toolBarArea(this);
    const Qt::ToolBarAreas areas = allowedAreas();

    movableAction->setChecked(isMovable());

    allowLeftAction->setChecked(isAreaAllowed(Qt::LeftToolBarArea));
    allowRightAction->setChecked(isAreaAllowed(Qt::RightToolBarArea));
    allowTopAction->setChecked(isAreaAllowed(Qt::TopToolBarArea));
    allowBottomAction->setChecked(isAreaAllowed(Qt::BottomToolBarArea));

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::LeftToolBarArea);
        allowRightAction->setEnabled(area != Qt::RightToolBarArea);
        allowTopAction->setEnabled(area != Qt::TopToolBarArea);
        allowBottomAction->setEnabled(area != Qt::BottomToolBarArea);
    }

    leftAction->setChecked(area == Qt::LeftToolBarArea);
    rightAction->setChecked(area == Qt::RightToolBarArea);
    topAction->setChecked(area == Qt::TopToolBarArea);
    bottomAction->setChecked(area == Qt::BottomToolBarArea);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::LeftToolBarArea);
        rightAction->setEnabled(areas & Qt::RightToolBarArea);
        topAction->setEnabled(areas & Qt::TopToolBarArea);
        bottomAction->setEnabled(areas & Qt::BottomToolBarArea);
    }
}

void ToolBar::order()
{
    QList<QAction *> ordered;
    QList<QAction *> actions1 = actions();
    const QList<QAction *> childActions = findChildren<QAction *>();
    for (QAction *action : childActions) {
        if (!actions1.contains(action))
            continue;
        actions1.removeAll(action);
        ordered.append(action);
    }

    clear();
    addActions(ordered);

    orderAction->setEnabled(false);
}

void ToolBar::randomize()
{
    QList<QAction *> randomized;
    QList<QAction *> actions = this->actions();
    while (!actions.isEmpty()) {
        QAction *action = actions.takeAt(QRandomGenerator::global()->bounded(actions.size()));
        randomized.append(action);
    }
    clear();
    addActions(randomized);

    orderAction->setEnabled(true);
}

void ToolBar::addSpinBox()
{
    if (!spinbox)
        spinbox = new QSpinBox(this);
    if (!spinboxAction)
        spinboxAction = addWidget(spinbox);
    else
        addAction(spinboxAction);

    addSpinBoxAction->setEnabled(false);
    removeSpinBoxAction->setEnabled(true);
}

void ToolBar::removeSpinBox()
{
    if (spinboxAction)
        removeAction(spinboxAction);

    addSpinBoxAction->setEnabled(true);
    removeSpinBoxAction->setEnabled(false);
}

void ToolBar::allow(Qt::ToolBarArea area, bool a)
{
    Qt::ToolBarAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::LeftToolBarArea);
        rightAction->setEnabled(areas & Qt::RightToolBarArea);
        topAction->setEnabled(areas & Qt::TopToolBarArea);
        bottomAction->setEnabled(areas & Qt::BottomToolBarArea);
    }
}

void ToolBar::place(Qt::ToolBarArea area, bool p)
{
    if (!p)
        return;

    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
    Q_ASSERT(mainWindow);

    mainWindow->addToolBar(area, this);

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::LeftToolBarArea);
        allowRightAction->setEnabled(area != Qt::RightToolBarArea);
        allowTopAction->setEnabled(area != Qt::TopToolBarArea);
        allowBottomAction->setEnabled(area != Qt::BottomToolBarArea);
    }
}

void ToolBar::changeMovable(bool movable)
{ setMovable(movable); }

void ToolBar::allowLeft(bool a)
{ allow(Qt::LeftToolBarArea, a); }

void ToolBar::allowRight(bool a)
{ allow(Qt::RightToolBarArea, a); }

void ToolBar::allowTop(bool a)
{ allow(Qt::TopToolBarArea, a); }

void ToolBar::allowBottom(bool a)
{ allow(Qt::BottomToolBarArea, a); }

void ToolBar::placeLeft(bool p)
{ place(Qt::LeftToolBarArea, p); }

void ToolBar::placeRight(bool p)
{ place(Qt::RightToolBarArea, p); }

void ToolBar::placeTop(bool p)
{ place(Qt::TopToolBarArea, p); }

void ToolBar::placeBottom(bool p)
{ place(Qt::BottomToolBarArea, p); }

void ToolBar::insertToolBarBreak()
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
    Q_ASSERT(mainWindow);

    mainWindow->insertToolBarBreak(this);
}

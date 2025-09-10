#include "mainwindow.h"
#include "polyline_scene.h"

#include <QGraphicsView>
#include <QVBoxLayout>
#include <QTabBar>
#include <QStackedWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QFrame>

static QWidget* makeRibbonGroup(QWidget *parent, const QString &title, QWidget *content) {
    auto *box = new QFrame(parent);
    box->setFrameShape(QFrame::StyledPanel);
    box->setFrameShadow(QFrame::Raised);
    auto *vl = new QVBoxLayout(box);
    vl->setContentsMargins(8, 8, 8, 4);
    vl->setSpacing(6);
    vl->addWidget(content);
    auto *lbl = new QLabel(title, box);
    lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    lbl->setStyleSheet("color: gray; font-size: 11px;");
    vl->addWidget(lbl);
    return box;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Polyline Editor — Рейка"));
    resize(1280, 840);
    buildUi();
}

void MainWindow::buildUi() {
    auto *central = new QWidget(this);
    auto *v = new QVBoxLayout(central);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    // СНАЧАЛА создаём сцену/вью, чтобы можно было корректно привязать сигналы вкладки «Рейка»
    m_view = new QGraphicsView(central);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setMouseTracking(true);
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    m_scene = new PolylineScene(m_view);
    m_view->setScene(m_scene);

    // Теперь строим верхнюю «ленту»
    buildRibbon();
    v->addWidget(m_tabBar);
    v->addWidget(m_tabStack);

    auto *line = new QFrame(central);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    v->addWidget(line);

    v->addWidget(m_view, 1);
    setCentralWidget(central);
}

void MainWindow::buildRibbon() {
    m_tabBar = new QTabBar(this);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setMovable(false);
    m_tabBar->setDrawBase(true);

    m_tabStack = new QStackedWidget(this);
    m_tabStack->setContentsMargins(6, 6, 6, 6);

    // ОДНА вкладка — «Рейка»
    m_reikaPage = buildReikaPage();
    int iReika = m_tabBar->addTab(tr("Рейка"));
    m_tabStack->insertWidget(iReika, m_reikaPage);

    connect(m_tabBar, &QTabBar::currentChanged, m_tabStack, &QStackedWidget::setCurrentIndex);
    m_tabBar->setCurrentIndex(iReika);
}

QWidget* MainWindow::buildReikaPage() {
    auto *page = new QWidget(this);
    auto *h = new QHBoxLayout(page);
    h->setContentsMargins(8, 4, 8, 4);
    h->setSpacing(8);

    // Группа «Сетка» — Шаг сетки
    auto *grpContent = new QWidget(page);
    auto *gl = new QHBoxLayout(grpContent);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setSpacing(6);

    auto *lblStep = new QLabel(tr("Шаг:"), grpContent);
    m_stepSpin = new QDoubleSpinBox(grpContent);
    m_stepSpin->setRange(1.0, 1000.0);
    m_stepSpin->setDecimals(1);
    m_stepSpin->setSingleStep(1.0);
    m_stepSpin->setMinimumWidth(90);

    // так как сцена уже создана — берём актуальное значение
    m_stepSpin->setValue(m_scene->gridStep());

    gl->addWidget(lblStep);
    gl->addWidget(m_stepSpin);

    auto *gridGroup = makeRibbonGroup(page, tr("Сетка"), grpContent);
    h->addWidget(gridGroup);
    h->addStretch(1);

    // связь спинбокса со сценой (теперь m_scene уже не nullptr)
    connect(m_stepSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            m_scene, &PolylineScene::setGridStep);

    return page;
}




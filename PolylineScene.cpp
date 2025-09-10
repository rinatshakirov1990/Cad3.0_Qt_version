#include "polyline_scene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMath>

// -------- ClippedGridItem ----------------------------------------------------
ClippedGridItem::ClippedGridItem(const QPainterPath &clipPath, qreal step)
    : m_clip(clipPath), m_step(step) {
    setZValue(-1);
    m_bounds = m_clip.boundingRect();
}

void ClippedGridItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_clip.isEmpty()) return;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(Qt::gray, 1, Qt::DotLine));
    p->setClipPath(m_clip, Qt::ReplaceClip);

    const qreal left   = m_bounds.left();
    const qreal right  = m_bounds.right();
    const qreal top    = m_bounds.top();
    const qreal bottom = m_bounds.bottom();

    for (qreal x = left; x <= right; x += m_step)
        p->drawLine(QLineF(x, top, x, bottom));
    for (qreal y = top; y <= bottom; y += m_step)
        p->drawLine(QLineF(left, y, right, y));

    p->restore();
}

// -------- CrosshairItem ------------------------------------------------------
CrosshairItem::CrosshairItem(qreal half) : m_half(half) {
    setZValue(10);
}

QRectF CrosshairItem::boundingRect() const {
    return QRectF(-m_half-1, -m_half-1, 2*m_half+2, 2*m_half+2);
}

void CrosshairItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
    p->save();
    p->setPen(QPen(Qt::blue, 1));
    p->drawLine(QPointF(-m_half, 0), QPointF(m_half, 0));
    p->drawLine(QPointF(0, -m_half), QPointF(0, m_half));
    p->restore();
}

// -------- PolylineScene ------------------------------------------------------
PolylineScene::PolylineScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 2000, 1400);

    m_previewPath = addPath(QPainterPath(), QPen(Qt::black, 1));
    m_previewPath->setZValue(1);

    m_previewLine = addLine(QLineF(), QPen(QBrush(Qt::black), 1, Qt::DashLine));
    m_previewLine->setVisible(false);

    m_cross = new CrosshairItem(5);
    addItem(m_cross);
}

void PolylineScene::setGridStep(qreal step) {
    if (step <= 0.0) return;
    if (qFuzzyCompare(step, m_gridStep)) return;

    m_gridStep = step;

    // обновляем все существующие сетки
    for (auto *fig : m_figures) {
        if (fig && fig->gridItem) {
            fig->gridItem->setStep(m_gridStep);
        }
    }
}

void PolylineScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    const QPointF clicked = event->scenePos();

    if (event->button() == Qt::LeftButton) {
        if (!m_isDrawing) {
            startNewIfNeeded(clicked);
        } else if (isCloseToFirst(clicked) && m_points.size() >= 3) {
            finalizeCurrent();
        } else {
            addVertex(clicked);
        }
        return;
    }

    if (event->button() == Qt::RightButton) {
        cancelCurrent();
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void PolylineScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    const QPointF pos = event->scenePos();
    m_cross->setPos(pos);

    if (m_isDrawing && !m_points.isEmpty()) {
        m_previewLine->setVisible(true);
        updatePreviewLineTo(pos);
    } else {
        m_previewLine->setVisible(false);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void PolylineScene::startNewIfNeeded(const QPointF &first) {
    m_isDrawing = true;
    m_points.clear();
    for (auto *dot : m_vertexDotsTmp) { removeItem(dot); delete dot; }
    m_vertexDotsTmp.clear();

    m_points.push_back(first);
    addVertexMarkerTemp(first);
    updatePreviewPath();
}

void PolylineScene::addVertex(const QPointF &pt) {
    m_points.push_back(pt);
    addVertexMarkerTemp(pt);
    updatePreviewPath();
}

void PolylineScene::finalizeCurrent() {
    if (!m_isDrawing || m_points.size() < 3) return;

    QPainterPath path;
    path.addPolygon(QPolygonF(m_points));
    path.closeSubpath();

    auto *pathItem = addPath(path, QPen(Qt::black, 1), Qt::NoBrush);
    pathItem->setZValue(2);

    auto *grid = new ClippedGridItem(path, m_gridStep);
    addItem(grid);
    grid->setZValue(0);

    auto *fig = new Figure;
    fig->polygon  = QPolygonF(m_points);
    fig->pathItem = pathItem;
    fig->gridItem = grid;
    fig->vertexDots = m_vertexDotsTmp;
    m_figures.push_back(fig);

    m_points.clear();
    m_vertexDotsTmp.clear();
    m_isDrawing = false;

    m_previewPath->setPath(QPainterPath());
    m_previewLine->setVisible(false);
}

void PolylineScene::cancelCurrent() {
    if (!m_isDrawing) return;
    m_isDrawing = false;
    m_points.clear();
    m_previewPath->setPath(QPainterPath());
    m_previewLine->setVisible(false);

    for (auto *dot : m_vertexDotsTmp) { removeItem(dot); delete dot; }
    m_vertexDotsTmp.clear();
}

void PolylineScene::updatePreviewPath() {
    QPainterPath path;
    if (!m_points.isEmpty()) {
        path.moveTo(m_points.first());
        for (int i = 1; i < m_points.size(); ++i)
            path.lineTo(m_points[i]);
    }
    m_previewPath->setPath(path);
}

void PolylineScene::updatePreviewLineTo(const QPointF &pos) {
    if (m_points.isEmpty()) { m_previewLine->setVisible(false); return; }
    m_previewLine->setLine(QLineF(m_points.last(), pos));
}

void PolylineScene::addVertexMarkerTemp(const QPointF &pt) {
    auto *dot = addEllipse(QRectF(pt.x()-4, pt.y()-4, 8, 8),
                           QPen(Qt::NoPen), QBrush(Qt::red));
    dot->setZValue(5);
    m_vertexDotsTmp << dot;
}

bool PolylineScene::isCloseToFirst(const QPointF &pt) const {
    if (m_points.isEmpty()) return false;
    return QLineF(m_points.front(), pt).length() <= m_closeThreshold;
}

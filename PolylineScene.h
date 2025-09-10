#ifndef POLYLINE_SCENE_H
#define POLYLINE_SCENE_H

#include <QGraphicsScene> // QGraphicsScene — сцена, на которой живут графические объекты.
#include <QGraphicsItem>  // QGraphicsItem — базовый класс любого графического элемента.
#include <QPainterPath>   // QPainterPath — путь для отрисовки сложных контуров.
#include <QVector>        // QVector, QList — контейнеры Qt.
#include <QList>          // QVector, QList — контейнеры Qt.
#include <QPointF>        // QPointF — точка с x, y в float/double.



//------------------------------------------------------------------------------------------
/*
Это отрезанная сетка — сетка, которая рисуется только внутри контура (clipPath).
clipPath — форма, внутри которой рисуем сетку.
step — шаг сетки (по умолчанию 10.0).
*/

class ClippedGridItem : public QGraphicsItem {

private:
    QPainterPath m_clip;  // Контур для "обрезки" сетки
    QRectF m_bounds;      // Границы
    qreal m_step;         // Шаг сетки

public:
    explicit ClippedGridItem(const QPainterPath &clipPath, qreal step = 10.0);
//------------------------------------------------------------------------------------------


    QRectF boundingRect() const override { return m_bounds; }  //Границы отрисовки — чтобы сцена знала, где перерисовывать.

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override; //Реализация отрисовки сетки.


  //------------------------------------------------------------------------------------------
 /*
Меняет шаг сетки, если он реально изменился (qFuzzyCompare — сравнение с учётом погрешности float).
update() перерисует элемент.
*/
    void setStep(qreal step) { if (!qFuzzyCompare(m_step, step)) { m_step = step; update(); } }
     qreal step() const { return m_step; }
 //------------------------------------------------------------------------------------------

// private:
//     QPainterPath m_clip;  // Контур для "обрезки" сетки
//     QRectF m_bounds;      // Границы
//     qreal m_step;         // Шаг сетки
};

//------------------------------------------------------------------------------------------
class CrosshairItem : public QGraphicsItem {
public:
    explicit CrosshairItem(qreal half = 5.0);
/*
Это перекрестие под курсором.
half — половина длины линии перекрестия.
*/
//------------------------------------------------------------------------------------------
    QRectF boundingRect() const override;                                               //Методы для отрисовки перекрестия.
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override;      //Методы для отрисовки перекрестия.

private:
    qreal m_half;
};
//------------------------------------------------------------------------------------------
struct Figure {
    QPolygonF polygon;                               // Геометрия фигуры
    QGraphicsPathItem *pathItem = nullptr;           // Отрисованный контур
    ClippedGridItem   *gridItem = nullptr;           // Сетка внутри
    QList<QGraphicsItem*> vertexDots;                // Точки-вершины
};
/*
Figure — одна замкнутая фигура на сцене с:
polygon — массив точек;
pathItem — нарисованная линия (контур);
gridItem — сетка внутри фигуры;
vertexDots — кружочки на вершинах.
*/
//------------------------------------------------------------------------------------------
class PolylineScene : public QGraphicsScene {     //Наследник QGraphicsScene — управляет процессом рисования фигур.
    Q_OBJECT
public:
    explicit PolylineScene(QObject *parent = nullptr);  //Конструктор и метод для получения текущего шага сетки.

    qreal gridStep() const { return m_gridStep; }  //Конструктор и метод для получения текущего шага сетки.

public slots:
    void setGridStep(qreal step); //Слот — меняет шаг сетки для всех фигур.

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override; //Обрабатывают клики и движения мыши на сцене.
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;  //Обрабатывают клики и движения мыши на сцене.

private:
    void startNewIfNeeded(const QPointF &first); // Начать новую фигуру, если ещё нет
    void addVertex(const QPointF &pt);           // Добавить точку в текущую фигуру
    void finalizeCurrent();                      // Завершить фигуру (замкнуть)
    void cancelCurrent();                         // Отменить рисование

    void updatePreviewPath();                      // Обновить "текущий путь"
    void updatePreviewLineTo(const QPointF &pos);  // Обновить линию от последней точки до курсора
    void addVertexMarkerTemp(const QPointF &pt);   // Временная точка на экране
    bool isCloseToFirst(const QPointF &pt) const;  // Проверить, близко ли к первой точке

private:
    QVector<QPointF> m_points;                        // Точки текущей фигуры
    QList<QGraphicsItem*> m_vertexDotsTmp;            // Временные точки
    bool m_isDrawing = false;                           // Идёт ли рисование

    QGraphicsPathItem *m_previewPath = nullptr;       // Линии текущей фигуры
    QGraphicsLineItem *m_previewLine = nullptr;        // Линия к курсору
    CrosshairItem     *m_cross       = nullptr;       // Перекрестие

    QVector<Figure*> m_figures;                       // Все завершённые фигуры

    qreal m_closeThreshold = 10.0;                    // Порог замыкания
    qreal m_gridStep       = 10.0;                    // Шаг сетки по умолчанию
};

#endif // POLYLINE_SCENE_H

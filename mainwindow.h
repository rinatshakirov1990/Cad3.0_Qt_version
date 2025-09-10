#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsView;
class PolylineScene;
class QTabBar;
class QStackedWidget;
class QDoubleSpinBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // Холст
    QGraphicsView  *m_view  = nullptr; //m_view — объект QGraphicsView, это «окно» через которое мы смотрим на сцену.
    PolylineScene  *m_scene = nullptr; //m_scene — объект PolylineScene (твоя кастомная сцена с фигурами).


    // Верхняя «лента»
    QTabBar        *m_tabBar   = nullptr; //m_tabBar — панель с вкладками (как в браузере).
    QStackedWidget *m_tabStack = nullptr; //m_tabStack — QStackedWidget хранит несколько виджетов, но показывает только один — как стопка страниц.


    // Вкладка «Рейка»
    QWidget        *m_reikaPage = nullptr; //m_reikaPage — отдельная страница/панель с настройками для режима «Рейка».
    QDoubleSpinBox *m_stepSpin  = nullptr; //m_stepSpin — QDoubleSpinBox, поле для ввода числа с плавающей точкой (например, шаг сетки в мм).




    void buildUi();             // центральный layout: лента + вью //buildUi() — собирает всё окно: ленту сверху и область с QGraphicsView снизу.
    void buildRibbon();         // верхняя панель-вкладка(и) // buildRibbon() — создаёт вкладки (QTabBar) и их содержимое (QStackedWidget).
    QWidget* buildReikaPage();  // содержимое «Рейки» //buildReikaPage() — создаёт конкретно страницу для вкладки «Рейка».
};

#endif // MAINWINDOW_H

#pragma once

#include <QtWidgets>

class Widget : public QWidget {
    Q_OBJECT

    static const double deltaAlpha = 0.01;

    QVector3D normal;
    QVector3D perp;
    double alpha;

    bool leftPressed, rightPressed;

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private:
    void init();
    QVector3D randomPerpendicular(const QVector3D &v);
    QVector3D randomVector();
    void updateNormal();
    QVector2D project(const QVector3D &v);
    double sign(double x);
    QVector3D rotated(const QVector3D &v, const QVector3D &u, double angle);

    float map(const QVector3D &p);
    QVector3D march(const QVector3D &origin, const QVector3D &ray);

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
};

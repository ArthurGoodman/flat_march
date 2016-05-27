#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent) {
    resize(qApp->desktop()->size() / 2);
    setMinimumSize(qApp->desktop()->size() / 2);
    move(qApp->desktop()->rect().center() - rect().center());

    startTimer(16);

    qsrand(QTime::currentTime().msec());

    init();
}

Widget::~Widget() {
}

void Widget::init() {
    leftPressed = rightPressed = false;

    updateNormal();
    alpha = 0;
}

QVector3D Widget::randomPerpendicular(const QVector3D &v) {
    return QVector3D::crossProduct(randomVector(), v).normalized();
}

QVector3D Widget::randomVector() {
    return QVector3D((double)qrand() / RAND_MAX * 2 - 1, (double)qrand() / RAND_MAX * 2 - 1, (double)qrand() / RAND_MAX * 2 - 1);
}

void Widget::updateNormal() {
    normal = QVector3D(sin(alpha), 0, cos(alpha));
    perp = randomPerpendicular(normal);
}

QVector2D Widget::project(const QVector3D &v) {
    double angle = acos(QVector3D::dotProduct(QVector3D(0, 0, 1), normal));
    return QVector2D(v.x() * cos(angle) + v.z() * -sin(angle) * sign(normal.x()), v.y());
}

double Widget::sign(double x) {
    return x >= 0 ? 1 : -1;
}

QVector3D Widget::rotated(const QVector3D &v, const QVector3D &u, double a) {
    return QVector3D((cos(a) + u.x() * u.x() * (1 - cos(a))) * v.x() + (u.x() * u.y() * (1 - cos(a)) - u.z() * sin(a)) * v.y() + (u.x() * u.z() * (1 - cos(a)) + u.y() * sin(a)) * v.z(),
                     (u.y() * u.x() * (1 - cos(a)) + u.z() * sin(a)) * v.x() + (cos(a) + u.y() * u.y() * (1 - cos(a))) * v.y() + (u.y() * u.z() * (1 - cos(a)) - u.x() * sin(a)) * v.z(),
                     (u.z() * u.x() * (1 - cos(a)) - u.y() * sin(a)) * v.x() + (u.z() * u.y() * (1 - cos(a)) + u.x() * sin(a)) * v.y() + (cos(a) + u.z() * u.z() * (1 - cos(a))) * v.z());
}

QVector3D vAbs(const QVector3D &v) {
    return QVector3D(fabs(v.x()), fabs(v.y()), fabs(v.z()));
}

QVector3D vMax(const QVector3D &v, float m) {
    return QVector3D(std::max(v.x(), m), std::max(v.y(), m), std::max(v.z(), m));
}

float sdBox(const QVector3D &p, const QVector3D &b) {
    QVector3D d = vAbs(p) - b;
    return std::min(std::max(d.x(), std::max(d.y(), d.z())), 0.0f) + vMax(d, 0.0f).length();
}

float Widget::map(const QVector3D &p) {
    return std::min(std::min(sdBox(p - QVector3D(0.5, 0.25, 1.0), QVector3D(0.25, 0.25, 0.25)), qAbs(p.y())), (p - QVector3D(0.35, 1.0, 0.75)).length() -  0.25f);
}

QVector3D Widget::march(const QVector3D &origin, const QVector3D &ray) {
    static const float epsilon = 1e-6;
    static const int maxIt = 1000;
    static const float maxDist = 100;

    QVector3D p = origin;

    for (int i = 0; i < maxIt; i++) {
        float d = map(p);

        if (d < epsilon || d > maxDist)
            break;

        p += ray * d;
    }

    return p;
}

void Widget::timerEvent(QTimerEvent *) {
    update();

    if (leftPressed) {
        alpha -= deltaAlpha;
        updateNormal();
    } else if (rightPressed) {
        alpha += deltaAlpha;
        updateNormal();
    }
}

void Widget::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        isFullScreen() ? showNormal() : qApp->quit();
        break;

    case Qt::Key_F11:
        isFullScreen() ? showNormal() : showFullScreen();
        break;

    case Qt::Key_Left:
        leftPressed = true;
        break;

    case Qt::Key_Right:
        rightPressed = true;
        break;
    }
}

void Widget::keyReleaseEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Left:
        leftPressed = false;
        break;

    case Qt::Key_Right:
        rightPressed = false;
        break;
    }
}

void Widget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::lightGray);

    p.setRenderHint(QPainter::Antialiasing);

    p.translate(rect().center());
    p.scale(1, -1);

    static const int numberOfRays = 1000;
    static const double scale = 100;

    QPainterPath path;
    QPointF firstPoint;

    for (int i = 0; i < numberOfRays; i++) {
        QVector3D ray = rotated(perp, normal, (double)i / (numberOfRays - 1) * 2 * M_PI);
        QVector3D p = project(march(QVector3D(0, 1, 0), ray));

        if (i == 0)
            path.moveTo(firstPoint = p.toPointF() * scale);
        else
            path.lineTo(p.toPointF() * scale);
    }

    path.lineTo(firstPoint);

    p.strokePath(path, QPen(Qt::black));
}

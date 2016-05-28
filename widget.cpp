#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent) {
    resize(qApp->desktop()->size() / 2);
    setMinimumSize(qApp->desktop()->size() / 2);
    move(qApp->desktop()->rect().center() - rect().center());

    startTimer(16);

    qsrand(QTime::currentTime().msec());

    init();

    setMouseTracking(true);

    centerCursor();
}

Widget::~Widget() {
}

void Widget::init() {
    aPressed = dPressed = false;
    onTheGround = false;

    updateNormal();
    alpha = 0;

    position = QVector3D(0, 1, 0);
}

QVector3D Widget::randomPerpendicular(const QVector3D &v) {
    return QVector3D::crossProduct(randomVector(), v).normalized();
}

QVector3D Widget::perpendicular(const QVector3D &v) {
    return QVector3D::crossProduct(QVector3D(0, 1, 0), v).normalized();
}

QVector3D Widget::randomVector() {
    return QVector3D((double)qrand() / RAND_MAX * 2 - 1, (double)qrand() / RAND_MAX * 2 - 1, (double)qrand() / RAND_MAX * 2 - 1);
}

void Widget::updateNormal() {
    normal = QVector3D(sin(alpha), 0, cos(alpha));
    perp = perpendicular(normal);
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
    return std::min(
        std::min(
            std::min(
                std::min(
                    std::min(
                        std::min(
                            std::min(
                                (p - QVector3D(1.5, 2.0, 2.0)).length() - 0.5f,
                                sdBox(p - QVector3D(0.5, 0.5, 1.0), QVector3D(0.5, 0.5, 0.5))),
                            sdBox(p - QVector3D(-0.5, 2.0, -0.75), QVector3D(0.5, 0.5, 0.5))),
                        sdBox(p - QVector3D(1.0, 4.0, 2.0), QVector3D(0.5, 0.5, 0.5))),
                    sdBox(p - QVector3D(-1.0, 6.0, 0.0), QVector3D(0.5, 0.5, 0.5))),
                sdBox(p - QVector3D(0.0, 8.0, -1.0), QVector3D(0.5, 0.5, 0.5))),
            sdBox(p - QVector3D(1.0, 10.0, 1.0), QVector3D(0.5, 0.5, 0.5))),
        p.y());
}

QVector3D Widget::normalToMap(const QVector3D &p) {
    float e = 1e-3;
    return QVector3D(map(p + QVector3D(e, 0, 0)) - map(p - QVector3D(e, 0, 0)),
                     map(p + QVector3D(0, e, 0)) - map(p - QVector3D(0, e, 0)),
                     map(p + QVector3D(0, 0, e)) - map(p - QVector3D(0, 0, e)))
        .normalized();
}

QVector3D Widget::march(const QVector3D &origin, const QVector3D &ray) {
    static const float epsilon = 1e-6;
    static const int maxIt = 1000;
    static const float maxDist = 100;

    QVector3D p = origin;

    for (int i = 0; i < maxIt; i++) {
        float d = fabs(map(p));

        if (d < epsilon || d > maxDist)
            break;

        p += ray * d;
    }

    return p;
}

void Widget::centerCursor() {
    QCursor c = cursor();
    c.setPos(mapToGlobal(rect().center()));
    c.setShape(Qt::BlankCursor);
    setCursor(c);
}

void Widget::timerEvent(QTimerEvent *) {
    static const float gravityForce = 0.005;

    velocity += QVector3D(0, -gravityForce, 0);

    if (aPressed)
        velocity -= QVector3D(perp.x(), 0, perp.z()).normalized() * gravityForce / 2 * (onTheGround ? 10 : 1);
    else if (dPressed)
        velocity += QVector3D(perp.x(), 0, perp.z()).normalized() * gravityForce / 2 * (onTheGround ? 10 : 1);

    position += velocity;

    QVector3D hit = march(position, velocity.normalized());

    if ((height = (hit - position).length()) < circleRadius) {
        position += normalToMap(hit) * (circleRadius - height);
        velocity /= 100;
    }

    onTheGround = height < circleRadius + 0.15;

    update();
}

void Widget::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        isFullScreen() ? showNormal() : qApp->quit();
        break;

    case Qt::Key_F11:
        isFullScreen() ? showNormal() : showFullScreen();
        break;

    case Qt::Key_A:
        aPressed = true;
        break;

    case Qt::Key_D:
        dPressed = true;
        break;

    case Qt::Key_Space:
        if (onTheGround)
            velocity += QVector3D(0, 0.15, 0);
        break;
    }
}

void Widget::keyReleaseEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_A:
        aPressed = false;
        break;

    case Qt::Key_D:
        dPressed = false;
        break;
    }
}

void Widget::mouseMoveEvent(QMouseEvent *e) {
    alpha += (double)(e->x() - width() / 2) / width();
    updateNormal();

    centerCursor();
}

void Widget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::lightGray);

    p.setRenderHint(QPainter::Antialiasing);

    static const int numberOfRays = 1000;
    static const double scale = 100;

    p.translate(rect().center());
    p.scale(1, -1);

    QPainterPath path;
    QPointF firstPoint;

    for (int i = 0; i < numberOfRays; i++) {
        QVector3D ray = rotated(perp, normal, (double)i / (numberOfRays - 1) * 2 * M_PI);
        QVector3D p = project(march(position, ray) - position);

        if (i == 0)
            path.moveTo(firstPoint = p.toPointF() * scale);
        else
            path.lineTo(p.toPointF() * scale);
    }

    path.lineTo(firstPoint);

    p.strokePath(path, QPen(Qt::black));

    p.drawEllipse(QPointF(), circleRadius * scale, circleRadius * scale);
}

#ifndef PALETTEITEMWIDGET_H
#define PALETTEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

class PaletteItemWidget : public QWidget {
Q_OBJECT

public:
    PaletteItemWidget(QString nodeType_, QString nodeClass_, QPixmap icon,
                      QWidget *parent = nullptr);

    ~PaletteItemWidget() override = default;

    QString getNodeTypeString() const {
        return nodeClassName;
    }


protected:

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *iconLabel;
    QLabel *textLabel;
    QString nodeClassName;
    QString nodeType;
    QPixmap icon;

    bool isDragging = false;
};


#endif //PALETTEITEMWIDGET_H

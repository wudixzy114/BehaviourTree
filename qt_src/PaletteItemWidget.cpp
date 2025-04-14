#include "PaletteItemWidget.h"

#include <utility>

PaletteItemWidget::PaletteItemWidget(QString nodeType_, QString nodeClass_, QPixmap icon_,
                                     QWidget *parent) : QWidget(parent), nodeType(std::move(nodeType_)),
                                                        nodeClassName(std::move(nodeClass_)),
                                                        icon(std::move(icon_)) {
    iconLabel = new QLabel(this);
    iconLabel->setPixmap(icon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);

    textLabel = new QLabel(nodeType, this);
    textLabel->setAlignment(Qt::AlignCenter);

    auto *nodeLayout = new QVBoxLayout(this);
    nodeLayout->addWidget(iconLabel);
    nodeLayout->addWidget(textLabel);

    setLayout(nodeLayout);
}

void PaletteItemWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
    }
    QWidget::mousePressEvent(event);
}

void PaletteItemWidget::mouseMoveEvent(QMouseEvent *event) {
    if ((event->button() == Qt::LeftButton) && isDragging) {
        auto *drag = new QDrag(this);
        auto *mimeData = new QMimeData;

        mimeData->setText(nodeClassName);
        drag->setMimeData(mimeData);

        drag->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
        Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
        isDragging = false;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void PaletteItemWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void PaletteItemWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    if (isDragging) {
        QPainter painter(this);
        QPen pen(Qt::blue);
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoButton);
        painter.drawRect(rect().adjusted(2, 2, -2, -2));
    }
}

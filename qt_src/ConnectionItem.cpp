#include <QPen>
#include <QPainterPath>
#include <QGraphicsPathItem>

#include "BehaviorTreeNodeItem.h"
#include "ConnectionItem.h"


ConnectionItem::ConnectionItem(BehaviorTreeNodeItem *startItem, BehaviorTreeNodeItem *endItem, QGraphicsItem *parent)
        : QGraphicsPathItem(parent), startItem(startItem), endItem(endItem) {
    QPen pen(Qt::black, 1.5);
    setPen(pen);
    setZValue(-1);
    updatePath();
}

void ConnectionItem::updatePath() {
    if (!startItem || !endItem) {
        setPath(QPainterPath());
        return;
    }

    QPointF startPos = startItem->mapToScene(startItem->outputPoint());
    QPointF endPos = endItem->mapToScene(endItem->inputPoint());

    QPainterPath newPath(startPos);
    qreal dx = endPos.x() - startPos.x();
    qreal dy = endPos.y() - startPos.y();

    QPointF ctr11 = startPos + QPointF(0, dy * 0.5);
    QPointF ctr12 = endPos - QPointF(0, dy * 0.5);
    newPath.cubicTo(ctr11, ctr12, endPos);

    setPath(newPath);
    update();
}
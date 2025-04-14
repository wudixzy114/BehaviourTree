#ifndef BEHAVIORTREENODEITEM_H
#define BEHAVIORTREENODEITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QPointF>
#include "TreeNode.h"
#include "Define.h"

class BehaviorTreeNodeItem : public QGraphicsObject {
Q_OBJECT

public:
    explicit BehaviorTreeNodeItem(std::weak_ptr<TreeNode> node, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    std::shared_ptr<TreeNode> getNode() const {
        return node.lock();
    }

    int getNodeId() const;

    void setStatus(NodeStatus status);

    QPointF inputPoint() const;

    QPointF outputPoint() const;

signals:

    void itemMoved();

protected:

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:

    std::weak_ptr<TreeNode> node;
    QString displayText;
    QColor color;
    NodeStatus currentStatus;
    QRectF bounds;

    void updateDisplayProperties();
};


#endif //BEHAVIORTREENODEITEM_H

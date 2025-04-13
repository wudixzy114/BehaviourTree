#include <QApplication>
#include <QPen>
#include <QBrush>
#include <iostream>
#include <utility>
#include <QColor>
#include "BehaviorTreeNodeItem.h"
#include "NodeRegistrar.h"

QColor mixColor(const QColor &color1, const QColor &color2, double ratio) {
    ratio = std::clamp(ratio, 0.0, 1.0);

    int red = color1.red() + (int) ((color2.red() - color1.red()) * ratio);
    int green = color1.green() + (int) ((color2.green() - color1.green()) * ratio);
    int blue = color1.blue() + (int) ((color2.blue() - color1.blue()) * ratio);
    int alpha = color1.alpha() + (int) ((color2.alpha() - color1.alpha()) * ratio);

    return {red, green, blue, alpha};
}

BehaviorTreeNodeItem::BehaviorTreeNodeItem(std::weak_ptr<TreeNode> node, QGraphicsItem *parent) : QGraphicsObject(
        parent), node(std::move(node)), currentStatus(NodeStatus::Invalid) {
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
    bounds = QRectF(0, 0, 120, 50);
    updateDisplayProperties();
}

void BehaviorTreeNodeItem::updateDisplayProperties() {
    if (auto nodeSP = node.lock()) {
        displayText = QString::fromStdString(nodeSP->name + "\n[" + NodeTypeRegistry::toString(nodeSP->type) + "]");
        switch (nodeSP->type) {
            case NodeType::Sequence:
                color = Qt::cyan;
                break;
            case NodeType::Selector:
                color = Qt::magenta;
                break;
            case NodeType::Action:
                color = Qt::yellow;
                break;
            case NodeType::Condition:
                color = Qt::green;
                break;
            default:
                color = Qt::lightGray;
                break;
        }
    } else {
        displayText = "Invalid Node";
        color = Qt::red;
    }
    setStatus(currentStatus);
    update();
}

QRectF BehaviorTreeNodeItem::boundingRect() const {
    return bounds.adjusted(-1, -1, 1, 1);
}

void BehaviorTreeNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(widget)
    QPen pen(Qt::black);
    if (option->state & QStyle::State_Selected) {
        pen.setWidth(2);
        pen.setColor(Qt::blue);
    }

    painter->setPen(pen);
    painter->setBrush(color);
    painter->drawRoundedRect(bounds, 5, 5);
    painter->setPen(Qt::black);
    painter->drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap, displayText);
}

QVariant BehaviorTreeNodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged && scene()) {
        QMetaObject::invokeMethod(this, "itemMoved", Qt::QueuedConnection);
    }
    return QGraphicsObject::itemChange(change, value);
}

int BehaviorTreeNodeItem::getNodeId() const {
    if (auto nodeSP = node.lock()) {
        return nodeSP->id;
    }
    return -1;
}

void BehaviorTreeNodeItem::setStatus(NodeStatus status) {
    if (currentStatus == status && !node.expired()) {
        return;
    }
    currentStatus = status;
    if (auto nodeSP = node.lock()) {
        updateDisplayProperties();
        switch (status) {
            case NodeStatus::Success:
                color = color.lighter(130);
                color = mixColor(color, Qt::green, 0.5);
                break;
            case NodeStatus::Failure:
                color = color.darker(130);
                color = mixColor(color, Qt::red, 0.5);
                break;
            case NodeStatus::Running:
                color = mixColor(color, Qt::yellow, 0.5);
                break;
            case NodeStatus::Invalid:
            default:
                color = color.darker(110);
                break;
        }
    } else {
        displayText = "Invalid Node";
        color = Qt::red;
    }
    update();
}

QPointF BehaviorTreeNodeItem::inputPoint() const {
    return bounds.topLeft() + QPointF(bounds.width() / 2, 0);
}

QPointF BehaviorTreeNodeItem::outputPoint() const {
    return bounds.bottomLeft() + QPointF(bounds.width() / 2, 0);
}
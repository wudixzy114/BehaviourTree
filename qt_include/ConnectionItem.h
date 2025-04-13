#ifndef CONNECTIONITEM_H
#define CONNECTIONITEM_H

#include <QGraphicsPathItem>

class BehaviorTreeNodeItem;

class ConnectionItem : public QGraphicsPathItem {
public:
    ConnectionItem(BehaviorTreeNodeItem *startItem, BehaviorTreeNodeItem *endItem, QGraphicsItem *parent = nullptr);

    void updatePath();

    BehaviorTreeNodeItem *getStart() const {
        return startItem;
    }

    BehaviorTreeNodeItem *getEnd() const {
        return endItem;
    }

protected:

private:
    BehaviorTreeNodeItem *startItem;
    BehaviorTreeNodeItem *endItem;
};


#endif //CONNECTIONITEM_H

#ifndef DEFINE_H
#define DEFINE_H

class Blackboard;

enum class NodeType {
    Sequence,
    Selector,
    Action,
    Condition,
    Custom
};

enum class NodeStatus {
    Invalid,
    Success,
    Failure,
    Running
};

using NodeCallback = std::function<NodeStatus(float, Blackboard &)>;

#endif //DEFINE_H

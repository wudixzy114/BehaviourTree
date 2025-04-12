#ifndef DEFINE_H
#define DEFINE_H

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

#endif //DEFINE_H

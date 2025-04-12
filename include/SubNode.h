#ifndef SUBNODE_H
#define SUBNODE_H

#include "TreeNode.h"
#include "Define.h"

class SequenceNode : public TreeNode {
public:
    SequenceNode(int id, const std::string &name, const std::string &className = "") : TreeNode(id, NodeType::Sequence,
                                                                                                className,
                                                                                                name),
                                                                                       runningChildIndex(0) {}

    NodeStatus update(float dt, Blackboard &blackboard) override;

    void reset() override;

private:
    size_t runningChildIndex;
};

class ActionNode : public TreeNode {
public:
    ActionNode(int id, const std::string &name, const std::string &className)
            : TreeNode(id, NodeType::Action, className, name) {}

    NodeStatus update(float dt, Blackboard &blackboard) override;
};

class SelectorNode : public TreeNode {
public:
    SelectorNode(int id, const std::string &name, const std::string &className = "") : TreeNode(id, NodeType::Selector,
                                                                                                className,
                                                                                                name),
                                                                                       runningChildIndex(0) {}

    NodeStatus update(float dt, Blackboard &blackboard) override;

    void reset() override;

private:
    size_t runningChildIndex;
};

class ConditionNode : public TreeNode {
public:
    ConditionNode(int id, const std::string &name, const std::string &className) : TreeNode(id, NodeType::Condition,
                                                                                            className,
                                                                                            name) {}

    NodeStatus update(float dt, Blackboard &blackboard) override;
};

#endif //SUBNODE_H

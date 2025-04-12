#ifndef SUBNODE_H
#define SUBNODE_H

#include "TreeNode.h"

class SequenceNode : public TreeNode {
public:
    SequenceNode(int id, const std::string &name) : TreeNode(id, NodeType::Sequence, name) {}

    void a() const override {}
};

class ActionNode : public TreeNode {
public:
    ActionNode(int id, const std::string &name)
            : TreeNode(id, NodeType::Action, name) {}

    void a() const override {}
};

class SelectorNode : public TreeNode {
public:
    SelectorNode(int id, const std::string &name) : TreeNode(id, NodeType::Selector, name) {}

    void a() const override {}
};

class ConditionNode : public TreeNode {
public:
    ConditionNode(int id, const std::string &name) : TreeNode(id, NodeType::Condition, name) {}

    void a() const override {}
};

#endif //SUBNODE_H

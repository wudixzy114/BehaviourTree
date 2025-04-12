#include "SubNode.h"
#include "NodeRegistrar.h"

/**
 * @brief 注册节点
 *
 * @param NodeType 传入节点类型，可以是自定义节点类型Custom,也可以是对Action等需要具体实现的节点
 * @param Class 传入自定义C++类，用于区分同一个节点的具体实现
 * @param String 用来自定义标识的字符串
 *
 * @note 使用节点工厂构造，构造函数格式必须一致
 *
 */
REGISTER_NODE_TYPE(NodeType::Sequence, SequenceNode, "Sequence")
REGISTER_NODE_TYPE(NodeType::Selector, SelectorNode, "Selector")
REGISTER_NODE_TYPE(NodeType::Condition, ConditionNode, "Condition")

NodeStatus SequenceNode::update(float dt, Blackboard &blackboard) {
    for (auto i = runningChildIndex; i < children.size(); ++i) {
        const auto &child = children[i];
        NodeStatus newStatus = child->update(dt, blackboard);
        switch (newStatus) {
            case NodeStatus::Success:
                continue;
            case NodeStatus::Failure:
                runningChildIndex = 0;
                status = NodeStatus::Failure;
                return status;
            case NodeStatus::Running:
                runningChildIndex = i;
                status = NodeStatus::Running;
                return status;
            case NodeStatus::Invalid:
                runningChildIndex = 0;
                status = NodeStatus::Failure;
                return status;
        }
    }

    runningChildIndex = 0;
    status = NodeStatus::Success;
    return status;
}

void SequenceNode::reset() {
    TreeNode::reset();
    runningChildIndex = 0;
    for (const auto &child: children) {
        child->reset();
    }
}

NodeStatus SelectorNode::update(float dt, Blackboard &blackboard) {
    for (auto i = runningChildIndex; i < children.size(); ++i) {
        const auto &child = children[i];
        NodeStatus newStatus = child->update(dt, blackboard);
        switch (newStatus) {
            case NodeStatus::Success:
                runningChildIndex = 0;
                status = NodeStatus::Success;
                return status;
            case NodeStatus::Failure:
                continue;
            case NodeStatus::Running:
                runningChildIndex = i;
                status = NodeStatus::Running;
                return status;
            case NodeStatus::Invalid:
                continue;
        }
    }

    runningChildIndex = 0;
    status = NodeStatus::Failure;
    return status;
}

void SelectorNode::reset() {
    TreeNode::reset();
    runningChildIndex = 0;
    for (const auto &child: children) {
        child->reset();
    }
}

NodeStatus ActionNode::update(float dt, Blackboard &blackboard) {
    if (callback != nullptr) {
        return callback(dt, blackboard);
    }
    return NodeStatus::Failure;
    //todo
}

NodeStatus ConditionNode::update(float dt, Blackboard &blackboard) {
    if (callback != nullptr) {
        return callback(dt, blackboard);
    }
    return NodeStatus::Failure;
    //todo
}
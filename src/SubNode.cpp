#include "SubNode.h"
#include "NodeRegistrar.h"

REGISTER_NODE_TYPE(NodeType::Sequence, SequenceNode, "Sequence")
REGISTER_NODE_TYPE(NodeType::Selector, SelectorNode, "Selector")

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

        child->invokeStatusCallbacks(newStatus);
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

        child->invokeStatusCallbacks(newStatus);
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
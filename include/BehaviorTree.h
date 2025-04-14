#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H

#include "TreeNode.h"
#include "Define.h"
#include "Blackboard.h"

class BehaviorTree {
public:
    using SP = TreeNode::SP;

    BehaviorTree() = default;

    void addNode(const SP &parent, NodeType type, int id, const std::string &name, const std::string &className = "");

    void removeNode(int id);

    void saveToJson(const std::string &filename);

    void loadFromJson(const std::string &filename);

    void setNodeCallback(int id, const NodeCallback &callback);

    NodeStatus update(float dt) {
        if (root) {
            return root->update(dt, blackboard);
        }
        return NodeStatus::Failure;
    }

    auto getRoot() const {
        return root;
    }

    Blackboard &getBlackboard() {
        return blackboard;
    }

    const Blackboard &getBlackboard() const {
        return blackboard;
    }

    auto &getNodeMap() {
        return nodeMap;
    }

private:
    SP root;
    std::map<int, SP> nodeMap;
    Blackboard blackboard;
};


#endif //BEHAVIORTREE_H

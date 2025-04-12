#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H

#include "TreeNode.h"

class BehaviorTree {
public:
    using SP = TreeNode::SP;

    SP root;
    std::map<int, SP> nodeMap;

    void addNode(const SP &parent, NodeType type, int id, const std::string &name);

    void removeNode(int id);

    void saveToJson(const std::string &filename);

    void loadFromJson(const std::string &filename);
};


#endif //BEHAVIORTREE_H

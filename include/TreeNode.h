#ifndef TREENODE_H
#define TREENODE_H

#include <memory>
#include <cstring>
#include <utility>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "Define.h"

class Blackboard;

class TreeNode : public std::enable_shared_from_this<TreeNode> {
public:
    using WP = std::weak_ptr<TreeNode>;
    using SP = std::shared_ptr<TreeNode>;
    using StatusCallback = std::function<void(TreeNode *, NodeStatus curStatus)>;

    void invokeStatusCallbacks(NodeStatus curStatus);

public:
    int id;
    std::string name;
    std::string className;
    NodeType type;
    std::map<std::string, std::string> properties;
    std::vector<SP> children;
    WP parent;

protected:
    NodeStatus status;

public:
    TreeNode(int id, NodeType type, std::string className, std::string name = "") : id(id), type(type),
                                                                                    name(std::move(name)),
                                                                                    status(NodeStatus::Invalid),
                                                                                    className(std::move(className)) {}

    virtual ~TreeNode() = default;

    virtual NodeStatus update(float dt, Blackboard &blackboard) = 0;

    virtual void reset() {
        status = NodeStatus::Invalid;
    }

    virtual void serialize(nlohmann::json &nodeJson) const;

    NodeStatus getStatus() const {
        return status;
    }

    void registerStatusCallback(const StatusCallback &callback);

private:
    std::vector<StatusCallback> statusCallbacks;
};


std::string toString(NodeType type);

NodeType parseNodeType(const std::string &type);

#endif //TREENODE_H

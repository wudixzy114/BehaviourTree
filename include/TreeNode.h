#ifndef TREENODE_H
#define TREENODE_H

#include <memory>
#include <cstring>
#include <utility>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

enum class NodeType {
    Sequence,
    Selector,
    Action,
    Condition,
    Custom
};

class TreeNode {
public:
    using WP = std::weak_ptr<TreeNode>;
    using SP = std::shared_ptr<TreeNode>;

public:
    int id;
    std::string name;
    NodeType type;
    std::map<std::string, std::string> properties;
    std::vector<SP> children;
    WP parent;

public:
    TreeNode(int id, NodeType type, std::string name) : id(id), type(type), name(std::move(name)) {}

    virtual ~TreeNode() = default;

    virtual void a() const {};

    virtual void serialize(nlohmann::json &nodeJson) const;
};

std::string toString(NodeType type);

NodeType parseNodeType(const std::string &type);

#endif //TREENODE_H

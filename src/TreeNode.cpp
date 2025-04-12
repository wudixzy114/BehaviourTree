#include "TreeNode.h"
#include "NodeRegistrar.h"

std::string toString(NodeType type) {
    try {
        return NodeTypeRegistry::toString(type);
    } catch (const std::out_of_range &e) {
        return "Unknown";
    }
}

NodeType parseNodeType(const std::string &type) {
    try {
        return NodeTypeRegistry::fromString(type);
    } catch (const std::invalid_argument &e) {
        throw std::invalid_argument("Failed to parse node type string: " + type + ". Reason: " + e.what());
    }
}

void TreeNode::serialize(nlohmann::json &nodeJson) const {
    nodeJson["id"] = id;
    nodeJson["type"] = toString(type);
    nodeJson["name"] = name;
    nodeJson["className"] = className;
    nodeJson["properties"] = properties;
    nodeJson["children"] = nlohmann::json::array();
    for (const auto &child: children) {
        nodeJson["children"].push_back(child->id);
    }
}

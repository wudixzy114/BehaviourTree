#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include <functional>
#include <map>
#include "TreeNode.h"

class NodeFactory {
public:
    static NodeFactory &getInstance() {
        static NodeFactory instance;
        return instance;
    }

    NodeFactory(NodeFactory const &) = delete;

    void operator=(NodeFactory const &) = delete;

    using Creator = std::function<TreeNode::SP(int, const std::string &)>;

    void registerType(NodeType type, Creator creator) {
        creatorMap[type] = std::move(creator);
    }

    [[nodiscard]] auto createNode(NodeType type, int id, const std::string &name) const {
        auto it = creatorMap.find(type);
        if (it == creatorMap.end()) {
            throw std::invalid_argument("Unknown node type: " + std::to_string(static_cast<int>(type)));
        }
        return it->second(id, name);
    }

private:
    NodeFactory() = default;

    std::map<NodeType, Creator> creatorMap;
};


#endif //NODEFACTORY_H

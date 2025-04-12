#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include <functional>
#include <map>
#include "TreeNode.h"
#include "Define.h"

class NodeFactory {
public:
    static NodeFactory &getInstance() {
        static NodeFactory instance;
        return instance;
    }

    NodeFactory(NodeFactory const &) = delete;

    void operator=(NodeFactory const &) = delete;

    using Creator = std::function<TreeNode::SP(int, const std::string &)>;

    void registerType(NodeType type, const std::string &className, Creator creator) {
        creatorMap[std::make_pair(type, className)] = std::move(creator);
    }

    [[nodiscard]] auto createNode(NodeType type, int id, const std::string &name, const std::string &className) const {
        auto it = creatorMap.find(std::make_pair(type, className));
        if (it == creatorMap.end()) {
            throw std::invalid_argument(
                    "Unknown node type and class: type=" + std::to_string(static_cast<int>(type)) + ", class=" +
                    className);
        }
        return it->second(id, name);
    }

private:
    NodeFactory() = default;

    std::map<std::pair<NodeType, std::string>, Creator> creatorMap;
};


#endif //NODEFACTORY_H

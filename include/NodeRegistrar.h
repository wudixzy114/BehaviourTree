#ifndef NODEREGISTRAR_H
#define NODEREGISTRAR_H

#include "NodeFactory.h"
#include "TreeNode.h"
#include "Define.h"


class NodeTypeRegistry {
public:
    NodeTypeRegistry() = delete;

    static void registerMapping(NodeType type, const std::string &typeString) {
        if (getRegistry().typeToStringMap.count(type) || getRegistry().stringToTypeMap.count(typeString)) {
            throw std::runtime_error("Node type or string representation already registered: " + typeString);
        }
        getRegistry().typeToStringMap[type] = typeString;
        getRegistry().stringToTypeMap[typeString] = type;
    }

    static std::string toString(NodeType type) {
        const auto &map = getRegistry().typeToStringMap;
        auto it = map.find(type);
        if (it == map.end()) {
            throw std::out_of_range("Unknown NodeType enum value encountered.");
        }
        return it->second;
    }

    static NodeType fromString(const std::string &typeString) {
        const auto &map = getRegistry().stringToTypeMap;
        auto it = map.find(typeString);
        if (it == map.end()) {
            throw std::invalid_argument("Unknown node type string: " + typeString);
        }
        return it->second;
    }

private:
    struct RegistryMaps {
        std::map<NodeType, std::string> typeToStringMap;
        std::map<std::string, NodeType> stringToTypeMap;
    };

    static RegistryMaps &getRegistry() {
        static RegistryMaps instance;
        return instance;
    }
};

struct NodeRegistrar {
    NodeRegistrar(NodeType type, const std::string &className, const std::string &typeString,
                  NodeFactory::Creator creator) {
        NodeFactory::getInstance().registerType(type, className, std::move(creator));
        NodeTypeRegistry::registerMapping(type, typeString);
    }
};

#define REGISTER_NODE_TYPE(NodeType, ClassName, NodeStringName) \
    namespace { \
        static NodeRegistrar registrar_##ClassName( \
                NodeType,                                       \
                #ClassName,                                               \
                NodeStringName, \
                [](int id, const std::string& name)->TreeNode::SP{ \
                    return std::make_shared<ClassName>(id,name); \
                } \
        );   \
    }

#endif //NODEREGISTRAR_H

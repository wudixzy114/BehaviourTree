#include <queue>
#include <fstream>
#include "BehaviorTree.h"
#include "TreeNode.h"
#include "NodeFactory.h"

void BehaviorTree::addNode(const BehaviorTree::SP &parent, NodeType type, int id, const std::string &name) {
    auto node = NodeFactory::getInstance().createNode(type, id, name);
    node->parent = parent;
    if (parent) {
        parent->children.push_back(node);
    } else {
        root = node;
    }
    nodeMap[id] = node;
}

void BehaviorTree::removeNode(int id) {
    auto it = nodeMap.find(id);
    if (it == nodeMap.end()) {
        return;
    }
    SP node = it->second;
    if (auto parent = node->parent.lock()) {
        auto &siblings = parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
    }
    nodeMap.erase(it);
    if (root == node) {
        root.reset();
    }
}

void BehaviorTree::saveToJson(const std::string &filename) {
    nlohmann::json json;

    if (!root) {
        json["root"] = nullptr;
        json["nodes"] = nlohmann::json::array();
    } else {
        json["root"] = root->id;
        auto &nodes = json["nodes"] = nlohmann::json::array();
        std::queue<SP> queue;
        queue.push(root);

        while (!queue.empty()) {
            SP node = queue.front();
            queue.pop();

            nlohmann::json nodeJson;
            node->serialize(nodeJson);
            nodes.push_back(std::move(nodeJson));

            for (const auto &child: node->children) {
                queue.push(child);
            }
        }
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    file << json.dump(4);
}

void BehaviorTree::loadFromJson(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    nlohmann::json json;
    file >> json;

    nodeMap.clear();
    root.reset();

    for (const auto &nodeJson: json["nodes"]) {
        int id = nodeJson["id"].get<int>();
        NodeType type = parseNodeType(nodeJson["type"].get<std::string>());
        std::string name = nodeJson["name"].get<std::string>();
        auto node = NodeFactory::getInstance().createNode(type, id, name);
        node->properties = nodeJson["properties"].get<std::map<std::string, std::string>>();
        nodeMap[id] = node;
    }

    for (const auto &nodeJson: json["nodes"]) {
        int id = nodeJson["id"].get<int>();
        auto node = nodeMap[id];
        for (const auto &childId: nodeJson["children"]) {
            int cid = childId.get<int>();
            auto child = nodeMap.at(cid);
            node->children.push_back(child);
            child->parent = node;
        }
    }

    if (json.contains("root")) {
        if (!json["root"].is_null()) {
            int rootId = json["root"].get<int>();
            auto it = nodeMap.find(rootId);
            if (it != nodeMap.end()) {
                root = it->second;
            } else {
                throw std::runtime_error("Root node ID not found: " + std::to_string(rootId));
            }
        }
    }
}
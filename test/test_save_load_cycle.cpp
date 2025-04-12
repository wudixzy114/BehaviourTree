#include "gtest/gtest.h"
#include "BehaviorTree.h"
#include "SubNode.h" // 引入实现以确保节点类型被注册
#include "NodeRegistrar.h" // 引入实现以确保节点类型被注册
#include <fstream>
#include <cstdio> // 用于 std::remove
#include <queue>
#include <map>
#include "nlohmann/json.hpp" // 确保引入 JSON 库

// 该文件包含针对 BehaviorTree 类的核心序列化/反序列化功能的测试，
// 主要验证一个包含节点和结构的树在保存到 JSON 再加载回来后，
// 其结构和数据是否保持不变（即所谓的“往返测试” Round-Trip Test）。

// 测试固件 (Test Fixture)，用于需要预设树结构和临时文件的测试
class BehaviorTreeSaveLoadTest : public ::testing::Test {
protected:
    const std::string test_filename = "save_load_cycle_test.json"; // 测试文件名
    BehaviorTree originalTree; // 用于存储原始树结构
    BehaviorTree loadedTree;   // 用于存储从文件加载回来的树结构

    // 在每个测试用例运行前执行，用于构建一个非空的、有结构的原始行为树
    void SetUp() override {
        // 构建一个示例树结构:
        // Root: Sequence (ID 1)
        //   |- Action (ID 2, name="ActionA", properties={"duration":"5s"})
        //   |- Selector (ID 3, name="SubSelector")
        //      |- Condition (ID 4, name="IsEnemyNear", properties={"range":"10m"})
        //      |- Action (ID 5, name="ActionB")
        originalTree.addNode(nullptr, NodeType::Sequence, 1, "RootSequence");
        ASSERT_NE(originalTree.root, nullptr); // 确保根节点已创建
        ASSERT_EQ(originalTree.root->id, 1);    // 验证根节点 ID

        originalTree.addNode(originalTree.root, NodeType::Action, 2, "ActionA");
        originalTree.nodeMap[2]->properties["duration"] = "5s"; // 为节点添加属性

        originalTree.addNode(originalTree.root, NodeType::Selector, 3, "SubSelector");

        originalTree.addNode(originalTree.nodeMap[3], NodeType::Condition, 4, "IsEnemyNear");
        originalTree.nodeMap[4]->properties["range"] = "10m"; // 为节点添加属性

        originalTree.addNode(originalTree.nodeMap[3], NodeType::Action, 5, "ActionB");

        ASSERT_EQ(originalTree.nodeMap.size(), 5); // 确认所有节点都已添加到映射表中
    }

    // 每个测试用例运行后执行，用于清理资源
    void TearDown() override {
        std::remove(test_filename.c_str()); // 删除测试文件
    }

    // 辅助函数：比较两棵行为树是否完全相等（包括结构和数据）
    // 遍历树结构，逐个比较节点的 ID, Name, Type, Properties, 子节点数量及 ID, 父节点 ID。
    void AssertTreesAreEqual(const BehaviorTree &tree1, const BehaviorTree &tree2) {
        ASSERT_NE(tree1.root, nullptr); // 两棵树都应有根节点
        ASSERT_NE(tree2.root, nullptr);
        ASSERT_EQ(tree1.nodeMap.size(), tree2.nodeMap.size()); // 节点总数应相等

        std::queue<std::pair<TreeNode::SP, TreeNode::SP>> nodeQueue; // 使用队列进行广度优先遍历比较
        nodeQueue.push({tree1.root, tree2.root});

        std::map<int, bool> visited; // 记录 tree2 中已访问比较过的节点 ID，防止重复或遗漏

        while (!nodeQueue.empty()) {
            auto [node1, node2] = nodeQueue.front();
            nodeQueue.pop();

            ASSERT_NE(node1, nullptr); // 节点指针不应为空
            ASSERT_NE(node2, nullptr);

            // --- 比较节点基本信息 ---
            ASSERT_EQ(node1->id, node2->id) << "Node ID mismatch";
            ASSERT_EQ(node1->name, node2->name) << "Node Name mismatch for ID: " << node1->id;
            ASSERT_EQ(node1->type, node2->type) << "Node Type mismatch for ID: " << node1->id;
            ASSERT_EQ(node1->properties, node2->properties) << "Node Properties mismatch for ID: " << node1->id;
            ASSERT_EQ(node1->children.size(), node2->children.size())
                                        << "Children count mismatch for ID: " << node1->id;

            // --- 比较父节点关系 ---
            auto parent1 = node1->parent.lock(); // 获取父节点 shared_ptr
            auto parent2 = node2->parent.lock();
            if (parent1) { // 如果 node1 有父节点
                ASSERT_NE(parent2, nullptr) << "Node " << node2->id << " should have update parent.";
                ASSERT_EQ(parent1->id, parent2->id) << "Parent ID mismatch for node " << node1->id;
            } else { // 如果 node1 是根节点
                ASSERT_EQ(parent2, nullptr) << "Node " << node2->id << " should be root (no parent).";
            }

            // --- 标记已访问 ---
            ASSERT_FALSE(visited[node2->id]) << "Node " << node2->id << " visited multiple times.";
            visited[node2->id] = true;

            // --- 将子节点加入比较队列 ---
            // 假设保存和加载能保持子节点顺序，直接按下标比较 ID
            for (size_t i = 0; i < node1->children.size(); ++i) {
                ASSERT_LT(i, node2->children.size()); // 确保 tree2 也有对应的子节点索引
                ASSERT_EQ(node1->children[i]->id, node2->children[i]->id)
                                            << "Child ID mismatch at index " << i << " for parent " << node1->id;
                nodeQueue.push({node1->children[i], node2->children[i]}); // 将对应的子节点对加入队列
            }
        }

        // --- 最终检查 ---
        // 确认 tree2 映射表中的所有节点都参与了比较
        ASSERT_EQ(visited.size(), tree2.nodeMap.size())
                                    << "Not all nodes in the loaded tree were visited during comparison.";
    }
};

// 测试套件：BehaviorTreeSaveLoadTest
// 测试用例：SaveAndLoad
// 目的：执行一次完整的保存再加载流程，并验证加载后的树 (loadedTree)
//       与原始构建的树 (originalTree) 在结构和所有节点数据上完全一致。
TEST_F(BehaviorTreeSaveLoadTest, SaveAndLoad) {
    // 1. 保存原始树到文件
    ASSERT_NO_THROW(originalTree.saveToJson(test_filename));

    // 2. （可选）基本文件存在性检查
    std::ifstream file_check(test_filename);
    ASSERT_TRUE(file_check.good());
    file_check.close();

    // 3. 从文件加载到新的树对象
    ASSERT_NO_THROW(loadedTree.loadFromJson(test_filename));

    // 4. 使用辅助函数断言原始树和加载后的树完全相等
    AssertTreesAreEqual(originalTree, loadedTree);
}

// 测试套件：BehaviorTreeSaveLoadTest
// 测试用例：LoadInvalidRoot
// 目的：测试加载函数的错误处理能力。验证当 JSON 文件中的 "root" 字段
//       指定了一个在 "nodes" 数组中不存在的 ID 时，loadFromJson 是否会
//       按预期抛出异常。
TEST_F(BehaviorTreeSaveLoadTest, LoadInvalidRoot) {
    // 1. 先保存一个有效的树结构到文件
    ASSERT_NO_THROW(originalTree.saveToJson(test_filename));

    // 2. 读取 JSON 文件内容并修改它
    nlohmann::json json;
    {
        std::ifstream infile(test_filename);
        ASSERT_TRUE(infile.is_open());
        infile >> json;
        // infile 在代码块结束时自动关闭 (RAII)
    }

    json["root"] = 999; // 将 root ID 修改为一个不存在的值

    {
        std::ofstream outfile(test_filename); // 重新打开文件以写入修改后的 JSON
        ASSERT_TRUE(outfile.is_open());
        outfile << json.dump(4);
        // outfile 在代码块结束时自动关闭 (RAII)
    }

    // 3. 尝试加载这个被篡改过的文件
    // 断言：加载操作应该抛出 std::runtime_error (或你定义的其他特定异常类型)
    ASSERT_THROW(loadedTree.loadFromJson(test_filename), std::runtime_error);
}

// 测试套件：BehaviorTreeSaveLoadTest
// 测试用例：LoadNonExistentFile
// 目的：测试基本的 I/O 错误处理。验证当尝试从一个不存在的文件加载时，
//       loadFromJson 是否会按预期抛出异常。
TEST_F(BehaviorTreeSaveLoadTest, LoadNonExistentFile) {
    // 1. 确保测试文件不存在 (即使 TearDown 会清理，这里也显式删除以防万一)
    std::remove(test_filename.c_str());
    // 2. 尝试加载一个不存在的文件
    // 断言：加载操作应该抛出 std::runtime_error (或其他指示文件错误的异常)
    ASSERT_THROW(loadedTree.loadFromJson(test_filename), std::runtime_error);
}
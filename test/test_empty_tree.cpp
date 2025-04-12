#include "gtest/gtest.h"
#include "BehaviorTree.h"
#include "SubNode.h" // 引入实现以确保节点类型被注册
#include "NodeRegistrar.h" // 引入实现以确保节点类型被注册
#include <fstream>
#include <cstdio> // 用于 std::remove
#include "nlohmann/json.hpp" // 确保引入 JSON 库

// 该文件包含针对 BehaviorTree 类处于空状态或表示空状态时的测试。

// 测试固件 (Test Fixture)，用于需要临时文件的测试
// 它负责创建测试文件名并在测试结束后清理文件。
class BehaviorTreeFileTest : public ::testing::Test {
protected:
    const std::string test_filename = "empty_tree_test.json"; // 定义测试用的文件名

    // 每个测试用例运行后执行，用于清理资源
    void TearDown() override {
        std::remove(test_filename.c_str()); // 删除测试过程中创建的文件
    }
};

// 测试套件：BehaviorTreeEmptyTest，关注非文件操作的空树状态
// 测试用例：InitialState
// 目的：验证一个新创建的、空的 BehaviorTree 对象的初始状态是否符合预期。
TEST(BehaviorTreeEmptyTest, InitialState) {
    BehaviorTree tree;

    // 断言：空树应该没有根节点 (root应为 nullptr)
    ASSERT_EQ(tree.root, nullptr);
    // 断言：空树的节点映射表 (nodeMap) 应该是空的
    ASSERT_TRUE(tree.nodeMap.empty());
}

// 测试套件：BehaviorTreeFileTest (使用文件操作固件)
// 测试用例：SaveEmptyTree
// 目的：验证将一个完全空的 BehaviorTree 对象保存到 JSON 文件是否能正常工作，
//       并且生成的 JSON 文件内容符合预期的空树格式 (即包含 "root": null)。
TEST_F(BehaviorTreeFileTest, SaveEmptyTree) {
    BehaviorTree tree;

    // 断言：保存空树的操作不应抛出任何异常
    ASSERT_NO_THROW(tree.saveToJson(test_filename));

    // 验证生成的 JSON 文件内容是否正确
    std::ifstream file(test_filename);
    ASSERT_TRUE(file.is_open()); // 确认文件已成功创建并打开
    nlohmann::json json;
    ASSERT_NO_THROW(file >> json); // 确认文件内容是有效的 JSON 且解析无误
    file.close();

    // ===== 修改这里的断言 =====
    // 因为 saveToJson 未修改，空树会输出 "root": null
    // 所以我们期望 "root" 键存在，且其值为 null

    // 确认 "root" 键存在
    ASSERT_TRUE(json.contains("root")) << "Expected JSON to contain 'root' key for an empty tree.";
    // 确认 "root" 键的值是 null
    ASSERT_TRUE(json["root"].is_null()) << "Expected 'root' key value to be null for an empty tree.";
    // ===== 修改结束 =====

    // 检查 nodes 键依然符合预期（存在且为空数组）
    ASSERT_TRUE(json.contains("nodes")); // 应包含 nodes 键
    ASSERT_TRUE(json["nodes"].is_array()); // nodes 的值应为数组
    ASSERT_TRUE(json["nodes"].empty()); // nodes 数组应为空
}

// 测试套件：BehaviorTreeFileTest (使用文件操作固件)
// 测试用例：LoadEmptyTree
// 目的：验证从一个表示空树的 JSON 文件 (包含 "root": null) 加载时，是否能正确地
//       在内存中重建一个空的 BehaviorTree 对象 (因为 loadFromJson 已修正)。
TEST_F(BehaviorTreeFileTest, LoadEmptyTree) {
    BehaviorTree tree;
    // 步骤1：先保存一个空树，生成包含 "root": null 的文件
    ASSERT_NO_THROW(tree.saveToJson(test_filename));

    // 步骤2：创建一个新的 BehaviorTree 对象用于加载
    BehaviorTree loadedTree;
    // 断言：从代表空树的文件加载不应抛出异常 (因为 loadFromJson 已修正以处理 null)
    ASSERT_NO_THROW(loadedTree.loadFromJson(test_filename));

    // 断言：加载后的树应该处于空的初始状态
    ASSERT_EQ(loadedTree.root, nullptr); // 根节点应为 nullptr
    ASSERT_TRUE(loadedTree.nodeMap.empty()); // 节点映射表应为空
}
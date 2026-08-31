# BehaviourTree

> **C++20 行为树库 + Qt 6 编辑器**：一个 28 个文件、~0.05MB 的紧凑原型，提供运行时核心库（基于 `std::shared_ptr` + `std::any` 黑板 + 工厂注册器）、Qt 6.9 + nlohmann::json 实现的图形化编辑器、以及 gtest 测试入口，目标是"能拼能跑能存能读"的极简行为树实现。

## 项目定位 / 背景

BehaviourTree 是一个**早期原型项目**（commit "alpha 1.005 qt test todo"），用 C++20 实现了一个**可序列化的行为树（Behavior Tree）核心库**，配套一个**Qt 6.9 图形编辑器**。它不是工业级的完整方案，但设计模式很清晰：

**核心库层**（`include/` + `src/`，C++20 标准库）：

- **`Define.h`**：极小的"契约"头——`enum class NodeType { Sequence, Selector, Action, Condition, Custom }` + `enum class NodeStatus { Invalid, Success, Failure, Running }` + `using NodeCallback = std::function<NodeStatus(float, Blackboard&)>`
- **`Blackboard.h`**：通用数据共享区——`std::unordered_map<std::string, std::any>` 存任意类型值；提供 `set<T>(key, value)` / `get<T>(key) -> std::optional<T>` / `getRequired<T>(key) -> T`（缺键或类型不匹配抛 `std::runtime_error`）/ `contains` / `remove` / `clear`。`get<T>` 类型不匹配会 `std::cerr` 报错并返回 `std::nullopt` 而不是抛异常
- **`TreeNode.h`**：行为树节点抽象基类——`int id` / `std::string name` / `std::string className` / `NodeType type` / `std::map<std::string, std::string> properties`（字符串属性表，方便编辑器和序列化）/ `std::vector<SP> children`（`SP = std::shared_ptr<TreeNode>`）/ `WP parent`（`WP = std::weak_ptr<TreeNode>`，打破循环引用）；继承 `std::enable_shared_from_this<TreeNode>`，让节点能安全获取 `shared_from_this()`；纯虚 `update(float dt, Blackboard&) -> NodeStatus` + 虚 `reset()` + 虚 `serialize(nlohmann::json&)`
- **`SubNode.h`**：四种内置节点类——`SequenceNode`（按顺序执行子节点，任一 Failure 即终止）/ `SelectorNode`（按顺序尝试，任一 Success 即终止）/ `ActionNode`（执行具体行为，通过 callback）/ `ConditionNode`（条件检查，返回 Success/Failure）。Sequence/Selector 都维护 `size_t runningChildIndex` 记录上次执行位置，实现"恢复执行"语义
- **`NodeFactory.h`**：单例工厂——`NodeFactory::getInstance()` 全局唯一；`registerType(NodeType, className, Creator)` 注册 `(type, className) -> Creator` 映射；`createNode(type, id, name, className)` 实例化（找不到抛 `std::invalid_argument`）
- **`NodeRegistrar.h`**：自动注册辅助——`NodeRegistrar(type, className, typeString, Creator)` 同时注册到工厂和 `NodeTypeRegistry`（enum ↔ string 映射），通过宏 `REGISTER_NODE_TYPE(NodeType, ClassName, "StringName")` 一行完成注册
- **`BehaviorTree.h`**：顶层容器——持有 `SP root` / `std::map<int, SP> nodeMap`（按 id 索引）/ `Blackboard blackboard`；`addNode(parent, type, id, name, className)` 走工厂创建 + 挂到 parent + 入 nodeMap；`removeNode(id)` 摘掉父子关系 + 释放；`saveToJson(filename)` 走 BFS 序列化整棵树 + 节点表；`loadFromJson(filename)` 反向重建；`update(float dt) -> NodeStatus` 委托给 root；`setNodeCallback(id, callback)` 给具体节点装行为

**编辑器层**（`qt_include/` + `qt_src/`，Qt 6.9 + nlohmann::json）：

- **`MainWindow.h`**：主窗口——持有 `BehaviorTree behaviorTree` / `QGraphicsView *graphicsView` / `QGraphicsScene *scene` / `QWidget *nodePalette`（左侧节点面板）/ `PropertyEditorWidget *propertyEditorWidget`（右侧属性编辑器）/ `QTimer *runtimeTimer`（运行 tick 驱动）/ `std::map<int, BehaviorTreeNodeItem*> graphicsNodeMap`（节点 id → 图形 item）/ `std::vector<ConnectionItem*> connectionItems`；`openFile` / `saveFile` 走 JSON 序列化；`addNodeFromPalette` 从面板拖入；`runTick` 定时器触发 `behaviorTree.update()`；override `dragEnterEvent/dragMoveEvent/dropEvent` 支持拖放
- **`BehaviorTreeNodeItem`**：QGraphicsItem 子类——渲染单个节点（按 type 显示不同颜色 + 名字 + 状态）
- **`ConnectionItem`**：节点之间的连线（parent → child）
- **`PaletteItemWidget`**：左侧"节点面板"中的一项
- **`PropertyEditorWidget`**：右侧属性编辑器（编辑 `properties` map）
- **`main_qt.cpp`**：标准 `QApplication` 启动 + `MainWindow::show()`

**测试层**（`test/`，gtest 1.16）：

- `CMakeLists.txt` 通过 `enable_testing()` + `add_executable(RunSaveLoadTest test/test_save_load_cycle.cpp)` + `add_executable(RunEmptyTest test/test_empty_tree.cpp)` 注册两个测试
- 两个测试文件当前都是 **0 字节空文件**（`Length 0`）——TODO 占位，没有实际测试用例

**项目定位**：这是一个**"我手写过一遍行为树"的教学/演示项目**，证明对节点继承、工厂注册、共享所有权、序列化有清晰理解，但**还远没到能作为产品级 AI 库用的程度**——没有可视化连线算法、没有运行时断点 / 单步执行、没有性能埋点、没有脚本化扩展。

## 仓库结构

```
BehaviourTree/
├── CMakeLists.txt                    # cmake 3.30, cxx_std_20, AUTOMOC/AUTORCC/AUTOUIC, Qt 6.9.0 mingw_64, nlohmann_json, GTest
├── vcpkg.json                        # name=behaviourtree, version 1.0.0, deps: nlohmann-json 3.11.3 / vcpkg-cmake / vcpkg-cmake-config / gtest 1.16
├── include/                          # 核心库头文件
│   ├── Define.h                      # NodeType / NodeStatus / NodeCallback
│   ├── Blackboard.h                  # std::any 黑板
│   ├── TreeNode.h                    # 抽象基类
│   ├── SubNode.h                     # Sequence / Selector / Action / Condition
│   ├── NodeFactory.h                 # 单例工厂
│   ├── NodeRegistrar.h               # 自动注册器 + REGISTER_NODE_TYPE 宏
│   └── BehaviorTree.h                # 顶层容器
├── src/                              # 核心库实现
│   ├── BehaviorTree.cpp              # addNode / removeNode / saveToJson / loadFromJson
│   ├── Blackboard.cpp                # (空实现，模板全在头里)
│   ├── TreeNode.cpp                  # serialize + toString/parseNodeType
│   ├── SubNode.cpp                   # 节点 update/reset + REGISTER_NODE_TYPE 调用
│   ├── NodeFactory.cpp               # 工厂实现
│   └── main.cpp                      # （空）
├── qt_include/                       # Qt 编辑器头
│   ├── MainWindow.h
│   ├── BehaviorTreeNodeItem.h
│   ├── ConnectionItem.h
│   ├── PaletteItemWidget.h
│   └── PropertyEditorWidget.h
├── qt_src/                           # Qt 编辑器实现
│   ├── main_qt.cpp                   # QApplication + MainWindow
│   ├── MainWindow.cpp                # setupUi / populateNodePalette / runTick
│   ├── BehaviorTreeNodeItem.cpp
│   ├── ConnectionItem.cpp
│   ├── PaletteItemWidget.cpp
│   └── PropertyEditorWidget.cpp
└── test/                             # gtest 入口（**当前两个文件都是 0 字节占位**）
    ├── test_empty_tree.cpp
    └── test_save_load_cycle.cpp
```

## 技术栈

| 领域 | 选型 | 版本 | 用途 |
|---|---|---|---|
| C++ 标准 | C++20 | – | 核心库 |
| 构建 | CMake ≥ 3.30 | – | 编译 |
| GUI | Qt 6 | 6.9.0 (mingw_64) | 编辑器 |
| GUI 集成 | CMake AUTOMOC / AUTORCC / AUTOUIC | – | 自动处理 Qt meta-object |
| 序列化 | nlohmann::json | 3.11.3 | 行为树 JSON 存读 |
| 测试 | GoogleTest (GTest) | 1.16 | 单元测试 |
| 包管理 | vcpkg | – | 依赖管理 |
| 容器 | `std::shared_ptr` / `std::weak_ptr` / `std::any` / `std::function` / `std::unordered_map` / `std::map` / `std::vector` | C++20 | 节点所有权 + 黑板 + 容器 |
| 模式 | Singleton / Factory / RAII / SFINAE | – | 工厂单例 + 自动注册宏 |

## 核心模块

**`Blackboard`（黑板 / 数据共享）**
`std::unordered_map<std::string, std::any>` 实现：模板 `set<T>` 用 `std::make_any<T>` 装值；`get<T>` 用 `std::any_cast<T>` 提取（类型不匹配 `catch (std::bad_any_cast&)` 返回 `std::nullopt` 并 stderr 报错）；`getRequired<T>` 类型不匹配/缺键直接抛 `std::runtime_error`（适合关键路径）；`contains` / `remove` / `clear` 是常规操作。`std::any` 让黑板能存任意类型，代价是丢失类型安全（取错类型会失败）。

**`TreeNode`（行为树节点抽象基类）**
继承 `std::enable_shared_from_this<TreeNode>`，让节点内部能安全取 `shared_from_this()`；`int id` 是唯一标识（用于 nodeMap 索引）；`std::string className` 是注册名（用于工厂创建）；`NodeType type` + `name` + `properties map` 是元数据；`std::vector<SP> children` + `WP parent` 形成树形结构。`callback` 是 `NodeCallback` 类型，让 ActionNode 装具体行为。`status` 是 protected 状态（`Invalid/Success/Failure/Running`），子类通过 `getStatus()` 读。`serialize` 把节点信息（id/type/name/className/properties + 子节点 id 列表）写到 `nlohmann::json`，子节点序列化在 `BehaviorTree::saveToJson` 里通过 BFS 遍历。

**`SequenceNode` / `SelectorNode`（组合节点）**
两者都维护 `size_t runningChildIndex` 记录上次执行位置，实现"中断后恢复"语义：
- `SequenceNode::update`：从 `runningChildIndex` 开始遍历子节点，返回 `Success` 继续；返回 `Failure` 立即终止并重置 `runningChildIndex = 0`；返回 `Running` 把 `runningChildIndex` 设为当前 i 并返回 `Running`；返回 `Invalid` 当作 `Failure`
- `SelectorNode::update`：对称实现——找到第一个 `Success/Running` 就返回

**`ActionNode` / `ConditionNode`（叶节点）**
- `ActionNode::update`：直接调用 `callback(dt, blackboard)`，没有 callback 时返回 `Success`
- `ConditionNode::update`：同样调 `callback` 把 `Success/Failure` 映射为条件结果

**`NodeFactory`（单例工厂）**
`NodeFactory::getInstance()` 返回 `static NodeFactory instance`；`registerType(type, className, Creator)` 把 `(type, className) -> Creator` 装进 `std::map<std::pair<NodeType, std::string>, Creator>`；`createNode(type, id, name, className)` 找不到抛 `std::invalid_argument`。**关键点**：相同 `NodeType` 不同 `className` 可以注册不同实现——这是"自定义节点"的扩展点。

**`NodeRegistrar` + `REGISTER_NODE_TYPE`（自动注册宏）**
宏定义（`#define REGISTER_NODE_TYPE(NodeType, ClassName, NodeStringName)`）展开成：
```cpp
static NodeRegistrar registrar_##ClassName(
    NodeType,
    #ClassName,
    NodeStringName,
    [](int id, const std::string& name) -> TreeNode::SP {
        return std::make_shared<ClassName>(id, name, #ClassName);
    }
);
```
把"注册到工厂 + 注册 enum↔string 映射 + 写好 Creator"一行搞定。**经典 Meyers singleton + static initializer 模式**，靠全局静态变量在 `main` 之前完成注册。

**`BehaviorTree`（顶层容器）**
`addNode(parent, type, id, name, className)` 走工厂创建节点 + 挂到 parent（如有）或设为 root + 入 `nodeMap`；`removeNode(id)` 摘父子关系 + 删 nodeMap 项 + 如果是 root 就 reset。`saveToJson` 走 BFS：root 入队 → 队首出队 → 写自己的 `json`（id/type/name/className/properties）→ 把 children id 列表入队。`loadFromJson` 反向：先建空节点 → 再建父子关系。`update(dt)` 直接调 `root->update(dt, blackboard)`。

**`MainWindow`（Qt 编辑器）**
`setupUi` 搭主窗口布局（左 nodePalette / 中 graphicsView / 右 propertyEditorWidget / 顶 toolbar）；`populateNodePalette` 从 `REGISTER_NODE_TYPE` 注册的节点类填面板；`addNodeFromPalette` 拖入节点到画布；`onSceneSelectionChanged` 选中节点时把 `properties` 加载到 `propertyEditorWidget`；`runTick` 定时器（`QTimer`）驱动 `behaviorTree.update()` 并刷新 `BehaviorTreeNodeItem` 的状态颜色；`openFile/saveFile` 调 `behaviorTree.saveToJson/loadFromJson`；override `dragEnterEvent/dragMoveEvent/dropEvent` 实现拖放。

## 已完成 / 进行中

- ✅ 核心库完整：节点类 + 黑板 + 工厂 + 顶层容器 + 序列化
- ✅ 4 种内置节点类（Sequence / Selector / Action / Condition）
- ✅ `REGISTER_NODE_TYPE` 自动注册宏
- ✅ JSON 序列化（`saveToJson` / `loadFromJson`）
- ✅ Qt 6.9 编辑器（节点面板 + 画布 + 属性编辑器 + 定时器驱动）
- ✅ gtest 测试入口（CMake 已配）
- ⏳ 测试用例（**两个 .cpp 文件都是 0 字节占位**——TODO 没填）
- ⏳ 节点的运行时断点 / 单步执行
- ⏳ 黑板的可视化检视器
- ⏳ 撤销 / 重做
- ⏳ 子树复用 / import
- ⏳ 性能分析（`update` 每帧耗时）
- ⏳ 与游戏引擎（Unity / Unreal / Godot）的实际集成示例
- ❌ 文档（README 是占位），仅有 commit "alpha 1.005 qt test todo" 说明项目还在 alpha 阶段

## 本地运行 / 构建

```powershell
# 前置：Qt 6.9.0 (mingw_64) 安装在 D:/Qt/6.9.0/mingw_64，vcpkg 配置好 nlohmann-json + gtest

# 构建
cmake -B build -S . -G "Ninja" `
      -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake `
      -DCMAKE_PREFIX_PATH="D:/Qt/6.9.0/mingw_64"
cmake --build build

# 跑测试
ctest --test-dir build --output-on-failure

# 启动编辑器
.\build\BehaviourTreeEditor.exe
```

> ⚠️ `CMakeLists.txt` 硬编码了 `set(Qt6_DIR "D:/Qt/6.9.0/mingw_64/lib/cmake/Qt6")`，需要 Qt 6.9.0 安装在固定路径。`vcpkg.json` 声明了 nlohmann-json 和 gtest 依赖，vcpkg baseline 是 `608d1dbcd6969679f82b1ca6b89d58939c9b228e`。

## 状态

**v1.0.0 alpha 早期原型**。C++20 行为树库 + Qt 6 编辑器骨架已经搭好，能注册节点、能画图、能存读 JSON、能跑 tick 循环。**但还没填任何实际测试**（`test/*.cpp` 都是 0 字节），核心库尚未经过任何 gtest 验证。**`src/main.cpp` 是空文件**——`BehaviourTree` 库本身没有 standalone 入口，只能作为 `SHARED` 库（CMakeLists 里的 `add_library(BehaviourTree SHARED ...)`）被其他项目 link。**属于"设计骨架清晰，但生产可用度还差很远"的 demo 阶段**。推荐使用方式：fork 后当作教学参考或起点实现，不要直接依赖。

## License

未指定 License。所有 commit 都来自 `wudixzy` 个人实验。

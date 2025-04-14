#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QGraphicsSceneMouseEvent>
#include <QSplitter>
#include <iostream>
#include <QApplication>
#include <QTextEdit>

#include "MainWindow.h"
#include "BehaviorTreeNodeItem.h"
#include "ConnectionItem.h"
#include "PropertyEditorWidget.h"
#include "NodeFactory.h"
#include "NodeRegistrar.h"
#include "PaletteItemWidget.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        graphicsView(new QGraphicsView(this)),
        scene(new QGraphicsScene(this)),
        nodePalette(new QWidget(this)),
        propertyEditorWidget(new PropertyEditorWidget(this)),
        runtimeTimer(new QTimer(this)) {
    setupUi();
    setupConnections();
    populateNodePalette();
    graphicsView->setAcceptDrops(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    // 创建中央垂直分割器
    auto *centerVerticalSplitter = new QSplitter(Qt::Vertical, this);

    // 图形视图添加到中央分割器上方
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView->setAcceptDrops(true);
    centerVerticalSplitter->addWidget(graphicsView);

    // 输出窗口添加到中央分割器下方
    auto *outputWindow = new QTextEdit(this);
    outputWindow->setReadOnly(true);
    outputWindow->setPlaceholderText("Output/Log Window");
    centerVerticalSplitter->addWidget(outputWindow);

    // 将中央垂直分割器设置为中央窗口部件
    setCentralWidget(centerVerticalSplitter);

    // 节点面板 Dock，停靠在左侧
    auto *paletteDock = new QDockWidget("Node Palette", this);
    paletteDock->setWidget(nodePalette);
    paletteDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock);

    // 属性编辑器 Dock，停靠在右侧
    auto *propertyDock = new QDockWidget("Property Editor", this);
    propertyDock->setWidget(propertyEditorWidget);
    propertyDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    // 设置中央垂直分割器的初始尺寸比例
    centerVerticalSplitter->setSizes({800, 200}); // GraphicsView 占大部分空间

    // 菜单栏 (保持不变)
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *openAction = fileMenu->addAction("&Open...");
    QAction *saveAction = fileMenu->addAction("&Save As...");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("&Exit");

    QMenu *runMenu = menuBar()->addMenu("&Run");
    QAction *runAction = runMenu->addAction("Tick Once");
    QAction *startAction = runMenu->addAction("Start Timer");
    QAction *stopAction = runMenu->addAction("Stop Timer");

    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(startAction, &QAction::triggered, runtimeTimer, qOverload<>(&QTimer::start));
    connect(stopAction, &QAction::triggered, runtimeTimer, &QTimer::stop);

    setWindowTitle("Behavior Tree Editor");
    resize(1000, 700);
}

void MainWindow::setupConnections() {
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
    connect(runtimeTimer, &QTimer::timeout, this, &MainWindow::runTick);
    runtimeTimer->setInterval(200);
}

void MainWindow::populateNodePalette() {
    if (!nodePalette) {
        nodePalette = new QWidget(this);
    }

    auto *gridLayout = new QGridLayout(nodePalette);
    nodePalette->setLayout(gridLayout);

    auto loadIcon = [](const QString &name) {
        QPixmap icon(":/icons/" + name + ".png");
        if (icon.isNull()) {
            icon = QPixmap(":/icons/default_node.png");
            qWarning() << "Icon not found for" << name;
        }
        return icon;
    };

    int row = 0;
    int col = 0;
    auto addPaletteItem = [&](const QString &nodeTypeName, const QString &nodeClassName) {
        QPixmap icon = loadIcon(nodeClassName.toLower());
        auto *itemWidget = new PaletteItemWidget(nodeTypeName, nodeClassName, icon, nodePalette);
        gridLayout->addWidget(itemWidget, row, col);
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    };

    addPaletteItem("Sequence", "Sequence");
    addPaletteItem("Selector", "Selector");
    addPaletteItem("Condition", "Condition");
    addPaletteItem("Action", "Action");

    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);

    auto *paletteDock = findChild<QDockWidget *>("Node Palette");
    if (paletteDock) {
        paletteDock->setWidget(nodePalette);
    }
}

void MainWindow::openFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Behavior Tree", "",
                                                    "JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }

    try {
        behaviorTree.loadFromJson(filePath.toStdString());
        loadTreeToScene();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error Loading File", QString("Failed to load tree: %1").arg(e.what()));
        clearScene();
        behaviorTree = BehaviorTree();
    }
}

void MainWindow::saveFile() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save Behavior Tree", "",
                                                    "JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }
    try {
        behaviorTree.saveToJson(filePath.toStdString());
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error Saving File", QString("Failed to save tree: %1").arg(e.what()));
    }
}

void MainWindow::clearScene() {
    scene->clear();
    graphicsNodeMap.clear();
    connectionItems.clear();
    propertyEditorWidget->clear();
}

void MainWindow::loadTreeToScene() {
    clearScene();
    auto nodes = behaviorTree.getNodeMap();
    TreeNode::SP root = behaviorTree.getRoot();

    if (!root) {
        return;
    }

    std::map<int, QPointF> nodePositions;
    //todo

    qreal currentY = 50.0;
    for (const auto &[id, nodeSP]: nodes) {
        auto item = new BehaviorTreeNodeItem(nodeSP);
        scene->addItem(item);
        graphicsNodeMap[id] = item;

        item->setPos(100, currentY);
        currentY += 80;

        connect(item, &BehaviorTreeNodeItem::itemMoved, this, [this, item]() {
            updateConnections(item);
        });
    }

    for (const auto &[id, nodeSP]: nodes) {
        BehaviorTreeNodeItem *parentItem = graphicsNodeMap[id];
        for (const auto &childSP: nodeSP->children) {
            if (graphicsNodeMap.count(childSP->id)) {
                BehaviorTreeNodeItem *childItem = graphicsNodeMap[childSP->id];
                auto *conn = new ConnectionItem(parentItem, childItem);
                scene->addItem(conn);
                connectionItems.push_back(conn);
            }
        }
    }

    //todo
}

void MainWindow::updateConnections(BehaviorTreeNodeItem *item) {
    for (ConnectionItem *conn: connectionItems) {
        if (conn->getStart() == item || conn->getEnd() == item) {
            conn->updatePath();
        }
    }
}

void MainWindow::onSceneSelectionChanged() {
    QList<QGraphicsItem *> selected = scene->selectedItems();
    if (selected.size() == 1) {
        auto *item = qgraphicsitem_cast<BehaviorTreeNodeItem *>(selected.first());
        if (item) {
            propertyEditorWidget->setNode(item->getNode()); // Update property editor
            return;
        }
    }
    propertyEditorWidget->clear();
}

void MainWindow::runTick() {
    float dt = runtimeTimer->isActive() ? (runtimeTimer->interval() / 1000.0f)
                                        : 0.1f; // Delta time NOLINT(*-narrowing-conversions)
    NodeStatus rootStatus = behaviorTree.update(dt); // Update the tree

    // Update visualization
    for (auto const &[id, item]: graphicsNodeMap) {
        if (auto node = item->getNode()) { // Check if node pointer is valid
            item->setStatus(node->getStatus()); // Update visual status on the item
        }
    }
}


void MainWindow::addNodeFromPalette(QListWidgetItem *item) {

    //todo
    QMessageBox::information(this, "Add Node", "Add Node functionality not fully implemented yet.");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasFormat("text/plain")) {
        QString nodeClassName = event->mimeData()->text();
        NodeType nodeType = NodeTypeRegistry::fromString(nodeClassName.toStdString());

        static int nodeIdCounter = 1000;

        int newNodeId = nodeIdCounter++;

        auto newNode = NodeFactory::getInstance().createNode(nodeType, newNodeId,
                                                             QString("%1_%2").arg(nodeClassName).arg(
                                                                     newNodeId).toStdString(),
                                                             nodeClassName.toStdString());
        if (!newNode) {
            QMessageBox::critical(this, "Error Creating Node",
                                  QString("Failed to create node of type: %1").arg(nodeClassName));
            return;
        }

        if (!behaviorTree.getRoot()) {
            behaviorTree.addNode(nullptr, nodeType, newNodeId, newNode->name, newNode->className);
        } else {
            behaviorTree.getNodeMap()[newNodeId] = newNode;
        }

        auto *item = new BehaviorTreeNodeItem(newNode);
        scene->addItem(item);
        graphicsNodeMap[newNodeId] = item;

        QPointF scenePos = graphicsView->mapToScene(event->pos());
        item->setPos(scenePos);

        connect(item, &BehaviorTreeNodeItem::itemMoved, this, [this, item]() {
            updateConnections(item);
        });
    } else {
        event->ignore();
    }
}
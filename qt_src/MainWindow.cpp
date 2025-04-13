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

#include "MainWindow.h"
#include "BehaviorTreeNodeItem.h"
#include "ConnectionItem.h"
#include "PropertyEditorWidget.h"
#include "NodeFactory.h"
#include "NodeRegistrar.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        graphicsView(new QGraphicsView(this)),
        scene(new QGraphicsScene(this)),
        nodePalette(new QListWidget(this)),
        propertyEditorWidget(new PropertyEditorWidget(this)),
        runtimeTimer(new QTimer(this)) {
    setupUi();
    setupConnections();
    populateNodePalette();
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUi() {
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    QSplitter *viewPropertySplitter = new QSplitter(Qt::Vertical, this);
    viewPropertySplitter->addWidget(graphicsView);
    viewPropertySplitter->addWidget(propertyEditorWidget);
    viewPropertySplitter->setSizes({400, 150});

    mainSplitter->addWidget(viewPropertySplitter);
    setCentralWidget(mainSplitter);

    QDockWidget *paletteDock = new QDockWidget("Node Palette", this);
    paletteDock->setWidget(nodePalette);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock);

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
    nodePalette->addItem("Sequence");
    nodePalette->addItem("Selector");
    nodePalette->addItem("Condition");
    nodePalette->addItem("Action");
    nodePalette->setDragEnabled(true);
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
        BehaviorTreeNodeItem *item = qgraphicsitem_cast<BehaviorTreeNodeItem *>(selected.first());
        if (item) {
            propertyEditorWidget->setNode(item->getNode()); // Update property editor
            return;
        }
    }
    propertyEditorWidget->clear();
}

void MainWindow::runTick() {
    float dt = runtimeTimer->isActive() ? (runtimeTimer->interval() / 1000.0f) : 0.1f; // Delta time
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
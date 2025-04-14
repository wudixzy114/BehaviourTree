#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QListWidget>
#include <QWidget>
#include <QTimer>
#include "BehaviorTree.h"

class QGraphicsScene;

class BehaviorTreeNodeItem;

class ConnectionItem;

class PropertyEditorWidget;

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void openFile();

    void saveFile();

    void addNodeFromPalette(QListWidgetItem *item);

    void onSceneSelectionChanged();

    void runTick();

private:
    void setupUi();

    void setupConnections();

    void populateNodePalette();

    void loadTreeToScene();

    void clearScene();

    void updateConnections(BehaviorTreeNodeItem *item);

    BehaviorTree behaviorTree;

    QGraphicsView *graphicsView;
    QGraphicsScene *scene;

    QWidget *nodePalette;
    PropertyEditorWidget *propertyEditorWidget;

    QTimer *runtimeTimer;

    std::map<int, BehaviorTreeNodeItem *> graphicsNodeMap;
    std::vector<ConnectionItem *> connectionItems;

protected:

    void dragEnterEvent(QDragEnterEvent *event) override;

    void dragMoveEvent(QDragMoveEvent *event) override;

    void dropEvent(QDropEvent *event) override;
};


#endif //MAINWINDOW_H

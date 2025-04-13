#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // --- Crucial: Register Node Types BEFORE creating the editor ---
    // This ensures the factory and registry are populated.
    // Usually done via static initializers triggered by linking SubNode.cpp,
    // but explicit calls might be needed if static linking behaves differently.
    // Example: If REGISTER_NODE_TYPE wasn't enough, you might need dummy function calls.
    // --- End Node Registration ---
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
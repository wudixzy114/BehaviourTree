#ifndef PROPERTYEDITORWIDGET_H
#define PROPERTYEDITORWIDGET_H

#include <QWidget>
#include <map>
#include <string>
#include <memory>

namespace Ui {
    class PropertyEditorWidget;
}

class QFormLayout;

class QLineEdit;

class QTextEdit;

class TreeNode;

class PropertyEditorWidget : public QWidget {
Q_OBJECT

public:
    explicit PropertyEditorWidget(QWidget *parent = nullptr);

    ~PropertyEditorWidget();


    void setNode(std::shared_ptr<TreeNode> node);

    void clear();

private slots:

    void onNameChanged(const QString &text);

private:
    void updateUiFromNode();

    void clearLayout(QLayout *layout);

    std::shared_ptr<TreeNode> currentNode;

    QFormLayout *layout;
    QLineEdit *idLabel;
    QLineEdit *nameEdit;
    QLineEdit *typeLabel;
    QLineEdit *classLabel;
    QWidget *propertiesWidget;
    QFormLayout *propertiesLayout;
};


#endif //PROPERTYEDITORWIDGET_H

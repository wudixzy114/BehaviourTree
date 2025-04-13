// clazy:excludeall=connect-non-signal
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <iostream>
#include <utility>

#include "PropertyEditorWidget.h"
#include "TreeNode.h"
#include "NodeRegistrar.h"

PropertyEditorWidget::PropertyEditorWidget(QWidget *parent) : QWidget(parent),
                                                              layout(new QFormLayout(this)),
                                                              idLabel(new QLineEdit(this)),
                                                              nameEdit(new QLineEdit(this)),
                                                              typeLabel(new QLineEdit(this)),
                                                              classLabel(new QLineEdit(this)),
                                                              propertiesWidget(new QWidget(this)),
                                                              propertiesLayout(new QFormLayout(propertiesWidget)) {
    idLabel->setReadOnly(true);
    typeLabel->setReadOnly(true);
    classLabel->setReadOnly(true);


    layout->addRow("ID:", idLabel);
    layout->addRow("Name", nameEdit);
    layout->addRow("Type", typeLabel);
    layout->addRow("Class", classLabel);
    layout->addRow("Properties", propertiesWidget);

    setLayout(layout);
    clear();

    connect(nameEdit, &QLineEdit::textChanged, this, &PropertyEditorWidget::onNameChanged);

}

PropertyEditorWidget::~PropertyEditorWidget() = default;

void PropertyEditorWidget::setNode(std::shared_ptr<TreeNode> node) {
    currentNode = std::move(node);
    updateUiFromNode();
    setEnabled(currentNode != nullptr);
}

void PropertyEditorWidget::clear() {
    if (currentNode) {
        currentNode->reset();
    }
    idLabel->clear();
    nameEdit->clear();
    typeLabel->clear();
    classLabel->clear();
    clearLayout(propertiesLayout);
    setEnabled(false);
}

void PropertyEditorWidget::updateUiFromNode() {
    if (!currentNode) {
        clear();
        return;
    }

    nameEdit->blockSignals(true);

    idLabel->setText(QString::number(currentNode->id));
    nameEdit->setText(QString::fromStdString(currentNode->name));
    typeLabel->setText(QString::fromStdString(NodeTypeRegistry::toString(currentNode->type)));
    classLabel->setText(QString::fromStdString(currentNode->className));

    clearLayout(propertiesLayout);
    for (const auto &[key, value]: currentNode->properties) {
        QLineEdit *propEdit = new QLineEdit(QString::fromStdString(value), this);
        propertiesLayout->addRow(QString::fromStdString(key) + ":", propEdit);
        connect(propEdit, &QLineEdit::textChanged, this, [this, key = key](const QString &text) {
            if (currentNode) {
                currentNode->properties[key] = text.toStdString();
            }
        });
    }

    nameEdit->blockSignals(false);
}

void PropertyEditorWidget::onNameChanged(const QString &text) {
    if (currentNode) {
        currentNode->name = text.toStdString();
    }
}

void PropertyEditorWidget::clearLayout(QLayout *layout) {
    if (!layout) {
        return;
    }
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}
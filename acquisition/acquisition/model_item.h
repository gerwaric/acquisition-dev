#pragma once

#include <QVariantList>

#include <vector>

// https://doc.qt.io/qt-6/qtwidgets-itemviews-simpletreemodel-example.html

class ModelItem
{
public:
    explicit ModelItem(QVariantList data, ModelItem* parentItem = nullptr);

    void appendChild(std::unique_ptr<ModelItem>&& child);

    ModelItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    ModelItem* parentItem();

private:
    std::vector<std::unique_ptr<ModelItem>> m_childItems;
    QVariantList m_itemData;
    ModelItem* m_parentItem;
};
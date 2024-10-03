#include <acquisition/model_item.h>

ModelItem::ModelItem(QVariantList data, ModelItem* parent)
    : m_itemData(std::move(data)), m_parentItem(parent)
{}

void ModelItem::appendChild(std::unique_ptr<ModelItem>&& child)
{
    m_childItems.push_back(std::move(child));
}

ModelItem* ModelItem::child(int row)
{
    return row >= 0 && row < childCount() ? m_childItems.at(row).get() : nullptr;
}

int ModelItem::childCount() const
{
    return int(m_childItems.size());
}

int ModelItem::row() const
{
    if (m_parentItem == nullptr) {
        return 0;
    };

    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
        [this](const std::unique_ptr<ModelItem>& treeItem) {
            return treeItem.get() == this;
        });

    if (it != m_parentItem->m_childItems.cend()) {
        return std::distance(m_parentItem->m_childItems.cbegin(), it);
    };
    Q_ASSERT(false); // should not happen
    return -1;
}

int ModelItem::columnCount() const
{
    return int(m_itemData.count());
}

QVariant ModelItem::data(int column) const
{
    return m_itemData.value(column);
}

ModelItem* ModelItem::parentItem()
{
    return m_parentItem;
}

#include <acquisition/tree_model.h>


TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_character_root(m_root.createChild("CHARACTERS"))
    , m_stash_root(m_root.createChild("STASH TABS"))
{
}

TreeModel::~TreeModel() {}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    const TreeNode* parentNode = getNode(parent);
    const TreeNode* childNode = parentNode->child(row);
    return childNode ? createIndex(row, column, childNode) : QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    const TreeNode* childNode = getNode(index);
    if (childNode != &m_root) {
        const TreeNode* parentNode = childNode->parent();
        if (parentNode != &m_root) {
            return createIndex(parentNode->row(), 0, parentNode);
        };
    };
    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    return getNode(parent)->childCount();
 }

int TreeModel::columnCount(const QModelIndex& parent) const
{
    return getNode(parent)->columnCount();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    };
    return getNode(index)->data(index.column());
}

void TreeModel::appendCharacter(const poe_api::Character& character)
{
    QModelIndex p = createIndex(0, 0, &m_character_root);
    const int k = m_character_root.childCount();
    beginInsertRows(p, k, k);
    m_character_root.addChild(character);
    endInsertRows();
}

void TreeModel::appendStash(const poe_api::StashTab& stash)
{
    QModelIndex p = createIndex(0, 0, &m_stash_root);
    const int k = m_stash_root.childCount();
    beginInsertRows(p, k, k);
    m_stash_root.addChild(stash);
    endInsertRows();
}

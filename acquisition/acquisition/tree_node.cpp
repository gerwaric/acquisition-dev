#include <acquisition/tree_node.h>

long long unsigned TreeNode::N = 0;

// Primary root node
TreeNode::TreeNode()
    : m_id(++N)
    , m_type(NodeType::Root)
    , m_parent(nullptr) {}

// Secondary root node
TreeNode::TreeNode(const QString& name, TreeNode* parent)
    : m_id(++N)
    , m_type(NodeType::Root)
    , m_parent(parent)
    , m_name(name) {}

// Item node
TreeNode::TreeNode(const poe_api::Item& item, TreeNode* parent)
    : m_id(++N)
    , m_type(NodeType::Item)
    , m_parent(parent)
    , m_item(&item)
{
    if (item.socketedItems) {
        addChildren(item.socketedItems.value());
    };
}

// StashTab node
TreeNode::TreeNode(const poe_api::StashTab& stash, TreeNode* parent)
    : m_id(++N), m_type(NodeType::StashTab)
    , m_parent(parent)
    , m_stash(&stash)
{
    if (stash.items) {
        addChildren(stash.items.value());
    };
    if (stash.children) {
        addChildren(stash.children.value());
    };
}

// Character node
TreeNode::TreeNode(const poe_api::Character& character, TreeNode* parent)
    : m_id(++N), m_type(NodeType::Character)
    , m_parent(parent)
    , m_character(&character)
{
    if (character.equipment) {
        addCollection("Equipment", character.equipment.value());
    };
    if (character.inventory) {
        addCollection("Inventory", character.inventory.value());
    };
    if (character.rucksack) {
        addCollection("Rucksack", character.rucksack.value());
    };
    if (character.jewels) {
        addCollection("Jewels", character.jewels.value());
    };
}

// Item collection node (e.g. one character's inventory or equipment)
TreeNode::TreeNode(const QString& collection, const std::vector<poe_api::Item>& items, TreeNode* parent)
    : m_id(++N), m_type(NodeType::Collection)
    , m_parent(parent)
    , m_name(collection)
{
    addChildren(items);
}

TreeNode::~TreeNode() {};

const TreeNode* TreeNode::parent() const
{
    return m_parent;
}

const TreeNode* TreeNode::child(int row) const
{
    if ((row >= 0) && (row < m_children.size())) {
        return m_children[row].get();
    } else {
        return nullptr;
    };
}

int TreeNode::childCount() const
{
    return static_cast<int>(m_children.size());
}

int TreeNode::row() const
{
    return m_parent ? m_parent->rowOfChild(this) : 0;
}

int TreeNode::columnCount() const
{
    return 2;
    switch (m_type) {
    case NodeType::Item:
        return 2;
    case NodeType::StashTab:
    case NodeType::Character:
    case NodeType::Collection:
        return 1;
    case NodeType::Root:
    default:
        return 0;
    };
}

QVariant TreeNode::data(int column) const
{
    switch (m_type) {
    case NodeType::Item:
        switch (column) {
        case 0:
            return m_item->typeLine;
        case 1:
            return m_item->name;
        default:
            return QVariant();
        }
    case NodeType::StashTab:
        return (column == 0) ? m_stash->id : QVariant();
    case NodeType::Character:
        return (column == 0) ? m_character->name : QVariant();
    case NodeType::Collection:
    case NodeType::Root:
        return (column == 0) ? m_name : QVariant();
    default:
        return QVariant();
    };
};

TreeNode& TreeNode::createChild(const QString& name)
{
    return *m_children.emplace_back(std::make_unique<TreeNode>(name, this));
}

void TreeNode::addChild(const poe_api::Character& character)
{
    m_children.emplace_back(std::make_unique<TreeNode>(character, this));
}

void TreeNode::addChild(const poe_api::StashTab& stash)
{
    m_children.emplace_back(std::make_unique<TreeNode>(stash, this));
}

void TreeNode::addChildren(const std::vector<poe_api::Item>& items)
{
    for (const auto& item : items) {
        m_children.emplace_back(std::make_unique<TreeNode>(item, this));
    };
}

void TreeNode::addChildren(const std::vector<poe_api::StashTab>& stashes)
{
    for (const auto& stash : stashes) {
        m_children.emplace_back(std::make_unique<TreeNode>(stash, this));
    };
}

void TreeNode::addChildren(const std::vector<poe_api::Character>& characters)
{
    for (const auto& character : characters) {
        m_children.emplace_back(std::make_unique<TreeNode>(character, this));
    };
}

void TreeNode::addCollection(const QString& name, const std::vector<poe_api::Item>& items)
{
    m_children.emplace_back(std::make_unique<TreeNode>(name, items, this));
}

int TreeNode::rowOfChild(const TreeNode* child) const {
    for (int i = 0; i < m_children.size(); ++i) {
        if (m_children[i].get() == child) {
            return i;
        };
    };
    return -1;
}
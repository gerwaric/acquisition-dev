#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/item.h>
#include <acquisition/api_types/stash_tab.h>

#include <QString>
#include <QVariant>

#include <vector>

class TreeNode {
public:

    // Only root nodes can be explicitly constructed.
    explicit TreeNode();

    // Secondary root nodes
    explicit TreeNode(const QString& name, TreeNode* parent);

    // Item node
    explicit TreeNode(const poe_api::Item& item, TreeNode* parent);

    // StashTab node
    explicit TreeNode(const poe_api::StashTab& stash, TreeNode* parent);

    // Character node
    explicit TreeNode(const poe_api::Character& character, TreeNode* parent);

    // Item collection node (e.g. one character's inventory or equipment)
    explicit TreeNode(const QString& collection, const std::vector<poe_api::Item>& items, TreeNode* parent);

    ~TreeNode();

    const TreeNode* parent() const;
    const TreeNode* child(int row) const;
    int childCount() const;
    int row() const;
    int columnCount() const;
    QVariant data(int column) const;

    TreeNode& createChild(const QString& name);
    void addChild(const poe_api::Character& character);
    void addChild(const poe_api::StashTab& stash);

    void addChildren(const std::vector<poe_api::Item>& items);
    void addChildren(const std::vector<poe_api::StashTab>& stashes);
    void addChildren(const std::vector<poe_api::Character>& characters);

private:

    void addCollection(const QString& name, const std::vector<poe_api::Item>& items);
    int rowOfChild(const TreeNode* child) const;

    enum class NodeType { Item, StashTab, Character, Collection, Root };
    enum class ItemType { None, Equipable, Stackable, Other };

    static long unsigned s_node_count;

    const long unsigned m_id;
    const NodeType m_node_type;
    const TreeNode* m_parent;
    std::vector<std::unique_ptr<TreeNode>> m_children;

    const QString m_name;
    const poe_api::Character* m_character{ nullptr };
    const poe_api::StashTab* m_stash{ nullptr };
    const poe_api::Item* m_item{ nullptr };
    const ItemType m_item_type;

    void* m_item_timestamp;
    void* m_item_pseudomods;
    void* m_item_buyout;

};
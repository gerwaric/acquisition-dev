#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/stash_tab.h>

#include <QAbstractItemModel>

class ItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(ItemModel);

    // Returns the index of the item in the model specified by the given row, column and parent index.
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    // Returns the parent of the model item with the given index. If the item has no parent, an invalid QModelIndex is returned
    QModelIndex parent(const QModelIndex& index) const override;

    // Returns the number of rows under the given parent. When the parent is valid it means that rowCount is returning the number of children of parent.
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // Returns the number of columns for the children of the given parent.
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // Returns the data stored under the given role for the item referred to by the index.
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    
    // Returns true if parent has any children; otherwise returns false.
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

    // Inserts count rows into the model before the given row. Items in the new row will be children of the item represented by the parent model index.
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // Removes count rows starting with the given row under parent parent from the model.
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:

    std::vector<poe_api::StashTab> stash_tabs_;
    std::vector<poe_api::Character> characters_;

};
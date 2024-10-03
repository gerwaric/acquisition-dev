/*
    Copyright 2024 Gerwaric

    This file is part of Acquisition.

    Acquisition is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Acquisition is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Acquisition.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <acquisition/rate_limit/rate_limit_status_dialog.h>

#include <acquisition/rate_limit/rate_limiter.h>
#include <acquisition/rate_limit/rate_limit_policy.h>
#include <acquisition/utils/utils.h>

#include <QCoreApplication>
#include <QPushButton>
#include <QSizePolicy>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QVBoxLayout>

RateLimitStatusDialog::RateLimitStatusDialog(RateLimiter& limiter)
{
    setSizeGripEnabled(true);

    // The spaces at the end of the header labels are intentional. Without them,
    // Qt was cutting off the last character or two. I could not figure out how
    // to avoid this.
    const QStringList columns = {
        "Policy / Rule  ",
        "Queue  ",
        "Hits / Limit  ",
        "Period (s)  ",
        "Timeout(s)  ",
        "Status  " };

    setWindowTitle("Acquisition : Rate Limit Status Window");

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeWidget->setColumnCount(columns.size());
    m_treeWidget->setHeaderLabels(columns);
    m_treeWidget->setFrameShape(QFrame::StyledPanel);
    m_treeWidget->setFrameShadow(QFrame::Sunken);
    m_treeWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    m_treeWidget->setSortingEnabled(false);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    for (int i = 0; i < m_treeWidget->columnCount(); ++i) {
        m_treeWidget->resizeColumnToContents(i);
    };

    m_statusLabel = new QLabel(this);
    m_statusLabel->setText("Rate limit status: UNKNOWN");
    m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(15, 15, 15, 15);
    m_layout->addWidget(m_treeWidget);
    m_layout->addWidget(m_statusLabel);

    resize(600, 400);
    setMinimumWidth(600);
    setMinimumHeight(400);

    connect(&limiter, &RateLimiter::PolicyUpdate, this, &RateLimitStatusDialog::onPolicyUpdate);
    connect(&limiter, &RateLimiter::QueueUpdate, this, &RateLimitStatusDialog::onQueueUpdate);
    connect(&limiter, &RateLimiter::Paused, this, &RateLimitStatusDialog::onPause);
}

void RateLimitStatusDialog::onPolicyUpdate(const RateLimitPolicy& policy)
{
    QString queued_state;

    // Look for an existing top-level item and remove it.
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        if (item->text(0) == policy.name()) {
            queued_state = item->text(1);
            delete m_treeWidget->takeTopLevelItem(i);
            break;
        };
    };

    // Create a new top-level item.
    QTreeWidgetItem* policy_item = new QTreeWidgetItem(m_treeWidget);
    policy_item->setFlags(Qt::ItemIsEnabled);
    policy_item->setText(0, policy.name());
    policy_item->setText(1, queued_state);
    for (const auto& rule : policy.rules()) {
        for (const auto& item : rule.items()) {
            QTreeWidgetItem* rule_item = new QTreeWidgetItem(policy_item);
            rule_item->setFlags(Qt::ItemIsEnabled);
            rule_item->setText(0, QString("%1 (%2s)").arg(rule.name()).arg(item.limit().period()));
            rule_item->setText(2, QString("%1 / %2").arg(
                QString::number(item.state().hits()),
                QString::number(item.limit().hits())));
            rule_item->setText(3, QString::number(item.limit().period()));
            rule_item->setText(4, QString::number(item.limit().restriction()));
            rule_item->setText(5, utils::QtEnumToString(item.status()));
            rule_item->setExpanded(true);
        };
    };
    policy_item->setExpanded(true);

    // Sort the rows by policy name.
    m_treeWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);

    // Resize the columns.
    for (int i = 0; i < m_treeWidget->columnCount(); ++i) {
        m_treeWidget->resizeColumnToContents(i);
    };
}

void RateLimitStatusDialog::onQueueUpdate(const QString& policy_name, int queued_requests) {
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        if (item->text(0) == policy_name) {
            if (queued_requests > 0) {
                item->setText(1, QString::number(queued_requests));
            } else {
                item->setText(1, "");
            };
            break;
        };
    };
}

void RateLimitStatusDialog::onPause(int pause, const QString& policy)
{
    const QString message = (pause <= 0)
        ? "Not rate limited"
        : QString("Paused for %1 seconds due to %2").arg(QString::number(pause), policy);
    m_statusLabel->setText(message);
}

#pragma once

#include <QDialog>

#include <QLabel>
#include <QPushButton>

class OAuthToken;

class AuthenticationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AuthenticationDialog();
signals:
    void requested();
public slots:
    void setToken(const OAuthToken& token);
private:
    QLabel m_upper_label;
    QPushButton m_authenticate_button;
    QPushButton m_offline_button;
    QPushButton m_continue_button;
    bool m_authenticated{ false };
};
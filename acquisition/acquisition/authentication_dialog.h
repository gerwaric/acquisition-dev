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
    void complete();
public slots:
    void setToken(const OAuthToken& token);
private:
    QLabel m_upper_label;
    QPushButton m_authenticate_button;
    QPushButton m_proceed_button;
};
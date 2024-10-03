#include <acquisition/authentication_dialog.h>

#include <acquisition/oauth/oauth_token.h>

#include <QCheckBox>
#include <QLineEdit>
#include <QVBoxLayout>

AuthenticationDialog::AuthenticationDialog()
    : QDialog(nullptr)
    , m_upper_label(this)
    , m_authenticate_button(this)
    , m_proceed_button(this)
{
    m_upper_label.setText("You must authenticate to proceed");
    m_authenticate_button.setText("Authenticate");
    m_authenticate_button.setEnabled(true);
    m_proceed_button.setText("Proceed");
    m_proceed_button.setEnabled(false);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(&m_upper_label);
    layout->addWidget(&m_authenticate_button);
    layout->addWidget(&m_proceed_button);

    connect(&m_authenticate_button, &QPushButton::clicked, this,
        [this]() {
            m_authenticate_button.setText("Authenticating");
            m_authenticate_button.setEnabled(false);
            m_proceed_button.setEnabled(false);
            emit requested();
        });

    connect(&m_proceed_button, &QPushButton::clicked, this,
        [this]() {
            emit complete();
            close();
        });
}

void AuthenticationDialog::setToken(const OAuthToken& token)
{
    const QString username = token.username;
    const QString expiration = token.expiration.value_or(QDateTime()).toString();
    m_upper_label.setText("You are authenticated as \"" + username + "\" until " + expiration);
    m_authenticate_button.setText("Re-authenticate");
    m_authenticate_button.setEnabled(true);
    m_proceed_button.setEnabled(true);
    activateWindow();
    raise();
}

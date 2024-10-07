#include <acquisition/authentication_dialog.h>

#include <acquisition/oauth/oauth_token.h>

#include <QCheckBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QStandardPaths>
#include <QVBoxLayout>

AuthenticationDialog::AuthenticationDialog()
    : QDialog(nullptr)
    , m_upper_label(this)
    , m_authenticate_button(this)
    , m_continue_button(this)
{
    m_upper_label.setText("You are not authenticated.");
    m_authenticate_button.setText("Authenticate");
    m_offline_button.setText("Continue Offline");
    m_continue_button.setText("Proceed");
    m_continue_button.setDefault(true);
    m_continue_button.setEnabled(false);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(&m_upper_label);
    layout->addWidget(&m_authenticate_button);
    layout->addWidget(&m_offline_button);
    layout->addWidget(&m_continue_button);

    connect(&m_authenticate_button, &QPushButton::clicked, this,
        [this]() {
            m_authenticate_button.setText("Authenticating");
            m_authenticate_button.setEnabled(false);
            emit requested();
        });

    connect(&m_offline_button, &QPushButton::clicked, this,
        [this]() {
            const QString filename = QFileDialog::getExistingDirectory(this,
                "Select a user directory",
                QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        });

    connect(&m_continue_button, &QPushButton::clicked, this,
        [this]() {
            if (m_authenticated) {
                accept();
            } else {
                reject();
            };
        });
}

void AuthenticationDialog::setToken(const OAuthToken& token)
{
    const QString username = token.username;
    const QString expiration = token.expiration.value_or(QDateTime()).toString();
    m_upper_label.setText("You are authenticated as \"" + username + "\" until " + expiration);
    m_authenticate_button.setText("Re-authenticate");
    m_authenticate_button.setEnabled(true);
    m_continue_button.setEnabled(true);
    m_authenticated = true;
    activateWindow();
    raise();
}

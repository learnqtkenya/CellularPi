#ifndef MODEMDBUSMANAGER_H
#define MODEMDBUSMANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <memory>
#include <QMutex>

class QTimer;
class QDBusPendingCallWatcher;

class ModemDBusManager : public QObject {
    Q_OBJECT
public:
    explicit ModemDBusManager(QObject* parent = nullptr);
    ~ModemDBusManager();

    bool initialize();
    void sendSMS(const QString& phoneNumber, const QString& message);

signals:
    void smsResult(bool success);
    void logInfo(const QString& message);
    void logError(const QString& message);

private:
    struct DBusInterfaces {
        QString messagingPath;
        std::unique_ptr<QDBusInterface> messaging;
        bool initialized{false};
    };

    static constexpr int DBUS_INIT_RETRY_INTERVAL = 2000;
    static constexpr int DBUS_INIT_MAX_RETRIES = 100;
    static constexpr int MAX_RETRY_ATTEMPTS = 3;
    static constexpr int RETRY_DELAY_MS = 1000;

    QDBusConnection m_dbusConnection;
    DBusInterfaces m_dbusInterfaces;
    std::unique_ptr<QTimer> m_dbusInitTimer;
    int m_dbusInitRetryCount{0};
    QMutex m_dbusInitMutex;

    bool initializeDBusInterfaces();
    bool ensureValidInterfaces();
    bool shouldRetryOperation(const QDBusError& error) const;
    void scheduleRetry(const QVariantMap& properties, int retryCount);
    void handleCreateSMSResponse(const QDBusPendingCallWatcher* watcher,
                                 const QVariantMap& properties,
                                 int retryCount);
    void handleSendSMSResponse(const QDBusPendingCallWatcher* watcher,
                               const QVariantMap& properties,
                               int retryCount);

private slots:
    void onModemManagerServiceChanged(bool available);
};

#endif// MODEMDBUSMANAGER_H

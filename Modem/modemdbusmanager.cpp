#include "modemdbusmanager.h"
#include <QTimer>
#include <QDBusServiceWatcher>
#include <QDBusPendingCallWatcher>
#include <QDBusArgument>
#include <QMutexLocker>
#include <QDBusPendingReply>

ModemDBusManager::ModemDBusManager(QObject* parent)
    : QObject(parent)
    , m_dbusConnection(QDBusConnection::systemBus())
{
    m_dbusInitTimer = std::make_unique<QTimer>(this);
    m_dbusInitTimer->setInterval(DBUS_INIT_RETRY_INTERVAL);
    m_dbusInitTimer->setSingleShot(false);

    connect(m_dbusInitTimer.get(), &QTimer::timeout, this, [this]() {
        if (!m_dbusInterfaces.initialized) {
            if (m_dbusInitRetryCount < DBUS_INIT_MAX_RETRIES) {
                m_dbusInitRetryCount++;
                emit logInfo(QString("[Modem] Retrying D-Bus initialization (attempt %1 of %2)")
                                 .arg(m_dbusInitRetryCount)
                                 .arg(DBUS_INIT_MAX_RETRIES));
                initializeDBusInterfaces();
            } else {
                m_dbusInitTimer->stop();
                emit logError("[Modem] Failed to initialize D-Bus interfaces after maximum retries");
            }
        } else {
            m_dbusInitTimer->stop();
        }
    });

    auto* watcher = new QDBusServiceWatcher(
        "org.freedesktop.ModemManager1",
        m_dbusConnection,
        QDBusServiceWatcher::WatchForRegistration |
            QDBusServiceWatcher::WatchForUnregistration,
        this
        );

    connect(watcher, &QDBusServiceWatcher::serviceRegistered,
            this, [this] { onModemManagerServiceChanged(true); });

    connect(watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, [this] { onModemManagerServiceChanged(false); });

    // Start initial initialization attempt
    QMetaObject::invokeMethod(this, [this] {
            if (!initializeDBusInterfaces()) {
                emit logInfo("[Modem] Starting D-Bus initialization retry timer");
                m_dbusInitTimer->start();
            }
        }, Qt::QueuedConnection);
}

ModemDBusManager::~ModemDBusManager() = default;

bool ModemDBusManager::initialize()
{
    if (!initializeDBusInterfaces()) {
        emit logInfo("[Modem] Starting D-Bus initialization retry timer");
        m_dbusInitTimer->start();
        return false;
    }
    return true;
}

void ModemDBusManager::sendSMS(const QString &phoneNumber, const QString &message)
{
    if (!ensureValidInterfaces()) {
        emit logError("[Modem] Failed to initialize D-Bus interfaces");
        emit smsResult(false);
        return;
    }

    QVariantMap properties;
    properties["number"] = phoneNumber;
    properties["text"] = message;

    QDBusPendingCall createCall = m_dbusInterfaces.messaging->asyncCall("Create", properties);
    auto* createWatcher = new QDBusPendingCallWatcher(createCall, this);

    connect(createWatcher, &QDBusPendingCallWatcher::finished,
            this, [this, createWatcher, properties]() {
                handleCreateSMSResponse(createWatcher, properties, 0);
                createWatcher->deleteLater();
            });
}

bool ModemDBusManager::initializeDBusInterfaces()
{
    QMutexLocker locker(&m_dbusInitMutex);

    if (m_dbusInterfaces.initialized) {
        return true;
    }

    // Reset any existing interface
    m_dbusInterfaces.messaging.reset();
    m_dbusInterfaces.messagingPath.clear();
    m_dbusInterfaces.initialized = false;

    // Create temporary manager interface
    QDBusInterface manager("org.freedesktop.ModemManager1",
                           "/org/freedesktop/ModemManager1",
                           "org.freedesktop.DBus.ObjectManager",
                           m_dbusConnection);

    if (!manager.isValid()) {
        emit logError("[Modem] Failed to create ModemManager interface");
        return false;
    }

    // Get managed objects
    QDBusMessage reply = manager.call("GetManagedObjects");
    if (reply.type() != QDBusMessage::ReplyMessage) {
        emit logError("[Modem] Failed to get ModemManager objects: " + reply.errorMessage());
        return false;
    }

    // Parse the reply
    const QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::MapType) {
        emit logError("[Modem] Invalid response format from ModemManager");
        return false;
    }

    // Look for messaging interface
    bool found = false;
    arg.beginMap();
    while (!arg.atEnd()) {
        QString path;
        QVariantMap interfaces;
        arg.beginMapEntry();
        arg >> path >> interfaces;
        arg.endMapEntry();

        if (interfaces.contains("org.freedesktop.ModemManager1.Modem.Messaging")) {
            m_dbusInterfaces.messagingPath = path;
            found = true;
            break;
        }
    }
    arg.endMap();

    if (!found) {
        emit logError("[Modem] No messaging-capable modem found");
        return false;
    }

    // Create messaging interface
    try {
        m_dbusInterfaces.messaging = std::make_unique<QDBusInterface>(
            "org.freedesktop.ModemManager1",
            m_dbusInterfaces.messagingPath,
            "org.freedesktop.ModemManager1.Modem.Messaging",
            m_dbusConnection,
            this
            );

        if (!m_dbusInterfaces.messaging->isValid()) {
            emit logError("[Modem] Failed to create messaging interface");
            return false;
        }

        m_dbusInterfaces.initialized = true;
        emit logInfo("[Modem] D-Bus interfaces initialized successfully");
        m_dbusInitTimer->stop();  // Stop retry timer on success
        m_dbusInitRetryCount = 0; // Reset retry count
        return true;

    } catch (const std::exception& e) {
        emit logError(QString("[Modem] Exception during interface creation: %1").arg(e.what()));
        return false;
    }
}

bool ModemDBusManager::ensureValidInterfaces()
{
    if (!m_dbusInterfaces.initialized ||
        !m_dbusInterfaces.messaging ||
        !m_dbusInterfaces.messaging->isValid()) {
        emit logInfo("[Modem] Cached D-Bus interfaces invalid, reinitializing...");
        return initializeDBusInterfaces();
    }
    return true;
}

bool ModemDBusManager::shouldRetryOperation(const QDBusError &error) const
{
    switch (error.type()) {
    case QDBusError::InvalidArgs:
    case QDBusError::UnknownObject:
    case QDBusError::ServiceUnknown:
    case QDBusError::Failed:
        return true;
    default:
        return false;
    }
}

void ModemDBusManager::scheduleRetry(const QVariantMap &properties, int retryCount)
{
    if (initializeDBusInterfaces()) {
        QTimer::singleShot(RETRY_DELAY_MS, this, [this, properties, retryCount]() {
            sendSMS(properties["number"].toString(),
                    properties["text"].toString());
        });
    } else {
        emit logError("[Modem] Failed to reinitialize D-Bus interfaces");
        emit smsResult(false);
    }
}

void ModemDBusManager::handleCreateSMSResponse(const QDBusPendingCallWatcher *watcher, const QVariantMap &properties, int retryCount)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;

    if (reply.isError()) {
        if (retryCount < MAX_RETRY_ATTEMPTS && shouldRetryOperation(reply.error())) {
            emit logInfo(QString("[Modem] SMS creation failed, retrying (attempt %1)...")
                             .arg(retryCount + 1));
            scheduleRetry(properties, retryCount + 1);
            return;
        }
        emit logError("[Modem] SMS creation failed: " + reply.error().message());
        emit smsResult(false);
        return;
    }

    // Create SMS interface for sending
    QDBusInterface sms("org.freedesktop.ModemManager1",
                       reply.value().path(),
                       "org.freedesktop.ModemManager1.Sms",
                       m_dbusConnection);

    if (!sms.isValid()) {
        if (retryCount < MAX_RETRY_ATTEMPTS) {
            emit logInfo(QString("[Modem] SMS interface invalid, retrying (attempt %1)...")
                             .arg(retryCount + 1));
            scheduleRetry(properties, retryCount + 1);
            return;
        }
        emit logError("[Modem] Failed to create SMS interface after retries");
        emit smsResult(false);
        return;
    }

    // Send the SMS
    QDBusPendingCall sendCall = sms.asyncCall("Send");
    auto* sendWatcher = new QDBusPendingCallWatcher(sendCall, this);

    connect(sendWatcher, &QDBusPendingCallWatcher::finished,
            this, [this, sendWatcher, properties, retryCount]() {
                handleSendSMSResponse(sendWatcher, properties, retryCount);
                sendWatcher->deleteLater();
            });
}

void ModemDBusManager::handleSendSMSResponse(const QDBusPendingCallWatcher *watcher, const QVariantMap &properties, int retryCount)
{
    QDBusPendingReply<void> sendReply = *watcher;

    if (sendReply.isError()) {
        if (retryCount < MAX_RETRY_ATTEMPTS && shouldRetryOperation(sendReply.error())) {
            emit logInfo(QString("[Modem] SMS sending failed, retrying (attempt %1)...")
                             .arg(retryCount + 1));
            scheduleRetry(properties, retryCount + 1);
            return;
        }
        emit logError("[Modem] SMS sending failed: " + sendReply.error().message());
        emit smsResult(false);
    } else {
        emit logInfo("[Modem] SMS sent successfully");
        emit smsResult(true);
    }
}

void ModemDBusManager::onModemManagerServiceChanged(bool available)
{
    if (available) {
        m_dbusInterfaces.initialized = false;
        m_dbusInitRetryCount = 0; // Reset retry count
        if (!initializeDBusInterfaces()) {
            m_dbusInitTimer->start();
        }
    } else {
        m_dbusInitTimer->stop();
        m_dbusInterfaces.initialized = false;
        m_dbusInterfaces.messaging.reset();
        emit logError("[Modem] ModemManager service disappeared");
    }
}

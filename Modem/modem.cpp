#include "modem.h"
#include "modemdbusmanager.h"
#include <QtConcurrent>
#include <QFutureWatcher>

Modem::Modem(QObject *parent)
    : QObject{parent}
{
     setupDBus();
}
Modem::~Modem() = default;


void Modem::setupDBus() {
    m_dbusManager = std::make_unique<ModemDBusManager>(this);
    connect(m_dbusManager.get(), &ModemDBusManager::smsResult,
            this, &Modem::handleSMSResult);
    connect(m_dbusManager.get(), &ModemDBusManager::logInfo,
            this, &Modem::logInfo);
    connect(m_dbusManager.get(), &ModemDBusManager::logError,
            this, &Modem::logError);
    m_dbusManager->initialize();
}

void Modem::sendSMS(const QString &phoneNo, const QString &message) {
    QMetaObject::invokeMethod(this, "queueSMS", Qt::QueuedConnection,
                              Q_ARG(QString, phoneNo), Q_ARG(QString, message));
}

void Modem::resend()
{
    sendSMS(m_mostRecentRecipient, m_mostRecentMessage);
}

void Modem::queueSMS(const QString &phoneNo, const QString &message) {
    QMutexLocker locker(&m_mutex);
    m_smsQueue.enqueue({phoneNo, message});

    if (!m_isProcessing) {
        QMetaObject::invokeMethod(this, "processSMSQueue", Qt::QueuedConnection);
    }
}

void Modem::processSMSQueue() {
    QMutexLocker locker(&m_mutex);

    if (m_isProcessing || m_smsQueue.isEmpty()) {
        return;
    }

    m_isProcessing = true;
    SMSData smsData = m_smsQueue.dequeue();
    m_mostRecentRecipient = smsData.phoneNumber;
    m_mostRecentMessage = smsData.message;

    emit smsSending(smsData.phoneNumber);

    emit logInfo("[Modem] Sending SMS over D-Bus");
    sendSMSOverDBus(smsData);
}

void Modem::sendSMSOverDBus(const SMSData& smsData) {
    m_dbusManager->sendSMS(smsData.phoneNumber, smsData.message);
}

void Modem::handleSMSResult(bool success) {
    if (success) {
        emit smsSent(m_mostRecentRecipient);
        emit logInfo("SMS sent successfully to " + m_mostRecentRecipient);
    } else {
        emit smsFailed(m_mostRecentRecipient);
        emit logError("Failed to send SMS to " + m_mostRecentRecipient);
    }

    QMetaObject::invokeMethod(this, [this]() {
            QMutexLocker locker(&m_mutex);
            m_isProcessing = false;

            if (!m_smsQueue.isEmpty()) {
                QMetaObject::invokeMethod(this, "processSMSQueue",
                                          Qt::QueuedConnection);
            }
        }, Qt::QueuedConnection);
}


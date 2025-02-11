#ifndef MODEM_MODULE_H
#define MODEM_MODULE_H

#include <QQmlEngine>
#include <QMutex>
#include <QQueue>

class ModemDBusManager;

class Modem : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    explicit Modem(QObject *parent = nullptr);
    ~Modem();

    Q_INVOKABLE void sendSMS(const QString &phoneNo, const QString &message);
    Q_INVOKABLE void resend();

signals:
    void smsSending(const QString &recipient);
    void smsSent(const QString &recipient);
    void smsFailed(const QString &recipient);
    void logInfo(const QString &message);
    void logError(const QString &message);

private slots:
    void queueSMS(const QString &phoneNo, const QString &message);
    void processSMSQueue();
    void handleSMSResult(bool success);

private:
    // Structure to hold SMS data
    struct SMSData {
        QString phoneNumber;
        QString message;
    };

    // Core components
    std::unique_ptr<ModemDBusManager> m_dbusManager;

    // State tracking
    bool m_isProcessing{false};
    QString m_mostRecentRecipient;
    QString m_mostRecentMessage;

    // Thread safety
    QMutex m_mutex;
    QQueue<SMSData> m_smsQueue;

    // Private methods
    void setupDBus();
    void sendSMSOverDBus(const SMSData &smsData);
    void sendSMSOverGateway(const SMSData& smsData);
};

#endif // MODEM_MODULE_H

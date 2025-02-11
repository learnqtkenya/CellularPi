#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <QQmlEngine>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QUrl>
#include <memory>

class QRestAccessManager;
class QNetworkRequestFactory;

class RestClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(bool sslSupported READ sslSupported CONSTANT)
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit RestClient(QObject *parent = nullptr);
    ~RestClient() override = default;

    // Property getters/setters
    QUrl baseUrl() const;
    void setBaseUrl(const QUrl &url);
    bool sslSupported() const;

    // QML invokable methods
    Q_INVOKABLE void get(const QString &endpoint);
    Q_INVOKABLE void post(const QString &endpoint, const QVariantMap &data);
    Q_INVOKABLE void put(const QString &endpoint, const QVariantMap &data);
    Q_INVOKABLE void deleteResource(const QString &endpoint);

signals:
    void baseUrlChanged();
    void responseReceived(const QJsonObject &response);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager m_qnam;
    std::shared_ptr<QRestAccessManager> m_manager;
    std::shared_ptr<QNetworkRequestFactory> m_requestFactory;
};

#endif // RESTCLIENT_H

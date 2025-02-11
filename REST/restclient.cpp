#include "restclient.h"
#include <QRestAccessManager>
#include <QNetworkRequestFactory>
#include <QRestReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslSocket>

RestClient::RestClient(QObject *parent)
    : QObject(parent)
{
    m_qnam.setAutoDeleteReplies(true);
    m_manager = std::make_shared<QRestAccessManager>(&m_qnam);
    m_requestFactory = std::make_shared<QNetworkRequestFactory>();
}

QUrl RestClient::baseUrl() const
{
    return m_requestFactory->baseUrl();
}

void RestClient::setBaseUrl(const QUrl &url)
{
    if (m_requestFactory->baseUrl() == url)
        return;
    m_requestFactory->setBaseUrl(url);
    emit baseUrlChanged();
}

bool RestClient::sslSupported() const
{
#if QT_CONFIG(ssl)
    return QSslSocket::supportsSsl();
#else
    return false;
#endif
}

void RestClient::get(const QString &endpoint)
{
    try {
        auto request = m_requestFactory->createRequest(endpoint);
        m_manager->get(request, this, [this](QRestReply &reply) {
            if (const auto json = reply.readJson()) {
                if (json->isObject()) {
                    emit responseReceived(json->object());
                }
            } else {
                emit errorOccurred(reply.errorString());
            }
        });
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Request failed: %1").arg(e.what()));
    }
}

void RestClient::post(const QString &endpoint, const QVariantMap &data)
{
    try {
        auto request = m_requestFactory->createRequest(endpoint);
        m_manager->post(request, data, this, [this](QRestReply &reply) {
            if (const auto json = reply.readJson()) {
                if (json->isObject()) {
                    emit responseReceived(json->object());
                }
            } else {
                emit errorOccurred(reply.errorString());
            }
        });
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Request failed: %1").arg(e.what()));
    }
}

void RestClient::put(const QString &endpoint, const QVariantMap &data)
{
    try {
        auto request = m_requestFactory->createRequest(endpoint);
        m_manager->put(request, data, this, [this](QRestReply &reply) {
            if (const auto json = reply.readJson()) {
                if (json->isObject()) {
                    emit responseReceived(json->object());
                }
            } else {
                emit errorOccurred(reply.errorString());
            }
        });
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Request failed: %1").arg(e.what()));
    }
}

void RestClient::deleteResource(const QString &endpoint)
{
    try {
        auto request = m_requestFactory->createRequest(endpoint);
        m_manager->deleteResource(request, this, [this](QRestReply &reply) {
            if (const auto json = reply.readJson()) {
                if (json->isObject()) {
                    emit responseReceived(json->object());
                }
            } else {
                emit errorOccurred(reply.errorString());
            }
        });
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Request failed: %1").arg(e.what()));
    }
}

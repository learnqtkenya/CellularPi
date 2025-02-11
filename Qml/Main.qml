import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Universal
import QtQuick.Window
import Modem
import REST

ApplicationWindow {
    id: window
    width: Math.min(Screen.width * 0.9, 800)
    height: Math.min(Screen.height * 0.9, 900)
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2

    minimumWidth: Screen.width * 0.3
    minimumHeight: Screen.height * 0.4

    visible: true
    title: qsTr("CellularPi")

    property real fontScale: Math.min(Screen.width, Screen.height) / 1920
    property real baseSize: Math.max(16 * fontScale, 12)

    Universal.theme: Universal.Light
    Universal.accent: Universal.Violet

    property bool isSending: false
    property string statusMessage: ""

    // Header with tabs
    header: ToolBar {
        height: Math.max(window.height * 0.09, 50)
        background: Rectangle {
            color: Universal.background
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Universal.chromeHighColor
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: window.width * 0.03
            anchors.rightMargin: window.width * 0.03

            Label {
                text: "CellularPi"
                font.pixelSize: baseSize * 1.5
                font.weight: Font.Medium
                color: Universal.foreground
            }

            TabBar {
                id: tabBar
                Layout.fillWidth: true

                TabButton {
                    text: qsTr("SMS")
                    font.pixelSize: baseSize
                }
                TabButton {
                    text: qsTr("Internet")
                    font.pixelSize: baseSize
                }
            }

            BusyIndicator {
                running: window.isSending
                visible: window.isSending
                Layout.preferredWidth: baseSize * 1.5
                Layout.preferredHeight: baseSize * 1.5
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        // SMS Page
        Page {
            padding: window.width * 0.03

            ColumnLayout {
                anchors.fill: parent
                spacing: window.height * 0.02

                Label {
                    text: window.statusMessage
                    visible: text.length > 0
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: baseSize
                    color: {
                        if (text.includes("successfully")) return Universal.color(Universal.Green)
                        if (text.includes("Failed") || text.includes("Error")) return Universal.color(Universal.Red)
                        return Universal.accent
                    }
                }

                TextField {
                    id: phoneNumberField
                    placeholderText: qsTr("Phone Number")
                    Layout.fillWidth: true
                    font.pixelSize: baseSize
                    selectByMouse: true
                    Layout.preferredHeight: baseSize * 2.5
                    validator: RegularExpressionValidator {
                        regularExpression: /^\+?[\d\s-]{0,15}$/
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: window.height * 0.2
                    clip: true

                    TextArea {
                        id: messageField
                        placeholderText: qsTr("Type your message here...")
                        wrapMode: TextArea.Wrap
                        font.pixelSize: baseSize
                        selectByMouse: true
                    }
                }

                Label {
                    text: qsTr("%1/160 characters").arg(messageField.length)
                    color: messageField.length > 160 ?
                           Universal.color(Universal.Red) :
                           Universal.color(Universal.Chromium)
                    font.pixelSize: baseSize * 0.75
                    Layout.alignment: Qt.AlignRight
                }

                Button {
                    text: qsTr("Send SMS")
                    enabled: !window.isSending &&
                            phoneNumberField.text.length > 0 &&
                            messageField.text.length > 0 &&
                            messageField.length <= 160
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(window.height * 0.08, baseSize * 3)

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: baseSize
                        font.weight: Font.Medium
                        color: parent.enabled ?
                               Universal.foreground :
                               Universal.color(Universal.Chromium)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        window.isSending = true
                        window.statusMessage = "Sending message..."
                        Modem.sendSMS(phoneNumberField.text, messageField.text)
                    }
                }
            }
        }

        // Internet/REST Page
        Page {
            padding: window.width * 0.03

            Component.onCompleted: {
                // Initialize REST client
                RestClient.baseUrl = "https://jsonplaceholder.typicode.com"
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: window.height * 0.02

                GroupBox {
                    title: "REST API Test"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: window.height * 0.02

                        Label {
                            text: "Base URL: " + RestClient.baseUrl
                            font.pixelSize: baseSize
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: endpointCombo
                            Layout.fillWidth: true
                            model: ["/posts/1", "/users/1", "/todos/1"]
                            font.pixelSize: baseSize
                        }

                        Button {
                            text: "GET Request"
                            Layout.fillWidth: true
                            font.pixelSize: baseSize
                            onClicked: RestClient.get(endpointCombo.currentText)
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: window.height * 0.3
                            clip: true

                            TextArea {
                                id: responseArea
                                readOnly: true
                                wrapMode: TextArea.Wrap
                                font.family: "Monospace"
                                font.pixelSize: baseSize * 0.9
                                placeholderText: "Response will appear here..."
                            }
                        }
                    }
                }

                GroupBox {
                    title: "POST Example"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: window.height * 0.02

                        TextArea {
                            id: postDataField
                            Layout.fillWidth: true
                            wrapMode: TextArea.Wrap
                            font.pixelSize: baseSize
                            text: '{\n    "title": "foo",\n    "body": "bar",\n    "userId": 1\n}'
                            font.family: "Monospace"
                        }

                        Button {
                            text: "POST Request"
                            Layout.fillWidth: true
                            font.pixelSize: baseSize
                            onClicked: {
                                try {
                                    const data = JSON.parse(postDataField.text)
                                    RestClient.post("/posts", data)
                                } catch (e) {
                                    responseArea.text = "Error parsing JSON: " + e.message
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // REST Client connections
    Connections {
        target: RestClient

        function onResponseReceived(response) {
            responseArea.text = JSON.stringify(response, null, 2)
        }

        function onErrorOccurred(error) {
            responseArea.text = "Error: " + error
        }
    }

    // Modem connections
    Connections {
        target: Modem

        function onSmsSending(recipient) {
            window.statusMessage = "Sending message to " + recipient + "..."
            window.isSending = true
        }

        function onSmsSent(recipient) {
            window.statusMessage = "Message sent successfully to " + recipient
            window.isSending = false
            messageField.clear()
        }

        function onSmsFailed(recipient) {
            window.statusMessage = "Failed to send message to " + recipient
            window.isSending = false
        }

        function onLogError(message) {
            window.statusMessage = message
        }
    }

    // Screen size handling
    onWidthChanged: {
        fontScale = Math.min(Screen.width, Screen.height) / 1920
        baseSize = Math.max(16 * fontScale, 12)
    }

    onHeightChanged: {
        fontScale = Math.min(Screen.width, Screen.height) / 1920
        baseSize = Math.max(16 * fontScale, 12)
    }
}

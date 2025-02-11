import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Universal
import QtQuick.Window
import Modem

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

    // Header pane
    Pane {
        id: header
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: Math.max(window.height * 0.09, 50) // 9% of window height, minimum 50px
        padding: 0

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
            anchors.leftMargin: window.width * 0.03 // 3% of window width
            anchors.rightMargin: window.width * 0.03

            Label {
                text: "CellularPi"
                font.pixelSize: baseSize * 1.5
                font.weight: Font.Medium
                color: Universal.foreground
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            BusyIndicator {
                running: window.isSending
                visible: window.isSending
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: baseSize * 1.5
                Layout.preferredHeight: baseSize * 1.5
            }
        }
    }

    // Main content
    ColumnLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
            margins: window.width * 0.03 // 3% of window width
        }
        spacing: window.height * 0.02 // 2% of window height

        // Status message
        Label {
            id: statusLabel
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

        // Phone number input
        TextField {
            id: phoneNumberField
            placeholderText: qsTr("Phone Number")
            Layout.fillWidth: true
            font.pixelSize: baseSize
            selectByMouse: true
            Universal.theme: Universal.Light
            Layout.preferredHeight: baseSize * 2.5
            validator: RegularExpressionValidator {
                regularExpression: /^\+?[\d\s-]{0,15}$/
            }
        }

        // Message input area
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: window.height * 0.2 // At least 20% of window height
            clip: true

            TextArea {
                id: messageField
                placeholderText: qsTr("Type your message here...")
                wrapMode: TextArea.Wrap
                font.pixelSize: baseSize
                selectByMouse: true
                Universal.theme: Universal.Light
            }
        }

        // Character counter
        Label {
            text: qsTr("%1/160 characters").arg(messageField.length)
            color: messageField.length > 160 ?
                   Universal.color(Universal.Red) :
                   Universal.color(Universal.Chromium)
            font.pixelSize: baseSize * 0.75
            Layout.alignment: Qt.AlignRight
        }

        // Send button
        Button {
            id: sendButton
            text: qsTr("Send SMS")
            enabled: !window.isSending &&
                    phoneNumberField.text.length > 0 &&
                    messageField.text.length > 0 &&
                    messageField.length <= 160
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(window.height * 0.08, baseSize * 3) // 8% of height or 3x base size

            Universal.accent: Universal.Violet

            contentItem: Text {
                text: sendButton.text
                font.pixelSize: baseSize
                font.weight: Font.Medium
                color: sendButton.enabled ?
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

    // Screen size change handling
    onWidthChanged: {
        fontScale = Math.min(Screen.width, Screen.height) / 1920
        baseSize = Math.max(16 * fontScale, 12)
    }

    onHeightChanged: {
        fontScale = Math.min(Screen.width, Screen.height) / 1920
        baseSize = Math.max(16 * fontScale, 12)
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
}

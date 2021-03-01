import QtQuick 2.15
import QtQuick.Controls 2.15
TabButton {
            text: name
            property color textColor: (checked ? "white" : (hovered?"cyan":"black"))
            property color buttonColor: checked ? "gray" : "transparent"
            implicitWidth: Math.max(
                               implicitBackgroundWidth + leftInset + rightInset,
                               implicitContentWidth + leftPadding + rightPadding)
            implicitHeight: Math.max(
                                implicitBackgroundHeight + topInset + bottomInset,
                                implicitContentHeight + topPadding + bottomPadding)
            padding: 1
            spacing: 6
            contentItem: Text {
                text: parent.text
                font: parent.font
                color: parent.textColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                implicitHeight: 30
                height: parent.height - 1
                color: parent.buttonColor
            }
        }

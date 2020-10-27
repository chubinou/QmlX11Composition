import QtQuick 2.10
import QtQuick.Window 2.10

Rectangle {
    visible: true
    color: "red"
    opacity: 0.5
    border.width: 5
    border.color: "blue"

    Text {
        id: text
        text: "This is QML code."
        font.pointSize: 14
        anchors.centerIn: parent
        PropertyAnimation {
            id: animation
            target: text
            property: "rotation"
            from: 0; to: 360; duration: 5000
            loops: Animation.Infinite
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("pop pop")
        }
    }
    Keys.onPressed: {
        console.log("yayaya")
    }

    Component.onCompleted: animation.start()
}

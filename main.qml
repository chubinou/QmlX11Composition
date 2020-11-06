import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    visible: true
    color: "#88FF0000"
    border.width: 5
    border.color: "blue"

    property bool rotateText: true

    Text {
        id: text
        text: input.text
        font.pointSize: 14
        color: "blue"
        anchors.centerIn: parent
        PropertyAnimation {
            id: animation
            target: text
            property: "rotation"
            running: rotateText
            from: 0; to: 360; duration: 5000
            loops: Animation.Infinite
        }
    }



    TextField {
        id:input
        text: "This is QML"
        width: 200
        anchors.left: parent.left
        anchors.top: parent.top
    }

    Button {
        text: "rotate"
        onClicked: rotateText = !rotateText
        anchors.left: parent.left
        anchors.top: input.bottom
    }

    Keys.onPressed: {
        console.log("yayaya")
    }

    Component.onCompleted: animation.start()
}

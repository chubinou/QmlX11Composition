import QtQuick 2.11
import QtQuick.Controls 2.11

Rectangle {
    visible: true
    color: "#88FF0000"
    border.width: 5
    border.color: "blue"

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

    //MouseArea {
    //    anchors.fill: parent
    //    onClicked: {
    //        console.log("pop pop")
    //    }
    //}
    Keys.onPressed: {
        console.log("yayaya")
    }

    Component.onCompleted: animation.start()
}

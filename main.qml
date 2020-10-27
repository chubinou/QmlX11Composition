import QtQuick 2.10
import QtQuick.Window 2.10

Rectangle {
    visible: true
    color: "red"
    opacity: 0.5
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
	Component.onCompleted: animation.start()

}

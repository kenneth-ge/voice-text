import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Voice to Text")
    //flags: Qt.ToolTip | Qt.WA_TranslucentBackground | Qt.FramelessWindowHint
    color: Qt.rgba(0.7, 0.7, 0.7, 0.8)

    ScrollView {
        width: parent.width * 0.8
        height: parent.height * 0.8
        //anchors.fill: parent

        TextArea {
            id: textarea
            font.pixelSize: 25
            text: "Lorem ipsum dolor sit amet, consectetur adipisicing elit, \n" +
                  "sed do eiusmod tempor incididunt ut labore et dolore magna \n" +
                  "aliqua. Ut enim ad minim veniam, quis nostrud exercitation \n" +
                  "ullamco laboris nisi ut aliquip ex ea commodo cosnsequat. "
            selectByMouse: true
            background: Qt.rgba(1, 1, 1, 1)
        }
    }
}

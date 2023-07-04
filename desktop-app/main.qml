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
        width: parent.width
        height: parent.height * 0.8
        //anchors.fill: parent

        TextArea {
            id: textarea
            font.pixelSize: 25
            text: Vtt.text
            selectByMouse: true
            background: Qt.rgba(1, 1, 1, 1)
            wrapMode: TextEdit.WordWrap
            readOnly: true
        }
    }
}

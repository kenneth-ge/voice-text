import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import com.voicetext 1.0
import Qt5Compat.GraphicalEffects

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Voice to Text")
    //flags: Qt.ToolTip | Qt.WA_TranslucentBackground | Qt.FramelessWindowHint
    color: Qt.rgba(0.7, 0.7, 0.7, 0.8)

    ScrollView {
        id: scroll
        width: parent.width * 0.8
        height: parent.height * 0.8
        //anchors.fill: parent

        TextArea {
            id: textarea
            font.pixelSize: 25
            text: Vtt.text
            selectByMouse: true
            background: Qt.rgba(1, 1, 1, 1)
            wrapMode: TextEdit.WordWrap
            //readOnly: true
            onTextChanged: Vtt.textHasChanged(text)

            Connections {
                target: Vtt
                function onTextChanged() {
                    textarea.cursorPosition = textarea.text.length
                }
            }
        }
    }

    Rectangle {
        x: scroll.width
        y: 0
        width: parent.width * 0.2
        height: parent.height
        color: "gray"

        Rectangle {
            width: parent.width
            height: parent.height * 0.1
            id: text1
            color: "lightgray"
            Text {
                anchors.centerIn: parent
                text: qsTr("Select")
                font.pixelSize: 24
            }
        }

        Rectangle {
            x: 0
            width: parent.width
            height: 2
            y: text1.height
            color: "black"
        }

        Text {
            padding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            visible: Edit.loadingScrn
            text: "Loading..."
            font.pixelSize: 24
            y: text1.height + 2
        }

        ListView {
            id: listview
            x: 0
            y: parent.height * 0.1
            width: parent.width
            height: parent.height * 0.9

            Connections {
                target: Edit
                onNewOptions: {
                    model.clear()
                    for(var x = 0; x < Edit.options.length; x++){
                        model.append({opt_num: x, frag: Edit.options[x].frag})
                    }
                }
            }

            model: model

            delegate:
                    Row {
                        width: parent.width
                        padding: 10
                        Text {
                            text: opt_num
                            font.pixelSize: 24
                            anchors.verticalCenter: parent.verticalCenter
                            padding: 5
                        }
                        Text {
                            text: frag
                            font.pixelSize: 20
                            width: parent.width
                            padding: 5
                            wrapMode: Text.WordWrap
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    Rectangle {
                        width: parent.width * 0.9
                        y: parent.height
                        x: parent.width * 0.05
                        height: 2
                        color: "black"
                    }
        }

        ListModel {
            id: model

            /*ListElement {
                opt_num: 1
                frag: "test"
            }*/
        }
    }

    Button {
        y: scroll.height
        width: scroll.width
        height: parent.height - scroll.height
        text: "Push for Command"
        onPressed: {
            Vtt.buttonPressed()
            commandPopupRect.visible = true
        }
        onReleased: {
            Vtt.buttonReleased()
            commandPopupRect.visible = false
        }
    }

    Rectangle {
        id: commandPopupRect
        anchors.centerIn: parent
        width: parent.width * 0.8
        height: parent.height * 0.8
        color: "mistyrose"
        visible: false

        Column {
            width: parent.width
            height: parent.height

            Text {
                text: "Listening for command..."
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 24
                padding: 15
            }

            Rectangle {
                width: parent.width
                height: 2
                color: "black"
            }

            Text {
                text: Vtt.commandText
                font.pixelSize: 24
            }
        }

        DropShadow {
            anchors.fill: parent
            source: parent
            radius: 10
            samples: 16
            color: "gray"
            horizontalOffset: 5
            verticalOffset: 5
            z: -1
        }
    }
}



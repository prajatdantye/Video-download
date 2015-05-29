/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import cuteTube 2.0
import QVimeo 1.0 as QVimeo
import ".."

MyPage {
    id: root

    title: qsTr("Accounts")
    showProgressIndicator: authRequest.status == QVimeo.AuthenticationRequest.Loading
    tools: ToolBarLayout {

        BackToolButton {}

        MyToolButton {
            iconSource: "toolbar-add"
            toolTipText: qsTr("Add account")
            onClicked: {
                loader.sourceComponent = authDialog;
                loader.item.open();
            }
        }
    }

    MyListView {
        id: view

        anchors.fill: parent
        enabled: !root.showProgressIndicator
        model: VimeoAccountModel {
            id: accountModel
        }
        delegate: LabelDelegate {
            text: username
            isSelected: active
            onClicked: infoBanner.showMessage(accountModel.selectAccount(index)
                                              ? qsTr("You have selected account") + " '" + username + "'"
                                              : accountModel.errorString)
            onPressAndHold: {
                view.currentIndex = index;
                contextMenu.open();
            }
        }
    }

    MyScrollBar {
        flickableItem: view
    }

    Label {
        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        font.pixelSize: 40
        color: platformStyle.colorNormalMid
        text: qsTr("No accounts")
        visible: accountModel.count == 0
    }

    MyContextMenu {
        id: contextMenu

        focusItem: view

        MenuLayout {

            MenuItem {
                text: qsTr("Remove")
                onClicked: infoBanner.showMessage(accountModel.removeAccount(view.currentIndex)
                           ? qsTr("Account removed. Please visit the Vimeo website to revoke the access token")
                           : accountModel.errorString)
            }
        }
    }

    Loader {
        id: loader
    }

    QVimeo.AuthenticationRequest {
        id: authRequest

        clientId: Vimeo.clientId
        clientSecret: Vimeo.clientSecret
        redirectUri: Vimeo.redirectUri
        scopes: Vimeo.scopes
        onFinished: {
            if (status == QVimeo.AuthenticationRequest.Ready) {
                if ((result.user) && (result.access_token)) {
                    var user = result.user;

                    if (accountModel.addAccount(user.uri.substring(user.uri.lastIndexOf("/")), user.name,
                                                result.access_token, result.scope)) {
                        infoBanner.showMessage(qsTr("You are signed in to account") + " '" + user.name + "'");
                    }
                    else {
                        infoBanner.showMessage(accountModel.errorString);
                    }

                    return;
                }

                infoBanner.showMessage(qsTr("Unable to add account"));
                return;
            }

            infoBanner.showMessage(Vimeo.getErrorString(result));
        }
    }

    Component {
        id: authDialog

        VimeoAuthDialog {
            focusItem: view
            onCodeReady: authRequest.exchangeCodeForAccessToken(code)
        }
    }
}

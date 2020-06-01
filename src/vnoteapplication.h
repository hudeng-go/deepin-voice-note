/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     V4fr3e <V4fr3e@deepin.io>
*
* Maintainer: V4fr3e <liujinli@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef VNOTEAPPLICATION_H
#define VNOTEAPPLICATION_H
#include "views/vnotemainwindow.h"

#include <QSettings>

#include <DApplication>

DWIDGET_USE_NAMESPACE

class VNoteApplication : public DApplication
{
    Q_OBJECT
public:
    explicit VNoteApplication(int &argc, char **argv);

    void activateWindow();
    /*
     * Comment:
     *     In order put our config file to deepin's config
     * dir,we should call the this function in main window.
     * We can't get correct path when app's consturctor.
     * */
    void initAppSetting();
    QSharedPointer<QSettings> appSetting() const;
    VNoteMainWindow* mainWindow() const;
signals:

public slots:
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);
protected:
    virtual void handleQuitAction() override;
protected:
    QScopedPointer<VNoteMainWindow> m_qspMainWnd {nullptr};

    //App setting
    QSharedPointer<QSettings> m_qspSetting {nullptr};
};

#endif // VNOTEAPPLICATION_H
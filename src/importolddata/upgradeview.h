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
#ifndef UPGRADEVIEW_H
#define UPGRADEVIEW_H

#include <QShowEvent>
#include <QWidget>
#include <QSettings>

#include <DLabel>
#include <DWaterProgress>

DWIDGET_USE_NAMESPACE

class UpgradeView : public QWidget
{
    Q_OBJECT
public:
    explicit UpgradeView(QWidget *parent = nullptr);

    void startUpgrade();
signals:
    void upgradeDone();
public slots:
    void setProgress(int progress);
    void onDataReady();
    void onUpgradeFinish();
protected:
    void initConnections();
    void initAppSetting();
private:
    DWaterProgress *m_waterProgress {nullptr};
    DLabel         *m_tooltipTextLabel {nullptr};

    //App setting
    QSharedPointer<QSettings> m_qspSetting {nullptr};
};

#endif // UPGRADEVIEW_H
/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     liuyanga <liuyanga@uniontech.com>
*
* Maintainer: liuyanga <liuyanga@uniontech.com>
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

#ifndef VNOTE2SICONBUTTON_H
#define VNOTE2SICONBUTTON_H

#include <QMouseEvent>

#include <DApplicationHelper>
#include <DFloatingButton>

DWIDGET_USE_NAMESPACE

class VNote2SIconButton : public DFloatingButton
{
    Q_OBJECT
public:
    explicit VNote2SIconButton(const QString normal
            ,const QString press
            ,QWidget *parent = nullptr);

    enum {
        Invalid = -1,
        Normal,
        Press,
//        Disabled, //Not support this state now
        MaxState
    };

    bool isPressed() const;
    void setState(int state);
signals:

public slots:
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void updateIcon();
    void setCommonIcon(bool isCommon);

    QString m_icons[MaxState];
    int     m_state {Normal};

    bool    m_useCommonIcons {true};
};

#endif // VNOTE2SICONBUTTON_H
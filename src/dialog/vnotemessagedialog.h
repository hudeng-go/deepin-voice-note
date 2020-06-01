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

#ifndef VNOTEMESSAGEDIALOG_H
#define VNOTEMESSAGEDIALOG_H

#include "vnotebasedialog.h"

#include <DPushButton>
#include <DWarningButton>
#include <DVerticalLine>

DWIDGET_USE_NAMESPACE;

class VNoteMessageDialog : public VNoteBaseDialog
{
    Q_OBJECT
public:
    explicit VNoteMessageDialog(int msgType, QWidget *parent = nullptr);

    enum MessageType {
        DeleteNote,
        AbortRecord,
        DeleteFolder,
        AsrTimeLimit,
        AborteAsr,
        VolumeTooLow,
        CutNote,
    };

protected:
    void initUI();
    void initConnections();
    void initMessage();
    void setSingleButton(); //Need to be Optimzed
signals:

public slots:

protected:
    DLabel         *m_pMessage {nullptr};
    DPushButton    *m_cancelBtn {nullptr};
    DWarningButton *m_confirmBtn {nullptr};
    DVerticalLine  *m_buttonSpliter {nullptr};

    MessageType    m_msgType;
};

#endif // VNOTEMESSAGEDIALOG_H
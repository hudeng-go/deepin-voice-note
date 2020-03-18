#include "rightview.h"
#include "textnoteitem.h"
#include "voicenoteitem.h"

#include "common/vnoteitem.h"
#include "common/vnotedatamanager.h"
#include "common/actionmanager.h"

#include "db/vnoteitemoper.h"

#include <QBoxLayout>
#include <QDebug>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollBar>
#include <QScrollArea>
#include <QAbstractTextDocumentLayout>
#include <QList>
#include <QClipboard>

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DFileDialog>
#include <DMessageBox>
#include <DApplication>
#include <DStyle>

//#define PlaceholderWidget "placeholder"
//#define VoiceWidget       "voiceitem"
//#define TextEditWidget    "textedititem"

RightView::RightView(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initMenu();
}

void RightView::initUi()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_viewportLayout = new QVBoxLayout;
    m_viewportLayout->setSpacing(0);
    m_viewportLayout->setContentsMargins(20, 0, 20, 0);
    this->setLayout(m_viewportLayout);
    m_placeholderWidget = new DWidget(this); //占位的
    //m_placeholderWidget->setObjectName(PlaceholderWidget);
    m_placeholderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_viewportLayout->addWidget(m_placeholderWidget, 1, Qt::AlignBottom);
    QString content = DApplication::translate(
                          "RightView",
                          "The voice note has been deleted");
    m_fileHasDelDialog = new DDialog("", content, this);
    m_fileHasDelDialog->setIcon(QIcon::fromTheme("dialog-warning"));
    m_fileHasDelDialog->addButton(DApplication::translate(
                                      "RightView",
                                      "OK"), false, DDialog::ButtonNormal);
    m_fileHasDelDialog->setEnabled(false);
}

void RightView::initMenu()
{
    //Init voice context Menu
    m_noteDetailContextMenu = ActionManager::Instance()->detialContextMenu();
}

DetailItemWidget *RightView::insertTextEdit(VNoteBlock *data, bool focus, QRegExp reg)
{
    TextNoteItem *editItem = new TextNoteItem(data, this, reg);
    int index = 0;
    if (m_curItemWidget != nullptr) {
        index = m_viewportLayout->indexOf(m_curItemWidget) + 1;
    }
    m_viewportLayout->insertWidget(index, editItem);
    if (focus) {
        QTextCursor cursor = editItem->getTextCursor();
        cursor.movePosition(QTextCursor::End);
        editItem->setTextCursor(cursor);
        editItem->setFocus();
    }
    connect(editItem, &TextNoteItem::sigCursorHeightChange, this, &RightView::adjustVerticalScrollBar);
    connect(editItem, &TextNoteItem::sigFocusOut, this, &RightView::onTextEditFocusOut);
    connect(editItem, &TextNoteItem::sigDelEmpty, this, &RightView::onTextEditDelEmpty);
    connect(editItem, &TextNoteItem::sigTextChanged, this, &RightView::onTextEditTextChange);
    return  editItem;
}

DetailItemWidget *RightView::insertVoiceItem(const QString &voicePath, qint64 voiceSize)
{
    VNoteItemOper noteOps(m_noteItemData);

    VNoteBlock *data = m_noteItemData->newBlock(VNoteBlock::Voice);

    data->ptrVoice->voiceSize = voiceSize;
    data->ptrVoice->voicePath = voicePath;
    data->ptrVoice->createTime = QDateTime::currentDateTime();
    data->ptrVoice->voiceTitle = noteOps.getDefaultVoiceName();

    VoiceNoteItem *item = new VoiceNoteItem(data, this);

    connect(item, &VoiceNoteItem::sigPlayBtnClicked, this, &RightView::onVoicePlay);
    connect(item, &VoiceNoteItem::sigPauseBtnClicked, this, &RightView::onVoicePause);
    QString cutStr = "";
    int curIndex = m_viewportLayout->indexOf(m_curItemWidget);
    DetailItemWidget *itemWidget = static_cast<DetailItemWidget *>(m_curItemWidget);
    if (itemWidget->getNoteBlock()->blockType == VNoteBlock::Text) {
        QTextCursor cursor = itemWidget->getTextCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cutStr = cursor.selectedText();
        cursor.removeSelectedText();
    }

    curIndex += 1;
    m_viewportLayout->insertWidget(curIndex, item);

    VNoteBlock *preBlock = nullptr;
    if (curIndex > 0) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(curIndex - 1);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        preBlock = widget->getNoteBlock();
    }

    m_noteItemData->addBlock(preBlock, data);
    m_curItemWidget = item;
    if (!cutStr.isEmpty() || curIndex + 1 == m_viewportLayout->indexOf(m_placeholderWidget)) {
        VNoteBlock *textBlock = m_noteItemData->newBlock(VNoteBlock::Text);
        textBlock->blockText = cutStr;
        m_curItemWidget = insertTextEdit(textBlock, true);
        m_noteItemData->addBlock(item->getNoteBlock(), textBlock);
    }
    int height = 0;
    QRect rc =  m_curItemWidget->getCursorRect();
    if (!rc.isEmpty()) {
        height = rc.bottom();
    }
    adjustVerticalScrollBar(m_curItemWidget, height);
    updateData();
    return m_curItemWidget;
}

void RightView::onTextEditFocusOut()
{
    DetailItemWidget *widget = static_cast<DetailItemWidget *>(sender());
    QString text = widget->getAllText();
    VNoteBlock *block =  widget->getNoteBlock();
    if (text != block->blockText) {
        block->blockText = text;
        updateData();
    }
}

void RightView::onTextEditDelEmpty()
{
    DetailItemWidget *widget = static_cast<DetailItemWidget *>(sender());
    delWidget(widget);
    updateData();
}

void RightView::onTextEditTextChange()
{
    m_fIsNoteModified = true;
    if (m_isFristTextChange == false) {
        m_isFristTextChange = true;
        saveNote();
    }
}

void RightView::initData(VNoteItem *data, QRegExp reg, bool fouse)
{
    while (m_viewportLayout->indexOf(m_placeholderWidget) != 0) {
        QWidget *widget = m_viewportLayout->itemAt(0)->widget();
        m_viewportLayout->removeWidget(widget);
        widget->deleteLater();
    }
    if (fouse) {
        this->setFocus();
    }
    m_isFristTextChange = false;
    m_noteItemData = data;
    if (m_noteItemData == nullptr) {
        m_curItemWidget = nullptr;
        return;
    }
    int size = m_noteItemData->datas.dataConstRef().size();
    if (size) {
        for (auto it : m_noteItemData->datas.dataConstRef()) {
            if (VNoteBlock::Text == it->getType()) {
                m_curItemWidget = insertTextEdit(it, fouse, reg);
            } else if (VNoteBlock::Voice == it->getType()) {
                VoiceNoteItem *item = new VoiceNoteItem(it, this);
                int preIndex = -1;
                if (m_curItemWidget != nullptr) {
                    preIndex = m_viewportLayout->indexOf(m_curItemWidget);
                }
                m_viewportLayout->insertWidget(preIndex + 1, item);
                m_curItemWidget = item;
                connect(item, &VoiceNoteItem::sigPlayBtnClicked, this, &RightView::onVoicePlay);
                connect(item, &VoiceNoteItem::sigPauseBtnClicked, this, &RightView::onVoicePause);
            }
        }
    } else {
        VNoteBlock *textBlock = m_noteItemData->newBlock(VNoteBlock::Text);
        m_curItemWidget = insertTextEdit(textBlock, false, reg);
        m_noteItemData->addBlock(nullptr, textBlock);
    }
}

void RightView::onVoicePlay(VoiceNoteItem *item)
{
    VNVoiceBlock *data = item->getNoteBlock()->ptrVoice;
    if (data) {
        if (!checkFileExist(data->voicePath)) {
            delWidget(item);
            updateData();
            return;
        }
        for (int i = 0; i < m_viewportLayout->count() - 1; i++) {
            QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
            DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
            if (widget->getNoteBlock()->blockType == VNoteBlock::Voice) {
                VoiceNoteItem *tmpWidget = static_cast< VoiceNoteItem *>(widget);
                tmpWidget->showPlayBtn();
            }
        }
        item->showPauseBtn();
        m_curPlayItem = item;
        emit sigVoicePlay(data);
    }
}

void RightView::onVoicePause(VoiceNoteItem *item)
{
    item->showPlayBtn();
    emit sigVoicePause(item->getNoteBlock()->ptrVoice);
}

void RightView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    //TODO:
    //    Add the note save code here.
    saveNote();
}

void RightView::saveNote()
{
    //qInfo() << __FUNCTION__ << "Is note changed:" << m_fIsNoteModified;
    if (m_noteItemData && m_fIsNoteModified) {
        for (int i = 0; i < m_viewportLayout->count() - 1; i++) {

            QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
            DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
            if (widget->hasFocus()) {
                widget->getNoteBlock()->blockText = widget->getAllText();
            }

            VNoteItemOper noteOper(m_noteItemData);
            if (!noteOper.updateNote()) {
                qInfo() << "Save note error:" << *m_noteItemData;
            } else {
                m_fIsNoteModified = false;

                //Notify middle view refresh.
                emit contentChanged();
            }
        }
    }
}

void RightView::initAction(DetailItemWidget *widget)
{
    QAction *voiceSaveAction = ActionManager::Instance()->getActionById(ActionManager::DetailVoiceSave);
    QAction *voice2TextAction = ActionManager::Instance()->getActionById(ActionManager::DetailVoice2Text);
    QAction *delAction = ActionManager::Instance()->getActionById(ActionManager::DetailDelete);
    QAction *copyAction = ActionManager::Instance()->getActionById(ActionManager::DetailCopy);
    QAction *cutAction = ActionManager::Instance()->getActionById(ActionManager::DetailCut);
    QAction *pasteAction = ActionManager::Instance()->getActionById(ActionManager::DetailPaste);

    voiceSaveAction->setEnabled(false);
    voice2TextAction->setEnabled(false);
    delAction->setEnabled(false);
    copyAction->setEnabled(false);
    cutAction->setEnabled(false);
    pasteAction->setEnabled(false);

    int voiceCount, textCount, tolCount;
    getSelectionCount(voiceCount, textCount);
    tolCount = voiceCount + textCount;
    VNoteBlock *blockData = nullptr;
    if (widget != nullptr) {
        blockData = widget->getNoteBlock();
    }

    if (tolCount) {
        copyAction->setEnabled(true);
        if (textCount != 0) {
            if (!blockData || blockData->blockType == VNoteBlock::Text) {
                cutAction->setEnabled(true);
                delAction->setEnabled(true);
            }
        }
    }
    if (blockData != nullptr) {
        if (blockData->blockType == VNoteBlock::Voice) {
            if (!checkFileExist(blockData->ptrVoice->voicePath)) {
                delWidget(widget);
                updateData();
                return;
            }

            if (!tolCount) {
                voiceSaveAction->setEnabled(true);

                if(blockData->ptrVoice->blockText.isEmpty()){
                   voice2TextAction->setEnabled(true);
                }

            }

            VoiceNoteItem *item = static_cast<VoiceNoteItem *>(widget);
            bool enable = true;

            if (m_curAsrItem && m_curAsrItem->isAsring() &&
                    (m_curAsrItem == item || m_curAsrItem->hasSelection())) {
                enable = false;
            }

            if ((m_curPlayItem == item) || (m_curPlayItem && m_curPlayItem->hasSelection())) {
                enable = false;
            }

            if (tolCount && textCount == 0) {
                if (tolCount != 1 || !item->hasSelection()) {
                    enable = false;
                }
            }
            delAction->setEnabled(enable);
            if (tolCount && enable) {
                cutAction->setEnabled(enable);
            }

        } else if (blockData->blockType == VNoteBlock::Text) {
            if (!tolCount) {
                pasteAction->setEnabled(true);
            }
        }
    }
}

void RightView::onMenuShow(DetailItemWidget *widget)
{
    initAction(widget);
    m_noteDetailContextMenu->exec(QCursor::pos());
}

void RightView::delWidget(DetailItemWidget *widget, bool merge)
{
    QList<VNoteBlock *> noteBlockList;
    if (widget != nullptr && widget != m_placeholderWidget) {
        int index = m_viewportLayout->indexOf(widget);
        VNoteBlock *noteBlock = widget->getNoteBlock();
        if (noteBlock->blockType == VNoteBlock::Text) {
            if (index == 0 || (index == m_viewportLayout->count() - 2 && index != 1)) { //第一个和最后一个编辑框不删，只清空内容
                widget->getNoteBlock()->blockText = "";
                widget->updateData();
                return;
            }
        }

        DetailItemWidget *preWidget = nullptr;
        DetailItemWidget *nextWidget = nullptr;
        QLayoutItem *layoutItem = nullptr;

        layoutItem = m_viewportLayout->itemAt(index - 1);
        if (layoutItem) {
            preWidget = static_cast<DetailItemWidget *>(layoutItem->widget());
        }

        layoutItem = m_viewportLayout->itemAt(index + 1);
        if (layoutItem) {
            nextWidget = static_cast<DetailItemWidget *>(layoutItem->widget());
        }

        if (preWidget && nextWidget) {
            //两个语音之间的编辑框不删除,只清空内容
            if (noteBlock->blockType == VNoteBlock::Text &&
                    preWidget->getNoteBlock()->blockType == VNoteBlock::Voice &&
                    nextWidget->getNoteBlock()->blockType == VNoteBlock::Voice) {
                widget->getNoteBlock()->blockText = "";
                widget->updateData();
                return;
            }
        }

        if (m_curItemWidget == widget) {
            if (nextWidget != m_placeholderWidget) {
                m_curItemWidget = nextWidget;
            } else if (preWidget != nullptr) {
                m_curItemWidget = preWidget;
            } else {
                m_curItemWidget = nullptr;
            }
        }

        widget->disconnect();
        noteBlockList.push_back(noteBlock);
        m_viewportLayout->removeWidget(widget);
        widget->deleteLater();

        if (merge && preWidget && nextWidget && //合并编辑框
                preWidget != m_placeholderWidget &&
                nextWidget != m_placeholderWidget &&
                preWidget->getNoteBlock()->blockType == VNoteBlock::Text &&
                nextWidget->getNoteBlock()->blockType == VNoteBlock::Text) {

            noteBlock = nextWidget->getNoteBlock();
            noteBlock->blockText = preWidget->getAllText() + nextWidget->getAllText();
            nextWidget->updateData();

            preWidget->disconnect();
            noteBlockList.push_back(preWidget->getNoteBlock());
            m_viewportLayout->removeWidget(preWidget);
            preWidget->deleteLater();

            QTextCursor cursor = nextWidget->getTextCursor();
            cursor.movePosition(QTextCursor::End);
            nextWidget->setTextCursor(cursor);
            nextWidget->setFocus();

            if (m_curItemWidget == preWidget) {
                m_curItemWidget = nextWidget;
            }
        }
    }

    for (auto i : noteBlockList) {
        i->releaseSpecificData();
        m_noteItemData->delBlock(i);
    }

    if (m_curItemWidget) { //调整滚动条
        int height = 0;
        if (m_curItemWidget->getNoteBlock()->blockType == VNoteBlock::Text) {
            QRect rc = m_curItemWidget->getCursorRect();
            height += rc.bottom();
            m_curItemWidget->setFocus();
        }
        adjustVerticalScrollBar(m_curItemWidget, height);
    }
}

void RightView::setEnablePlayBtn(bool enable)
{
    for (int i = 0; i < m_viewportLayout->count() - 1; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());

        if (widget->getNoteBlock()->blockType == VNoteBlock::Voice) {
            VoiceNoteItem *tmpWidget = static_cast< VoiceNoteItem *>(widget);
            tmpWidget->enblePlayBtn(enable);
        }
    }
}

DetailItemWidget *RightView::getMenuItem()
{
    return m_curItemWidget;
}

void RightView::updateData()
{
    m_fIsNoteModified = true;
    saveNote();
}

bool RightView::checkFileExist(const QString &file)
{
    QFileInfo fi(file);
    if (!fi.exists()) {
        m_fileHasDelDialog->exec();
        return false;
    }
    return true;
}

void RightView::adjustVerticalScrollBar(QWidget *widget, int defaultHeight)
{
    int tolHeight = defaultHeight + 20;
    int index = m_viewportLayout->indexOf(widget);
    for (int i = 0; i < index; i++) {
        tolHeight += m_viewportLayout->itemAt(i)->widget()->height();
    }
    emit sigCursorChange(tolHeight, false);
}

void RightView::mouseMoveEvent(QMouseEvent *event)
{
    emit sigCursorChange(event->pos().y(), true);
    mouseMoveSelect(event);
}

void RightView::mousePressEvent(QMouseEvent *event)
{
    DWidget::mousePressEvent(event);
    Qt::MouseButton btn = event->button();
    DetailItemWidget *widget = getWidgetByPos(event->pos());

    if (widget != nullptr) {
        m_curItemWidget = widget;
    }

    if (btn == Qt::RightButton) {
        onMenuShow(widget);
    } else if (btn == Qt::LeftButton) {
        clearAllSelection();
        if (m_curItemWidget != nullptr) {
            m_curItemWidget->setFocus();
        }
    }
}

void RightView::mouseReleaseEvent(QMouseEvent *event)
{
    DWidget::mouseReleaseEvent(event);
}

DetailItemWidget *RightView::getWidgetByPos(const QPoint &pos)
{
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QWidget *widget = m_viewportLayout->itemAt(i)->widget();
        if (widget->geometry().contains(pos)) {
            return static_cast<DetailItemWidget *>(widget);
        }
    }
    return  nullptr;
}

void RightView::mouseMoveSelect(QMouseEvent *event)
{
    DetailItemWidget *widget = getWidgetByPos(event->pos());

    if (widget && m_curItemWidget) {
        int widgetIndex = m_viewportLayout->indexOf(widget);
        int curIndex = m_viewportLayout->indexOf(m_curItemWidget);
        QTextCursor::MoveOperation op = QTextCursor::NoMove;
        int minIndex = 0;
        int maxIndex = 0;
        if (widgetIndex != curIndex) {
            if (widgetIndex < curIndex) {
                op = QTextCursor::End;
                minIndex = widgetIndex + 1;
                maxIndex = curIndex - 1;
            } else {
                op = QTextCursor::Start;
                minIndex = curIndex + 1;
                maxIndex = widgetIndex - 1;
            }
        }

        widget->selectText(event->globalPos(), op);
        op = op == QTextCursor::Start ? QTextCursor::End : QTextCursor::Start;

        if (minIndex > maxIndex) {
            m_curItemWidget->selectText(op);
        } else if (minIndex && maxIndex) {
            for (int i = 0 ; i < m_viewportLayout->count() - 1; i++) {
                QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
                DetailItemWidget *tmpWidget = static_cast<DetailItemWidget *>(layoutItem->widget());
                if (i < minIndex || i > maxIndex) {
                    if (i == curIndex) {
                        tmpWidget->selectText(op);
                    } else if (i != widgetIndex) {
                        tmpWidget->clearSelection();
                    }
                } else {
                    tmpWidget->selectAllText();
                }
            }
        }
    }
}

QString RightView::copySelectText()
{
    QString text = "";
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast< DetailItemWidget *>(layoutItem->widget());

        bool hasSelection = widget->hasSelection();
        if (hasSelection) {
            text.append(widget->getSelectText());
        }
    }
    QClipboard *board = QApplication::clipboard();
    if (board) {
        board->clear();
        board->setText(text);
    }
    return text;
}

void RightView::cutSelectText(bool isByAction)
{
    if (!isByAction) {
        initAction(m_curItemWidget);
        QAction *cutAction = ActionManager::Instance()->getActionById(ActionManager::DetailCut);
        if (!cutAction->isEnabled()) {
            qDebug() << "can not cut";
            return;
        }
    }

    copySelectText();
    delSelectText();
}

void RightView::delSelectText()
{
    QList<DetailItemWidget *> delListWidget;
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        if (widget->hasSelection()) {
            if (i == 0 || i == m_viewportLayout->count() - 2) {
                VNoteBlock *block = widget->getNoteBlock();
                if (block->blockType == VNoteBlock::Text) {
                    if (widget->getAllText() != widget->getSelectText()) {
                        widget->removeSelectText();
                        widget->getNoteBlock()->blockText =  widget->getAllText();
                        continue;
                    }
                }
            }
            delListWidget.push_back(widget);
        }
    }
    int size = delListWidget.size();
    if (size) {
        for (int i = 0; i < size; i++) {
            bool merge = i == delListWidget.size() - 1 ? true : false;
            delWidget(delListWidget[i], merge);
        }
        updateData();
    }
}

void RightView::clearAllSelection()
{
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        widget->clearSelection();
    }
}

void RightView::selectAllItem()
{
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        widget->selectAllText();
    }
}

void RightView::getSelectionCount(int &voiceCount, int &textCount)
{
    voiceCount = textCount  = 0;
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        if (widget->hasSelection()) {
            if (widget->getNoteBlock()->blockType == VNoteBlock::Text) {
                textCount ++;
            } else {
                voiceCount ++;
            }
        }
    }
}

void RightView::pasteText()
{
    if (m_curItemWidget && m_curItemWidget->getNoteBlock()->blockType == VNoteBlock::Text) {
        auto textCursor = m_curItemWidget->getTextCursor();
        QClipboard *board = QApplication::clipboard();
        if (board) {
            QString clipBoardText = board->text();
            textCursor.insertText(clipBoardText);
        }
    }
}

void RightView::keyPressEvent(QKeyEvent *e)
{
    DWidget::keyPressEvent(e);
    if (e->modifiers() == Qt::ControlModifier) {
        switch (e->key()) {
        case Qt::Key_C:
            copySelectText();
            break;
        case Qt::Key_A:
            selectAllItem();
            break;
        case Qt::Key_X:
            cutSelectText(false);
            break;
        case Qt::Key_V:
            pasteText();
            break;
        default:
            break;
        }
    }else if (e->key() == Qt::Key_Delete) {
        doDelAction(false);
    }
}

void RightView::setCurVoicePlay(VoiceNoteItem *item)
{
    m_curPlayItem = item;
}

void RightView::setCurVoiceAsr(VoiceNoteItem *item)
{
    m_curAsrItem = item;
}

VoiceNoteItem *RightView::getCurVoicePlay()
{
    return  m_curPlayItem;
}

VoiceNoteItem *RightView::getCurVoiceAsr()
{
    return  m_curAsrItem;
}

bool RightView::hasSelection()
{
    for (int i = 0; i < m_viewportLayout->count() - 1 ; i++) {
        QLayoutItem *layoutItem = m_viewportLayout->itemAt(i);
        DetailItemWidget *widget = static_cast<DetailItemWidget *>(layoutItem->widget());
        if (widget->hasSelection()) {
            return true;
        }
    }
    return false;
}

void RightView::doDelAction(bool isByAction)
{
    if (!isByAction) {
        initAction(m_curItemWidget);
        QAction *delAction = ActionManager::Instance()->getActionById(ActionManager::DetailDelete);
        if (!delAction->isEnabled()) {
            qDebug() << "can not del";
            return;
        }
    }
    if (hasSelection()) {
        delSelectText();
    } else if (m_curItemWidget) {
        VNoteBlock *block = m_curItemWidget->getNoteBlock();
        if (block->blockType == VNoteBlock::Voice) {
            delWidget(m_curItemWidget);
            updateData();
        }
    }
}

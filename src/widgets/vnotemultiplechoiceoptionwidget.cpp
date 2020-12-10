#include "vnotemultiplechoiceoptionwidget.h"
#include "globaldef.h"

#include <DApplication>
#include <QDebug>
#include <QImageReader>

//多选-多选详情页
VnoteMultipleChoiceOptionWidget::VnoteMultipleChoiceOptionWidget(QWidget *parent)
    :DWidget(parent)
{
    initUi();
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::initUi
 * @param
 *///初始化ui
void VnoteMultipleChoiceOptionWidget::initUi()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);

    DLabel *iconLabel = new DLabel(this);
    QImageReader reader;
    QPixmap pixmap;
    QSize size(162,156);
    reader.setFileName(QString(STAND_ICON_PAHT).append("detail_icon/detail_icon_note.svg"));
    const qreal ratio = qApp->devicePixelRatio();
    if (reader.canRead()) {
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(QString(STAND_ICON_PAHT).append("detail_icon/detail_icon_note.svg"));
    }
    iconLabel->setPixmap(pixmap);
//    iconLabel->setFixedSize(186,186);

    QHBoxLayout *iconLayout = new QHBoxLayout(this);
//    iconLayout->addSpacing(58);
    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();

    m_tipsLabel = new DLabel(this);
    m_tipsLabel->setFixedHeight(29);

    DFontSizeManager::instance()->bind(m_tipsLabel,DFontSizeManager::T4);
    QHBoxLayout *tipsLayout = new QHBoxLayout(this);
    tipsLayout->addStretch();
    tipsLayout->addWidget(m_tipsLabel);
    tipsLayout->addStretch();

    m_moveButton = new DToolButton(this);
    m_moveButton->setFixedHeight(26);
    m_moveButton->setIconSize(QSize(24,24));
    m_saveTextButton = new DToolButton(this);
    m_saveTextButton->setFixedHeight(26);
    m_saveTextButton->setIconSize(QSize(24,24));
    m_saveVoiceButton = new DToolButton(this);
    m_saveVoiceButton->setFixedHeight(26);
    m_saveVoiceButton->setIconSize(QSize(24,24));
    m_deleteButton = new DToolButton(this);
    m_deleteButton->setFixedHeight(26);
    m_deleteButton->setIconSize(QSize(24,24));
    m_moveButton->setText(DApplication::translate("NotesContextMenu", "Move"));
    m_deleteButton->setText(DApplication::translate("NotesContextMenu", "Delete"));
    onFontChanged();
    //设置主题
    changeFromThemeType();
    //初始化链接
    initConnections();

    vlayout->addStretch(4);
    vlayout->addLayout(iconLayout);
    vlayout->addSpacing(10);
    vlayout->addLayout(tipsLayout);
    vlayout->addSpacing(20);
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addStretch();
    hlayout->addWidget(m_moveButton);
    hlayout->addWidget(m_saveTextButton);
    hlayout->addWidget(m_saveVoiceButton);
    hlayout->addWidget(m_deleteButton);
//    hlayout->addSpacing(10);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    vlayout->addStretch(7);
    setLayout(vlayout);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::initConnections
 * @param number
 *///初始化链接
void VnoteMultipleChoiceOptionWidget::initConnections(){
    //移动
    connect(m_moveButton,&DToolButton::pressed,this,[=]{
       buttonPressed(ButtonValue::Move);
    });
    connect(m_moveButton,&DToolButton::clicked,this,[=]{
        trigger(ButtonValue::Move);
    });
    //保存文本
    connect(m_saveTextButton,&DToolButton::pressed,this,[=]{
       buttonPressed(ButtonValue::SaveAsTxT);
    });
    connect(m_saveTextButton,&DToolButton::clicked,this,[=]{
        trigger(ButtonValue::SaveAsTxT);
    });
    //保存语音
    connect(m_saveVoiceButton,&DToolButton::pressed,this,[=]{
       buttonPressed(ButtonValue::SaveAsVoice);
    });
    connect(m_saveVoiceButton,&DToolButton::clicked,this,[=]{
        trigger(ButtonValue::SaveAsVoice);
    });
    //删除
    connect(m_deleteButton,&DToolButton::pressed,this,[=]{
       buttonPressed(ButtonValue::Delete);
    });
    connect(m_deleteButton,&DToolButton::clicked,this,[=]{
        trigger(ButtonValue::Delete);
    });
    //恢复图标
    connect(m_saveVoiceButton,&DToolButton::released,this,[=]{
        changeFromThemeType();
    });
    connect(m_saveTextButton,&DToolButton::released,this,[=]{
        changeFromThemeType();
    });
    connect(m_moveButton,&DToolButton::released,this,[=]{
       changeFromThemeType();
    });
    connect(m_deleteButton,&DToolButton::released,this,[=]{
        changeFromThemeType();
    });
    //主题切换更换按钮和文本颜色
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &VnoteMultipleChoiceOptionWidget::changeFromThemeType);

    //字体切换长度适应
    connect(qApp, &DApplication::fontChanged, this, &VnoteMultipleChoiceOptionWidget::onFontChanged);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::buttonPressed
 * @param value
 *///press更新svg
void VnoteMultipleChoiceOptionWidget::buttonPressed(ButtonValue value){
    DPalette dpalette  = DApplicationHelper::instance()->palette(m_deleteButton);
    QColor textColor = dpalette.color(DPalette::Highlight);
   QString color = textColor.name(QColor::HexRgb);
   QString iconPath = QString(STAND_ICON_PAHT).append("light/");
   QPixmap pixmap;
    if(ButtonValue::Move == value){
        pixmap = setSvgColor(iconPath.append("detail_notes_move.svg"),color);
        m_moveButton->setIcon(pixmap);
    }else if (ButtonValue::SaveAsTxT == value) {
        pixmap = setSvgColor(iconPath.append("detail_notes_saveText.svg"),color);
        m_saveTextButton->setIcon(pixmap);
    }else if (ButtonValue::SaveAsVoice == value) {
        pixmap = setSvgColor(iconPath.append("detail_notes_saveVoice.svg"),color);
        m_saveVoiceButton->setIcon(pixmap);
    }else{
        pixmap = setSvgColor(iconPath.append("detail_notes_delete.svg"),color);
        m_deleteButton->setIcon(pixmap);
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setNoteNumber
 * @param number
 *///设置笔记数量
void VnoteMultipleChoiceOptionWidget::setNoteNumber(int number)
{
    if(number != m_noteNumber){
        m_noteNumber = number;
        QString str = QString(DApplication::translate("DetailPage", "%1 notes selected").arg(number));
        m_tipsLabel->setText(str);
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::enableButtons
 * @param saveAsTxtButtonStatus,saveAsVoiceButtonStatus
 *///设置按钮是否置灰
void VnoteMultipleChoiceOptionWidget::enableButtons(bool saveAsTxtButtonStatus, bool saveAsVoiceButtonStatus,bool moveButtonStatus)
{
    m_saveTextButton->setEnabled(saveAsTxtButtonStatus);
    m_saveVoiceButton->setEnabled(saveAsVoiceButtonStatus);
    m_moveButton->setEnabled(moveButtonStatus);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setSvgColor
 * @param svgPath
 * @param color
 *///获得svg
QPixmap VnoteMultipleChoiceOptionWidget::setSvgColor(QString svgPath ,QString color)
{
  //设置图标颜色
    QString path = svgPath;
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    QDomDocument doc;
    doc.setContent(data);
    file.close();
    QDomElement elem = doc.documentElement();
    setSVGBackColor(elem, "fill", color);

//    int scaled =qApp->devicePixelRatio() == 1.25 ? 2 : 1;
    double scaled = qApp->devicePixelRatio();
    QSvgRenderer svg_render(doc.toByteArray());

    QPixmap pixmap(QSize(24,24)*scaled);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(scaled);

    QPainter painter(&pixmap);
    svg_render.render(&painter,QRect(0,0,24,24));
    return  pixmap;
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setSVGBackColor
 * @param ielem
 * @param attr
 * @param val
 *///设置svg颜色属性
void VnoteMultipleChoiceOptionWidget::setSVGBackColor(QDomElement &elem, QString attr, QString val)
{
    if (elem.tagName().compare("g") == 0 && elem.attribute("id").compare("color") == 0)
    {
        QString before_color = elem.attribute(attr);
        elem.setAttribute(attr, val);
    }
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement()) continue;
        QDomElement element = elem.childNodes().at(i).toElement();
        setSVGBackColor(element, attr, val);
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::trigger
 * @param id
 *///触发菜单操作
void VnoteMultipleChoiceOptionWidget::trigger(int id){
    m_moveButton->setAttribute(Qt::WA_UnderMouse, false);
    m_saveTextButton->setAttribute(Qt::WA_UnderMouse, false);
    m_saveVoiceButton->setAttribute(Qt::WA_UnderMouse, false);
    m_deleteButton->setAttribute(Qt::WA_UnderMouse, false);
    emit requestMultipleOption(id);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::changeFromThemeType
 * @param
 *///多选-根据主题设置图标与删除按钮文本颜色
void VnoteMultipleChoiceOptionWidget::changeFromThemeType(){
    bool isDark = (DApplicationHelper::DarkType ==  DApplicationHelper::instance()->themeType())? true:false;
    QString iconPath = QString(STAND_ICON_PAHT);
    if(isDark){
        iconPath.append("dark/");
        //设置字体颜色（特殊颜色与UI沟通可以不根据DTK色板单独设置）
        DPalette deletePalette = DApplicationHelper::instance()->palette(m_deleteButton);
        deletePalette.setBrush(DPalette::ButtonText,QColor("#9A2F2F"));
        DApplicationHelper::instance()->setPalette(m_deleteButton, deletePalette);
    }else {
        iconPath.append("light/");
        //设置字体颜色（特殊颜色与UI沟通可以不根据DTK色板单独设置）
        DPalette deletePalette = DApplicationHelper::instance()->palette(m_deleteButton);
        deletePalette.setBrush(DPalette::ButtonText,QColor("#FF5736"));
        DApplicationHelper::instance()->setPalette(m_deleteButton, deletePalette);
    }
    //根据主题设置图标
    m_moveButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_move.svg")));
    m_saveTextButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_saveText.svg")));
    m_saveVoiceButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_saveVoice.svg")));
    m_deleteButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_delete.svg")));
}

void VnoteMultipleChoiceOptionWidget::onFontChanged(){
    QFontMetrics fontMetrics(m_deleteButton->font());
    int midWidth = width()-m_moveButton->width()-m_deleteButton->width();
    int iconWidth = qCeil(qApp->devicePixelRatio()*24)+11;
    int saveTextWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Save as TXT"))+iconWidth;
    int saveVoiceWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Save voice recording"))+iconWidth;
    if(midWidth>saveTextWidth+saveVoiceWidth+11){
        m_saveTextButton->setFixedWidth(saveTextWidth+5);
        m_saveVoiceButton->setFixedWidth(saveVoiceWidth+5);
        m_saveTextButton->setText(DApplication::translate("NotesContextMenu", "Save as TXT"));
        m_saveVoiceButton->setText(DApplication::translate("NotesContextMenu", "Save voice recording"));
    }else {
        m_saveTextButton->setFixedWidth(midWidth/2-11);
        m_saveVoiceButton->setFixedWidth(midWidth/2-11);
        m_saveTextButton->setText(fontMetrics.elidedText(DApplication::translate("NotesContextMenu", "Save as TXT"),Qt::ElideRight,midWidth/2-iconWidth-11));
        m_saveVoiceButton->setText(fontMetrics.elidedText(DApplication::translate("NotesContextMenu", "Save voice recording"),Qt::ElideRight,midWidth/2-iconWidth-11));
    }
}

void VnoteMultipleChoiceOptionWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    onFontChanged();
}

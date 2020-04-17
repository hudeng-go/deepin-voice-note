#include "vnoteforlder.h"

#include <DLog>

VNoteFolder::VNoteFolder()
{
}

VNoteFolder::~VNoteFolder()
{
}

bool VNoteFolder::isValid()
{
    return (id > INVALID_ID) ? true : false;
}

qint32 &VNoteFolder::maxNoteIdRef()
{
    return maxNoteId;
}

QDebug & operator <<(QDebug &out, VNoteFolder &folder)
{
    out << "\n{ "
        << "id=" << folder.id << ","
        << "name=" << folder.name << ","
        << "defaultIcon=" << folder.defaultIcon << ","
        << "iconPath=" << folder.iconPath << ","
        << "notesCount=" << folder.notesCount << ","
        << "folder_state=" << folder.folder_state << ","
        << "maxNoteId=" << folder.maxNoteId << ","
        << "createTime=" << folder.createTime << ","
        << "modifyTime=" << folder.modifyTime << ","
        << "deleteTime=" << folder.deleteTime
        << " }\n";

    return out;
}
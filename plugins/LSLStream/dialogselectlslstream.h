#ifndef DIALOGSELECTLSLSTREAM_H
#define DIALOGSELECTLSLSTREAM_H

#include <QDialog>
#include "LSL/include/lsl_cpp.h"
#include <QStringListModel>
#include <QStandardItemModel>

namespace Ui {
class DialogSelectLSLStream;
}

class DialogSelectLSLStream : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectLSLStream(QWidget *parent = nullptr);
    ~DialogSelectLSLStream();

    QStringList getSelectedStreams();

private:
    void resolveLSLStreams();

private:
    Ui::DialogSelectLSLStream *ui;
    QStandardItemModel model;
};

#endif // DIALOGSELECTLSLSTREAM_H

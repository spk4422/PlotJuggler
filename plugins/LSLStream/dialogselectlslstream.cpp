#include "dialogselectlslstream.h"
#include "ui_dialogselectlslstream.h"
#include <QtDebug>

DialogSelectLSLStream::DialogSelectLSLStream(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectLSLStream)
{
    ui->setupUi(this);

    ui->tableView->setModel(&model);

    resolveLSLStreams();
}

DialogSelectLSLStream::~DialogSelectLSLStream()
{
    delete ui;
}

QStringList DialogSelectLSLStream::getSelectedStreams()
{
    QStringList selected_streams;

    QModelIndexList list =ui->tableView->selectionModel()->selectedRows(0);


    foreach(const QModelIndex &index, list){
        selected_streams.append(index.data(Qt::DisplayRole).toString());
        qDebug() << "Selected Indexes : " << index.data(Qt::DisplayRole).toString();
    }

    return selected_streams;
}

void DialogSelectLSLStream::resolveLSLStreams()
{
    std::vector<lsl::stream_info> streams = lsl::resolve_streams();
    QStringList stream_list;

    model.setRowCount(int(streams.size()));
    model.setColumnCount(3);

    model.setHeaderData(0, Qt::Horizontal, tr("ID"));
    model.setHeaderData(1, Qt::Horizontal, tr("Name"));
    model.setHeaderData(2, Qt::Horizontal, tr("Type"));

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for (unsigned int i = 0; i < streams.size(); ++i) {

        lsl::stream_info info = streams.at(i);

        stream_list.append(QString::fromStdString(info.name()));

        QStandardItem *item0 = new QStandardItem(QString::fromStdString(info.source_id()));
        QStandardItem *item1 = new QStandardItem(QString::fromStdString(info.name()));
        QStandardItem *item2 = new QStandardItem(QString::fromStdString(info.type()));

        model.setItem(i, 0, item0);
        model.setItem(i, 1, item1);
        model.setItem(i, 2, item2);

    }
}

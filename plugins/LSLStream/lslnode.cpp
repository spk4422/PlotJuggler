#include "lslnode.h"
#include <iostream>
#include <bits/stdc++.h>
#include <utility>
#include <QMessageBox>

LSLNode::LSLNode()
{
    setRunning(false);
}

LSLNode::~LSLNode()
{
    shutdown();
}

bool LSLNode::start(QStringList *)
{

    DialogSelectLSLStream dialog;

    int res = dialog.exec();

    QStringList selection = dialog.getSelectedStreams();


    if( res != QDialog::Accepted || selection.empty() ) {
        return false;
    }


    {
        std::lock_guard<std::mutex> lock( mutex() );
        dataMap().numeric.clear();
        dataMap().user_defined.clear();
    }

    if(!initStreams(selection)) {
        QMessageBox::information(nullptr, tr("Info"), tr("Cannot find any streams!, Make sure LSL Node is running!"));
        return false;
    }

    setRunning(true);
    worker_thread_ = std::thread(&LSLNode::loop, this);

    return true;
}

void LSLNode::shutdown()
{
    setRunning(false);
    setStreamsCMD(LSLStream::CMD::STOP);


    for (unsigned int i = 0; i < streams_.size(); ++i) {
        streams_[i]->setStreamCMD(LSLStream::CMD::STOP);
    }

    if( worker_thread_.joinable()) worker_thread_.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    qDeleteAll(streams_);
}

bool LSLNode::isRunning() const
{
    return is_running_.get();
}

bool LSLNode::initStreams(QStringList streams)
{

    for (QString name : streams) {

        std::cout << "Now resolving streams..." << name.toStdString() << std::endl;
        std::vector<lsl::stream_info> resolved_streams = lsl::resolve_stream("source_id", name.toStdString());

        if(resolved_streams.empty()) {
            std::cout << "Cannot Resolve stream " << name.toStdString() << std::endl;
            return false;
        }

        std::cout << "TOTAL NUMBER OF STREAMS FOUND " << resolved_streams.size() << std::endl;
        for(lsl::stream_info stream_info : resolved_streams) {
            streams_.push_back(new LSLStream(stream_info));
        }
    }

    return true;
}

void LSLNode::loop()
{
    for(auto stream : streams_) {
        stream->init();
    }

    setStreamsCMD(LSLStream::CMD::START);

    //loop over each stream fetch latest data frame -> update plots
    while(isRunning()) {
        pushSingleCycle();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void LSLNode::setStreamsCMD(LSLStream::CMD cmd)
{
    for(auto stream : streams_)
    {
        stream->setStreamCMD(cmd);
    }
}


void LSLNode::setRunning(bool is_running)
{
    is_running_.set(is_running);
}

void LSLNode::pushSingleCycle()
{
    std::lock_guard<std::mutex> lock( mutex() );

    for(LSLStream* stream : streams_)
    {
        std::vector<std::vector<std::pair<double, double>>> stream_data_chunk = stream->getDataChunks();

        if(stream_data_chunk.empty())
            continue;

        std::vector<std::string> channel_names = stream->streamChannelsIDs();

        for (unsigned int i = 0; i < channel_names.size(); ++i) {
            std::string ch = channel_names[i];
            auto index_it = dataMap().numeric.find(ch);

            if( index_it == dataMap().numeric.end()) {
                index_it = dataMap().addNumeric(ch);
            }

            index_it->second.pushBack( PlotData::Point( stream_data_chunk.at(i).back().second, stream_data_chunk.at(i).back().first ));
        }
    }
}

bool LSLNode::xmlSaveState(QDomDocument &doc, QDomElement &parent_element) const
{
    Q_UNUSED(doc)
    Q_UNUSED(parent_element)

    return true;
}

bool LSLNode::xmlLoadState(const QDomElement &parent_element)
{
    Q_UNUSED(parent_element)
    return true;
}

#include "lslnode.h"
#include <iostream>
#include <bits/stdc++.h>
#include  <utility>

LSLNode::LSLNode()
{
    //    worker_thread_cmd_.set(LSLStream::CMD::IDLE);
    run(false);
}

LSLNode::~LSLNode()
{
    shutdown();
}

bool LSLNode::xmlSaveState(QDomDocument &doc, QDomElement &parent_element) const
{

    return true;
}

bool LSLNode::xmlLoadState(const QDomElement &parent_element)
{

    return true;
}


bool LSLNode::start(QStringList *)
{

    //    DialogSelectLSLStream dialog;

    //    int res = dialog.exec();

    //    QStringList selection = dialog.getSelectedStreams();



    //    if( res != QDialog::Accepted || selection.empty() )
    //    {
    //        return false;
    //    }


    //    qDebug() << selection;


    {
        std::lock_guard<std::mutex> lock( mutex() );
        dataMap().numeric.clear();
        dataMap().user_defined.clear();
    }

    getAvailableStreams();


    if(streams_.empty())
    {
        std::cout << "CAN'T FIND SHIT!!!!!!!!!" << std::endl;
        return false;
    }

    //    for(auto stream : streams_)
    //    {
    //        initializeStreamPlot(stream);
    //    }

    //initializeStreamsPlots();

    run(true);
    worker_thread_ = std::thread(&LSLNode::loop, this);

    return true;
}

void LSLNode::shutdown()
{
    run(false);
    setStreamsCMD(LSLStream::CMD::STOP);

    if( worker_thread_.joinable()) worker_thread_.join();
}

bool LSLNode::isRunning() const
{
//    std::cout << "isRunning " << is_running_.get() << std::endl;
    return is_running_.get();
}

bool LSLNode::getAvailableStreams()
{
    std::cout << "Now resolving streams..." << std::endl;
    std::vector<lsl::stream_info> available_streams_ = lsl::resolve_streams();
    if(available_streams_.empty())
    {
        std::cout << "CAN'T FIND SHIT!!!!!!!!!" << std::endl;
        return false;
    }

    std::cout << "TOTAL NUMBER OF STREAMS FOUND " << available_streams_.size() << std::endl;
    for(lsl::stream_info stream_info : available_streams_)
    {
        streams_.push_back(new LSLStream(stream_info));
    }

    return true;
}

void LSLNode::initializeStreamPlot(LSLStream* stream)
{
    std::vector<std::string> channel_names = stream->streamChannelsIDs();

    for (std::string name : channel_names)
    {
        std::cout << "Channel Name: " << name <<std::endl;
        dataMap().addNumeric(name);
    }
}

//loop over each stream fetch latest data frame -> update plots
void LSLNode::loop()
{
    for(auto stream : streams_) {
        stream->init();
    }

    setStreamsCMD(LSLStream::CMD::START);
    //    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    //worker_thread_cmd_.set(LSLStream::CMD::START);

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


void LSLNode::run(bool is_running)
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

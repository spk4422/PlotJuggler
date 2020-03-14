#include "lslnode.h"
#include <iostream>

LSLNode::LSLNode()
{
}

void LSLNode::pullData(lsl::stream_info stream_info)
{
    lsl::stream_inlet inlet(stream_info);
    std::vector<float> sample;
    while (isRunning()) {
        inlet.pull_sample(sample);

        std::cout << std::this_thread::get_id() ;
        for (int i = 0; i < sample.size(); ++i) {
            std::cout << sample[i];
        }
        std::cout << std::endl;
    }
}

bool LSLNode::start(QStringList *)
{
    available_streams_ = getAvailableStreams();
    if(available_streams_.empty())
        return false;
    // make an inlet to get data from it
    std::cout << "Now creating the inlet..." << std::endl;

    run(true);
    thread_ = std::thread(&LSLNode::pullData, this, available_streams_[0]);
    return true;
}

void LSLNode::shutdown()
{
    //stop the thread
    run(false);
    if(thread_.joinable())
        thread_.join();
}

std::vector<lsl::stream_info> LSLNode::getAvailableStreams()
{
    std::cout << "Now resolving streams..." << std::endl;
    return lsl::resolve_streams(1.0);
}

bool LSLNode::isRunning() const
{
    return is_running_;
}

void LSLNode::run(bool is_running)
{
    std::lock_guard<std::mutex> lock( mutex() );
    is_running_ = is_running;
}

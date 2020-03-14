#include "lslnode.h"
#include <iostream>

LSLNode::LSLNode()
{
  PRINT_FUNCTION_NAME
}

void LSLNode::initPlot(lsl::stream_info stream_info)
{
  PRINT_FUNCTION_NAME
  if(stream_info.channel_count() == 0)
    throw std::runtime_error("SHIT DUDE ");
  lsl::xml_element ch = stream_info.desc().child("channels").child("channel");
  for (size_t i = 0; i < stream_info.channel_count(); ++i)
  {
    dataMap().addNumeric(ch.child_value("label"));
    ch = ch.next_sibling();
  }
}

void LSLNode::plotFrame(const std::vector<float>& frame)
{
  static std::chrono::high_resolution_clock::time_point initial_time = std::chrono::high_resolution_clock::now();
  const double offset = std::chrono::duration_cast< std::chrono::duration<double>>( initial_time.time_since_epoch() ).count() ;

  auto now =  std::chrono::high_resolution_clock::now();
  for (auto& it: dataMap().numeric )
  {
    auto& plot = it.second;
    const double t = std::chrono::duration_cast< std::chrono::duration<double>>( now - initial_time ).count() ;
    plot.pushBack( PlotData::Point( t + offset, frame[0] ) );
  }
}

void LSLNode::pullData(lsl::stream_info stream_info)
{
  PRINT_FUNCTION_NAME
  lsl::stream_inlet inlet(stream_info);
  initPlot(inlet.info());
  while (isRunning()) {
    std::vector<float> sample;
    inlet.pull_sample(sample);
    plotFrame(sample);
  }
}

bool LSLNode::start(QStringList *)
{
  PRINT_FUNCTION_NAME
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
  run(false);
  if(thread_.joinable())
    thread_.join();
}

std::vector<lsl::stream_info> LSLNode::getAvailableStreams()
{
  PRINT_FUNCTION_NAME
  std::cout << "Now resolving streams..." << std::endl;
  return lsl::resolve_streams(1.0);
}

bool LSLNode::isRunning() const
{
  //  PRINT_FUNCTION_NAME
  return is_running_;
}

void LSLNode::run(bool is_running)
{
  //  PRINT_FUNCTION_NAME
  std::lock_guard<std::mutex> lock( mutex() );
  is_running_ = is_running;
}

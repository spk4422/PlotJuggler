#include "lslnode.h"
#include <iostream>
#include <bits/stdc++.h>
#include  <utility>
LSLNode::LSLNode()
{
  worker_thread_cmd_.set(LSLStream::CMD::IDLE);
  getAvailableStreams();
}


bool LSLNode::start(QStringList *)
{
  PRINT_FUNCTION_NAME
  // make an inlet to get data from it
  std::cout << "Now creating the inlet..." << std::endl;

  if(streams_.empty())
  {
    std::cout << "CAN'T FIND SHIT!!!!!!!!!" << std::endl;
    return false;
  }
  run(true);
  initializeStreamsPlots();
  setStreamsCMD(LSLStream::CMD::START);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  worker_thread_cmd_.set(LSLStream::CMD::START);
  return true;
}

void LSLNode::shutdown()
{
  run(false);
  setStreamsCMD(LSLStream::CMD::STOP);
}

bool LSLNode::isRunning() const
{
  //  PRINT_FUNCTION_NAME
  return is_running_.get();
}

bool LSLNode::getAvailableStreams()
{
  PRINT_FUNCTION_NAME
  std::cout << "Now resolving streams..." << std::endl;
  std::vector<lsl::stream_info> available_streams_ = lsl::resolve_streams(1.0);;
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
  PRINT_FUNCTION_NAME
  for (std::string name : channel_names)
  {
    std::cout << "Channel Name: " << name <<std::endl;
    dataMap().addNumeric(name);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  worker_thread_ = std::thread(&LSLNode::plotFrame,this);
}

//loop over each stream fetch latest data frame -> update plots
void LSLNode::plotFrame()
{
  PRINT_FUNCTION_NAME
  while(isRunning())
  {
    if (worker_thread_cmd_.get()!=LSLStream::CMD::START)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }

    static std::chrono::high_resolution_clock::time_point initial_time = std::chrono::high_resolution_clock::now();
    const double offset = std::chrono::duration_cast< std::chrono::duration<double>>( initial_time.time_since_epoch() ).count() ;
    auto now =  std::chrono::high_resolution_clock::now();
    for(LSLStream* stream : streams_)
    {
      std::vector<std::string> channel_names = stream->streamChannelsIDs();

      for(std::string channel_id : channel_names)
      {
         std::lock_guard<std::mutex> lock( mutex() );

        std::unordered_map<std::string, PlotData>::iterator it = dataMap().numeric.find(channel_id);

        std::cout << "Channel Id: " << channel_id << std::endl;

//        if( it != dataMap().numeric.end())
//        {
//        PRINT_LINEgetDataChunks
          std::vector<std::vector<std::pair<double, double>>> stream_data_chunk = stream->getDataChunks();
//          std::cout << "stream_data_chunk Size: " << stream_data_chunk.size() << std::endl;
        if(stream_data_chunk.empty())
          continue;
        int index =0 ;
        for (auto& it: dataMap().numeric )
        {

          if( it.first == "empty")  throw;

          std::cout << "Keys: " << it.first << std::endl;
          auto& plot = it.second;
//          std::cout << "Plot Itereator Index: " <<  index << std::endl;
//          std::cout << "Curr Frame Size: " << stream_data_chunk.at(index).size() << std::endl;
//          PlotData& plot = it->second;
//          std::vector<std::vector<std::pair<double, double>>> stream_data_chunk = stream->getDataChunks();
          const double t = std::chrono::duration_cast< std::chrono::duration<double>>( now - initial_time ).count() ;
          std::cout << "Timestamp: " << stream_data_chunk.at(index).back().second << " t + offset : " <<  t + offset << std::endl;
          std::cout << "Value: " << stream_data_chunk.at(index).back().first << std::endl;
          plot.pushBack( PlotData::Point( stream_data_chunk.at(index).back().second, stream_data_chunk.at(index).back().first ));
          index++;
          //          for(const std::vector<std::pair<double, double>>& channel : stream_data_chunk)
//          {
//            for(const std::pair<double, double>& data_point_time_stamp_pair : channel)
//            {
//                std::cout << "Data Timestamp: " << data_point_time_stamp_pair.second << std::endl;
//                plot.pushBack(PlotData::Point(data_point_time_stamp_pair.second, data_point_time_stamp_pair.first));
//            }
//          }
        }
//        }
//        else
//        {

//          std::cout<< "CANT FIND CHANNEL?? " <<  channel_id << std::endl;
//          throw;
//        }
      }
    }
    //sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void LSLNode::setStreamsCMD(LSLStream::CMD cmd)
{
  for(auto stream : streams_)
  {
    stream->setStreamCMD(cmd);
  }
}


void LSLNode::initializeStreamsPlots()
{
  PRINT_FUNCTION_NAME
  for(auto stream : streams_)
  {
    initializeStreamPlot(stream);
    stream->init();
  }
}

void LSLNode::run(bool is_running)
{
  is_running_.set(is_running);
}

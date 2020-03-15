#ifndef LSLSTREAM_H
#define LSLSTREAM_H

#include "ThreadSafeVariable.h"
#include "LSL/include/lsl_cpp.h"
#include <thread>
#include <iostream>

#define PRINT_FUNCTION_NAME std::cout << __FUNCTION__ << std::endl;
#define PRINT_LINE std::cout << __LINE__ << std::endl;
class LSLStream
{
public:
  typedef enum{
    START,
    PAUSE,
    STOP,
    IDLE,
    } CMD;

  LSLStream(lsl::stream_info stream_info)
  {
    PRINT_FUNCTION_NAME
    stream_inlet_ = new lsl::stream_inlet(stream_info);
    stream_info_ = stream_inlet_->info();
    stream_command_.set(CMD::IDLE);
  }
  ~LSLStream(){}
  std::thread thread_;
  std::vector<std::vector<std::pair<double, double>>> getDataChunks();
  CMD getStreamCMD() const;
  void setStreamCMD(const CMD & stream_command);
  void init();
  lsl::stream_info getStreaminfo() const;
  //  void fetchDataChunks(lsl::stream_inlet &stream_inlet, ThreadSafeVariable<std::vector<std::vector<std::pair<double, double>>>> &stream_data_chunks);
  void fetchDataChunks();
  std::vector<std::string> streamChannelsIDs();

private:
  lsl::stream_inlet* stream_inlet_;
  lsl::stream_info stream_info_;
  ThreadSafeVariable<CMD> stream_command_;
  void pullData(lsl::stream_info stream_info);
  ThreadSafeVariable<std::vector<std::vector<std::pair<double, double>>>> stream_data_chunks_;
};
#endif // LSLSTREAM_H

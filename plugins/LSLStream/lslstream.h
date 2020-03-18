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

    LSLStream(lsl::stream_info stream_info);
    ~LSLStream(){}

    CMD getStreamCMD() const;
    void setStreamCMD(const CMD & stream_command);
    void init();

    void fetchDataChunks();
    lsl::stream_info getStreaminfo() const;
    std::vector<std::string> streamChannelsIDs();
    std::vector<std::vector<std::pair<double, double>>> getDataChunks();

private:
    std::thread thread_;
    lsl::stream_inlet* stream_inlet_;
    lsl::stream_info stream_info_;
    ThreadSafeVariable<CMD> stream_command_;

    void pullData(lsl::stream_info stream_info);
    ThreadSafeVariable<std::vector<std::vector<std::pair<double, double>>>> stream_data_chunks_;

    std::vector<std::string> channel_list;
};
#endif // LSLSTREAM_H

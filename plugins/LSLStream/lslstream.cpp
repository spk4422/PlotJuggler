#include "lslstream.h"

void LSLStream::setStreamCMD(const CMD &stream_command)
{
  std::cout <<  "stream_command " << stream_command << std::endl;
  stream_command_.set(stream_command);
}

LSLStream::CMD LSLStream::getStreamCMD() const
{
  return stream_command_.get();
}

void LSLStream::init()
{
  thread_ = std::thread(&LSLStream::fetchDataChunks, this);
}

lsl::stream_info LSLStream::getStreaminfo() const
{
  return stream_info_;
}

std::vector<std::vector<std::pair<double, double>>> LSLStream::getDataChunks()
{
  std::vector<std::vector<std::pair<double, double>>> local_copy = stream_data_chunks_.get();
  std::vector<std::vector<std::pair<double, double>>> v;
  stream_data_chunks_.set(v);
  return local_copy;
}

/*loop over each channel
     *loop over each time stamp
     *push back std::pair for corresponding buffer channel
     * std::vector<std::vector<double>> & std::vector<double>  ===> std::vector<std::vector<std::pair<double, double>>>
     */

void LSLStream::fetchDataChunks()
{
  PRINT_FUNCTION_NAME
  double freq = stream_info_.nominal_srate();
  int sleep_time = (1000*2)/freq;
  const int number_of_channels = stream_info_.channel_count();

  std::cout <<  "number_of_channels " << number_of_channels << std::endl;
  std::cout << stream_info_.as_xml() << std::endl;
  while(getStreamCMD() != LSLStream::CMD::STOP)
  {
    if(getStreamCMD() == LSLStream::CMD::START)
    {
      //fetch the data from the lsl inlet
      std::vector<std::vector<double>> latest_stream_data_chunk_values;
      std::vector<double> latest_stream_data_chunk_time_stamps;
      bool did_get_data = stream_inlet_->pull_chunk(latest_stream_data_chunk_values,latest_stream_data_chunk_time_stamps);

//      std::cout << "did_get_data " << did_get_data << std::endl;
      if(latest_stream_data_chunk_time_stamps.empty())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        continue;
      }


      std::vector<std::vector<std::pair<double, double>>> latest_stream_data_chunk(number_of_channels);
      //format the new data chunk
      for(size_t i = 0; i < number_of_channels; i++)
      {

         std::vector<std::pair<double, double>> channel_value_time_stamp_pairs;

        channel_value_time_stamp_pairs.resize(latest_stream_data_chunk_time_stamps.size());
        const int number_of_frames = latest_stream_data_chunk_time_stamps.size();
        for(size_t j = 0; j < number_of_frames; j++)
        {
          channel_value_time_stamp_pairs[j] = std::make_pair(latest_stream_data_chunk_values.at(j).at(i),latest_stream_data_chunk_time_stamps.at(j));
        }
        latest_stream_data_chunk[i] = channel_value_time_stamp_pairs;
      }

      //resize
      std::vector<std::vector<std::pair<double, double>>> buffer_stream_data_chunks(number_of_channels);// = stream_data_chunks.get();
      std::vector<std::vector<std::pair<double, double>>> current_buffered_data_chunk = stream_data_chunks_.get();
      unsigned int sample_index = 0;

      for(size_t i = 0; i < number_of_channels; ++i)
      {
        int total_number_of_samples_to_buffer;

        if(!current_buffered_data_chunk.empty())
        {
          total_number_of_samples_to_buffer = current_buffered_data_chunk.at(i).size() + latest_stream_data_chunk.at(i).size();
        }
        else
          total_number_of_samples_to_buffer = latest_stream_data_chunk.at(i).size();
        //copy current buffered data
        if(!current_buffered_data_chunk.empty()){
          for (int k = 0; k < current_buffered_data_chunk.at(i).size(); ++k)
          {
            //buffer_stream_data_chunks[i][sample_index] = current_buffered_data_chunk.at(i).at(k);
            buffer_stream_data_chunks[i].push_back(current_buffered_data_chunk.at(i).at(k));
            //            sample_index++;
          }

        }

        //copy latest buffer data from lsl stream
        for (int k = 0; k < latest_stream_data_chunk.at(i).size(); ++k)
        {
          //          buffer_stream_data_chunks[i][sample_index] = latest_stream_data_chunk.at(i).at(k);
          //          sample_index++;
          buffer_stream_data_chunks[i].push_back(latest_stream_data_chunk.at(i).at(k));
        }
        //        std::copy(current_buffered_data_chunk.at(i).begin(), current_buffered_data_chunk.at(i).end(),buffer_stream_data_chunks.at(i).begin());
        //        std::copy(latest_stream_data_chunk.at(i).begin(), latest_stream_data_chunk.at(i).end(),buffer_stream_data_chunks.at(i).begin()+current_buffered_data_chunk.at(i).size()-1);
      }

      stream_data_chunks_.set(buffer_stream_data_chunks);


      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
  }
}

std::vector<std::string> LSLStream::streamChannelsIDs()
{
  PRINT_FUNCTION_NAME
  std::vector<std::string> stream_channels_ids_;
  lsl::xml_element ch = stream_info_.desc().child("channels").child("channel");
  std::string stream_prefix = stream_info_.source_id() + "_" +stream_info_.type();
  for (size_t i = 0; i < stream_info_.channel_count(); ++i)
  {
    std::string ch_name = std::string(ch.child_value("label"));
    if(ch_name.empty())
      ch_name = "channel_" + std::to_string(i);
    stream_channels_ids_.push_back( stream_prefix +"_" + ch_name);
    ch = ch.next_sibling();
  }
  return stream_channels_ids_;
}

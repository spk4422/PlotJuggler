#include "lslnode.h"
#include <iostream>

using namespace std;

LSLNode::LSLNode(QObject *parent) : QObject(parent)
{

     is_running = false;

     std::cout << "Now resolving streams..." << std::endl;
     std::vector<lsl::stream_info> results = lsl::resolve_streams(1.0);

     QStringList s_list;
     for (int i = 0; i < results.size(); ++i) {
         s_list.append(QString::fromStdString(results[0].name()));
     }

     streams.setStringList(s_list);

     std::cout << "Here is what was resolved: " << std::endl;
     //    std::cout << results[0].as_xml() << std::endl;

     cout << results[0].name() << endl;

     // make an inlet to get data from it
     std::cout << "Now creating the inlet..." << std::endl;


     is_running = true;
     thread_ = std::thread(&LSLNode::pullData, this, results[0]);

}

void LSLNode::pullData(lsl::stream_info info)
{
    lsl::stream_inlet inlet(info);
    // start receiving & displaying the data
    std::cout << "Now pulling samples..." << std::endl;

    std::vector<float> sample;
    while (is_running) {
        inlet.pull_sample(sample);

        cout << std::this_thread::get_id();
        for (int i = 0; i < sample.size(); ++i) {
            cout << sample[i];
        }
        cout << endl;
    }

}

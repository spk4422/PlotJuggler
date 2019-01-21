#ifndef DATALOAD_ROS_H
#define DATALOAD_ROS_H

#include <QObject>
#include <QtPlugin>

#include <ros/ros.h>
#include <rosbag/bag.h>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "PlotJuggler/dataloader_base.h"
#include <ros_type_introspection/ros_introspection.hpp>

class  DataLoadROS: public QObject, DataLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.icarustechnology.PlotJuggler.DataLoader" "../dataloader.json")
    Q_INTERFACES(DataLoader)

public:
    DataLoadROS();
    virtual const std::vector<const char*>& compatibleFileExtensions() const override;

    virtual PlotDataMapRef readDataFromFile(const QString& file_name, bool use_previous_configuration ) override;

    virtual const char* name() const override { return "DataLoad ROS bags"; }

    virtual ~DataLoadROS();

    virtual QDomElement xmlSaveState(QDomDocument &doc) const override;

    virtual bool xmlLoadState(QDomElement &parent_element ) override;

protected:
    void loadSubstitutionRule(QStringList all_topic_names);
    std::shared_ptr<rosbag::Bag> _bag;

private:
    RosIntrospection::SubstitutionRuleMap  _rules;

    std::vector<const char*> _extensions;

    QStringList _default_topic_names;

    std::unique_ptr<RosIntrospection::Parser> _parser;

    bool _use_renaming_rules;

    std::vector<std::pair<QString, QString>> getAndRegisterAllTopics();

    void storeMessageInstancesAsUserDefined(PlotDataMapRef& plot_map,
                                            const std::string &prefix,
                                            bool use_header_stamp);

    std::unordered_set<std::string> _warning_headerstamp;
    std::unordered_set<std::string> _warning_monotonic;
    std::unordered_set<std::string> _warning_cancellation;
    std::unordered_set<std::string> _warning_max_arraysize;

    struct PackagedMessage
    {
        std::string topic_name;
        std::vector<uint8_t> buffer;
        double msg_time;
    };
    std::deque<PackagedMessage> _task_queue;
    std::mutex _mutex_queue;
    std::condition_variable _signal_empty_queue;
    std::thread _worker_thread;

    void workerLoop(PlotDataMapRef& plot_data, const std::string& prefix,
                    int max_array_size, bool use_header_stamp);
};

#endif // DATALOAD_CSV_H

#ifndef LSLNODE_H
#define LSLNODE_H

#include <QtPlugin>
#include <QMenu>
#include <thread>
//#include <PlotJuggler/optional.hpp>
//#include "LSL/include/lsl_cpp.h"
#include "lslstream.h"
#include "PlotJuggler/datastreamer_base.h"
#include "dialogselectlslstream.h"


class LSLNode : public DataStreamer
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.icarustechnology.PlotJuggler.DataStreamer" "../datastreamer.json")
    Q_INTERFACES(DataStreamer)

public:
    LSLNode();
    virtual bool start(QStringList*) override;
    virtual void shutdown() override;
    virtual bool isRunning() const override;
    virtual ~LSLNode() override;
    virtual const char* name() const override {return PLUGIN_NAME_.c_str();}
    virtual bool isDebugPlugin() override { return false; }
    virtual bool xmlSaveState(QDomDocument &doc, QDomElement &parent_element) const override;
    virtual bool xmlLoadState(const QDomElement &parent_element ) override;

signals:

private:
    void setRunning(bool is_running);
    bool initStreams(QStringList streams);
    void loop();
    void setStreamsCMD(LSLStream::CMD cmd);

    void pushSingleCycle();

    QAction* action_LSL_;
    const std::string PLUGIN_NAME_ = "LSL Stream";
    ThreadSafeVariable<bool> is_running_;

    std::vector<LSLStream*> streams_;
    std::thread worker_thread_;
};

#endif // LSLNODE_H

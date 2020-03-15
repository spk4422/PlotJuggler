#ifndef LSLNODE_H
#define LSLNODE_H

#include <QtPlugin>
#include <QMenu>
#include <thread>
//#include <PlotJuggler/optional.hpp>
//#include "LSL/include/lsl_cpp.h"
#include "lslstream.h"
#include "PlotJuggler/datastreamer_base.h"


class LSLNode : public DataStreamer
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "com.icarustechnology.PlotJuggler.DataStreamer" "../datastreamer.json")
  Q_INTERFACES(DataStreamer)

public:
  LSLNode();
  virtual ~LSLNode() {}
  virtual const char* name() const override {return PLUGIN_NAME_.c_str();}
  virtual bool start(QStringList*) override;
  virtual void shutdown() override;
  virtual bool isRunning() const override;
  virtual bool isDebugPlugin() override { return false; }

signals:

private:
  void initializeStreamPlot(LSLStream *stream);
  bool getAvailableStreams();
  void plotFrame();
  void setStreamsCMD(LSLStream::CMD cmd);
  void initializeStreamsPlots();
  void run(bool is_running);
  QAction* action_LSL_;
  const std::string PLUGIN_NAME_ = "LSL Stream";
  ThreadSafeVariable<bool> is_running_;
  ThreadSafeVariable<LSLStream::CMD> worker_thread_cmd_;
  std::vector<LSLStream*> streams_;
  std::thread worker_thread_;
};

#endif // LSLNODE_H

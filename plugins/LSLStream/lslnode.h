#ifndef LSLNODE_H
#define LSLNODE_H

#include <QtPlugin>
#include <QStringListModel>
#include <QMenu>
#include <thread>
#include <lsl_cpp.h>
#include "PlotJuggler/datastreamer_base.h"

class LSLNode : public DataStreamer
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "com.icarustechnology.PlotJuggler.DataStreamer" "../datastreamer.json")
  Q_INTERFACES(DataStreamer)

public:

  LSLNode();

  //inhereted functions
  virtual const char* name() const{return PLUGIN_NAME_.c_str();}

  virtual bool start(QStringList*);

  virtual void shutdown();

  virtual bool isRunning() const;

  virtual std::vector<QString> appendData(PlotDataMapRef& destination);

  std::vector<lsl::stream_info> getAvailableStreams();

  virtual void addActionsToParentMenu( QMenu* menu ) override;

signals:

private:
  void run(bool is_running);
  void pullData(lsl::stream_info stream_info);


  QAction* _action_LSL;
  const std::string PLUGIN_NAME_ = "LSL";
  lsl::stream_outlet *outlet_;
  std::thread thread_;
  bool is_running_ = false;
  std::vector<lsl::stream_info> available_streams_;
};

#endif // LSLNODE_H

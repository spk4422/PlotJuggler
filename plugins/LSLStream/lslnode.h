#ifndef LSLNODE_H
#define LSLNODE_H

#include <QObject>
#include <QStringListModel>
#include <thread>
#include <lsl_cpp.h>

class LSLNode : public QObject
{
    Q_OBJECT
public:
    explicit LSLNode(QObject *parent = nullptr);
    void pullData(lsl::stream_info info);

signals:

private:
    lsl::stream_outlet *outlet;
    std::thread thread_;
    bool is_running;

    QStringListModel streams;

};

#endif // LSLNODE_H

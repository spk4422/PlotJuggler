#ifndef THDSAFEVARIABLE_H
#define THDSAFEVARIABLE_H

#include<mutex>
template <typename T>
class ThreadSafeVariable
{
public:
  ThreadSafeVariable(){}

  ~ThreadSafeVariable(){}
  void set(T var)
  {
    std::lock_guard<std::mutex>lock(mutex_);
    var_ = var;
  }
  T get() const
  {
    return var_;
  }
private:
  T var_;
  std::mutex mutex_;
};


#endif // THDSAFEVARIABLE_H

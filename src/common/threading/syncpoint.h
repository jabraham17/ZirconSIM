#ifndef ZIRCON_COMMON_THREADING_SYNCPOINT_H_
#define ZIRCON_COMMON_THREADING_SYNCPOINT_H_

#include <condition_variable>
#include <mutex>

namespace common {
namespace threading {

class syncpoint {
  private:
    std::mutex lock_;
    std::condition_variable signal_;
    bool ready_;

  public:
    syncpoint() : lock_(), signal_(), ready_(false) {}
    void signal() {
        std::unique_lock lk(lock_);
        ready_ = true;
        lk.unlock();
        signal_.notify_all();
    }
    void wait() {
        std::unique_lock lk(lock_);
        // stop waiting when ready
        signal_.wait(lk, [this] { return this->ready_; });
        ready_ = true; // reset syncpoint
    }
};

} // namespace threading
} // namespace common

#endif

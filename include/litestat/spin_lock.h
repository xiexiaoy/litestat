#pragma once

#include <pthread.h>

#include <cerrno>
#include <cstdlib>

namespace litestat
{
class SpinLock
{
public:
    SpinLock()
    {
        int rc = pthread_spin_init(&spin_lock_, PTHREAD_PROCESS_PRIVATE);
        if (rc != 0)
        {
            std::abort();
        }
    }

    ~SpinLock()
    {
        int rc = pthread_spin_destroy(&spin_lock_);
        if (rc != 0)
        {
            std::abort();
        }
    }

    void lock()
    {
        int rc = pthread_spin_lock(&spin_lock_);
        if (rc != 0)
        {
            std::abort();
        }
    }

    bool try_lock()
    {
        int rc = pthread_spin_trylock(&spin_lock_);
        if (rc == 0)
        {
            return true;
        }
        else
        {
            if (rc != EBUSY)
            {
                std::abort();
            }
            return false;
        }
    }

    void unlock()
    {
        int rc = pthread_spin_unlock(&spin_lock_);
        if (rc != 0)
        {
            std::abort();
        }
    }

private:
    pthread_spinlock_t spin_lock_;
};
}  // namespace litestat

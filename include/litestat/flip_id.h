#pragma once

#include <atomic>
#include <cstdint>

namespace litestat
{
class FlipID
{
public:
    static constexpr int8_t Double = 2;

    int8_t Mutable()
    {
        return mutable_id_.load(std::memory_order_acquire);
    }

    int8_t Immutable()
    {
        return Double - mutable_id_.load(std::memory_order_acquire);
    }

    void Flip()
    {
        int8_t immutable_id = Immutable();
        mutable_id_.exchange(immutable_id, std::memory_order_release);
    }

private:
    static std::atomic<int8_t> mutable_id_;
};
}  // namespace litestat

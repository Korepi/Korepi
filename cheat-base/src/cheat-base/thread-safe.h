#pragma once

#include <mutex>
#include <queue>
#include <optional>

template<typename T>
class SafeQueue 
{
public:
    SafeQueue() = default;
    SafeQueue(const SafeQueue<T>&) = delete;
    SafeQueue& operator=(const SafeQueue<T>&) = delete;

    SafeQueue(SafeQueue<T>&& other) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = std::move(other.queue_);
    }

    virtual ~SafeQueue() { }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    std::optional<T> pop() 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return {};
        }
        T tmp = queue_.front();
        queue_.pop();
        return tmp;
    }

    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;

    bool empty() const {
        return queue_.empty();
    }
};

template <class T>
class SafeValue
{
public:
    SafeValue(T initValue) : value(initValue) {}

    SafeValue(const SafeValue<T>&) = delete;
    SafeValue& operator=(const SafeValue<T>&) = delete;

    T GetValue() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return value;
    }

    void SetValue(const T& newValue)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        value = newValue;
    }

    SafeValue& operator=(const T& newValue)
    {
        SetValue(newValue);
        return *this;
    }

    operator T() const 
    {
        return GetValue();
    }

private:
    T value;
    mutable std::mutex mutex_;
};
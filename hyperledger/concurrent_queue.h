#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace hama {

    template <typename T>
    class ConcurrentQueue {

    public:
        ConcurrentQueue() = default;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        void push(const T& item) {

            std::unique_lock<std::mutex> lock(_mtx);
            _que.push(item);
            lock.unlock();
            _cond.notify_one();
        }

        T pop() {

            std::unique_lock<std::mutex> lock(_mtx);
            while (_que.empty()) {
                _cond.wait(lock);
            }

            auto item = _que.front();
            _que.pop();
            return item;
        }

        std::vector<T> pop(int n) {

            std::unique_lock<std::mutex> lock(_mtx);
            while ( size() < n) {
                _cond.wait(lock);
            }

            std::vector<T> items;
            for (int i= 0 ; i < n ; i++){
                items.push_back(_que.front());
                _que.pop();
            }

            return items;
        }

        void pop(T& item) {

            std::unique_lock<std::mutex> lock(_mtx);
            while (_que.empty()) {
                _cond.wait(lock);
            }

            item = _que.front();
            _que.pop();
        }

        const size_t size() const {

            return _que.size();
        }

    private:
        std::queue<T> _que;
        std::mutex    _mtx;
        std::condition_variable _cond;
    };
}


/*
** EPITECH PROJECT, 2023
** Untitled (Workspace)
** File description:
** TSqueue
*/

#ifndef TSQUEUE_HPP_
#define TSQUEUE_HPP_

#include <atomic>

#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class TSqueue
{
public:
    TSqueue() = default;

public:
    const T& front()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
        return m_queue.front();
    }

    const T& back()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
        return m_queue.back();
    }

    T pop_front()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
        T item = std::move(m_queue.front());
        m_queue.pop();
        return item;
    }

    T pop_back()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
        T item = std::move(m_queue.back());
        m_queue.pop();
        return item;
    }

    void push_back(const T& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(item);
        lock.unlock();
        m_condition.notify_one();
    }

    void push_front(const T& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(item);
        lock.unlock();
        m_condition.notify_one();
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t count()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
        {
            m_queue.pop();
        }
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
};

#endif /* !TSQUEUE_HPP_ */

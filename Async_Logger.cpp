#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

struct LogMessage
{
    LogLevel level;
    std::string component;
    std::string message;
};

class Logger
{
private:
    // Queue containing messages waiting to be written
    std::queue<LogMessage> logQueue;

    // Protects the queue
    std::mutex mtx;

    // Wakes the logger thread when a new message arrives
    std::condition_variable cv;

    // Output file
    std::ofstream file;

    // Background thread that writes logs
    std::thread worker;

    // Used to tell the worker thread to stop
    bool stopping = false;

private:

    std::string levelToString(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::DEBUG:
                return "DEBUG";

            case LogLevel::INFO:
                return "INFO";

            case LogLevel::WARNING:
                return "WARNING";

            case LogLevel::ERROR:
                return "ERROR";
        }

        return "UNKNOWN";
    }

    void processLogs()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mtx);

            // Sleep until:
            // 1. There is a log message
            // OR
            // 2. Logger is stopping
            cv.wait(lock, [this]()
            {
                return !logQueue.empty() || stopping;
            });

            // If stopping AND there are no remaining logs,
            // we can safely exit.
            if (stopping && logQueue.empty())
            {
                break;
            }

            // Take one message from the queue
            LogMessage msg = logQueue.front();
            logQueue.pop();

            // Unlock before doing slow file I/O
            lock.unlock();

            // Write to file
            file << levelToString(msg.level)
                 << " | "
                 << msg.component
                 << " | "
                 << msg.message
                 << std::endl;
        }
    }

public:

    Logger(const std::string& filename)
    {
        file.open(filename);

        if (!file.is_open())
        {
            throw std::runtime_error("Unable to open log file");
        }

        // Start background logger thread
        worker = std::thread(&Logger::processLogs, this);
    }

    void log(LogLevel level,
             const std::string& component,
             const std::string& message)
    {
        {
            // Lock only while modifying the queue
            std::lock_guard<std::mutex> lock(mtx);

            logQueue.push({
                level,
                component,
                message
            });
        }

        // Tell the worker thread that a message is available
        cv.notify_one();
    }

    ~Logger()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);

            stopping = true;
        }

        // Wake the worker thread
        cv.notify_one();

        // Wait for worker thread to finish
        if (worker.joinable())
        {
            worker.join();
        }

        file.close();
    }
};


int main()
{
    Logger logger("server.log");

    logger.log(
        LogLevel::INFO,
        "Storage",
        "Disk connected"
    );

    logger.log(
        LogLevel::WARNING,
        "Network",
        "Network latency is high"
    );

    logger.log(
        LogLevel::ERROR,
        "Storage",
        "Disk read failed"
    );

    logger.log(
        LogLevel::INFO,
        "Memory",
        "Memory initialization completed"
    );

    // Give background thread some time to process logs
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100)
    );

    return 0;
}

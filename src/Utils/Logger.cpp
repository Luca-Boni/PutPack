#include "Utils/Logger.hpp"
#include <filesystem>
#include <chrono>

bool Logger::instanceFlag = false;
Mutex Logger::logMutex = Mutex();
Logger *Logger::logger = NULL;

Logger::Logger(std::string logFilename) : Thread(std::bind(&Logger::execute, this, std::placeholders::_1), NULL)
{
    logMutex.lock();
    if (!instanceFlag)
    {
        if (logFilename == "")
        {
            this->logFilename = "Logger";
        }
        else
        {
            this->logFilename = logFilename;
        }

        instanceFlag = true;

        char path[FILENAME_MAX];
        ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
        std::string exe_dir = std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();
        this->logFilename = exe_dir + "/" + this->logFilename;

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "_%Y%m%d_%H%M%S");
        this->logFilename += oss.str() + ".log";

        socketServer = SocketServer();
        start();
        socketClient = SocketClient("localhost", socketServer.getPort());
        socketClient.connect();
    }
    logMutex.unlock();
}

void *Logger::execute(void *dummy)
{
    Logger::logger = this;
    logFile = std::ofstream(logFilename);
    socket = SocketServerSession(socketServer.listenAndAccept());

    while (true)
    {
        char *buffer = socket.read();

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = std::chrono::system_clock::to_time_t(now);
        std::tm bt = *std::localtime(&timer);

        logFile << std::put_time(&bt, "%d/%m/%Y %H:%M:%S")
                << '.' << std::setfill('0') << std::setw(3) << ms.count() << " : "
                << buffer << std::endl;

        delete[] buffer;
    }
}

void Logger::stop()
{
    logFile.close();
    Thread::stop();
}

void Logger::log(const std::string &message)
{
    if (logger != NULL)
    {
        logger->socketClient.write(message.c_str());
    }
}
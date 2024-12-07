#ifndef MYLOGGER_H
#define MYLOGGER_H

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifdef QT_CORE_LIB
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#endif

// 定义宏
#define LOG_INFO(...) MyLogger::getInstance().info(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define LOG_WARN(...) MyLogger::getInstance().warn(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define LOG_ERROR(...) MyLogger::getInstance().error(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define LOG_CRITICAL(...) MyLogger::getInstance().critical(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)

class MyLogger
{
private:
    std::shared_ptr<spdlog::logger> logger;

public:
    MyLogger(const MyLogger &) = delete;
    MyLogger &operator=(const MyLogger &) = delete;
    MyLogger()
    {
#ifdef QT_CORE_LIB
        if (QCoreApplication::instance())
        {
            qInstallMessageHandler(customMessageHandler);
        }
#endif

        // 创建控制台和文件日志记录器
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace); // 控制台日志记录器级别为 trace
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] [%s:%#] [%!] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.txt", true);
        file_sink->set_level(spdlog::level::trace); // 文件日志记录器级别为 trace
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] [%s:%#] [%!] %v");

        // 创建多线程日志记录器
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        logger = std::make_shared<spdlog::logger>("multi_logger", sinks.begin(), sinks.end());
        spdlog::register_logger(logger);
        logger->set_level(spdlog::level::trace);
        spdlog::flush_every(std::chrono::seconds(5));
    }

    static MyLogger &getInstance()
    {
        static MyLogger instance;
        return instance;
    }

    std::shared_ptr<spdlog::logger> get_logger()
    {
        return logger;
    }

    template<typename... Args>
    void log(const spdlog::source_loc &loc, spdlog::level::level_enum lvl, const char *fmt, const Args &...args)
    {
#ifdef QT_CORE_LIB
        if (QCoreApplication::instance())
        {
            // 格式化消息
            QString message = QString::asprintf(fmt, args...);

            // 使用spdlog将日志消息记录到文件
            logger->log(loc, lvl, fmt, args...);

            // 使用Qt的调试输出
            switch (lvl)
            {
            case spdlog::level::info:
                qInfo().noquote() << message;
                break;
            case spdlog::level::warn:
                qWarning().noquote() << message;
                break;
            case spdlog::level::err:
                qCritical().noquote() << message;
                break;
            case spdlog::level::critical:
                qCritical().noquote() << "CRITICAL:" << message;
                break;
            default:
                qDebug().noquote() << message;
                break;
            }
        }
        else
#endif
        {
            // 在非Qt应用程序中使用spdlog
            logger->log(loc, lvl, fmt, args...);
        }
    }

    template<typename... Args>
    void info(const spdlog::source_loc &loc, const char *fmt, const Args &...args)
    {
        log(loc, spdlog::level::info, fmt, args...);
    }

    template<typename... Args>
    void warn(const spdlog::source_loc &loc, const char *fmt, const Args &...args)
    {
        log(loc, spdlog::level::warn, fmt, args...);
    }

    template<typename... Args>
    void error(const spdlog::source_loc &loc, const char *fmt, const Args &...args)
    {
        log(loc, spdlog::level::err, fmt, args...);
    }

    template<typename... Args>
    void critical(const spdlog::source_loc &loc, const char *fmt, const Args &...args)
    {
        log(loc, spdlog::level::critical, fmt, args...);
    }

#ifdef QT_CORE_LIB
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &str)
    {
        QString typeStr;
        spdlog::level::level_enum spdlogLevel;

        switch (type)
        {
        case QtDebugMsg:
            typeStr = "Debug";
            spdlogLevel = spdlog::level::debug;
            break;
        case QtWarningMsg:
            typeStr = "Warning";
            spdlogLevel = spdlog::level::warn;
            break;
        case QtCriticalMsg:
            typeStr = "Critical";
            spdlogLevel = spdlog::level::err;
            break;
        case QtFatalMsg:
            typeStr = "Fatal";
            spdlogLevel = spdlog::level::critical;
            break;
        case QtInfoMsg:
            typeStr = "Info";
            spdlogLevel = spdlog::level::info;
            break;
        }

        QString msg = QString("%1 [%2] %3:%4 - %5")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                          .arg(typeStr)
                          .arg(QFileInfo(context.file).fileName())
                          .arg(context.line)
                          .arg(str);

        // 使用spdlog将日志消息记录到文件
        MyLogger::getInstance().get_logger()->log(spdlogLevel, "{}", msg.toStdString());

        // 输出到控制台
        fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    }
#endif
};

#endif // MYLOGGER_H

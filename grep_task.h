#ifndef GREP_TASK_H
#define GREP_TASK_H

#include <QString>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <QTextBrowser>

static unsigned const KB = 1024;
static unsigned const MB = 1024 * KB;

struct grep_task_t
{
    grep_task_t() = default;
    grep_task_t(QString, QString, std::vector<QString>*, std::mutex*, std::atomic_bool*, int* file_cnt);

    void grep_file() const;

private:
    std::vector<QString>* result;
    std::mutex* res_m;
    std::atomic_bool* cancel;
    int* files_cnt;

    QString path;
    QString substr;
};

#endif // GREP_TASK_H

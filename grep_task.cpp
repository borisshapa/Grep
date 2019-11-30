#include "grep_task.h"
#include <mutex>
#include <QDirIterator>
#include <QDebug>
#include <algorithm>

grep_task_t::grep_task_t(QString path, QString substr, std::vector<QString>* result, std::mutex* res_m, std::atomic_bool* cancel, int* files_cnt)
    : result(result),
      res_m(res_m),
      cancel(cancel),
      files_cnt(files_cnt),
      path(std::move(path)),
      substr(std::move(substr))
{

}

void grep_task_t::grep_file() const {
    if (cancel->load()) {
        return;
    }

    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString buffer;

        size_t substr_size = static_cast<size_t>(substr.size());
        std::vector<unsigned int> pi(substr_size);

        if (substr_size > 0) {
            pi[0] = 0;
        }

        for (unsigned int i = 1; i < substr_size; i++) {
            if (cancel->load()) {
                return;
            }

            unsigned int j = pi[i - 1];
            while (j > 0 && substr[i] != substr[j]) {
                j = pi[j - 1];
            }
            if (substr[i] == substr[j]) {
                j++;
            }
            pi[i] = j;
        }

        std::vector<QString> file_result;
        while (!stream.atEnd()) {
            if (cancel->load()) {
                return;
            }

            buffer = stream.read(4 * KB);
            unsigned int pref_func = 0;
            int lines_cnt = 1, pos = 0;

            for (int i = 0; i < buffer.size(); i++) {
                QChar ch = buffer[i];
                if (ch == '\n') {
                    lines_cnt++;
                    pos = 0;
                }

                while (pref_func > 0 && (pref_func == substr_size || ch != substr[pref_func])) {
                    pref_func = pi[pref_func - 1];
                }
                if (ch == substr[pref_func]) {
                    pref_func++;
                }

                if (pref_func == substr_size) {
                    int first = std::max(i - pos + 1, 0);
                    int sz = i - first + 10;
                    QString res = buffer.mid(first, sz)
                            .left(buffer.indexOf('\n', i))
                            .toHtmlEscaped();
                    {
                        std::unique_lock<std::mutex> lg(*res_m);
                        file_result.push_back("<font color=\"purple\">"
                                     + path + "</font><font color=\"red\">:</font>"
                                     + QString::number(lines_cnt) + " "
                                     + res + "<br>");
                    }
                }
                pos++;
            }
        }

        {
            std::unique_lock<std::mutex> lg(*res_m);
            result->insert(result->end(), file_result.begin(), file_result.end());
            (*files_cnt)++;
        }
        file.close();
    } else {
        std::unique_lock<std::mutex> lg(*res_m);
        result->push_back(path + "</font><font color=\"red\">Permision denied.</font>");
    }
}

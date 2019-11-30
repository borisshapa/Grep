#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <mutex>
#include <QTimer>

#include "grep_task.h"
#include "thread_pool.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_treeView_clicked(const QModelIndex &index);

private:
    void grep_dir(QString const&, QString const&);
    int get_total_files(QString const&);

    Ui::MainWindow *ui;
    QFileSystemModel *file_system_model;

    std::vector<QString> result;
    std::mutex res_m;
    std::atomic_bool cancel;

    thread_pool tp;

    QTimer timer;
    int files_cnt = 0;
};

#endif // MAINWINDOW_H

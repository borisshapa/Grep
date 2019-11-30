#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cancel(false),
    tp(4)
{
    ui->setupUi(this);

    QString ROOT_PATH = "/";
    file_system_model = new QFileSystemModel(this);
    file_system_model->setRootPath(ROOT_PATH);

    ui->treeView->setModel(file_system_model);

    ui->dirLineEdit->setText(ROOT_PATH);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);

    ui->treeView->setExpanded(file_system_model->index(ROOT_PATH), true);

    ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);

    connect(ui->findButton, &QPushButton::clicked, this, [this] {
        cancel = true;
        tp.wait_empty_queue();
        files_cnt = 0;
        cancel = false;

        QString path = ui->dirLineEdit->text();
        QString substr = ui->substrLineEdit->text();
        ui->textEdit->clear();
        QFileInfo const path_info(path);

        if (path_info.isDir()) {
            int total_files = get_total_files(path);
            ui->progressBar->setRange(0, total_files);
            grep_dir(path, substr);
        } else if (path_info.isFile() && !path_info.isExecutable()) {
            grep_task_t grep_task(path, substr, &result, &res_m, &cancel, &files_cnt);
            grep_task.grep_file();
        } else {
            result.push_back("<font color=\"red\">" + path + ": No such file or directory </font>");
        }
    });

    timer.setInterval(10);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this] {
        ui->progressBar->setValue(files_cnt);
        std::unique_lock<std::mutex> lg(res_m);
        for (auto str : result) {
            ui->textEdit->append(str);
        }
        result.clear();
    });

    connect(ui->cancelButton, &QPushButton::clicked, this, [this] {
        cancel = true;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    QString sPath = file_system_model->fileInfo(index).absoluteFilePath();
    ui->dirLineEdit->setText(sPath);
}

void MainWindow::grep_dir(QString const& dir, QString const& substr) {
    QDirIterator it(dir,
                    QDir::Files
                    | QDir::NoSymLinks
                    | QDir::NoDotAndDotDot
                    | QDir::Readable,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (cancel.load()) {
            return;
        }

        it.next();
        tp.enqueue(std::make_shared<grep_task_t>(it.filePath(), substr, &result, &res_m, &cancel, &files_cnt));
    }
}

int MainWindow::get_total_files(QString const& dir) {
    int ans = 0;
    QDirIterator it(dir,
                    QDir::Files
                    | QDir::NoSymLinks
                    | QDir::NoDotAndDotDot
                    | QDir::Readable,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (cancel.load()) {
            return 0;
        }
        it.next();
        ans++;
    }
    return ans;
}

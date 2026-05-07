#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QFontMetrics>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QTextEdit>

// TaskDetailsDialog 实现
TaskDetailsDialog::TaskDetailsDialog(const Task& task, QWidget* parent)
    : QDialog(parent), task(task) {
    setWindowTitle("任务详情");
    setModal(true);
    setMinimumWidth(400);
    setMinimumHeight(300);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 标题
    QLabel* titleLabel = new QLabel("标题:", this);
    titleEdit = new QLineEdit(this);
    titleEdit->setText(task.title);
    layout->addWidget(titleLabel);
    layout->addWidget(titleEdit);

    // 描述
    QLabel* descLabel = new QLabel("描述:", this);
    descriptionEdit = new QLineEdit(this);
    descriptionEdit->setText(task.description);
    layout->addWidget(descLabel);
    layout->addWidget(descriptionEdit);

    // 详情
    QLabel* detailsLabel = new QLabel("详情:", this);
    detailsEdit = new QTextEdit(this);
    detailsEdit->setText(task.details);
    layout->addWidget(detailsLabel);
    layout->addWidget(detailsEdit);

    // 进度
    QLabel* progressLabel = new QLabel("进度 (0-100):", this);
    progressSpinBox = new QSpinBox(this);
    progressSpinBox->setMinimum(0);
    progressSpinBox->setMaximum(100);
    progressSpinBox->setValue(task.progress);
    progressSpinBox->setSuffix("%");
    layout->addWidget(progressLabel);
    layout->addWidget(progressSpinBox);

    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

Task TaskDetailsDialog::getTask() const {
    Task result = task;
    result.title = titleEdit->text();
    result.description = descriptionEdit->text();
    result.details = detailsEdit->toPlainText();
    result.progress = progressSpinBox->value();
    return result;
}

// EditTaskDialog 实现
EditTaskDialog::EditTaskDialog(const QString& description, int progress, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("编辑任务");
    setModal(true);
    setMinimumWidth(300);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 描述输入
    QLabel* descLabel = new QLabel("任务描述:", this);
    descriptionEdit = new QLineEdit(this);
    descriptionEdit->setText(description);
    layout->addWidget(descLabel);
    layout->addWidget(descriptionEdit);

    // 进度输入
    QLabel* progressLabel = new QLabel("进度 (0-100):", this);
    progressSpinBox = new QSpinBox(this);
    progressSpinBox->setMinimum(0);
    progressSpinBox->setMaximum(100);
    progressSpinBox->setValue(progress);
    progressSpinBox->setSuffix("%");
    layout->addWidget(progressLabel);
    layout->addWidget(progressSpinBox);

    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QString EditTaskDialog::getDescription() const {
    return descriptionEdit->text();
}

int EditTaskDialog::getProgress() const {
    return progressSpinBox->value();
}

// TaskItemDelegate 实现
TaskItemDelegate::TaskItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void TaskItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    // 绘制背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        QColor bgColor = index.data(Qt::BackgroundRole).value<QColor>();
        if (bgColor.isValid()) {
            painter->fillRect(option.rect, bgColor);
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }
    }

    // 获取任务信息
    QString fullText = index.data(Qt::DisplayRole).toString();
    QString taskId = index.data(Qt::UserRole).toString();

    // 分离任务内容和状态信息
    // 格式: "1. 标题 - 描述 [状态] 百分比%"
    int lastBracketPos = fullText.lastIndexOf('[');
    QString taskContent = fullText.left(lastBracketPos).trimmed();
    QString statusInfo = fullText.mid(lastBracketPos);

    // 绘制任务内容（左对齐）
    QFont font = option.font;
    painter->setFont(font);
    painter->setPen(option.palette.text().color());

    QRect contentRect = option.rect;
    contentRect.setLeft(contentRect.left() + 5);
    contentRect.setRight(contentRect.right() - 150);  // 为状态信息预留空间

    painter->drawText(contentRect, Qt::AlignLeft | Qt::AlignVCenter, taskContent);

    // 绘制状态信息（右对齐）
    QRect statusRect = option.rect;
    statusRect.setLeft(option.rect.right() - 145);
    statusRect.setRight(option.rect.right() - 5);

    painter->drawText(statusRect, Qt::AlignRight | Qt::AlignVCenter, statusInfo);
}

QSize TaskItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
    return QSize(option.rect.width(), 25);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("待做事情清单");
    setGeometry(100, 100, 800, 600);

    dataFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasks.json";
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    setupUI();
    loadTasks();
    refreshTaskList();
}

MainWindow::~MainWindow() {
    saveTasks();
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 状态过滤
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel("筛选:", this);
    statusFilterCombo = new QComboBox(this);
    statusFilterCombo->addItem("全部");
    statusFilterCombo->addItem("待开始");
    statusFilterCombo->addItem("进行中");
    statusFilterCombo->addItem("已完成");
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStatusFilterChanged);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // 任务列表
    taskListWidget = new QListWidget(this);
    taskListWidget->setStyleSheet("QListWidget::item { padding: 5px; height: 25px; }");
    taskListWidget->setWordWrap(false);  // 禁用自动换行

    // 设置自定义委托
    taskListWidget->setItemDelegate(new TaskItemDelegate(this));

    connect(taskListWidget, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onTaskDoubleClicked);
    connect(taskListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onTaskSelectionChanged);
    mainLayout->addWidget(taskListWidget);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    addButton = new QPushButton("添加", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddTask);
    buttonLayout->addWidget(addButton);

    deleteButton = new QPushButton("删除", this);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteTask);
    buttonLayout->addWidget(deleteButton);

    buttonLayout->addSpacing(20);

    upButton = new QPushButton("↑ 上移", this);
    connect(upButton, &QPushButton::clicked, this, &MainWindow::onMoveUp);
    buttonLayout->addWidget(upButton);

    downButton = new QPushButton("↓ 下移", this);
    connect(downButton, &QPushButton::clicked, this, &MainWindow::onMoveDown);
    buttonLayout->addWidget(downButton);

    buttonLayout->addSpacing(20);

    detailsButton = new QPushButton("详情", this);
    connect(detailsButton, &QPushButton::clicked, this, &MainWindow::onViewTaskDetails);
    buttonLayout->addWidget(detailsButton);

    mainLayout->addLayout(buttonLayout);

    onTaskSelectionChanged();
}

void MainWindow::loadTasks() {
    taskManager.loadFromFile(dataFile);
}

void MainWindow::saveTasks() {
    taskManager.saveToFile(dataFile);
}

void MainWindow::refreshTaskList() {
    taskListWidget->clear();
    QVector<Task> allTasks = taskManager.getAllTasks();

    // 修复任务状态与进度的匹配
    bool needsSave = false;
    for (auto& task : allTasks) {
        TaskStatus expectedStatus;
        if (task.progress == 0) {
            expectedStatus = TaskStatus::Todo;
        } else if (task.progress == 100) {
            expectedStatus = TaskStatus::Done;
        } else {
            expectedStatus = TaskStatus::InProgress;
        }

        if (task.status != expectedStatus) {
            task.status = expectedStatus;
            taskManager.updateTask(task);
            needsSave = true;
        }
    }

    if (needsSave) {
        saveTasks();
    }

    int filterIndex = statusFilterCombo->currentIndex();
    TaskStatus filterStatus = TaskStatus::Todo;
    bool filterAll = (filterIndex == 0);

    if (filterIndex == 1) filterStatus = TaskStatus::Todo;
    else if (filterIndex == 2) filterStatus = TaskStatus::InProgress;
    else if (filterIndex == 3) filterStatus = TaskStatus::Done;

    int index = 0;
    for (const auto& task : allTasks) {
        if (filterAll || task.status == filterStatus) {
            index++;
            QListWidgetItem* item = createTaskItem(task, index);
            taskListWidget->addItem(item);
        }
    }
}

QListWidgetItem* MainWindow::createTaskItem(const Task& task, int index) {
    QString statusStr = getStatusString(task.status);
    QString displayText;

    if (index > 0) {
        displayText = QString::number(index) + ". ";
    }

    displayText += task.description;

    // 添加状态和进度信息
    displayText += QString(" [%1] %2%").arg(statusStr).arg(task.progress);

    QListWidgetItem* item = new QListWidgetItem(displayText);
    item->setData(Qt::UserRole, task.id);
    item->setData(Qt::UserRole + 1, task.progress);

    // 根据状态设置背景色
    if (task.status == TaskStatus::InProgress) {
        item->setData(Qt::BackgroundRole, QColor(255, 250, 205)); // 浅黄色
    } else if (task.status == TaskStatus::Done) {
        item->setData(Qt::BackgroundRole, QColor(200, 255, 200)); // 浅绿色
    }

    return item;
}

void MainWindow::onAddTask() {
    bool ok;
    QString description = QInputDialog::getText(this, "添加任务", "任务描述:",
                                               QLineEdit::Normal, "", &ok);
    if (!ok || description.isEmpty()) {
        return;
    }

    Task task;
    task.title = "";
    task.description = description;
    task.status = TaskStatus::Todo;
    task.progress = 0;

    taskManager.addTask(task);
    saveTasks();
    refreshTaskList();
}

void MainWindow::onDeleteTask() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择一个任务");
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    int ret = QMessageBox::question(this, "确认删除", "确定要删除这个任务吗?");
    if (ret == QMessageBox::Yes) {
        taskManager.removeTask(id);
        saveTasks();
        refreshTaskList();
    }
}

void MainWindow::onEditTask() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择一个任务");
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Task task = taskManager.getTask(id);

    bool ok;
    QString title = QInputDialog::getText(this, "编辑任务", "任务标题:",
                                         QLineEdit::Normal, task.title, &ok);
    if (!ok) {
        return;
    }

    QString description = QInputDialog::getText(this, "编辑任务", "任务描述:",
                                               QLineEdit::Normal, task.description, &ok);
    if (!ok) {
        return;
    }

    task.title = title;
    task.description = description;

    taskManager.updateTask(task);
    saveTasks();
    refreshTaskList();
}

void MainWindow::editTaskProgress(const QString& taskId) {
    Task task = taskManager.getTask(taskId);
    if (task.id.isEmpty()) {
        return;
    }

    EditTaskDialog dialog(task.description, task.progress, this);
    if (dialog.exec() == QDialog::Accepted) {
        task.description = dialog.getDescription();
        task.progress = dialog.getProgress();

        // 根据进度自动改变状态
        if (task.progress == 0) {
            task.status = TaskStatus::Todo;
        } else if (task.progress == 100) {
            task.status = TaskStatus::Done;
        } else {
            task.status = TaskStatus::InProgress;
        }

        taskManager.updateTask(task);
        saveTasks();
        refreshTaskList();
    }
}

void MainWindow::onTaskDoubleClicked(QListWidgetItem* item) {
    QString id = item->data(Qt::UserRole).toString();
    editTaskProgress(id);
}

void MainWindow::onMoveUp() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    taskManager.moveTaskUp(id);
    saveTasks();
    refreshTaskList();

    // 重新选中该项
    for (int i = 0; i < taskListWidget->count(); ++i) {
        if (taskListWidget->item(i)->data(Qt::UserRole).toString() == id) {
            taskListWidget->setCurrentRow(i);
            break;
        }
    }
}

void MainWindow::onMoveDown() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    taskManager.moveTaskDown(id);
    saveTasks();
    refreshTaskList();

    // 重新选中该项
    for (int i = 0; i < taskListWidget->count(); ++i) {
        if (taskListWidget->item(i)->data(Qt::UserRole).toString() == id) {
            taskListWidget->setCurrentRow(i);
            break;
        }
    }
}

void MainWindow::onTaskSelectionChanged() {
    bool hasSelection = taskListWidget->currentItem() != nullptr;
    deleteButton->setEnabled(hasSelection);
    upButton->setEnabled(hasSelection);
    downButton->setEnabled(hasSelection);
    detailsButton->setEnabled(hasSelection);
}

void MainWindow::onViewTaskDetails() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择一个任务");
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Task task = taskManager.getTask(id);

    TaskDetailsDialog dialog(task, this);
    if (dialog.exec() == QDialog::Accepted) {
        Task updatedTask = dialog.getTask();

        // 根据进度自动改变状态
        if (updatedTask.progress == 0) {
            updatedTask.status = TaskStatus::Todo;
        } else if (updatedTask.progress == 100) {
            updatedTask.status = TaskStatus::Done;
        } else {
            updatedTask.status = TaskStatus::InProgress;
        }

        taskManager.updateTask(updatedTask);
        saveTasks();
        refreshTaskList();
    }
}

void MainWindow::onStatusFilterChanged(int index) {
    refreshTaskList();
}

void MainWindow::moveTaskToStatus(const QString& taskId, TaskStatus newStatus) {
    Task task = taskManager.getTask(taskId);
    if (!task.id.isEmpty()) {
        task.status = newStatus;
        taskManager.updateTask(task);
        saveTasks();
        refreshTaskList();
    }
}

QString MainWindow::getStatusString(TaskStatus status) const {
    switch (status) {
        case TaskStatus::Todo:
            return "待开始";
        case TaskStatus::InProgress:
            return "进行中";
        case TaskStatus::Done:
            return "已完成";
    }
    return "";
}

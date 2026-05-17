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

// TaskDetailsDialog 实现 - 纵向单列布局
TaskDetailsDialog::TaskDetailsDialog(Task& task, QWidget* parent)
    : QDialog(parent), task(task), currentPersonIndex(-1) {
    setWindowTitle("任务详情 - 人员管理");
    setModal(true);
    setMinimumWidth(600);
    setMinimumHeight(500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 上部：任务基本信息
    QVBoxLayout* taskInfoLayout = new QVBoxLayout();

    // 描述
    QLabel* descLabel = new QLabel("描述:", this);
    descriptionEdit = new QLineEdit(this);
    descriptionEdit->setText(task.description);
    taskInfoLayout->addWidget(descLabel);
    taskInfoLayout->addWidget(descriptionEdit);

    // 详情
    QLabel* detailsLabel = new QLabel("详情:", this);
    detailsEdit = new QTextEdit(this);
    detailsEdit->setText(task.details);
    detailsEdit->setMinimumHeight(40);
    connect(detailsEdit, &QTextEdit::textChanged, this, [this]() {
        int height = qMin(200, (int)detailsEdit->document()->size().height() + 10);
        detailsEdit->setFixedHeight(height);
    });
    taskInfoLayout->addWidget(detailsLabel);
    taskInfoLayout->addWidget(detailsEdit);

    // 进度
    QHBoxLayout* progressLayout = new QHBoxLayout();
    QLabel* progressLabel = new QLabel("进度 (0-100):", this);
    progressSpinBox = new QSpinBox(this);
    progressSpinBox->setMinimum(0);
    progressSpinBox->setMaximum(100);
    progressSpinBox->setValue(task.progress);
    progressSpinBox->setSuffix("%");
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressSpinBox);
    progressLayout->addStretch();
    taskInfoLayout->addLayout(progressLayout);

    mainLayout->addLayout(taskInfoLayout);

    // 中部：涉及人员（按钮行）
    QHBoxLayout* peopleRowLayout = new QHBoxLayout();
    QLabel* peopleLabel = new QLabel("涉及人员:", this);
    peopleRowLayout->addWidget(peopleLabel);

    personButtonsWidget = new QWidget(this);
    QHBoxLayout* buttonsLayout = new QHBoxLayout(personButtonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(5);

    // 动态添加人员按钮
    for (int i = 0; i < task.people.size(); ++i) {
        QPushButton* btn = new QPushButton(task.people[i].name, this);
        btn->setProperty("index", i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSelectPerson(i); });
        buttonsLayout->addWidget(btn);
    }

    addPersonButton = new QPushButton("添加", this);
    addPersonButton->setMaximumWidth(50);
    connect(addPersonButton, &QPushButton::clicked, this, &TaskDetailsDialog::onAddPerson);
    buttonsLayout->addWidget(addPersonButton);
    buttonsLayout->addStretch();

    peopleRowLayout->addWidget(personButtonsWidget, 1);
    mainLayout->addLayout(peopleRowLayout);

    // 下部：人员详情编辑
    QLabel* personNameLabel = new QLabel("人员名字:", this);
    personNameEdit = new QLineEdit(this);
    connect(personNameEdit, &QLineEdit::textChanged, this, &TaskDetailsDialog::onNameChanged);
    mainLayout->addWidget(personNameLabel);
    mainLayout->addWidget(personNameEdit);

    QLabel* personDetailsLabel = new QLabel("工作安排:", this);
    personDetailsEdit = new QTextEdit(this);
    personDetailsEdit->setMinimumHeight(40);
    connect(personDetailsEdit, &QTextEdit::textChanged, this, [this]() {
        int height = qMin(200, (int)personDetailsEdit->document()->size().height() + 10);
        personDetailsEdit->setFixedHeight(height);
    });
    connect(personDetailsEdit, &QTextEdit::textChanged, this, &TaskDetailsDialog::onDetailsChanged);
    mainLayout->addWidget(personDetailsLabel);
    mainLayout->addWidget(personDetailsEdit);

    QHBoxLayout* personProgressLayout = new QHBoxLayout();
    QLabel* personProgressLabel = new QLabel("进度 (0-100):", this);
    personProgressSpinBox = new QSpinBox(this);
    personProgressSpinBox->setMinimum(0);
    personProgressSpinBox->setMaximum(100);
    personProgressSpinBox->setSuffix("%");
    connect(personProgressSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TaskDetailsDialog::onProgressChanged);
    personProgressLayout->addWidget(personProgressLabel);
    personProgressLayout->addWidget(personProgressSpinBox);
    personProgressLayout->addStretch();
    mainLayout->addLayout(personProgressLayout);

    mainLayout->addStretch();

    // 最下部：确定/取消按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    onSelectPerson(0);
}

Task TaskDetailsDialog::getTask() const {
    Task result = task;
    result.description = descriptionEdit->text();
    result.details = detailsEdit->toPlainText();
    result.progress = progressSpinBox->value();
    return result;
}

void TaskDetailsDialog::refreshPersonList() {
    // 清空旧按钮
    QLayout* layout = personButtonsWidget->layout();
    QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(layout);

    while (QLayoutItem* item = boxLayout->takeAt(0)) {
        if (item->widget() && item->widget() != addPersonButton) {
            delete item->widget();
        }
        delete item;
    }

    // 重新添加人员按钮
    for (int i = 0; i < task.people.size(); ++i) {
        QPushButton* btn = new QPushButton(task.people[i].name, this);
        btn->setProperty("index", i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSelectPerson(i); });
        boxLayout->insertWidget(i, btn);
    }

    boxLayout->insertWidget(task.people.size(), addPersonButton);
    boxLayout->addStretch();
}

void TaskDetailsDialog::onAddPerson() {
    bool ok;
    QString name = QInputDialog::getText(this, "添加人员", "人员名字:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }

    Person newPerson;
    newPerson.name = name;
    newPerson.details = "";
    newPerson.progress = 0;
    task.people.append(newPerson);
    refreshPersonList();
    onSelectPerson(task.people.size() - 1);
}

void TaskDetailsDialog::onDeletePerson(int index) {
    if (index < 0 || index >= task.people.size()) {
        return;
    }

    int ret = QMessageBox::question(this, "确认删除", "确定要删除这个人员吗?");
    if (ret == QMessageBox::Yes) {
        task.people.removeAt(index);
        refreshPersonList();
        if (currentPersonIndex >= task.people.size()) {
            currentPersonIndex = task.people.size() - 1;
        }
        if (currentPersonIndex >= 0) {
            onSelectPerson(currentPersonIndex);
        } else {
            personNameEdit->clear();
            personDetailsEdit->clear();
            personNameEdit->setEnabled(false);
            personDetailsEdit->setEnabled(false);
        }
    }
}

void TaskDetailsDialog::onSelectPerson(int index) {
    if (index < 0 || index >= task.people.size()) {
        currentPersonIndex = -1;
        personNameEdit->clear();
        personDetailsEdit->clear();
        personProgressSpinBox->setValue(0);
        personNameEdit->setEnabled(false);
        personDetailsEdit->setEnabled(false);
        personProgressSpinBox->setEnabled(false);
        return;
    }

    currentPersonIndex = index;
    personNameEdit->setEnabled(true);
    personDetailsEdit->setEnabled(true);
    personProgressSpinBox->setEnabled(true);

    personNameEdit->blockSignals(true);
    personDetailsEdit->blockSignals(true);
    personProgressSpinBox->blockSignals(true);

    personNameEdit->setText(task.people[index].name);
    personDetailsEdit->setText(task.people[index].details);
    personProgressSpinBox->setValue(task.people[index].progress);

    personNameEdit->blockSignals(false);
    personDetailsEdit->blockSignals(false);
    personProgressSpinBox->blockSignals(false);
}

void TaskDetailsDialog::onNameChanged() {
    if (currentPersonIndex >= 0 && currentPersonIndex < task.people.size()) {
        task.people[currentPersonIndex].name = personNameEdit->text();
        refreshPersonList();
    }
}

void TaskDetailsDialog::onDetailsChanged() {
    if (currentPersonIndex >= 0 && currentPersonIndex < task.people.size()) {
        task.people[currentPersonIndex].details = personDetailsEdit->toPlainText();
    }
}

void TaskDetailsDialog::onProgressChanged(int value) {
    if (currentPersonIndex >= 0 && currentPersonIndex < task.people.size()) {
        task.people[currentPersonIndex].progress = value;
        refreshPersonList();
    }
}

// PeopleSummaryDialog 实现
PeopleSummaryDialog::PeopleSummaryDialog(const TaskManager& taskManager, QWidget* parent)
    : QDialog(parent), taskManager(taskManager) {
    setWindowTitle("人员汇总");
    setModal(true);
    setMinimumWidth(800);
    setMinimumHeight(600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 上部：人员按钮行
    QLabel* peopleLabel = new QLabel("所有人员:", this);
    mainLayout->addWidget(peopleLabel);

    peopleButtonsWidget = new QWidget(this);
    QHBoxLayout* buttonsLayout = new QHBoxLayout(peopleButtonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(5);
    peopleButtonsWidget->setLayout(buttonsLayout);
    mainLayout->addWidget(peopleButtonsWidget);

    // 中部：任务列表
    QLabel* tasksLabel = new QLabel("该人员的任务:", this);
    tasksListWidget = new QListWidget(this);
    tasksListWidget->setMaximumHeight(150);
    connect(tasksListWidget, &QListWidget::itemSelectionChanged,
            this, [this]() {
                if (tasksListWidget->currentRow() >= 0) {
                    // 找到当前选中的人员
                    QLayout* layout = peopleButtonsWidget->layout();
                    for (int i = 0; i < layout->count(); ++i) {
                        QPushButton* btn = qobject_cast<QPushButton*>(layout->itemAt(i)->widget());
                        if (btn && btn->isChecked()) {
                            QString personName = btn->text();
                            int row = tasksListWidget->currentRow();
                            if (peopleSummary.contains(personName) && row < peopleSummary[personName].size()) {
                                const PersonTaskInfo& info = peopleSummary[personName][row];
                                detailsEdit->setPlainText(info.details);
                                progressSpinBox->setValue(info.progress);
                            }
                            break;
                        }
                    }
                }
            });
    mainLayout->addWidget(tasksLabel);
    mainLayout->addWidget(tasksListWidget);

    // 工作安排（只读）
    QLabel* detailsLabel = new QLabel("工作安排:", this);
    detailsEdit = new QTextEdit(this);
    detailsEdit->setReadOnly(true);
    detailsEdit->setMaximumHeight(150);
    mainLayout->addWidget(detailsLabel);
    mainLayout->addWidget(detailsEdit);

    // 进度（只读）
    QHBoxLayout* progressLayout = new QHBoxLayout();
    QLabel* progressLabel = new QLabel("进度:", this);
    progressSpinBox = new QSpinBox(this);
    progressSpinBox->setMinimum(0);
    progressSpinBox->setMaximum(100);
    progressSpinBox->setSuffix("%");
    progressSpinBox->setReadOnly(true);
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressSpinBox);
    progressLayout->addStretch();
    mainLayout->addLayout(progressLayout);

    mainLayout->addStretch();

    // 关闭按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    buildPeopleSummary();
}

void PeopleSummaryDialog::buildPeopleSummary() {
    peopleSummary.clear();

    // 遍历所有任务，按人名分组
    QVector<Task> allTasks = taskManager.getAllTasks();
    for (const auto& task : allTasks) {
        for (const auto& person : task.people) {
            PersonTaskInfo info;
            info.taskDescription = task.description;
            info.details = person.details;
            info.progress = person.progress;

            if (!peopleSummary.contains(person.name)) {
                peopleSummary[person.name] = QVector<PersonTaskInfo>();
            }
            peopleSummary[person.name].append(info);
        }
    }

    // 按名字排序并创建按钮
    QStringList names = peopleSummary.keys();
    std::sort(names.begin(), names.end());

    QLayout* layout = peopleButtonsWidget->layout();
    for (const auto& name : names) {
        QPushButton* btn = new QPushButton(name, this);
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [this, name]() {
            // 取消其他按钮的选中状态
            QLayout* layout = peopleButtonsWidget->layout();
            for (int i = 0; i < layout->count(); ++i) {
                QPushButton* otherBtn = qobject_cast<QPushButton*>(layout->itemAt(i)->widget());
                if (otherBtn && otherBtn != sender()) {
                    otherBtn->setChecked(false);
                }
            }
            onPersonSelected(name);
        });
        layout->addWidget(btn);
    }

    // 添加伸缩空间
    QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(layout);
    if (boxLayout) {
        boxLayout->addStretch();
    }

    // 默认选中第一个人员
    if (names.size() > 0) {
        QLayout* layout = peopleButtonsWidget->layout();
        QPushButton* firstBtn = qobject_cast<QPushButton*>(layout->itemAt(0)->widget());
        if (firstBtn) {
            firstBtn->setChecked(true);
            onPersonSelected(names[0]);
        }
    }
}

void PeopleSummaryDialog::onPersonSelected(const QString& personName) {
    tasksListWidget->clear();
    detailsEdit->clear();
    progressSpinBox->setValue(0);

    displayPersonTasks(personName);
}

void PeopleSummaryDialog::displayPersonTasks(const QString& personName) {
    if (!peopleSummary.contains(personName)) {
        return;
    }

    const auto& tasks = peopleSummary[personName];
    for (const auto& info : tasks) {
        tasksListWidget->addItem(info.taskDescription);
    }

    // 默认选中第一个任务
    if (tasksListWidget->count() > 0) {
        tasksListWidget->setCurrentRow(0);
        const PersonTaskInfo& info = tasks[0];
        detailsEdit->setPlainText(info.details);
        progressSpinBox->setValue(info.progress);
    }
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

    topButton = new QPushButton("⇈ 最前", this);
    connect(topButton, &QPushButton::clicked, this, &MainWindow::onMoveToTop);
    buttonLayout->addWidget(topButton);

    downButton = new QPushButton("↓ 下移", this);
    connect(downButton, &QPushButton::clicked, this, &MainWindow::onMoveDown);
    buttonLayout->addWidget(downButton);

    bottomButton = new QPushButton("⇉ 最后", this);
    connect(bottomButton, &QPushButton::clicked, this, &MainWindow::onMoveToBottom);
    buttonLayout->addWidget(bottomButton);

    buttonLayout->addSpacing(20);

    detailsButton = new QPushButton("详情", this);
    connect(detailsButton, &QPushButton::clicked, this, &MainWindow::onViewTaskDetails);
    buttonLayout->addWidget(detailsButton);

    peopleButton = new QPushButton("人员汇总", this);
    connect(peopleButton, &QPushButton::clicked, this, &MainWindow::onViewPeopleSummary);
    buttonLayout->addWidget(peopleButton);

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

void MainWindow::onTaskDoubleClicked(QListWidgetItem* item) {
    // 双击不做任何操作
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

void MainWindow::onMoveToTop() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    taskManager.moveTaskToTop(id);
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

void MainWindow::onMoveToBottom() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    taskManager.moveTaskToBottom(id);
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
    topButton->setEnabled(hasSelection);
    bottomButton->setEnabled(hasSelection);
    detailsButton->setEnabled(hasSelection);
    // peopleButton 不需要选中任务，始终启用
}

void MainWindow::onViewTaskDetails() {
    QListWidgetItem* item = taskListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择一个任务");
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Task task = taskManager.getTask(id);

    // 如果没有人员列表但有详情，临时为旧任务创建"甲"人员
    bool isLegacyTask = task.people.isEmpty() && !task.details.isEmpty();
    if (isLegacyTask) {
        Person defaultPerson;
        defaultPerson.name = "甲";
        defaultPerson.details = task.details;
        defaultPerson.progress = 0;
        task.people.append(defaultPerson);
    }

    TaskDetailsDialog dialog(task, this);
    if (dialog.exec() == QDialog::Accepted) {
        Task updatedTask = dialog.getTask();

        // 如果是旧任务，将"甲"的详情写回到 task.details
        if (isLegacyTask && !updatedTask.people.isEmpty()) {
            updatedTask.details = updatedTask.people[0].details;
        }

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

void MainWindow::onViewPeopleSummary() {
    PeopleSummaryDialog dialog(taskManager, this);
    dialog.exec();
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

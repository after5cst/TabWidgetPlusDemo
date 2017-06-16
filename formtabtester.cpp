#include "formtabtester.h"
#include "tabwidgetplus.h"
#include "ui_formtabtester.h"
#include <QTimer>
#include <QtGlobal>
#include <assert.h>

FormTabTester::FormTabTester(QWidget *parent)
    : QWidget(parent), ui(new Ui::FormTabTester)
{
    ui->setupUi(this);

    connect(ui->btnBarrage, &QPushButton::clicked, this,
            &FormTabTester::barrage);
    connect(ui->btnCreate, &QPushButton::clicked, this,
            &FormTabTester::createTab);
    connect(ui->btnDelete, &QPushButton::clicked, this,
            &FormTabTester::deleteTab);
    connect(ui->listWidget, &QListWidget::currentTextChanged, this,
            &FormTabTester::tabSelected);
    connect(ui->isEnabled, &QCheckBox::stateChanged, this,
            &FormTabTester::enabledStateChanged);
    connect(ui->isVisible, &QCheckBox::stateChanged, this,
            &FormTabTester::visibleStateChanged);
}

FormTabTester::~FormTabTester()
{
    delete ui;
}

TabWidgetPlus *FormTabTester::getTabWidget()
{
    if (nullptr == cachedTabWidget) {
        auto parent = parentWidget()->parentWidget();
        cachedTabWidget = dynamic_cast<TabWidgetPlus *>(parent);
        assert(nullptr != cachedTabWidget);
    }
    return cachedTabWidget;
}

void FormTabTester::createTab()
{
    auto button = new QPushButton("Hide", this);
    auto tabWidget = getTabWidget();

    connect(button, &QPushButton::clicked, tabWidget,
    [ = ](bool) {
        tabWidget->hideTab(button);
    });

    auto title = "Item " + QString::number(nextTabNumber++);
    tabWidget->addTab(button, title);

    // Now, track the mapping of title to object, and add the title
    // to the list box for future use (and select it).
    tabInfo[title.toStdString()] = button;
    ui->listWidget->addItem(title);
    ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
}

void FormTabTester::deleteTab()
{
    // Get the tab name (also the key) from the list widget and
    // then delete the entry from the list widget.
    auto selectedItem = ui->listWidget->currentItem();
    if (nullptr == selectedItem) {
        // Nothing currently selected.
        return;
    }
    auto key = selectedItem->text().toStdString();
    delete ui->listWidget->takeItem(ui->listWidget->row(selectedItem));

    if (0 == tabInfo.count(key)) {
        // Invalid tab name.  (should never happen)
        assert(false);
        return;
    }
    tabInfo[key]->deleteLater();
    tabInfo.erase(key);
}

void FormTabTester::tabSelected(const QString &currentText)
{
    auto key = currentText.toStdString();
    auto isValid = (0 != tabInfo.count(key));
    QWidget *widget = nullptr;
    auto tabWidget = getTabWidget();

    if (isValid) {
        widget = tabInfo[key];
        auto tabState = tabWidget->tabState(widget);
        switch (tabState) {
        case TabWidgetPlus::tabWidgetState::hidden:
            ui->isVisible->setChecked(false);
            break;
        case TabWidgetPlus::tabWidgetState::unknown:
            isValid = false;
            break;
        case TabWidgetPlus::tabWidgetState::visible:
            ui->isVisible->setChecked(true);
            break;
        }
    }

    ui->isVisible->setEnabled(isValid);
    ui->isEnabled->setEnabled(isValid);
    ui->btnDelete->setEnabled(isValid);

    if (isValid) {
        // ui->isVisible already set.
        ui->isEnabled->setChecked(widget->isEnabled());
    } else {
        ui->isVisible->setCheckState(Qt::CheckState::Unchecked);
        ui->isEnabled->setCheckState(Qt::CheckState::Unchecked);
    }
}

void FormTabTester::enabledStateChanged(int state)
{
    auto selectedItem = ui->listWidget->currentItem();
    if (nullptr == selectedItem) {
        return;
    }
    auto key = selectedItem->text().toStdString();
    if (0 == tabInfo.count(key)) {
        // Nothing currently selected.
        return;
    }
    auto widget = tabInfo[key];

    switch (static_cast<Qt::CheckState>(state)) {
    case Qt::CheckState::Unchecked:
        widget->setEnabled(false);
        break;
    case Qt::CheckState::Checked:
        widget->setEnabled(true);
        break;
    case Qt::CheckState::PartiallyChecked:
        break;
    }
    ui->listWidget->setFocus();
}

void FormTabTester::visibleStateChanged(int state)
{
    auto selectedItem = ui->listWidget->currentItem();
    if (nullptr == selectedItem) {
        return;
    }
    auto key = selectedItem->text().toStdString();
    if (0 == tabInfo.count(key)) {
        // Nothing currently selected.
        return;
    }
    auto widget = tabInfo[key];
    auto tabWidget = getTabWidget();

    switch (static_cast<Qt::CheckState>(state)) {
    case Qt::CheckState::Unchecked:
        tabWidget->hideTab(widget);
        break;
    case Qt::CheckState::Checked:
        tabWidget->showTab(widget);
        break;
    case Qt::CheckState::PartiallyChecked:
        break;
    }
    ui->listWidget->setFocus();
}

void FormTabTester::barrage()
{
    qsrand(qrand());
    if (nullptr != barrageTimer) {
        barrageTimer->deleteLater();
        barrageTimer = nullptr;
    } else {
        barrageTimer = new QTimer(this);
        connect(barrageTimer, &QTimer::timeout, this, &FormTabTester::barrageTimeout);
        barrageTimer->start(50);
    }
}

void FormTabTester::barrageTimeout()
{
    auto tabWidget = getTabWidget();

    // Check: make sure any visible tabs are in the correct order.
    auto tabNumber = -1;
    auto visibleTabs = tabWidget->count();
    for (auto i = 1; i < visibleTabs; ++i) {
        auto temp = tabWidget->tabText(i).mid(4).toInt();
        assert(temp > tabNumber);
        tabNumber = temp;
    }

    enum class Actions { CreateTab, DeleteTab, ShowTab, HideTab };
    auto action = Actions::CreateTab;
    auto totalItems = ui->listWidget->count();

    if (0 == totalItems) {
        action = Actions::CreateTab;
    } else if (20 < totalItems) {
        action = Actions::DeleteTab;
    } else {
        switch (qrand() % 6) {
        case 0:
        case 1:
            action = Actions::HideTab;
            break;
        case 2:
        case 3:
            action = Actions::ShowTab;
            break;
        case 4:
            action = Actions::CreateTab;
            break;
        case 5:
            action = Actions::DeleteTab;
            break;
        }
    }

    QWidget *widget = nullptr;
    auto index = 0;
    if (action != Actions::CreateTab) {
        index = qrand() % totalItems;
        auto item = ui->listWidget->item(index);
        auto key = item->text().toStdString();
        if (0 != tabInfo.count(key)) {
            widget = tabInfo[key];
        }
        if (nullptr == widget) {
            assert(false);
            action = Actions::CreateTab;
        }
    }

    switch (action) {
    case Actions::HideTab:
        tabWidget->hideTab(widget);
        break;
    case Actions::ShowTab:
        tabWidget->showTab(widget);
        break;
    case Actions::DeleteTab:
        ui->listWidget->setCurrentRow(index);
        deleteTab();
        break;
    case Actions::CreateTab:
        createTab();
        break;
    }
}

#ifndef FORMTABTESTER_H
#define FORMTABTESTER_H

#include <QWidget>
#include <string>
#include <unordered_map>

namespace Ui {
class FormTabTester;
}

class TabWidgetPlus;
class QTimer;

class FormTabTester : public QWidget
{
    Q_OBJECT

public:
    explicit FormTabTester(QWidget *parent = 0);
    ~FormTabTester();

private slots:
    void barrage();
    void createTab();
    void deleteTab();
    void tabSelected(const QString &currentText);
    void enabledStateChanged(int state);
    void visibleStateChanged(int state);
    void barrageTimeout();

private:
    Ui::FormTabTester *ui = nullptr;
    int nextTabNumber = 0;
    TabWidgetPlus *cachedTabWidget = nullptr;
    QTimer *barrageTimer = nullptr;

    typedef std::unordered_map<std::string, QWidget *> MapType;
    MapType tabInfo;

    TabWidgetPlus *getTabWidget();
};

#endif // FORMTABTESTER_H

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
    /**
     * @brief Create constant stream of create/delete/show/hide tab requests.
     *
     * Use this function to simulate random (and frequently nonsensical)
     * requests to the TabWidgetPlus. It can find edge cases in testing
     * that might not be obvious to the developer.
     */
    void barrage();

    void createTab();
    void deleteTab();

    /**
     * @brief Update fields in the demo tab.
     *
     * When a tab name is select in the demo tab's list box, update the other
     * fields on the demo tab to reflect the tab' current state.
     *
     * @param currentText The name of the tab used to update contents.
     */
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

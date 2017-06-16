/*----------------------------------------------------------------------------
 * MIT License
 * Copyright (c) 2017 after5cst
 * https://github.com/after5cst/TabWidgetPlusDemo.git
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in allcopies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 **-------------------------------------------------------------------------*/
#ifndef TABWIDGETPLUS_H
#define TABWIDGETPLUS_H

#include <QTabWidget>
#include <deque>

class TabWidgetPlus : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidgetPlus(QWidget *parent = 0);
    virtual ~TabWidgetPlus() = default;

    enum class tabWidgetState { unknown, hidden, visible };
    tabWidgetState tabState( QWidget *tab) const;
signals:

public slots:
    void hideTab(QWidget *widget);
    void showTab(QWidget *widget);

protected:
    virtual void tabInserted(int index) override;
    virtual void tabRemoved(int index) override;

    // methods called from the lambda slot methods in tabInserted().
    virtual void removeHiddenTabData(QWidget *widget);
    virtual void updateTabText(QWidget *widget);

    // event filter installed for tab objects in tabInserted().
    bool eventFilter(QObject *obj, QEvent *event) override;

    // Tracked data for hidden tabs.
    struct HiddenTab {
        int index = -1;
        QWidget *widget = nullptr;
    };
    std::deque<HiddenTab> hidden;
};

#endif // TABWIDGETPLUS_H

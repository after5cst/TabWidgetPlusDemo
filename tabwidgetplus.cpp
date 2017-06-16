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
#include "tabwidgetplus.h"
#include <QEvent>
#include <QLabel>
#include <QTabBar>
#include <QTimer>
#include <assert.h>

TabWidgetPlus::TabWidgetPlus(QWidget *parent) : QTabWidget(parent)
{
}

void TabWidgetPlus::tabInserted(int index)
{
    QTabWidget::tabInserted(index);
    auto newWidget = widget(index);
    if (nullptr == newWidget) {
        assert(false); // should NEVER happen
        return;
    }

    // Set the enabled flag in the tab header based on
    // whether or not the widget itself is enabled.
    tabBar()->setTabEnabled(index, newWidget->isEnabled());

    auto objectNotSeenBefore = true;

    //
    // This class tracks indexes on hidden tabs.  When a new tab
    // has been inserted, then we have to adjust the hidden indexes
    // for tabs that would come *after* the newly inserted tab.
    //
    // There are two possibilities for what *after* means:
    // 1)  For a new (previously unknown) tab, "after" means
    //     any index higher than the one that was just inserted.
    // 2)  For a tab that was hidden and has now been shown, "after"
    //     means everything listed after that tab (note that hidden
    //     tabs can have duplicate indexes).  In this case, we
    //     also need to remove the hidden tab entry because it's
    //     not hidden any more.
    //
    // Find where we should start incrementing tabs in the hidden list...
    auto incrementorIter = hidden.end();
    for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
        if (iter->widget == newWidget) {
            // This hidden tab is now shown (thus it's not hidden any more)
            // Remove this item and start incrementing at next item.
            incrementorIter = hidden.erase(iter);
            objectNotSeenBefore = false;
            break;
        } else if (iter->index > index ) {
            // Start incrementing with this item.
            incrementorIter = iter;
            break;
        }
    }
    // ... and increment them.
    while (incrementorIter != hidden.end()) {
        ++(incrementorIter->index);
        ++incrementorIter;
    }

    // If this object has never been seen before, then we want
    // to add hooks so we can detect changes to the object and change
    // the related tab entry.
    if (objectNotSeenBefore) {
        // Set ourselves up to receive events sent to the new object.
        // This helps us to find things like enabled/disabled changes.
        newWidget->installEventFilter(this);

        QList<QMetaObject::Connection> connections;

        // When the new object is destroyed, call tabDestroyed()
        // on this object so we can get rid of any references to it.
        connections.push_back(connect(newWidget, &QObject::destroyed, this,
        [ = ](QObject *) {
            this->removeHiddenTabData(newWidget);
        }));

        // When the window title on the new object changes, call
        // updateTabText() so we can change the tab text.
        connections.push_back(connect(newWidget, &QWidget::windowTitleChanged, this,
        [ = ](const QString &) {
            this->updateTabText(newWidget);
        }));

        // If this object is destroyed, remove the connections
        // made above so that the callbacks will not happen
        // (since this object is no longer valid).
        connect(this, &QObject::destroyed, this, [ = ](QObject *) {
            for (const auto &connection : connections) {
                this->disconnect(connection);
            }
        });
    }
}

void TabWidgetPlus::tabRemoved(int index)
{
    //
    // The indexes in hidden need to be decremented for any tab
    // following the one that was just removed, so that things
    // are still in sync.
    //
    QTabWidget::tabRemoved(index);
    for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
        if (iter->index > index) {
            --(iter->index);
        }
    }
}

void TabWidgetPlus::removeHiddenTabData(QWidget *widget)
{
    //
    // An object has been deleted.  If it's visible, then the
    // underlying base class takes care of everything.  If it was
    // hidden, though, we need to find the pointer to the widget
    // and remove it.
    //
    for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
        if (iter->widget == widget) {
            hidden.erase(iter);
            return;
        }
    }
}

void TabWidgetPlus::updateTabText(QWidget *widget)
{
    //
    // The window title on the widget was updated.  Since we use
    // the window title as a way to update the tab text for the
    // widget's related tab, we need to also update the tab text.
    //
    auto index = indexOf(widget);
    if (-1 < index) {
        auto newText = widget->windowTitle();
        tabBar()->setTabText(index, newText);
    }
}

void TabWidgetPlus::hideTab(QWidget *widget)
{
    auto index = indexOf(widget);
    if (-1 == index) {
        return;
    }

    // By design, this uses the 'window title' of the object
    // to retain a copy of the tab text.  When/If the object
    // is re-shown the title can then be restored.
    auto currentTitle = tabBar()->tabText(index);
    widget->setWindowTitle(currentTitle);

    // Now, add the widget to the hidden list.  Since the indexes
    // will be adjusted down once the actual removal is done,
    // the index on the insertion needs to be one higher than
    // the final position.
    auto insertLocation = hidden.end();
    for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
        if (iter->index > index) {
            insertLocation = iter;
            break;
        }
    }
    HiddenTab hiddenItem;
    hiddenItem.index = 1 + index;
    hiddenItem.widget = widget;
    hidden.insert(insertLocation, std::move(hiddenItem));
    removeTab(index);
}


void TabWidgetPlus::showTab(QWidget *widget)
{
    for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
        if (iter->widget == widget) {
            // Note that I CANNOT remove the item from the
            // hidden list here:  I have to know where the
            // hidden item came from after it shows up in order
            // to adjust the hidden indexes correctly.
            // (one use case: think of showing a hidden item between
            // two other hidden items.)
            insertTab(iter->index, iter->widget, widget->windowTitle());
            break;
        }
    }
}

bool TabWidgetPlus::eventFilter(QObject *obj, QEvent *event)
{
    if (QEvent::EnabledChange == event->type()) {
        //
        // Since we installed the event filter for a QWidget, we
        // *absolutely know* the object provided is a QWidget.
        //
        auto widget = reinterpret_cast<QWidget *>(obj);
        auto index = indexOf(widget);
        if (-1 != index) {
            tabBar()->setTabEnabled(index, widget->isEnabled());
        }
    }
    return false;
}

TabWidgetPlus::tabWidgetState TabWidgetPlus::tabState( QWidget *widget) const
{
    auto result = tabWidgetState::unknown;
    if (-1 != indexOf(widget)) {
        result = tabWidgetState::visible;
    } else {
        for (auto &item : hidden) {
            if (item.widget == widget) {
                result = tabWidgetState::hidden;
                break;
            }
        }
    }
    return result;
}

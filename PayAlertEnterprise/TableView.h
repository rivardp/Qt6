#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include <QEvent>
#include <QMouseEvent>

class TableView: public QTableView{
    Q_OBJECT

public:
    TableView(QWidget *parent = Q_NULLPTR):QTableView(parent)
    {
        viewport()->installEventFilter(this);
        setMouseTracking(true);
    }

    bool eventFilter(QObject *watched, QEvent *event)
    {
        if(viewport() == watched)
        {
            if(event->type() == QEvent::MouseMove)
            {
                QModelIndex index;
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                index = indexAt(mouseEvent->pos());
                if(index.isValid())
                    emit addHoverRowSignal(index);
            }
            else
            {
                if(event->type() == QEvent::Leave)
                    emit removeHoverRowSignal();
            }
        }

        return QTableView::eventFilter(watched, event);
    }

private:

signals:
    void addHoverRowSignal(const QModelIndex &index);
    void removeHoverRowSignal(const QModelIndex &index);
};

#endif // TABLEVIEW_H

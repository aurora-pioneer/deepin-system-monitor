#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QWidget>
#include "list_item.h"

class ListView : public QWidget
{
    Q_OBJECT
    
public:
    ListView(int height=32, QWidget *parent = 0);
    
    void addItems(QList<ListItem*> items);
    void clearItems();
    
    void addSelections(QList<ListItem*> items);
    void clearSelections();
    
    void setTitles(QStringList *titles);
    void setSortAlgorithm();
    void setColumnWidths(QList<int> widths);
    
protected:
    void paintEvent(QPaintEvent *event);
    QList<ListItem*> listItems;
    int rowHeight;
    
private:
    int renderY;
    QList<int> columnWidths;
};

#endif
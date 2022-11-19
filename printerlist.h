#ifndef PRINTERLIST_H
#define PRINTERLIST_H

#include <QListView>
#include <QObject>
#include <QWidget>
#include <QStringListModel>

class PrinterList : public QListView
{
    Q_OBJECT
private:
    QStringList printer_list;
    QStringListModel *model;
public:
    PrinterList();
    PrinterList(const PrinterList &);
    PrinterList(QWidget *parent);
    ~PrinterList();

public slots:
    void updatePrinterList();
};

#endif // PRINTERLIST_H

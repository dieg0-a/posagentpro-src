#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QSettings>
#include <QGraphicsScene>
#include <QPrinter>
#include "object.hpp"

#include <map>

#include <sstream>

#include "eventclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, Object
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit MainWindow(QWidget *parent = nullptr);

    static MainWindow *active_window;
    
    EventClient eventclient;


    ~MainWindow();

    void onEvent(EventData d);

public slots:
    void setComboOption();
    void setStringOption();
    void setIntOption();
    void optionSliderReleased(int value);
    void optionCheckBoxToggled(int value);
    void optionTextChanged(const QString &text);
    void optionComboChanged(const QString &text);
    void optionButtonClicked();
    void updatePrintConfigWidget();
    void refreshTimer();
    void startNetworkThread();
    void stopNetworkThread();
    void restartNetworkThread();
    void updatePrinterDriver(int index);
    void setHttpProxyPort(int port);
    void setStartInTray(int state);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void setDemoMode();
    void closeApplication();
    void updateReceiptPreview();
    void toggleDisplayPreview(bool checked);
    void scheduleDiplayPreviewUpdate();
    void paintPrintPreview(QPrinter *printer);

private:
    Ui::MainWindow *ui;
//    Printer *current_printer_interface;

    QAction *quitAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *minimizeAction;

    QMenu *trayIconMenu;

    QWidget *p_settings_top_widget;

    QSystemTrayIcon *trayIcon;

    QWidget *printer_status_widget;
    QWidget *demo_mode_widget;

    QLabel *printer_status_label;
    QLabel *printer_status_icon_label;
    QLabel *demo_mode_on_off;

    QIcon *printer_status_on_icon;
    QIcon *printer_status_off_icon;

    QTimer *display_timer;

    static QSettings program_settings;

    static bool read_str_from_settings(const std::string &key, std::string &val);
    static bool read_int_from_settings(const std::string &key, int &val);
    static bool read_bool_from_settings(const std::string &key, bool &val);

    static bool save_str_to_settings(const std::string &key, const std::string &val);
    static bool save_int_to_settings(const std::string &key, int val);
    static bool save_bool_to_settings(const std::string &key, bool val);

    void gammaUpdated(EventData data);

    bool closing = false;
    bool demo_mode_last;
    bool showpreview = false;

    void updateGUIControls();

    std::stringstream receipt_buf;

    QPixmap *receipt_preview_pixmap;
    QGraphicsScene receipt_preview_scene;
    int gamma = 240;

    bool display_preview_update_schedule = true;
};

#endif // MAINWINDOW_H

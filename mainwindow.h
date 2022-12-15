#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QSettings>
#include <QGraphicsScene>

#include <map>

#include <sstream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

public slots:
    void setComboOption();
    void setStringOption();
    void setNumberOption();

    void updatePrintConfigWidget();

    void refreshTimer();

    void startNetworkThread();
    void stopNetworkThread();

    void restartNetworkThread();

    void setPixelWidth(int);
    void setFeedLines(int);
    void setPrintStandard(bool escpos_toggled);
    void setCashDrawerEnabled(int state);
    void setCutterEnabled(int state);

    void updatePrinterDriver(int index);

    void setHttpProxyPort(int port);

    void setStartInTray(int state);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void setDemoMode();

    void closeApplication();

    void updateReceiptPreview();

    void toggleDisplayPreview(bool checked);

    void gammaSliderMoved(int);

    void gammaSliderChanged(int);

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

    QLabel *printer_status_label;
    QIcon *printer_status_on_icon;
    QIcon *printer_status_off_icon;

    QLabel *printer_status_icon_label;


    QWidget *demo_mode_widget;
    QLabel *demo_mode_on_off;

    static QSettings program_settings;

    static bool read_str_from_settings(const std::string &key, std::string &val);
    static bool read_int_from_settings(const std::string &key, int &val);
    static bool read_bool_from_settings(const std::string &key, bool &val);

    static bool save_str_to_settings(const std::string &key, const std::string &val);
    static bool save_int_to_settings(const std::string &key, int val);
    static bool save_bool_to_settings(const std::string &key, bool val);

    bool closing = false;

    void updateGUIControls();

    bool demo_mode_last;

    std::stringstream receipt_buf;

    QPixmap receipt_preview_pixmap;
    QGraphicsScene receipt_preview_scene;

    bool showpreview = false;

    int gamma = 240;

};

#endif // MAINWINDOW_H

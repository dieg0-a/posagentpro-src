#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "object.hpp"
#include <QBoxLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLabel>
#include <QMainWindow>
#include <QPrinter>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QDialog>

#include <map>

#include <sstream>

#include "eventclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, Object {
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
  void optionSpinboxChanged(int value);
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
  void closeApplication();
  void updateReceiptPreview();
  void toggleDisplayPreview(bool checked);
  void scheduleDiplayPreviewUpdate();
  void paintPrintPreview(QPrinter *printer);
  void saveOptionChanges();
  void showAbout(bool toggled);

private:
  Ui::MainWindow *ui;
  //    Printer *current_printer_interface;

  QAction *quitAction;
  QAction *maximizeAction;
  QAction *restoreAction;
  QAction *minimizeAction;

  QDialog *about;

  QMenu *trayIconMenu;

  QWidget *p_settings_top_widget;

  QSystemTrayIcon *trayIcon;

  QWidget *printer_status_widget;

  QLabel *printer_status_label;
  QLabel *printer_status_icon_label;

  QIcon *printer_status_on_icon;
  QIcon *printer_status_off_icon;

  QTimer *display_timer;

  static QSettings program_settings;

  static bool read_str_from_settings(const std::string &key, std::string &val);
  static bool read_int_from_settings(const std::string &key, int &val);
  static bool read_bool_from_settings(const std::string &key, bool &val);

  static bool save_str_to_settings(const std::string &key,
                                   const std::string &val);
  static bool save_int_to_settings(const std::string &key, int val);
  static bool save_bool_to_settings(const std::string &key, bool val);

  void gammaUpdated(EventData data);

  bool closing = false;
  bool showpreview = false;

  void updateGUIControls();

  std::stringstream receipt_buf;

  QPixmap receipt_preview_pixmap;
  unsigned char receipt_preview_image_data[2000000];
  //    QGraphicsPixmapItem *receipt_preview_pixmap_item = nullptr;
  QGraphicsScene receipt_preview_scene;
  int gamma = 240;

  bool display_preview_update_schedule = true;

  QTimer *option_save_timer;

  static bool scheduleOptionSaveStr(const std::string &name,
                                    const std::string &value);
  static bool scheduleOptionSaveInt(const std::string &name, int value);
  static bool scheduleOptionSaveBool(const std::string &name, bool value);

  static std::map<std::string, std::string> str_to_save;
  static std::map<std::string, int> int_to_save;
  static std::map<std::string, bool> bool_to_save;
};

#endif // MAINWINDOW_H

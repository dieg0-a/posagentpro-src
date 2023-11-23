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
#include <QScrollArea>

#include <map>

#include <sstream>

#include "eventclient.h"
#include "ui_about.h"

namespace Ui {
class MainWindow;
}

class Printer;

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
  void setComboOption(bool label = false);
  void setStringOption(bool label = false);
  void setIntOption(bool label = false);
  void optionSliderReleased(int value, bool label = false);
  void optionCheckBoxToggled(int value, bool label = false);
  void optionTextChanged(const QString &text, bool label = false);
  void optionComboChanged(const QString &text, bool label = false);


  void setComboOptionPrinter() {setComboOption();};
  void setStringOptionPrinter() {setStringOption();};
  void setIntOptionPrinter() {setIntOption();};
  void optionSliderReleasedPrinter(int value) {optionSliderReleased(value);};
  void optionCheckBoxToggledPrinter(int value) {optionCheckBoxToggled(value);};
  void optionTextChangedPrinter(const QString &text) {optionTextChanged(text);};
  void optionComboChangedPrinter(const QString &text) {optionComboChanged(text);};


  void setComboOptionLabel() {setComboOption(true);};
  void setStringOptionLabel() {setStringOption(true);};
  void setIntOptionLabel() {setIntOption(true);};
  void optionSliderReleasedLabel(int value) {optionSliderReleased(value, true);};
  void optionCheckBoxToggledLabel(int value) {optionCheckBoxToggled(value, true);};
  void optionTextChangedLabel(const QString &text) {optionTextChanged(text, true);};
  void optionComboChangedLabel(const QString &text) {optionComboChanged(text, true);};

  void optionButtonClicked();
  void updatePrintConfigWidget(QScrollArea *parent, Printer *printer, bool label=false);
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

  static bool save_str_to_settings(const std::string &key,
                                   const std::string &val);
  static bool save_int_to_settings(const std::string &key, int val);
  static bool save_bool_to_settings(const std::string &key, bool val);

  void gammaUpdated(EventData data);

  bool closing = false;
  bool demo_mode_last;
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

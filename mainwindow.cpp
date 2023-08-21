#include "mainwindow.h"
#include "hardware.hpp"
#include "messagesystem.h"
#include "printercombofieldmodel.h"
#include "printerdriverlistmodel.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QComboBox>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QTimer>
#include <iostream>
#include <math.h>
#include <qlayoutitem.h>
#include <qscrollarea.h>
#include <sstream>

MainWindow *MainWindow::active_window;

QSettings MainWindow::program_settings =
    QSettings("PosAgentPRO", "PosAgentPRO");

extern int bayer_matrix[];

inline double bayer(int i, int j) {
  return double(bayer_matrix[i * 4 + j]) / 64.0;
}

inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = {a};
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

void MainWindow::setHttpProxyPort(int port) { GlobalState::setHttpPort(port); }

void MainWindow::setComboOption() {
  auto j = qobject_cast<QPushButton *>(sender());
  auto siblings = j->parent()->findChildren<QComboBox *>();
  for (auto &s : siblings) {

    //        GlobalState::printerSetCombo(s->objectName().toStdString(),
    //        s->currentData(Qt::DisplayRole).toString().toStdString());
    GlobalState::printerSetIndex(s->objectName().toStdString(),
                                 s->currentIndex());
  }
  updatePrintConfigWidget();
}

void MainWindow::setStringOption() {
  auto j = qobject_cast<QPushButton *>(sender());
  auto siblings = j->parent()->findChildren<QLineEdit *>();
  for (auto &s : siblings) {

    GlobalState::printerSetString(s->objectName().toStdString(),
                                  s->text().toStdString());
  }
  updatePrintConfigWidget();
}

void MainWindow::setIntOption() {
  auto j = qobject_cast<QPushButton *>(sender());
  auto siblings = j->parent()->findChildren<QLineEdit *>();
  for (auto &s : siblings) {
    GlobalState::printerSetInt(s->objectName().toStdString(),
                               s->text().toInt());
  }
  updatePrintConfigWidget();
}

void MainWindow::optionSliderReleased(int value) {
  auto j = qobject_cast<QSlider *>(sender());
  auto slider_bottom_widget = j->parent()->parent()->children().at(2);
  qobject_cast<QLabel *>(slider_bottom_widget->children().at(2))
      ->setNum(j->value());
  GlobalState::printerSetInt(j->objectName().toStdString(), j->value());
}

void MainWindow::optionCheckBoxToggled(int value) {
  auto j = qobject_cast<QCheckBox *>(sender());
  GlobalState::printerSetInt(j->objectName().toStdString(), value);
}

void MainWindow::optionTextChanged(const QString &text) {
  auto j = qobject_cast<QLineEdit *>(sender());
  GlobalState::printerSetString(j->objectName().toStdString(),
                                text.toStdString());
}

void MainWindow::optionComboChanged(const QString &text) {
  auto j = qobject_cast<QComboBox *>(sender());
  GlobalState::printerSetString(j->objectName().toStdString(),
                                text.toStdString());
}

void MainWindow::optionButtonClicked() {
  auto j = qobject_cast<QPushButton *>(sender());

  EventSystem::instance().emitEvent(
      j->objectName().toStdString() + "_executed",
      EventData(j->objectName().toStdString() + "_executed",
                eventclient.getID()));
}

void MainWindow::paintPrintPreview(QPrinter *printer) {
  int max_width = 512;
  if (GlobalState::getLastReceipt() == nullptr)
    return;
  receipt_buf.str(std::string());
  auto s = GlobalState::getLastReceipt()->image_memory_ptr();
  int height = GlobalState::getLastReceipt()->height();
  int width = GlobalState::getLastReceipt()->width();
  int bytespp = GlobalState::getLastReceipt()->components();
  double ratio = double(width) / double(max_width);
  int new_height = height / ratio;

  double gamma_d = (double)gamma;

  short bitcounter = 0;
  unsigned char buffer_byte = '\0';
  for (int i = 0; i < new_height; i++) {
    for (int j = 0; j < (max_width); j++) {

      double i_s = double(i) * ratio;
      double j_s = double(j) * ratio;

      double i_bottom_d = floor(i_s);
      double i_top_d = ceil(i_s) <= height - 1 ? ceil(i_s) : height - 1;
      double j_bottom_d = floor(j_s);
      double j_top_d = ceil(j_s) <= width - 1 ? ceil(j_s) : width - 1;

      int i_bottom = i_bottom_d;
      int i_top = i_top_d;
      int j_bottom = j_bottom_d;
      int j_top = j_top_d;

      double dx = i_s - i_bottom_d;
      double dy = j_s - j_bottom_d;

      int colorbyte = 0;

      for (int k = 0; k < bytespp; k++) {
        double c_bottom_left = s[i_bottom][(j_bottom * bytespp) + k];
        double c_bottom_right = s[i_bottom][(j_top * bytespp) + k];
        double c_top_left = s[i_top][(j_bottom * bytespp) + k];
        double c_top_right = s[i_top][(j_top * bytespp) + k];

        double dcdytop = c_top_right - c_top_left;
        double dcdybottom = c_bottom_right - c_bottom_left;
        double c_top = c_top_left + dcdytop * dy;
        double c_bottom = c_bottom_left + dcdybottom * dy;

        double dcdx = c_top - c_bottom;

        double component = c_bottom + dcdx * dx;
        colorbyte += component;
      }
      colorbyte /= bytespp;

      colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
      // Transform to linear RGB

      double color = double(colorbyte) / 255.0;

      if (color < 0.04045)
        color = color / 12.92;
      else
        color = fastPow((color + 0.055) / 1.055, gamma_d / 100.0);

      int colorbyte_linear = color * 255.0;
      if (colorbyte_linear < 0xAA) {
        if (colorbyte_linear > 0x60) {
          if (bayer(i % 4, j % 4) * 255 < colorbyte_linear) {
            buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));
          }
        }
      } else
        buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));

      bitcounter++;
      if (bitcounter == 8) {
        bitcounter = 0;
        receipt_buf << buffer_byte;
        buffer_byte = 0;
      }
    }
  }

  QImage i = QImage((unsigned char *)receipt_buf.str().c_str(), max_width,
                    new_height, QImage::Format::Format_Mono);
  QPainter painter;
  painter.begin(printer);
  // auto pagerect =
  // printer->pageLayout().fullRectPixels(printer->resolution());
  auto margins = printer->pageLayout().marginsPixels(printer->resolution());
  QRectF pagerect =
      printer->pageLayout().paintRectPixels(printer->resolution());
  QRectF source_rect_float = QRectF(0, 0, max_width, new_height);

  QRectF printable_area_pixels = QRectF(
      0, 0, pagerect.width(),
      ((float)(pagerect.width()) / (float)max_width) * (float)new_height);
  painter.drawImage(printable_area_pixels, i,
                    QRect(0, 0, max_width, new_height));

  // Use the painter to draw on the page.

  painter.end();
}

void MainWindow::updatePrintConfigWidget() {
  //auto temp = ui->printer_driver_settings;
  auto printer_driver_settings = new QWidget(ui->printer_driver_settings_scroll_area);
  ui->printer_driver_settings_scroll_area->setWidget(printer_driver_settings);
//  ui->printer_driver_settings->setTitle("Printer Driver Settings");
//  ui->printer_driver_settings_scroll_area->layout()->replaceWidget(temp,
//                                                ui->printer_driver_settings);
  //delete temp;
  //    ui->printer_driver_settings = new QGroupBox(ui->centralwidget);
  //    ui->centralwidget->layout()->addWidget(ui->printer_driver_settings);
  auto hl = new QHBoxLayout();
  printer_driver_settings->setLayout(hl);

  p_settings_top_widget = new QWidget(printer_driver_settings);

  hl->addWidget(p_settings_top_widget);
  auto vl = new QVBoxLayout();  
  p_settings_top_widget->setLayout(vl);

  for (auto &options : GlobalState::getCurrentPrinter()->getFieldsByOrder()) {
    if (options.second->get_type() == STRING) {
      auto string_field = new QWidget(p_settings_top_widget);
      auto hl = new QHBoxLayout();
      hl->setSpacing(0);
      //            hl->setMargin(0);
      p_settings_top_widget->layout()->addWidget(string_field);
      //            p_settings_top_widget->layout()->addItem(hl);
      string_field->setLayout(hl);

      auto label = new QLabel(string_field);
      hl->addWidget(label);
      label->setText(options.second->display_name().c_str());
      label->setMaximumWidth(70);
      label->setMinimumWidth(70);

      auto input = new QLineEdit(string_field);
      input->setObjectName(options.second->name().c_str());
      hl->addWidget(input);
      input->setText(options.second->get_string().c_str());

      QObject::connect(input, SIGNAL(textEdited(const QString &)), this,
                       SLOT(optionTextChanged(const QString &)));

      string_field->setMaximumHeight(40);
    } else if (options.second->get_type() == INTEGER) {
      auto number_field = new QWidget(p_settings_top_widget);
      p_settings_top_widget->layout()->addWidget(number_field);
      number_field->setLayout(new QHBoxLayout());

      auto label = new QLabel(number_field);
      number_field->layout()->addWidget(label);
      label->setText(options.second->display_name().c_str());
      label->setMaximumWidth(70);
      label->setMinimumWidth(70);

      auto input = new QLineEdit(number_field);
      input->setValidator(new QIntValidator(0, 100, number_field));
      input->setObjectName(options.second->name().c_str());
      number_field->layout()->addWidget(input);
      input->setText(QString::number(options.second->get_int()));

      QObject::connect(input, SIGNAL(textEdited(const QString &)), this,
                       SLOT(optionTextChanged(const QString &)));

      number_field->setMaximumHeight(50);
    } else if (options.second->get_type() == INTEGER_RANGE) {
      integer_range_field *r = (integer_range_field *)options.second;

      if (r->get_widget_type() == "lineedit") {
        auto number_field = new QWidget(p_settings_top_widget);
        p_settings_top_widget->layout()->addWidget(number_field);
        number_field->setLayout(new QHBoxLayout());

        auto label = new QLabel(number_field);
        number_field->layout()->addWidget(label);
        label->setText(options.second->display_name().c_str());
        label->setMaximumWidth(70);
        label->setMinimumWidth(70);
        auto input = new QLineEdit(number_field);
        input->setValidator(new QIntValidator(
            r->getRange().first, r->getRange().second, number_field));
        input->setObjectName(options.second->name().c_str());
        number_field->layout()->addWidget(input);
        input->setText(QString::number(options.second->get_int()));
      } else if (r->get_widget_type() == "spinbox") {
        auto number_field = new QWidget(p_settings_top_widget);
        p_settings_top_widget->layout()->addWidget(number_field);
        number_field->setLayout(new QHBoxLayout());

        auto label = new QLabel(number_field);
        number_field->layout()->addWidget(label);
        label->setText(options.second->display_name().c_str());
        label->setMaximumWidth(70);
        label->setMinimumWidth(70);
        auto input = new QSpinBox(number_field);
        input->setRange(r->getRange().first, r->getRange().second);
        input->setValue(options.second->get_int());
        input->setObjectName(options.second->name().c_str());
        number_field->layout()->addWidget(input);
      } else if (r->get_widget_type() == "slider") {
        auto number_field = new QWidget(p_settings_top_widget);
        p_settings_top_widget->layout()->addWidget(number_field);
        number_field->setLayout(new QVBoxLayout());
        //        number_field->layout()->setMargin(0);
        //        number_field->layout()->setSpacing(0);

        auto number_field_top = new QWidget(number_field);
        number_field->layout()->addWidget(number_field_top);

        number_field_top->setLayout(new QHBoxLayout());
        number_field_top->layout()->setMargin(0);
        number_field_top->layout()->setSpacing(0);

        auto label = new QLabel(number_field_top);
        number_field_top->layout()->addWidget(label);
        label->setText(options.second->display_name().c_str());
        label->setMaximumWidth(70);
        label->setMinimumWidth(70);
        auto input = new QSlider(Qt::Orientation::Horizontal, number_field_top);
        input->setRange(r->getRange().first, r->getRange().second);
        input->setValue(options.second->get_int());
        input->setObjectName(options.second->name().c_str());
        number_field_top->layout()->addWidget(input);

        //        p_settings_top_widget->layout()->removeWidget(number_field);

        auto number_field_bottom = new QWidget(number_field);
        number_field->layout()->addWidget(number_field_bottom);
        number_field_bottom->setLayout(new QHBoxLayout());
        number_field_bottom->layout()->setMargin(0);
        number_field_bottom->layout()->setSpacing(0);

        auto minmax = new QLabel(number_field_bottom);
        number_field_bottom->layout()->addWidget(minmax);
        minmax->setText(
            QString::number(
                ((integer_range_field *)options.second)->get_min()) +
            "-" +
            QString::number(
                ((integer_range_field *)options.second)->get_max()));
        minmax->setAlignment(Qt::AlignmentFlag::AlignBottom);

        auto value = new QLabel(number_field_bottom);
        number_field_bottom->layout()->addWidget(value);
        value->setText(QString::number(
            ((integer_range_field *)options.second)->get_int()));
        value->setAlignment(Qt::AlignmentFlag::AlignRight |
                            Qt::AlignmentFlag::AlignBottom);

        //        number_field_top->setMinimumHeight(40);
        //        number_field_top->setMaximumHeight(40);
        //        number_field_bottom->setMinimumHeight(40);
        //        number_field_bottom->setMaximumHeight(40);
        //        number_field->setMaximumHeight(120);
        //        number_field->setMinimumHeight(120);
        //        connect(input, SIGNAL(valueChanged(int)), label,
        //        SLOT(setNum(int)));

        connect(input, SIGNAL(valueChanged(int)), this,
                SLOT(optionSliderReleased(int)));
        input->setTracking(false);
      }
    } else if (options.second->get_type() == COMBO_LIST_STRING) {
      string_combo_list_field *combo =
          (string_combo_list_field *)options.second;
      auto combo_widget = new QWidget(p_settings_top_widget);
      auto combo_field = new QComboBox(combo_widget);
      combo_field->setObjectName(options.second->name().c_str());
      p_settings_top_widget->layout()->addWidget(combo_widget);
      combo_field->setModel(new StringComboListModel(*combo, combo_field));
      combo_field->setCurrentIndex(options.second->get_index());

      auto hl = new QHBoxLayout();
      hl->setSpacing(0);
      //            hl->setMargin(0);
      //            p_settings_top_widget->layout()->addItem(hl);
      combo_widget->setLayout(hl);

      auto label = new QLabel(combo_widget);
      hl->addWidget(label);
      label->setText(options.second->display_name().c_str());
      label->setMaximumWidth(70);
      label->setMinimumWidth(70);

      hl->addWidget(combo_field);
      connect(combo_field, SIGNAL(currentTextChanged(const QString &)), this,
              SLOT(optionComboChanged(const QString &)));
      combo_widget->setMaximumHeight(50);

    } else if (options.second->get_type() == BOOLEAN_FIELD) {
      auto boolean_field = new QWidget(p_settings_top_widget);
      auto hl = new QHBoxLayout();
      hl->setSpacing(0);
      //            hl->setMargin(0);
      p_settings_top_widget->layout()->addWidget(boolean_field);
      //            p_settings_top_widget->layout()->addItem(hl);
      boolean_field->setLayout(hl);

      auto label = new QLabel(boolean_field);
      hl->addWidget(label);
      label->setText(options.second->display_name().c_str());
      //      label->setMaximumWidth(70);
      label->setMinimumWidth(70);

      auto input = new QCheckBox(boolean_field);
      input->setObjectName(options.second->name().c_str());
      hl->addWidget(input);
      input->setChecked(options.second->get_int());
      input->setFixedWidth(20);
      QObject::connect(input, SIGNAL(stateChanged(int)), this,
                       SLOT(optionCheckBoxToggled(int)));
    } else if (options.second->get_type() == ACTION) {
      auto button = new QWidget(p_settings_top_widget);
      auto hl = new QHBoxLayout();
      hl->setSpacing(0);

      //            hl->setMargin(0);
      p_settings_top_widget->layout()->addWidget(button);
      //            p_settings_top_widget->layout()->addItem(hl);
      button->setLayout(hl);

      auto input = new QPushButton(button);
      input->setObjectName(options.second->name().c_str());
      input->setText(options.second->display_name().c_str());
      input->setMaximumWidth(150);
      input->setMinimumWidth(150);
      hl->addWidget(input);
      hl->addStretch();
      //      input->setFixedWidth(200);
      QObject::connect(input, SIGNAL(clicked()), this,
                       SLOT(optionButtonClicked()));
    }
  }
  static_cast<QVBoxLayout *>(p_settings_top_widget->layout())->addStretch();
}

void MainWindow::gammaUpdated(EventData data) {
  std::cout << "Gamma Updated event received!\n";
  std::cout << "With sender ID: " << data.sender() << std::endl;
  std::cout << "Event name: " << data.name() << std::endl;
  auto new_gamma = data.get_int();
  if (new_gamma != gamma) {
    gamma = new_gamma;
    scheduleDiplayPreviewUpdate();
  }
}

void MainWindow::updateReceiptPreview() {
  if (!display_preview_update_schedule)
    return;
  else
    display_preview_update_schedule = false;
  int max_width = 512;
  if (GlobalState::getLastReceipt() == nullptr)
    return;
  receipt_buf.str(std::string());
  auto s = GlobalState::getLastReceipt()->image_memory_ptr();
  int height = GlobalState::getLastReceipt()->height();
  int width = GlobalState::getLastReceipt()->width();
  int bytespp = GlobalState::getLastReceipt()->components();
  double ratio = double(width) / double(max_width);
  int new_height = height / ratio;

  double gamma_d = (double)gamma;

  short bitcounter = 0;
  unsigned char buffer_byte = '\0';
  for (int i = 0; i < new_height; i++) {
    for (int j = 0; j < (max_width); j++) {

      double i_s = double(i) * ratio;
      double j_s = double(j) * ratio;

      double i_bottom_d = floor(i_s);
      double i_top_d = ceil(i_s) <= height - 1 ? ceil(i_s) : height - 1;
      double j_bottom_d = floor(j_s);
      double j_top_d = ceil(j_s) <= width - 1 ? ceil(j_s) : width - 1;

      int i_bottom = i_bottom_d;
      int i_top = i_top_d;
      int j_bottom = j_bottom_d;
      int j_top = j_top_d;

      double dx = i_s - i_bottom_d;
      double dy = j_s - j_bottom_d;

      int colorbyte = 0;

      for (int k = 0; k < bytespp; k++) {
        double c_bottom_left = s[i_bottom][(j_bottom * bytespp) + k];
        double c_bottom_right = s[i_bottom][(j_top * bytespp) + k];
        double c_top_left = s[i_top][(j_bottom * bytespp) + k];
        double c_top_right = s[i_top][(j_top * bytespp) + k];

        double dcdytop = c_top_right - c_top_left;
        double dcdybottom = c_bottom_right - c_bottom_left;
        double c_top = c_top_left + dcdytop * dy;
        double c_bottom = c_bottom_left + dcdybottom * dy;

        double dcdx = c_top - c_bottom;

        double component = c_bottom + dcdx * dx;
        colorbyte += component;
      }
      colorbyte /= bytespp;

      colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
      // Transform to linear RGB

      double color = double(colorbyte) / 255.0;

      if (color < 0.04045)
        color = color / 12.92;
      else
        color = fastPow((color + 0.055) / 1.055, gamma_d / 100.0);

      int colorbyte_linear = color * 255.0;
      if (colorbyte_linear < 0xAA) {
        if (colorbyte_linear > 0x60) {
          if (bayer(i % 4, j % 4) * 255 < colorbyte_linear) {
            buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));
          }
        }
      } else
        buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));

      bitcounter++;
      if (bitcounter == 8) {
        bitcounter = 0;
        receipt_buf << buffer_byte;
        buffer_byte = 0;
      }
    }
  }

  QImage i = QImage((unsigned char *)receipt_buf.str().c_str(), max_width,
                    new_height, QImage::Format::Format_Mono);
  receipt_preview_scene.clear();
  if (receipt_preview_pixmap != nullptr)
    delete receipt_preview_pixmap;
  receipt_preview_pixmap = new QPixmap();
  //       ui->receipt_preview_view->update();
  receipt_preview_pixmap->convertFromImage(i);
  receipt_preview_scene.addPixmap(*receipt_preview_pixmap);
  //       receipt_preview_scene.update();
  receipt_preview_scene.setSceneRect(receipt_preview_pixmap->rect());
  //       ui->receipt_preview_view->fitInView(receipt_preview_pixmap.rect().x(),
  //       receipt_preview_pixmap.rect().y(),
  //                                           receipt_preview_pixmap.rect().width(),
  //                                           ui->receipt_preview_view->height());
  //       ui->receipt_preview_view->update();
}

void MainWindow::refreshTimer() {
  if (GlobalState::processQueue()) {
    if (showpreview)
      scheduleDiplayPreviewUpdate();
  }

  if (GlobalState::getPrinterStatus() == CONNECTED) {
    printer_status_icon_label->setPixmap(
        printer_status_on_icon->pixmap(16, 16));
    printer_status_label->setText("Printer ON ");
  } else {
    printer_status_icon_label->setPixmap(
        printer_status_off_icon->pixmap(16, 16));
    printer_status_label->setText("Printer OFF");
  };

  if (GlobalState::demo_mode != demo_mode_last)
    setDemoMode();
}

void MainWindow::startNetworkThread() { GlobalState::startNetworkThread(); }
void MainWindow::stopNetworkThread() { GlobalState::stopNetworkThread(); }

void MainWindow::restartNetworkThread() {
  GlobalState::stopNetworkThread();
  GlobalState::startNetworkThread();
}

bool MainWindow::read_str_from_settings(const std::string &key,
                                        std::string &val) {
  if (program_settings.contains(key.c_str())) {
    auto v = program_settings.value(QString::fromStdString(key));
    val = v.toString().toStdString();
    return true;
  }
  return false;
}

bool MainWindow::read_int_from_settings(const std::string &key, int &val) {
  if (program_settings.contains(key.c_str())) {
    auto v = program_settings.value(QString::fromStdString(key));
    val = v.toInt();
    return true;
  } else
    return false;
}

bool MainWindow::read_bool_from_settings(const std::string &key, bool &val) {
  if (program_settings.contains(key.c_str())) {
    auto v = program_settings.value(QString::fromStdString(key));
    val = v.toBool();
    return true;
  }
  return false;
}

bool MainWindow::save_str_to_settings(const std::string &key,
                                      const std::string &val) {
  program_settings.setValue(QString::fromStdString(key),
                            QString::fromStdString(val));
  return true;
}

bool MainWindow::save_int_to_settings(const std::string &key, int val) {
  program_settings.setValue(QString::fromStdString(key), val);
  return true;
}

bool MainWindow::save_bool_to_settings(const std::string &key, bool val) {
  program_settings.setValue(QString::fromStdString(key), val);
  return true;
}

inline void MainWindow::updateGUIControls() {
  ui->http_port_spinbox->setValue(GlobalState::getHttpPort());
}

void MainWindow::updatePrinterDriver(int index) {
  GlobalState::setCurrentPrinter(index);
  updatePrintConfigWidget();
  updateGUIControls();
  auto fields = GlobalState::getCurrentPrinter()->getFieldsByName();
  if (fields.contains("gamma")) {
    gamma = fields["gamma"]->get_int();
    scheduleDiplayPreviewUpdate();
  }
}

void MainWindow::setStartInTray(int state) {
  program_settings.setValue("start_in_tray",
                            state == Qt::CheckState::Checked ? true : false);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger) {
    if (this->isHidden() || this->isMinimized()) {
      this->showNormal();
    } else
      this->hide();
  }
}

void MainWindow::setDemoMode() {
  demo_mode_last = GlobalState::demo_mode;
  if (demo_mode_last)
    demo_mode_on_off->setText("Demo mode: ON ");
  else
    demo_mode_on_off->setText("Demo mode: OFF");
}

void MainWindow::closeApplication() {
  closing = true;
  showNormal(); //// Workaround for Windows
  close();
}

MainWindow::MainWindow(QWidget *parent)
    : eventclient(this), QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  //    setFixedSize(geometry().width(), geometry().height());

  active_window = this;
  receipt_preview_pixmap = new QPixmap();

  minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

  maximizeAction = new QAction(tr("Ma&ximize"), this);
  connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, SIGNAL(triggered()), this, SLOT(closeApplication()));

  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(minimizeAction);
  trayIconMenu->addAction(maximizeAction);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
  trayIcon->setIcon(QIcon(":/images/images/printagent_icon_big.png"));
  trayIcon->show();

  program_settings.setDefaultFormat(QSettings::Format::NativeFormat);

  GlobalState::setSettingsHooks(read_str_from_settings, read_int_from_settings,
                                read_bool_from_settings, save_str_to_settings,
                                save_int_to_settings, save_bool_to_settings);
  GlobalState::loadSettings();

  updateGUIControls();

  GlobalState::autosave = true;

  //    p_settings_top_widget = new QWidget(ui->printer_driver_settings);
  //    ui->printer_driver_settings->setLayout(new QVBoxLayout());
  //    p_settings_top_widget = ui->printer_driver_settings;

  /*
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) &&
  !defined(__CYGWIN__) printer_drivers.emplace("Windows Printer", &winprint);
      current_printer_interface = &winprint;
  #endif
  #ifdef __linux
      printer_drivers.emplace("Linux USB Printer", &linux_usb_print);
      current_printer_interface = &linux_usb_print;
      GlobalState::setCurrentPrinter(current_printer_interface);
  #endif
  */

  ////Status bar widgets here

  printer_status_widget = new QWidget(this);
  printer_status_label = new QLabel(printer_status_widget);
  printer_status_icon_label = new QLabel(printer_status_widget);

  printer_status_on_icon = new QIcon(":/images/images/printer_green.png");
  printer_status_off_icon = new QIcon(":/images/images/printer_red.png");

  GlobalState::updatePrinterStatus();

  printer_status_widget->setLayout(new QHBoxLayout());
  printer_status_widget->layout()->addWidget(printer_status_label);
  printer_status_widget->layout()->addWidget(printer_status_icon_label);

  GlobalState::updatePrinterStatus();

  if (GlobalState::getPrinterStatus() == CONNECTED) {
    printer_status_icon_label->setPixmap(
        printer_status_on_icon->pixmap(16, 16));
    printer_status_label->setText("Printer ON ");
  } else {
    printer_status_icon_label->setPixmap(
        printer_status_off_icon->pixmap(16, 16));
    printer_status_label->setText("Printer OFF");
  };

  ui->statusbar->addPermanentWidget(printer_status_widget);

  demo_mode_widget = new QWidget(this);
  demo_mode_widget->setLayout(new QHBoxLayout());
  demo_mode_on_off = new QLabel(demo_mode_widget);
  demo_mode_widget->layout()->addWidget(demo_mode_on_off);

  demo_mode_on_off->setToolTip(
      "In demo mode a single receipt will be printed normally, then diagonal "
      "stripes with the word DEMO will "
      "be printed on it");
  setDemoMode();

  ui->statusbar->addPermanentWidget(demo_mode_widget);

  ui->printer_driver_combo->setModel(new PrinterDriverListModel(this));
  ui->printer_driver_combo->setCurrentIndex(
      GlobalState::getCurrentPrinterIndex());
  updatePrintConfigWidget();

  auto t = new QTimer(this); // Printer queue polling timer
  t->setInterval(100);

  connect(t, SIGNAL(timeout()), this, SLOT(refreshTimer()));

  if (program_settings.contains("start_in_tray")) {
    bool tray = program_settings.value("start_in_tray").toBool();
    if (tray)
      hide();
    else
      show();
    ui->start_in_tray->setCheckState(tray ? Qt::CheckState::Checked
                                          : Qt::CheckState::Unchecked);
  } else
    show();

  connect(ui->restart_network_button, SIGNAL(clicked()), this,
          SLOT(restartNetworkThread()));
  //    connect(ui->stop_network_button, SIGNAL(clicked()), this,
  //    SLOT(stopNetworkThread()));
  connect(ui->printer_driver_combo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updatePrinterDriver(int)));
  connect(ui->http_port_spinbox, SIGNAL(valueChanged(int)), this,
          SLOT(setHttpProxyPort(int)));
  connect(ui->start_in_tray, SIGNAL(stateChanged(int)), this,
          SLOT(setStartInTray(int)));
  connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
          SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
  connect(ui->show_preview_button, SIGNAL(toggled(bool)), this,
          SLOT(toggleDisplayPreview(bool)));

  ui->receipt_preview_view->setScene(&receipt_preview_scene);

  receipt_preview_scene.addPixmap(*receipt_preview_pixmap);

  //  receipt_preview_scene.setSceneRect(receipt_preview_pixmap->rect());

  QFile receipt_placeholder_f = QFile(":/images/images/receipt.jpg");
  receipt_placeholder_f.open(QFile::OpenModeFlag::ReadOnly);

  auto receipt_placeholder_binary_data =
      receipt_placeholder_f.readAll().toStdString();
  GlobalState::setLastReceiptImage(receipt_placeholder_binary_data);

  updateReceiptPreview();

  read_bool_from_settings("show_preview_window", showpreview);
  ui->show_preview_button->setChecked(showpreview);

  display_timer = new QTimer(this); // Dispaly preview window update timer

  display_timer->setInterval(300);
  display_timer->start();
  connect(display_timer, SIGNAL(timeout()), this, SLOT(updateReceiptPreview()));

  toggleDisplayPreview(showpreview);

  GlobalState::startNetworkThread();
  t->start();
  eventclient.subscribeToEvent("gamma_changed");
  EventSystem::instance().emitEvent(
      "qapplication_ready",
      EventData("qapplication_ready", eventclient.getID()));
}

void MainWindow::scheduleDiplayPreviewUpdate() {
  display_preview_update_schedule = true;
}

void MainWindow::onEvent(EventData d) {
  if (d.name() == "gamma_changed") {
    gammaUpdated(d);
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (closing) {
    GlobalState::stopNetworkThread();
    close();
    event->accept();
  } else {
    hide();
    event->ignore();
  }
}

void MainWindow::toggleDisplayPreview(bool checked) {
  showpreview = checked;
  save_bool_to_settings("show_preview_window", showpreview);
  if (!checked)
    ui->receipt_preview_view->hide();
  else {
    ui->receipt_preview_view->show();
    scheduleDiplayPreviewUpdate();
  }

//  ui->tabWidget->adjustSize();
//  ui->centralwidget->adjustSize();
  //    setFixedSize(geometry().width(), geometry().height());
  //    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Preferred,
  //    QSizePolicy::Policy::Preferred));
//  adjustSize();
}

MainWindow::~MainWindow() { delete ui; }

#include "mainwindow.h"

#include <iostream>

#include "./ui_mainwindow.h"
#include "manager_draw.h"

main_window::main_window(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::main_window)
{
  ui->setupUi(this);

  connect(
    this, &main_window::notify_progress, this, &main_window::change_progress);
  connect(this, &main_window::img_rendered, this, &main_window::draw_img);

  scene_ptr = std::make_unique<QGraphicsScene>(ui->gv_canvas);
  ui->gv_canvas->setScene(scene_ptr.get());
  ui->gv_canvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->gv_canvas->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void
main_window::on_pb_draw_clicked()
{
  pd_rend_ptr = std::make_unique<QProgressDialog>("Rendering", "Stop", 0, 100);
  pd_rend_ptr->setMinimumDuration(0);
  pd_rend_ptr->show();

  unsigned ray_pp = 50;

  if (ui->rb_q_p->isChecked()) {
    ray_pp = 300;
  } else if (ui->rb_q_b->isChecked()) {
    ray_pp = 100;
  } else if (ui->rb_q_q->isChecked()) {
    ray_pp = 10;
  } else {
  }

  render_settings rs{ static_cast<unsigned int>(ui->gv_canvas->width()),
                      static_cast<unsigned int>(ui->gv_canvas->height()),
                      ray_pp };

  std::cout << "Canvas: " << rs.width_ << 'x' << rs.height_
            << "; ray_pp: " << rs.ray_pp_ << std::endl;

  manager_draw{}.draw(
    rs,
    [this](double progress) { emit notify_progress(progress); },
    // TODO: fix thread race ;(
    [this]() -> bool {
      return nullptr == pd_rend_ptr || pd_rend_ptr->wasCanceled();
    },
    [this](QImage img) { emit img_rendered(img); });
}

void
main_window::draw_img(QImage image)
{
  scene_ptr->clear();
  scene_ptr->addPixmap(QPixmap::fromImage(image));
  pd_rend_ptr->close();
  pd_rend_ptr.reset();
}

void
main_window::change_progress(double progress)
{
  if (pd_rend_ptr)
    pd_rend_ptr->setValue(progress);
}

void
main_window::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
  ui->statusbar->showMessage(QString("Canvas: %1x%2")
                               .arg(ui->gv_canvas->width())
                               .arg(ui->gv_canvas->height()));
}

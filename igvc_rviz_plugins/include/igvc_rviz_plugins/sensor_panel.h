#ifndef SENSORPANEL_H
#define SENSORPANEL_H

#include <ros/ros.h>
#include <rviz/panel.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/PointCloud2.h>
#include <QLabel>
#include <QObject>
#include <QTimer>
#include <string>

#define INTERVAL 250//in milliseconds

typedef struct LabelManager{
  QTimer *sensor_timer;
  std::map<std::string, QLabel *> labels;// map of pointers to labels
  bool *isActive; // list of activity for corresponding sensors in map
} LabelManager;

/*
 * All of our panels need to be under the igvc_rviz_plugins namespace.
 */
namespace igvc_rviz_plugins
{
/*
 * Each panel is a subclass of rviz::Panel
 */
class sensor_panel : public rviz::Panel
{
  /*
   * rviz is based on QT, so our panels need to be QObjects, which requires this macro keyword.
   */
  Q_OBJECT
public:
  /**
   * This is a standard QWidget constructor.
   * @param parent The parent widget, which will be responsible for the lifetime of this widget.
   */
  sensor_panel(QWidget *parent = 0);

public slots:
  void timer();

protected:
  /*
   * Subscribers for each sensor. 
   */
  ros::Subscriber imu_sub;
  ros::Subscriber lidar_sub;
  ros::Subscriber gps_sub;
  ros::Subscriber cam_center_sub;

  LabelManager lm;

  /**
   * Callbacks for each sensor.
   */
  void imu_callback(const sensor_msgs::ImuConstPtr &msg, QLabel *label);
  void lidar_callback(const sensor_msgs::PointCloud2ConstPtr &msg, QLabel *label);
  void cam_center_callback(const sensor_msgs::ImageConstPtr &msg, QLabel *label);
  void gps_callback(const sensor_msgs::NavSatFixConstPtr &msg, QLabel *label);


private:
  void reset_labels();
};
}

#endif  // SENSORPANEL_H

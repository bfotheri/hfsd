//============================================================================
// Name        : cloudTransformer.cpp
// Author      : Ryan Fite - ryanfite@live.com
// Version     : 1.0
// Date Created: Jul 27, 2018
// Description : 
//============================================================================




#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2/buffer_core.h>
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Twist.h>
#include "sensor_msgs/PointCloud2.h"
#include "pcl_ros/point_cloud.h"
#include "pcl/point_cloud.h"
#include <pcl/point_types.h>
#include "Eigen/Geometry"
#include "pcl/common/transforms.h"
#include "tf2_eigen/tf2_eigen.h"
#include <ros/callback_queue.h>
using namespace std;
string body;
ros::Publisher pubPoints;
geometry_msgs::TransformStamped transformStamped;
void messageReceivedCloud(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& msg){
	pcl::PointCloud<pcl::PointXYZ>::Ptr temp(new pcl::PointCloud<pcl::PointXYZ>(*msg));
	Eigen::Affine3d affine = tf2::transformToEigen(transformStamped);
	pcl::transformPointCloud(*temp,*temp,affine);
	temp->header.frame_id = body;
	pubPoints.publish(*temp);
}

int main(int argc, char** argv){
  ros::init(argc, argv, "my_tf2_listener");
  std::string topicPoints;
	if(ros::param::get("CloudInput", topicPoints)){
		ROS_INFO("CLOUD INPUT SET CORRECTLY");
	}else{
		ROS_INFO("ERROR: CLOUD INPUT SET INCORRECTLY. SETTING TO DEFAULT");
		topicPoints = "stereo_vision";
	}
  body = argv[1];
  string state = argv[2];

  ros::NodeHandle node;
  ros::Subscriber subPoints = node.subscribe<pcl::PointCloud<pcl::PointXYZ> >(topicPoints,10,&messageReceivedCloud);
  pubPoints = node.advertise<sensor_msgs::PointCloud2 >("points", 10);
  tf2_ros::Buffer tfBuffer;
  tf2_ros::TransformListener tfListener(tfBuffer);

  ros::Rate rate(10.0);
  while (node.ok()){

    try{
      transformStamped = tfBuffer.lookupTransform(body, state,
                               ros::Time(0));

    }
    catch (tf2::TransformException &ex) {
      ROS_WARN("%s",ex.what());
      ros::Duration(1.0).sleep();
      continue;
    }
    ros::getGlobalCallbackQueue()->callAvailable(ros::WallDuration(0));
    rate.sleep();
  }
  return 0;
};
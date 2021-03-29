/*
 * Copyright (C) 2020 Agribot. All rights reserved.
 */

#include <agribot_msgs/AgribotCaterpillarWheelsRotation.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/Float32.h>
#include <agribot_msgs/AgribotTool.h>

#include <string>

#define PI 3.14159265359
#define RAD_TO_DEG (180 / PI)
#define DEG_TO_RAD (PI / 180)
constexpr int joints_cnt = 12;

double dFrontCartAngle = 0.0, dBackCartAngle = 0.0;
double dFrontTplHeight = 0.0, dFrontTplAngle = 0.0;
double dBackTplHeight = 0.0, dBackTplAngle = 0.0;
agribot_msgs::AgribotCaterpillarWheelsRotation caterpilarWheels;
bool bDataReceived = false;

void encoderFrontCallback(const std_msgs::Float32 front_cart_msg)
{
  dFrontCartAngle = front_cart_msg.data;
  bDataReceived = true;
}

void encoderBackCallback(const std_msgs::Float32 back_cart_msg)
{
  dBackCartAngle = back_cart_msg.data;
  bDataReceived = true;
}

void tplFrontCallback(const agribot_msgs::AgribotTool &front_tpl_config)
{
  // Joints implemented in simulation have inverted limits:
  // - revolute joint from -60 to 15 deg
  // - prismatic joint from -0.45 to 0 m
  // To correspond to real values, the inversion of sign is
  // needed in below lines.
  dFrontTplAngle = -front_tpl_config.tpl_angle;
  dFrontTplHeight = -front_tpl_config.tpl_height;
}

void tplBackCallback(const agribot_msgs::AgribotTool &back_tpl_config)
{
  dBackTplAngle = -back_tpl_config.tpl_angle;
  dBackTplHeight = -back_tpl_config.tpl_height;
}

void caterpillarWheelsCallback(const agribot_msgs::AgribotCaterpillarWheelsRotation msg)
{
  caterpilarWheels = msg;
  bDataReceived = true;
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "state_publisher");
  ros::NodeHandle n;
  ros::Rate loop_rate(100);
  ros::Publisher joint_pub = n.advertise<sensor_msgs::JointState>("joint_states", 1);
  ros::Subscriber front_cart_sub = n.subscribe("/agribot/encoder/cart/front", 1000, encoderFrontCallback);
  ros::Subscriber back_cart_sub = n.subscribe("/agribot/encoder/cart/back", 1000, encoderBackCallback);
  ros::Subscriber front_tpl_sub = n.subscribe("agribot/control/tool/front/valves", 1, tplFrontCallback);
  ros::Subscriber back_tpl_sub = n.subscribe("agribot/control/tool/back/valves", 1, tplBackCallback);
  ros::Subscriber catterpilar_wheels_sub =
      n.subscribe("agribot/encoder/caterpillar/rotations", 1000, caterpillarWheelsCallback);
  sensor_msgs::JointState joint_state;

  while (ros::ok())
  {
    if (bDataReceived)
    {
      // update joint_state
      joint_state.header.stamp = ros::Time::now();
      joint_state.name.resize(joints_cnt);
      joint_state.position.resize(joints_cnt);
      joint_state.name[0] = "front_cart_joint";
      joint_state.position[0] = dFrontCartAngle;
      joint_state.name[1] = "back_cart_joint";
      joint_state.position[1] = dBackCartAngle;

      joint_state.name[2] = "front_left_wheel_joint";
      joint_state.position[2] = caterpilarWheels.front_left;
      joint_state.name[3] = "front_right_wheel_joint";
      joint_state.position[3] = caterpilarWheels.front_right;
      joint_state.name[4] = "back_left_wheel_joint";
      joint_state.position[4] = caterpilarWheels.back_left;
      joint_state.name[5] = "back_right_wheel_joint";
      joint_state.position[5] = caterpilarWheels.back_right;

      joint_state.name[6] = "front_tpl_lower_hitches_revolute_joint";
      joint_state.effort[6] = dFrontTplAngle;
      joint_state.name[7] = "front_tpl_upper_hitch_revolute_joint";
      joint_state.position[7] = 0;
      joint_state.name[8] = "front_tpl_upper_hitch_prismatic_joint";
      joint_state.effort[8] = dFrontTplHeight;
      joint_state.name[9] = "back_tpl_lower_hitches_revolute_joint";
      joint_state.effort[9] = dBackTplAngle;
      joint_state.name[10] = "back_tpl_upper_hitch_revolute_joint";
      joint_state.position[10] = 0;
      joint_state.name[11] = "back_tpl_upper_hitch_prismatic_joint";
      joint_state.effort[11] = dBackTplHeight;

      // send the joint state and transform
      joint_pub.publish(joint_state);
    }
    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}

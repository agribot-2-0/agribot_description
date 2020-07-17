/*
* Copyright (C) 2020 Agribot. All rights reserved.
*/

#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/Float32.h>

#include <string>

#define PI 3.14159265359
#define RAD_TO_DEG (180/PI)
#define DEG_TO_RAD (PI/180)

double dFrontCartAngle, dBackCartAngle = 0.0;
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

int main(int argc, char** argv)
{
    ros::init(argc, argv, "state_publisher");
    ros::NodeHandle n;
    ros::Rate loop_rate(100);
    ros::Publisher joint_pub =
                        n.advertise<sensor_msgs::JointState>("joint_states", 1);
    ros::Subscriber front_cart_sub = n.subscribe("/agribot/encoder/cart/front",
                                                 1000, encoderFrontCallback);
    ros::Subscriber back_cart_sub = n.subscribe("/agribot/encoder/cart/back",
                                                1000, encoderBackCallback);
    sensor_msgs::JointState joint_state;

    while (ros::ok())
    {
        if (bDataReceived)
        {
            // update joint_state
            joint_state.header.stamp = ros::Time::now();
            joint_state.name.resize(6);
            joint_state.position.resize(6);
            joint_state.name[0] ="front_cart_joint";
            joint_state.position[0] = dFrontCartAngle;
            joint_state.name[1] ="back_cart_joint";
            joint_state.position[1] = dBackCartAngle;

            joint_state.name[2] ="front_left_wheel_joint";
            joint_state.position[2] = 0;
            joint_state.name[3] ="front_right_wheel_joint";
            joint_state.position[3] = 0;
            joint_state.name[4] ="back_left_wheel_joint";
            joint_state.position[4] = 0;
            joint_state.name[5] ="back_right_wheel_joint";
            joint_state.position[5] = 0;
            // send the joint state and transform
            joint_pub.publish(joint_state);
        }
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}

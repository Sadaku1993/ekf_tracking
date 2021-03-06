
//
// include: cluster.h
//
// last update: '18.1.19
// author: matchey
//
// memo:
//   tracking対象のそれぞれのクラスタclass
//   ekfをもちいて計算(二次元で推定)
//   位置のみを使用(<-- 特徴量をいれる)
//

#ifndef CLUSTER_H
#define CLUSTER_H

#include <ros/ros.h>// ostream等
#include <geometry_msgs/Point.h>
#include <visualization_msgs/MarkerArray.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include "ekf_tracking/ekf.h"

class Cluster
{
	//feature <-- 今後増やしていくけるように.. 曲率とかサイズとか色とか(Trackerの方で判断)
	// int id;
	// geometry_msgs::Point position;//struct Point2Dでいまは十分だけど今後高さを特徴とできるように
	// double linear;
	// double angular;
	// double velocity;
	double likelihood;
	// bool flag_obs;

	ExtendedKalmanFilter ekf;
	Eigen::Vector5d x;	    // システム(系)の状態推定値(x, y, θ, v, ω) 5x1
	// Eigen::Vector2d u;	// 制御無し
	// Eigen::Vector2d G;  	// 時間遷移モデル(x'', y'') 2x1
	Eigen::Matrix5d P;  	// 誤差の共分散行列(推定値の精度) 5x5
	// Eigen::Matrix2d Q;  	// 共分散行列(時間変化) 2x2
	Eigen::Matrix5d R;		// 共分散行列(観測の信頼度) 3x3
	// Eigen::Matrix3d R;		// 共分散行列(観測の信頼度) 3x3
	// Eigen::Vector3d obs;	// 観測(x, y, θ) 3x1
	// Eigen::Vector3d obs;	// 観測(x, y, θ) 3x1
	Eigen::Vector5d obs;	// 観測(x, y, θ, v, ω) 5x1

	int lifetime;
	int age_; //トラックが最初に検出されてからのフレーム数
	int totalVisibleCount_; //トラックが検出されたフレームの合計数
	int consecutiveInvisibleCount_; //トラックが連続して検出されなかったフレームの数

	ros::Time current_time, last_time; // Trackerでdt出したほうが高速だけど

	void measurement(const pcl::PointXYZ&);
	void update();

	public:
	Cluster();
	Cluster(const pcl::PointXYZ&, const double&, const double&);
	void initialize(const geometry_msgs::Point&);
	void initialize(const pcl::PointXYZ&);
	void measurementUpdate(const pcl::PointXYZ&);
	void predict();
	void setParams();
	void setLifetime(const int);
	void setSigma(double);
	void setLikelihood(const double&);
	int age() const;
	int totalVisibleCount() const;
	int consecutiveInvisibleCount() const;
	int getLifetime();
	double getDist(const geometry_msgs::Point&) const;
	double getDist(const pcl::PointXYZ&) const;
	double getDist2ObsLast(const pcl::PointXYZ&) const;
	void getPoint(pcl::PointXYZ&);
	void getPoint(pcl::PointCloud<pcl::PointXYZ>::Ptr&);
	double getLikelihood();
	void getTrackingPoint(pcl::PointCloud<pcl::PointNormal>::Ptr&, const int);
	void getVelocityArrow(visualization_msgs::MarkerArray&, const int);
	void getErrorEllipse(visualization_msgs::MarkerArray&, const int);
	void getLinkLines(visualization_msgs::Marker&, const pcl::PointXYZ&);

	friend std::ostream& operator << (std::ostream&, const Cluster&);
};

#endif


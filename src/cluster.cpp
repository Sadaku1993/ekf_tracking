
//
// src: cluster.cpp
//
// last update: '18.1.19
// author: matchey
//
// memo:
//   tracking対象のそれぞれのクラスタclass
//

#include <tf/transform_broadcaster.h>
#include <Eigen/Dense> // for EigenSolver
#include "mmath/common/angles.h"
#include "mmath/binarion.h"
#include "mmath/pca.h"
#include "mmath/differential.h"
#include "ekf_tracking/cluster.h"

using namespace std;

Cluster::Cluster()
	: likelihood(1.0), lifetime(10), age_(0), totalVisibleCount_(0), consecutiveInvisibleCount_(0)
{
	last_time = ros::Time::now();
	current_time = ros::Time::now();

	P << 0.05,    0,   0,   0,   0,
	        0, 0.05,   0,   0,   0,
	        0,    0, 1.0,   0,   0,
	        0,    0,   0, 1.0,   0,
	        0,    0,   0,   0, 1.0;

	// R << 0.05,    0,    0,
	//         0, 0.05,    0,
	// 		0,    0, 0.05;
}

Cluster::Cluster(const pcl::PointXYZ &p, const double &sig_p, const double &sig_r)
	: likelihood(1.0), lifetime(10), age_(0), totalVisibleCount_(0), consecutiveInvisibleCount_(0)
{
	current_time = ros::Time::now();
	last_time = ros::Time::now();

	x << p.x, p.y, 0.0, 0.0, 0.0;

	obs(0) = p.x;
	obs(1) = p.y;

	P << sig_p,     0,     0,     0,     0,
	         0, sig_p,     0,     0,     0,
	         0,     0, 100.0,     0,     0,
	         0,     0,     0, 100.0,     0,
	         0,     0,     0,     0, 100.0;

	R << sig_r,      0,     0,     0,     0,
	         0,  sig_r,     0,     0,     0,
		     0,      0, sig_r,     0,     0,
	         0,      0,     0, sig_r,     0,
	         0,      0,     0,     0, sig_r;
}

void Cluster::initialize(const geometry_msgs::Point &p)
{
	x << p.x, p.y, 0.0, 0.0, 0.0;

	P << 0.01,    0,   0,   0,   0,
	        0, 0.01,   0,   0,   0,
	        0,    0, 1.0,   0,   0,
	        0,    0,   0, 1.0,   0,
	        0,    0,   0,   0, 1.0;

	// R <<  0.1,    0,    0,
	//         0,  0.1,    0,
	// 		0,    0,  0.1;

	age_ = totalVisibleCount_ = consecutiveInvisibleCount_ = 0;
}

void Cluster::initialize(const pcl::PointXYZ &p)
{
	x << p.x, p.y, 0.0, 0.0, 0.0;

	P << 0.05,    0,   0,   0,   0,
	        0, 0.05,   0,   0,   0,
	        0,    0, 1.0,   0,   0,
	        0,    0,   0, 1.0,   0,
	        0,    0,   0,   0, 1.0;

	// R << 0.05,    0,    0,
	//         0, 0.05,    0,
	// 		0,    0, 0.05;

	age_ = totalVisibleCount_ = consecutiveInvisibleCount_ = 0;
}

void Cluster::measurement(const pcl::PointXYZ &p)
{
	// const int N = 20;
	// Eigen::Vector3d vec; // 移動の向き(vec --> θ)
	// double theta = 0.0;
	// double linear;
	// double angular;
	// double curvature;
	// static Differential v(p.x, p.y); // クラスごとに共有されてしまう
	// static Differential w(theta, true);

	// PrincipalComponentAnalysis pca; // staticだめ
    //
	// pca.setPoints2d(p, N);
	// vec = pca.vector(0);
	// curvature = pca.curvature();
	// theta = atan2(vec(1), vec(0));
    //
	// double div = Binarion::deviation(theta, pca.direction());

	// cout << "obs theta : " << theta*180/M_PI << endl;
	// cout << "direction : " << pca.direction()*180/M_PI << endl;
	// cout << "div : " << div * 180 / M_PI << endl;
	
	// if(div > M_PI / 2){
	// 	theta += M_PI;
	// }else if(div < - M_PI / 2){
	// 	theta -= M_PI;
	// }

	// theta = pca.direction();

	// cout << "obs theta : " << theta*180/M_PI << ", curvature : " << curvature << endl;
	// cout << "direction : " << pca.direction()*180/M_PI << endl;
	
	// Eigen::Vector5d observe;
	// cout << "observe :\n" << observe << endl;

	// obs << p.x, p.y, theta;
	// linear = v.get(p.x, p.y);
	// angular = w.get(theta);

	// if(linear > 5) linear = 5;
	// linear = 1.0 - 2 * curvature;

	// obs << p.x, p.y, theta, linear, angular;
	// obs << p.x, p.y;

	// x(2) = theta;
	// x(3) = linear;

	// cout << "obs :\n" << obs << endl;

	// if(curvature < 0.05){
	// 	R(2, 2) = R(0, 0) * 10;
	// 	R(4, 4) = R(0, 0) * 100;
	// }else if(curvature < 0.2){
	// 	R(2, 2) = R(0, 0) * 100;
	// 	R(4, 4) = R(0, 0) * 1000;
	// }else{
	// 	R(2, 2) = R(0, 0) * 1000;
	// 	R(4, 4) = R(0, 0) * 10000;
	// }
}

void Cluster::update()
{
	Eigen::Matrix5d I  = Eigen::Matrix5d::Identity();
	// Eigen::Matrix3d I3 = Eigen::Matrix3d::Identity();
	// Eigen::Matrix<double, 3, 5> H = ekf.jacobH(); //観測モデルのヤコビアン
	Eigen::Matrix5d H = I;
	// Eigen::Vector3d e = obs - ekf.h(x);
	Eigen::Vector5d e = ekf.deviation(x, obs); //観測残差、innovation
	// Eigen::Vector3d e = ekf.deviation(ekf.h(x), obs); //観測残差、innovation
	// e(2) = mmath::acuteAngle(e(2));
	Eigen::Matrix5d S = H * P * H.transpose() + R; // 観測残差の共分散
	// Eigen::Matrix3d S = H * P * H.transpose() + R; // 観測残差の共分散
	// Eigen::Matrix<double, 5, 2> K = P * H.transpose() * S.inverse(); // 最適 カルマンゲイン
	Eigen::Matrix5d K = P * H.transpose() * S.colPivHouseholderQr().solve(I);
	// Eigen::Matrix<double, 5, 3> K = P * H.transpose() * S.colPivHouseholderQr().solve(I3);

	x = x + K * e; // 更新された状態の推定値
	x = obs;
	P = (I - K * H) * P; // 更新された誤差の共分散

	cout << "K :\n" << K << endl;


	totalVisibleCount_++;
	consecutiveInvisibleCount_ = 0;

	cout << "updated :\n" << x << endl;
}

void Cluster::measurementUpdate(const pcl::PointXYZ &p)
{
	measurement(p);
	update();
}

void Cluster::predict()
{
	// cout << "theta 3: " << x(2)*180/M_PI << ", omega : " << x(4) << endl;

	// cout << "\np1 :\n" << P << endl;

	current_time = ros::Time::now();
	double dt = (current_time - last_time).toSec();
	last_time = current_time;

	// obs(3) = 
	// obs(4) = obs(4) / dt;

	Eigen::Matrix5d F = ekf.jacobF(x, dt); // 動作モデルのヤコビアン
	// x = ekf.f(x, dt); // 時間発展動作モデル
	P = F * P * F.transpose();
	// x(2) = mmath::normAngle(x(2));

	age_++;
	consecutiveInvisibleCount_++;

	cout << "predicted : \n" << x << endl;
}

void Cluster::setParams()
{
}

void Cluster::setLifetime(const int life)
{
	lifetime = life;
}

void Cluster::setLikelihood(const double &sigma)
{
	likelihood = sigma;
}

int Cluster::age() const
{
	return age_;
}

int Cluster::totalVisibleCount() const
{
	return totalVisibleCount_;
}

int Cluster::consecutiveInvisibleCount() const
{
	return consecutiveInvisibleCount_;
}

int Cluster::getLifetime()
{
	return lifetime;
}

double Cluster::getDist(const geometry_msgs::Point &point) const
{
	pcl::PointXYZ p;

	p.x = point.x;
	p.y = point.y;
	p.z = point.z;

	return getDist(p);
}

double Cluster::getDist(const pcl::PointXYZ &p) const
{
	return sqrt(pow(p.x - x(0), 2) + pow(p.y - x(1), 2));
	// return sqrt(pow(p.x - position.x, 2) + pow(p.y - position.y, 2) + pow(p.z - position.z, 2));
}

double Cluster::getDist2ObsLast(const pcl::PointXYZ &p) const
{
	return sqrt(pow(p.x - obs(0), 2) + pow(p.y - obs(1), 2));
}

void Cluster::getPoint(pcl::PointXYZ &p)
{
	p.x = x(0);
	p.y = x(1);
	p.z = 0.0;
}

void Cluster::getPoint(pcl::PointCloud<pcl::PointXYZ>::Ptr &pc)
{
	pcl::PointXYZ p;

	p.x = x(0);
	p.y = x(1);
	p.z = 0.0;

	pc->points.push_back(p);
}

double Cluster::getLikelihood()
{
	return likelihood;
}

void Cluster::getTrackingPoint(pcl::PointCloud<pcl::PointNormal>::Ptr &pc, const int id)
{
	// if(likelihood < 0.1){
		pcl::PointNormal pn;

		pn.x = x(0);
		pn.y = x(1);
		pn.z = 0.0;
		pn.curvature = id;

		pc->points.push_back(pn);
	// }
}

void Cluster::getVelocityArrow(visualization_msgs::MarkerArray &markers, const int id)
{
	visualization_msgs::Marker arrow;

	// marker.header.frame_id = "/map";
	arrow.header.stamp = ros::Time::now();
	arrow.header.frame_id = "/velodyne";

	arrow.ns = "/cluster/arrow";
	arrow.id = id;

	arrow.type = visualization_msgs::Marker::ARROW;
	arrow.action = visualization_msgs::Marker::ADD;
	// arrow.action = visualization_msgs::Marker::DELETE;
	// arrow.action = visualization_msgs::Marker::DELETEALL;

	arrow.pose.position.x = x(0);
	arrow.pose.position.y = x(1);
	arrow.pose.position.z = 0.0;
	geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(x(2));
	arrow.pose.orientation = odom_quat;

	// arrow.scale.x = 1.0; // length
	arrow.scale.x = x(3); // length
	arrow.scale.y = 0.1; // width
	arrow.scale.z = 0.1; // height

	arrow.color.r = 1.0f;
	arrow.color.g = 0.0f;
	arrow.color.b = 0.9f;
	arrow.color.a = 0.6;

	arrow.lifetime = ros::Duration(1.0);

	markers.markers.push_back(arrow);
}

void Cluster::getErrorEllipse(visualization_msgs::MarkerArray &markers, const int id)
{
	visualization_msgs::Marker ellipse;
	ellipse.header.stamp = ros::Time::now();
	ellipse.header.frame_id = "/velodyne";

	ellipse.ns = "/cluster/ellipse";
	ellipse.id = id;

	ellipse.type = visualization_msgs::Marker::CYLINDER;
	ellipse.action = visualization_msgs::Marker::ADD;
	// ellipse.action = visualization_msgs::Marker::DELETE;


	double a, b;
	geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(0.0);
	Eigen::Matrix2d m = P.block<2, 2>(0, 0);
	Eigen::EigenSolver<Eigen::Matrix2d> es(m);
	if(!es.info()){ // == "Success"
		Eigen::Vector2d values = es.eigenvalues().real();
		Eigen::Matrix2d vectors = es.eigenvectors().real();
		Eigen::Vector2d vec;
		double lambda1, lambda2, theta;
		theta = 0.0;
		double kai2 = 9.21034; // χ² (chi-square) distribution 99% (95%:5.99146)
		if(values(0) < values(1)){
			lambda1 = values(1);
			lambda2 = values(0);
			vec = vectors.col(1);
		}else{
			lambda1 = values(0);
			lambda2 = values(1);
			vec = vectors.col(0);
		}
		theta = atan2(vec(1), vec(0));
		odom_quat = tf::createQuaternionMsgFromYaw(theta);
		a = sqrt(kai2 * lambda1);
		b = sqrt(kai2 * lambda2);
		if(a*b < 1e-5){
			likelihood = 1e6;
		}else{
			likelihood = 10 / (a * b);
		}
		ellipse.pose.position.x = x(0);
		ellipse.pose.position.y = x(1);
		ellipse.pose.position.z = 0.0;
		ellipse.pose.orientation = odom_quat;

		ellipse.scale.x = a;
		ellipse.scale.y = b;
		ellipse.scale.z = 0.01;

		ellipse.color.r = 0.0f;
		ellipse.color.g = 1.0f;
		ellipse.color.b = 1.0f;
		ellipse.color.a = 0.3;

		ellipse.lifetime = ros::Duration(1.0);

		markers.markers.push_back(ellipse);
	}else{
		cerr << "Eigen solver error info : " << es.info() << endl;
	}
}

void Cluster::getLinkLines(visualization_msgs::Marker& link, const pcl::PointXYZ& tgt)
{
	geometry_msgs::Point p;

	p.x = obs(0);
	p.y = obs(1);
	// p.x = x(0);
	// p.y = x(1);
	link.points.push_back(p);

	p.x = tgt.x + 0.5;
	p.y = tgt.y + 0.5;
	link.points.push_back(p);
}

ostream& operator << (ostream &os, const Cluster &cluster)
{
	os << "    position : (" << cluster.x(0) << ", " << cluster.x(1) << ")\n"
	   << "    likelihood : " << cluster.likelihood << ", " 
	   << "    age : " << cluster.age_ << ", " 
	   << "    totalVisibleCount : " << cluster.totalVisibleCount_ << ", " 
	   << "    consecutiveInvisibleCount : " << cluster.consecutiveInvisibleCount_ << endl;

	return os;
}


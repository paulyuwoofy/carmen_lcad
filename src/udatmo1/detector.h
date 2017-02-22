#ifndef DATOMIC_DETECTOR_H
#define DATOMIC_DETECTOR_H

#include "obstacle.h"
#include "sample_filter.h"
#include "udatmo_messages.h"

#include <carmen/carmen.h>
#include <carmen/obstacle_distance_mapper_interface.h>
#include <carmen/rddf_messages.h>

namespace udatmo
{

#define MOVING_OBJECT_HISTORY_SIZE 40

class Detector
{
	/** @brief System configuration settings. */
	carmen_robot_ackerman_config_t robot_config;

	/** @brief Current robot pose, speed and phi as estimated by the localization module. */
	carmen_ackerman_traj_point_t robot_pose;

	/** @brief Current map of distances between detected obstacles and plane coordinates. */
	carmen_obstacle_distance_mapper_message *current_map;

	/** @brief Latest RDDF information. */
	carmen_rddf_road_profile_message rddf;

	/** @brief Minimum number of hypothetical poses to consider ahead of the current one. */
	int min_poses_ahead;

	/** @brief Maximum number of hypothetical poses to consider ahead of the current one. */
	int max_poses_ahead;

	/** @brief History of the front moving obstacle. */
	Obstacle moving_object[MOVING_OBJECT_HISTORY_SIZE];

	/** @brief Message describing detected moving obstacles. */
	carmen_udatmo_moving_obstacles_message message;

	int compute_num_poses_ahead();

	/**
	 * @brief Update obstacle speed estimates across its history.
	 */
	void update_moving_object_velocity(carmen_ackerman_traj_point_t &robot_pose);

	inline bool set_detected(bool value)
	{
		detected = value;
		return detected;
	}

	int detect(int rddf_pose_index);

public:
	/** @brief Result of last detection operation. */
	bool detected;

	SampleFilter speed;

	/**
	 * @brief Create a new moving obstacle detector.
	 */
	Detector();

	/**
	 * @brief Perform moving obstacle detection in the front of the car.
	 */
	carmen_udatmo_moving_obstacles_message *detect();

	/**
	 * @brief Setup detector parameters.
	 */
	void setup(const carmen_robot_ackerman_config_t &robot_config, int min_poses_ahead, int max_poses_ahead);

	/**
	 * @brief Setup detector parameters through the CARMEN parameter server.
	 */
	void setup(int argc, char *argv[]);

	/**
	 * @brief Update the current global position.
	 */
	void update(const carmen_ackerman_traj_point_t &robot_pose);

	/**
	 * @brief Update the current distance map.
	 */
	void update(carmen_obstacle_distance_mapper_message *map);

	/**
	 * @brief Update the state of the RDDF encapsulated in this object.
	 */
	void update(carmen_rddf_road_profile_message *rddf);

	/**
	 * @brief Shift the obstacle history one position towards the back.
	 */
	void shift();

	double speed_front();

	carmen_ackerman_traj_point_t get_moving_obstacle_position(void);
	double get_moving_obstacle_distance(carmen_ackerman_traj_point_t *robot_pose);
};

/**
 * @brief Return a reference to a singleton detector instance.
 */
Detector &getDetector();

} // namespace udatmo

#endif

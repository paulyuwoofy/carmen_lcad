/*
 * route_planner_messages.h
 *
 *  Created on: 14/04/2020
 *      Author: Alberto
 */

#ifndef ROUTE_PLANNER_MESSAGES_H_
#define ROUTE_PLANNER_MESSAGES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>

typedef struct
{
	int number_of_poses;
	int number_of_poses_back;
	carmen_ackerman_traj_point_t *poses;
	carmen_ackerman_traj_point_t *poses_back;
	int *annotations;
	int *annotations_codes;

	int number_of_nearby_lanes;
	int *nearby_lanes_indexes;	// O ponto em nearby_lanes onde comecca cada lane.
	int *nearby_lanes_ids;		// Cada id eh um codigo que identifica uma lane unicamente.
	int nearby_lanes_size;		// Igual ao numero de poses de todas as lanes somado.
	carmen_ackerman_traj_point_t *nearby_lanes;	// Todas as lanes (number_of_nearby_lanes), uma apos a outra. A primeira lane eh sempre a rota e sempre deve ter id = 0, jah que ela eh uma composicao de lanes do grafo
	int *traffic_restrictions; 	// LEFT_MARKING | RIGHT_MARKING | LEVEL | YIELD | BIFURCATION
								//    enum            enum       2 bits

    //  Uma network com tres lanes com tamanhos 5, 3 e 6 poses teria:
    //  number_of_nearby_lanes = 3
    //	nearby_lanes_indexes -> 0, 5, 8
    //	nearby_lanes_size = 5+3+6 = 14
    //	nearby_lanes (p_lane_pose) -> p_0_0, p_0_1, p_0_2, p_0_3, p_0_4, p_1_0, p_1_1, p_1_2, p_2_0, p_2_1, p_2_2, p_2_3, p_2_4, p_2_5

    double timestamp;
    char *host;
} carmen_route_planner_road_network_message;

#define		CARMEN_ROUTE_PLANNER_ROAD_NETWORK_MESSAGE_NAME		"carmen_route_planner_road_network_message"
#define		CARMEN_ROUTE_PLANNER_ROAD_NETWORK_MESSAGE_FMT		"{int, int, <{double, double, double, double, double}:1>, <{double, double, double, double, double}:2>, <int:1>, <int:1>, int, <int:7>, <int:7>, int, <{double, double, double, double, double}:10>, <int:10>, double, string}"


#ifdef __cplusplus
}
#endif

#endif /* ROUTE_PLANNER_MESSAGES_H_ */

//#include <carmen/carmen.h>
//#include <carmen/velodyne_interface.h>
//#include <carmen/velodyne_messages.h>
//#include "rsdriver.h"
//
//static carmen_velodyne_variable_scan_message variable_scan;
//using namespace rslidar_driver;
//
//int velodyne_max_laser_shots_per_revolution = 100000;
//rslidar_param private_nh;
//rslidar_driver::rslidarDriver* robosense = NULL;

int main()
{
//	private_nh.cut_angle = -0.01;
//	private_nh.device_ip = "192.168.1.200";
//	private_nh.difop_udp_port = 7788;
//	private_nh.model = "RS16";
//	private_nh.msop_port = 6699;
//	private_nh.rpm = 1200; //pegar na conf
//	private_nh.npackets = 38; // calcula dentro da mensagem e preenche se precisar depois
//
//	variable_scan.host = carmen_get_host();

//	while (true)
//	{
//		if (robosense == NULL)
//		{
//			robosense = new rslidar_driver::rslidarDriver(variable_scan, 16,
//					velodyne_max_laser_shots_per_revolution, 6699, 7788, private_nh);
//		}
//		else
//		{
//			if(robosense->poll(variable_scan, 16,
//					velodyne_max_laser_shots_per_revolution, 6699, 7788, private_nh))
//			{
//				carmen_velodyne_publish_variable_scan_message(&variable_scan, 2);
//			}
//			else
//				break;
//		}
//	}
		return 0;
}

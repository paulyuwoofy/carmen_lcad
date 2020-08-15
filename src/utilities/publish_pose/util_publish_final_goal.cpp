#include <carmen/carmen.h>
#include <carmen/rddf_interface.h>


void
publish_final_goal(carmen_point_t pose)
{
	carmen_rddf_publish_end_point_message(50, pose);
}


void
define_messages()
{
	IPC_RETURN_TYPE err;

    err = IPC_defineMsg(CARMEN_RDDF_END_POINT_MESSAGE_NAME, IPC_VARIABLE_LENGTH, CARMEN_RDDF_END_POINT_MESSAGE_FMT);
    carmen_test_ipc_exit(err, "Could not define", CARMEN_RDDF_END_POINT_MESSAGE_NAME);
}


int
main(int argc, char **argv)
{
	carmen_point_t pose;
	int time = 4;

	if (argc < 4)
	{
		printf("Use %s <x> <y> <theta> <OPTIONAL wait_time in seconds>\n "
				"Time to wait before publishing the final goal\n", argv[0]);
		exit(-1);
	}
	if (argc == 5)
		time = atoi(argv[4]);

	pose.x = atof(argv[1]);
	pose.y = atof(argv[2]);
	pose.theta = atof(argv[3]);

	carmen_ipc_initialize(argc, argv);
	define_messages();

	sleep(time);
	publish_final_goal(pose);

	while (1)
		sleep(10); // Para não morrer nunca e não gastar CPU

	return 0;
}


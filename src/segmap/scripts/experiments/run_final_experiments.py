
import os
from multiprocessing import Pool
from experiments import *

USE_LANE_SEGMENTATION = False


def run_experiment(pair):
    global USE_LANE_SEGMENTATIONM, USE_CAMERA_LATENCY
    
    m = pair[0]
    t = pair[1]
    
    mapper_args = ""
    localization_args = "--n_particles 200 --gps_xy_std 2.5 --gps_h_std 20 --color_red_std 1 --color_green_std 1 --color_blue_std 1"
    
    if USE_LANE_SEGMENTATION and ("aeroporto" in t or "aeroporto" in m):
        map_path = "/tmp/semantic_with_lane_map_%s" % (m)
        localization_args += " --segment_lane_marks 1 "
        mapper_args += " --segment_lane_marks 1 "
        output = "localization_semantic_with_lane_%s.txt" % (t)
    else:
        map_path = "/tmp/semantic_map_%s" % (m)
        output = "localization_semantic_%s.txt" % (t)
    
    if ("aeroporto" in t or "aeroporto" in m):
        localization_args += " --camera_latency 0.42 "
        mapper_args += " --camera_latency 0.42 "
    else:
        localization_args += " --camera_latency 0.0 "
        mapper_args += " --camera_latency 0.0 "
    
    mapper_cmd = "time ./mapper /dados/%s --v_thresh 1 -i semantic -m  %s %s" % (m, map_path, mapper_args)
    localization_cmd = "time ./localizer /dados/%s -m %s -i semantic %s > %s" % (t, map_path, localization_args, output)

    run_command(mapper_cmd)
    run_command(localization_cmd)    


if __name__ == "__main__":
    global experiments
    pairs = []
    for e in experiments:
        for t in e['test']:
            pairs.append([e['map'], t])
    
    process_pool = Pool(len(pairs)) 
    process_pool.map(run_experiment, pairs)
    print("Done.")
        

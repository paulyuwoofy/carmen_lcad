
import time
import os
from rl.util import dist, ackerman_motion_model, draw_rectangle
import numpy as np
import cv2
import carmen_comm.carmen_comm as carmen
import carmen_sim.pycarmen_sim as pycarmen_sim 
import panel.pycarmen_panel as pycarmen_panel

from scipy.interpolate import CubicSpline
#import matplotlib.pyplot as plt
#plt.ion()
#plt.show()


class SimpleEnv:
    def __init__(self, params):
        self.params = params

        if self.params['model'] == 'ackerman':
            self.env_size = 100.0
            self.zoom = 3.5
            self.wheel_angle = np.deg2rad(28.)
            self.goal_radius = 2.0
            self.dt = 0.1
            self.max_speed_forward = 10.
            self.max_speed_backward = -10.
            self.panel = pycarmen_panel.CarPanel()
            if self.params['use_acceleration']:
                self.max_acceleration_v = 1.0  # m/s^2
                self.max_velocity_phi = np.deg2rad(2.)  # rad/s 
        else:
            self.env_size = 9.0
            self.zoom = 20.0
            self.goal_radius = 0.5
            self.dt = 0.5

        # 100 rays to support convolutional laser preprocessing
        n_rays = 1
        self.laser = np.zeros(n_rays).reshape(n_rays, 1)

    def reset(self):
        self.pose = self.previous_p = np.zeros(5)

        self.env_border = int(self.env_size - 0.1 * self.env_size)
        
        self.goal = np.array(list((np.random.random(2) * 2.0 - 1.0) * self.env_border) + [0., 0.])
        
        # self.goal = [self.env_border + 5., self.env_border + 5.]
        
        # self.goal = np.random.randn(2) * 0.3 * self.env_border
        self.goal = np.clip(self.goal, -self.env_border, self.env_border)
        
        self.goal = list(self.goal) + [0., 0., 0.]
        
        self.obstacles = []
        self.n_steps = 0

        if self.params['use_acceleration'] and self.params['model'] == 'ackerman':
            self.v = 0
            self.phi = 0

        return {'pose': np.copy(self.pose), 'laser': self.laser}, self.goal

    def step(self, cmd):
        self.n_steps += 1
        self.previous_p = np.copy(self.pose)

        if self.params['model'] == 'ackerman':
            if self.params['use_acceleration']:
                self.v += cmd[0] * self.max_acceleration_v * self.dt
                self.phi += cmd[1] * self.max_velocity_phi * self.dt
                
                self.v = np.clip(self.v, a_min=self.max_speed_backward, a_max=self.max_speed_forward)
                self.phi = np.clip(self.phi, a_min=-np.deg2rad(28.), a_max=np.deg2rad(28.))
                
                v = self.v
                phi = self.phi
            else:
                if cmd[0] > 0: v = cmd[0] * self.max_speed_forward
                else: v = cmd[0] * np.abs(self.max_speed_backward)
                phi = cmd[1] * self.wheel_angle
            
            self.pose = ackerman_motion_model(self.pose, v, phi, dt=self.dt)
            self.pose[3] = v
            self.pose[4] = phi
        else:
            self.pose[0] += cmd[0] * self.dt
            self.pose[1] += cmd[1] * self.dt

        self.pose = np.clip(self.pose, -self.env_border, self.env_border)

        success = True if dist(self.pose, self.goal) < self.goal_radius else False
        starved = True if self.n_steps >= self.params['n_steps_episode'] else False

        info = {'success': success, 'hit_obstacle': False, 'starved': starved}
        done = success or starved

        return {'pose': np.copy(self.pose), 'laser': self.laser}, done, info

    def finalize(self):
        pass

    def line_params(self, line_vs):
        ms = [(v1[1] - v2[1]) / (v1[0] - v2[0]) for v1, v2 in line_vs]
        bs = [line_vs[i][0][1] - ms[i] * line_vs[i][0][0] for i in range(len(ms))]

        return list(zip(ms, bs))

    def view(self):
        goal = self.goal
        ob = self.pose

        img_size = int(self.zoom * self.env_size * 2)
        img = np.ones((img_size, img_size, 3))

        p1 = (int(img.shape[1]/2), 0)
        p2 = (int(img.shape[1]/2), img.shape[0])
        cv2.line(img, p1, p2, (1, 0, 0))

        p1 = (0, int(img.shape[0]/2))
        p2 = (img.shape[1], int(img.shape[0]/2))
        cv2.line(img, p1, p2, (1, 0, 0))

        g_px = int(goal[0] * self.zoom + img.shape[1] / 2)
        g_py = int(goal[1] * self.zoom + img.shape[0] / 2)

        img = cv2.circle(img, (g_px, g_py), int(self.goal_radius * self.zoom), (0, 1, 0), -1)

        for o in self.obstacles:
            img = o.draw(img, self.zoom)

        x = int(ob[0] * self.zoom + img.shape[1] / 2)
        y = int(ob[1] * self.zoom + img.shape[0] / 2)

        cv2.circle(img, (x, y), 2, (0, 0, 1), 0)

        x1 = int(self.previous_p[0] * self.zoom + img.shape[1] / 2)
        y1 = int(self.previous_p[1] * self.zoom + img.shape[0] / 2)

        cv2.circle(img, (x1, y1), 2, (0, 0, 1), 0)
        cv2.line(img, (x, y), (x1, y1), (0, 0, 1 ), 1)

        if self.params['model'] == 'ackerman':
            draw_rectangle(img, self.pose, 1.5, 5.0, self.zoom)
            draw_rectangle(img, self.previous_p, 1.5, 5.0, self.zoom)
            self.panel.draw(self.pose[3], self.pose[4], self.n_steps * self.dt)

        cv2.imshow('img', img)
        cv2.waitKey(1)


class CarmenEnv:
    def __init__(self, params):
        self.params = params
        carmen_path = os.environ['CARMEN_HOME']
        rddf_path = carmen_path + '/data/rndf/' + params['rddf']
        self.rddf = [[float(field) for field in line.rstrip().rsplit(' ')] for line in open(rddf_path, 'r').readlines()]
        print('Connecting to carmen')
        carmen.init()

    def _read_state(self):
        carmen.handle_messages()
        laser = carmen.read_laser()

        state = {
            'pose': np.copy(carmen.read_truepos()),
            'laser': np.copy(laser).reshape(len(laser), 1),
        }

        return state

    def rear_laser_is_active(self):
        return carmen.config_rear_laser_is_active()

    def reset(self):
        carmen.publish_stop_command()

        max_pose_shift = 30
        min_pose_shift = 10

        init_pos_id = np.random.randint(max_pose_shift, len(self.rddf) - (max_pose_shift + 1))
        init_pos = self.rddf[init_pos_id]

        carmen.reset_initial_pose(init_pos[0], init_pos[1], init_pos[2])

        if self.params['allow_negative_commands']: forw_or_back = np.random.randint(2) * 2 - 1
        else: forw_or_back = 1

        goal_id = init_pos_id + np.random.randint(min_pose_shift, max_pose_shift) * forw_or_back
        goal = self.rddf[goal_id]
        goal = goal[:4]
        self.goal = goal

        carmen.publish_goal_list([goal[0]], [goal[1]], [goal[2]], [goal[3]], [0.0], time.time())
        self.n_steps = 0

        return self._read_state(), goal

    def step(self, cmd):
        carmen.publish_goal_list([self.goal[0]], [self.goal[1]], [self.goal[2]], [self.goal[3]], [0.0], time.time())

        v = cmd[0] * 10.0
        phi = cmd[1] * np.deg2rad(28.)
        # v = 10.
        # phi = cmd[0] * np.deg2rad(28.0)

        carmen.publish_command([v] * 10, [phi] * 10, [0.1] * 10, True)

        state = self._read_state()

        achieved_goal = dist(state['pose'], self.goal) < self.params['goal_achievement_dist']
        vel_is_correct = np.abs(state['pose'][3] - self.goal[3]) < self.params['vel_achievement_dist']

        hit_obstacle = carmen.hit_obstacle()
        starved = self.n_steps >= self.params['n_steps_episode']
        success = achieved_goal  # and vel_is_correct

        done = success or hit_obstacle or starved
        info = {'success': success, 'hit_obstacle': hit_obstacle, 'starved': starved}

        self.n_steps += 1

        """
        if hit_obstacle:
            print('\n\n** HIT OBSTACLE\n\nLaser [size: ' + str(len(state['laser'])) + ']:\n')
            print(state['laser'])
            print('\n\n')
        """

        if done:
            carmen.publish_stop_command()

        return state, done, info

    def finalize(self):
        carmen.publish_stop_command()

    def view(self):
        pass


class CarmenSimEnv:
    def __init__(self, params):
        self.sim = pycarmen_sim.CarmenSim(params['fix_initial_position'],
                                          True, params['allow_negative_commands'], not params['train'],
                                          params['use_latency'])
        self.params = params
        
        # add as a parameter
        self.sim_dt = 0.1  
        self.max_speed_forward = 10.
        self.max_speed_backward = -10. if params['allow_negative_commands'] else 0.
        self.phi_update_rate = 0.1
        self.v_update_rate = 0.1

        if self.params['use_acceleration']:
            self.max_acceleration_v = 1.0  # m/s^2
            self.max_velocity_phi = np.deg2rad(5.)  # rad/s 

        if params['view']:
            self.panel = pycarmen_panel.CarPanel()

    def _state(self):
        laser = self.sim.laser()

        state = {
            'pose': np.copy(self.sim.pose()),
            'laser': np.copy(laser).reshape(len(laser), 1),
        }

        return state

    def rear_laser_is_active(self):
        return True

    def reset(self):
        self.sim.reset()
        self.n_steps = 0
        self.sim_t = 0
        
        self.v = 0
        self.phi = 0
        
        return self._state(), self.sim.goal()

    def step(self, cmd):
        if self.params['use_spline']:
            state = self._state()
        
            max_phi = np.deg2rad(28)
            curr_phi = state['pose'][4] / max_phi
            curr_v = state['pose'][3] / self.max_speed_forward
            
            """
            ts = np.arange(start=0., stop=5.5, step=5./3.)
            phis = [curr_phi, cmd[0], cmd[1], cmd[2]]
            
            spl = CubicSpline(ts, phis)
            phi = spl(0.15)
            phi = np.clip(phi, -1., 1.)
            
            ts_plot = np.arange(start=0., stop=5.1, step=0.1)
            phis_plot = np.clip(spl(ts_plot))
            
            """
            ts = ts_plot = [0., 1.66]

            v = (cmd[0] - curr_v) * (0.15 / 1.66) + curr_v
            vs = vs_plot = [curr_v, cmd[0]]
            
            phi = (cmd[1] - curr_phi) * (0.15 / 1.66) + curr_phi
            phis = phis_plot = [curr_phi, cmd[1]]

            # uncomment to visualize the spline
            #"""
            plt.clf()
            plt.ylim(-1.2, 1.2)
            plt.plot(ts, phis, 'o')
            plt.plot(ts_plot, phis_plot, '-b')
            plt.plot(ts, vs, 'o')
            plt.plot(ts_plot, vs_plot, '-r')
            plt.draw()
            plt.pause(0.001)
            print("ts:", ts, "phis:", phis, "phi:", phi, "vs:", vs, "v:", v)
            #"""
            
            self.v = v * self.max_speed_forward
            self.phi = phi * np.deg2rad(28.)
        
        elif self.params['use_acceleration']:
            self.v += cmd[0] * self.max_acceleration_v * self.sim_dt
            self.phi += cmd[1] * self.max_velocity_phi * self.sim_dt
        
        else:
            if cmd[0] > 0: d_v = cmd[0] * self.max_speed_forward
            else: d_v = cmd[0] * np.abs(self.max_speed_backward)
            d_phi = cmd[1] * np.deg2rad(28.)

            self.v = self.v * (1. - self.v_update_rate) + d_v * self.v_update_rate
            self.phi = self.phi * (1. - self.phi_update_rate) + d_phi * self.phi_update_rate   

        self.v = np.clip(self.v, a_min=self.max_speed_backward, a_max=self.max_speed_forward)
        self.phi = np.clip(self.phi, a_min=-np.deg2rad(28.), a_max=np.deg2rad(28.))

        self.sim.step(self.v, self.phi, self.sim_dt)

        state = self._state()
        goal = self.sim.goal()

        achieved_goal = dist(state['pose'], goal) < self.params['goal_achievement_dist']
        vel_is_correct = np.abs(state['pose'][3] - goal[3]) < self.params['vel_achievement_dist']

        hit_obstacle = self.sim.hit_obstacle()
        starved = self.n_steps >= self.params['n_steps_episode'] and self.params['n_steps_episode'] > 0
        success = achieved_goal  # and vel_is_correct

        done = success or hit_obstacle or starved
        info = {'success': success, 'hit_obstacle': hit_obstacle, 'starved': starved}

        self.n_steps += 1
        self.sim_t += self.sim_dt

        return state, done, info

    def finalize(self):
        pass

    def view(self):
        p = self.sim.pose()
        g = self.sim.goal()
        
        self.sim.draw_occupancy_map()
        self.sim.draw_pose(p[0], p[1], p[2], 0, 0, 0)
        self.sim.draw_pose(g[0], g[1], g[2], 0, 200, 200)
        
        self.sim.show()
        self.panel.draw(p[3], p[4], self.sim_t)



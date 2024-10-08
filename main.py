import sys
import argparse

global log_dir
global args

if __name__ == "__main__":
    parser = argparse.ArgumentParser("Mahai - AI for playing mahjong")
    parser.add_argument("-l", "--lib_path", type=str, help="Directory where compiled pymahai shared library",
                        required=True)
    parser.add_argument("-c", "--command", choices=["train", "test", "plot", "play"], required=True)
    parser.add_argument("-p", "--log_path", type=str, help="Path where network data will be stored, or used from",
                        required=True)
    args = parser.parse_args()

    sys.path.insert(0, args.lib_path)
    log_dir = args.log_path

import pymahai
import os
import gymnasium as gym
from stable_baselines3.common.env_checker import check_env
import time
from stable_baselines3 import PPO
from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.callbacks import EvalCallback
from stable_baselines3.common.monitor import load_results
from stable_baselines3.common.evaluation import evaluate_policy
import matplotlib.pyplot as plt

from websockets.asyncio.server import serve
import asyncio

import numpy as np

REWARD_LOSS = -30
REWARD_DRAW = -1
REWARD_WIN = 30
REWARD_FREE_TILE_DIFF = 3
REWARD_HAN_DIFF = 10


class Mahjong(gym.Env):
    def __init__(self):
        """Define possible actions and observation data format"""
        self.game = pymahai.Game()
        self.game.reset()
        self.wins = 0
        self.losses = 0
        self.draws = 0
        self.latest_result = pymahai.GameResult.Nothing
        self.render_mode = "human"

        self.action_space = gym.spaces.Discrete(n=14)  # Fourteen possible tiles to be discarded
        self.observation_space = gym.spaces.Dict(spaces={
            "discard": gym.spaces.Box(low=0, high=255, shape=(87,), dtype=np.uint8),  # Largest possible discard pile
            "hand": gym.spaces.Box(low=0, high=255, shape=(14,), dtype=np.uint8)  # Tile values in hand
        })

    def _get_obs(self):
        return {
            "discard": self.game.discard_obs(),
            "hand": self.game.hand_obs()
        }

    def reset(self, seed=0):
        self.game.reset()
        return self._get_obs(), {}

    def step(self, action):
        s = self.game.step(action)
        self.latest_result = s.result
        done = s.result != pymahai.GameResult.Nothing
        reward = 0
        if s.result == pymahai.GameResult.Win:
            reward += REWARD_WIN
            self.wins += 1
        elif s.result == pymahai.GameResult.Loss:
            reward += REWARD_LOSS
            self.losses += 1
        elif s.result == pymahai.GameResult.Draw:
            reward += REWARD_DRAW
            self.draws += 1
        reward += s.free_tile_diff * REWARD_FREE_TILE_DIFF
        reward += s.han_diff * REWARD_HAN_DIFF
        return self._get_obs(), reward, done, False, {}

    def render(self):
        self.game.render()

        if self.latest_result != pymahai.GameResult.Nothing:
            print("END")
            print(pymahai.GameResult(self.latest_result).name)

    def record(self):
        print(F"{self.wins=}")
        print(F"{self.losses=}")
        print(F"{self.draws=}")


def learning_rate_schedule(progress_remaining):
    start_rate = 0.0001
    return start_rate * progress_remaining


PPO_model_args = {
    "learning_rate": learning_rate_schedule,
    "gamma": 0.99,
    # 0.99, discount factor for futurer rewards, between 0 (only immediate reward matters) and 1 (future reward equivalent to immediate),
    "verbose": 0,  # change to 1 to get more info on training steps
    # "seed": 137, #fixing the random seed
    "ent_coef": 0.0,  # 0, entropy coefficient, to encourage exploration
    "clip_range": 0.2  # 0.2, very roughly: probability of an action can not change by more than a factor 1+clip_range
}


def train():
    os.makedirs(log_dir, exist_ok=True)
    env = Mahjong()
    # wrap it
    env = Monitor(env, log_dir)

    # Callback, this built-in function will periodically evaluate the model and save the best version
    eval_callback = EvalCallback(env, best_model_save_path=F'{log_dir}/',
                                 log_path=F'{log_dir}/', eval_freq=5000,
                                 deterministic=False, render=False)
    check_env(env, warn=True)

    starttime = time.time()
    model = PPO('MultiInputPolicy', env, **PPO_model_args)
    # Load previous best model parameters, we start from that
    if os.path.exists(F"{log_dir}/best_model.zip"):
        model.set_parameters(F"{log_dir}/best_model.zip")
    max_total_step_num = 1e6
    model.learn(max_total_step_num, callback=eval_callback)
    dt = time.time() - starttime
    print("Calculation took %g hr %g min %g s" % (dt // 3600, (dt // 60) % 60, dt % 60))


def test():
    env = Mahjong()
    model = PPO('MultiInputPolicy', env, **PPO_model_args)
    model.set_parameters(F"{log_dir}/best_model.zip")
    # Evaluate the trained model
    mean_reward, std_reward = evaluate_policy(model, env, n_eval_episodes=500)
    print("Best model's reward: %3.3g +/- %3.3g" % (mean_reward, std_reward))
    env.record()

import threading

class OutputGrabber:
    def __enter__(self):
        self.stdout = sys.stdout
        self.stdout_fileno = self.stdout.fileno()
        self.stdout_save = os.dup(self.stdout_fileno)
        self.stdout_pipe = os.pipe()
        os.dup2(self.stdout_pipe[1], self.stdout_fileno)
        os.close(self.stdout_pipe[1])

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stdout.flush()
        self.captured_stdout = ''
        def drain_pipe():
            while True:
                data = os.read(self.stdout_pipe[0], 1024).decode()
                if not data:
                    break
                self.captured_stdout += data

        self.t = threading.Thread(target=drain_pipe)
        self.t.start()
        os.close(self.stdout_fileno)
        self.t.join()
        os.close(self.stdout_pipe[0])
        os.dup2(self.stdout_save, self.stdout_fileno)
        os.close(self.stdout_save)


async def display(ws):
    env = Mahjong()
    model = PPO('MultiInputPolicy', env, **PPO_model_args)
    model.set_parameters(F"{log_dir}/best_model.zip")
    vec_env = model.get_env()
    obs = vec_env.reset()

    while True:
        state = ""
        action, _ = model.predict(observation=obs, deterministic=False)
        og = OutputGrabber()
        with og:
            obs, _, done, _ = vec_env.step(action)
        if not done:
            with og:
                vec_env.render()
        state += og.captured_stdout
        if done:
            if env.latest_result != pymahai.GameResult.Win:
                vec_env.reset()
            else:
                state += "\nWIN\n"
                await ws.send(state.replace("\n", "<br>"))
                break
        await ws.send(state.replace("\n", "<br>"))
        await asyncio.sleep(0.05)


async def display_async():
    async with serve(display, "localhost", 5678):
        await asyncio.get_running_loop().create_future()  # run forever


def plot():
    train_step_log = load_results(log_dir)
    x = np.array(train_step_log["l"].cumsum())
    y = np.array(train_step_log["r"])

    plot_from_step = 0
    y = y[x >= plot_from_step];
    x = x[x >= plot_from_step]

    fig1, ax1 = plt.subplots(1, 1)
    fig1.set_size_inches(16, 9)

    max_points_to_plot = 20000
    index_to_plot = np.linspace(0, len(train_step_log) - 1,
                                np.clip(len(train_step_log), None, max_points_to_plot)).astype(int)
    plt.scatter(x[index_to_plot], y[index_to_plot], alpha=0.3, s=10)

    x_edges = np.linspace(x.min(), x.max(), num=30)
    xbins = (x_edges[:-1] + x_edges[1:]) / 2.0
    binnumber = np.digitize(x, x_edges) - 1
    reward50 = np.zeros_like(xbins);
    reward75 = np.zeros_like(xbins);
    reward25 = np.zeros_like(xbins);
    reward_mean = np.zeros_like(xbins)
    reward_max = np.zeros_like(xbins)
    for i in range(len(xbins)):
        ind = (binnumber == i)
        if (np.sum(ind) > 0):
            reward_mean[i] = np.mean(y[ind])
            reward50[i] = np.median(y[ind])
            reward25[i] = np.percentile(y[ind], 25)
            reward75[i] = np.percentile(y[ind], 75)
            reward_max[i] = np.max(y[ind])

    plt.plot(xbins, reward_max, c='g', lw=2, label="Best")
    plt.plot(xbins, reward_mean, c='r', lw=2, label="Mean")
    # plt.plot(xbins,reward50,c='k',lw=2, label="Median")
    # plt.plot(xbins,reward75,'--',c='k',lw=2, label="Interquartile range")
    # plt.plot(xbins,reward25,'--',c='k',lw=2)
    plt.xlim([0, x.max()])
    plt.xlabel('Timesteps')
    plt.ylabel('Reward')
    plt.legend(fontsize=16)
    plt.savefig("snake_rewards.png", dpi=150, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    match args.command:
        case "train":
            train()
        case "test":
            test()
        case "plot":
            plot()
        case "play":
            asyncio.run(display_async())

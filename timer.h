#pragma once

#include<functional>
class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	void restart()
	{
		pass_time = 0;
		shotted = false;
	}
	void set_wait_time(float value)
	{
		wait_time = value;

	}
	void set_one_shot(bool value)
	{
		one_shot = value;
	}

	void set_on_timeout(std::function<void()> _on_timeout)
	{
		this->on_timeout = _on_timeout;
	}

	void pause()
	{
		paused = true;
	}

	void resume()
	{
		paused = false;
	}

	void on_update(float value)
	{
		if (paused) return;

		pass_time += value;
		if (pass_time >= wait_time)
		{
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = true;
			if (can_shot && on_timeout)
			{
				on_timeout();
			}
			pass_time -= wait_time;
		}
	}

	float get_pass_time()
	{
		return pass_time;
	}

	float get_wait_time()
	{
		return wait_time;
	}

	float get_progress()
	{
		return pass_time / wait_time;
	}
private:
	float pass_time = 0;
	float wait_time = 0;
	bool paused = false;
	bool shotted = false;
	bool one_shot = false;
	std::function<void()> on_timeout;
};

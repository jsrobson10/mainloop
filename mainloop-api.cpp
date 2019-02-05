#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <vector>
#include <iostream>
#include <string.h>

struct ActionReg
{
	void (*reg)(int*);
	unsigned long millis;
	unsigned long millis_start;
	bool active = false;
	bool repeat;
	int *args;
};

struct Mainloop
{
	std::vector<ActionReg> reg_actions;
	unsigned long clock_start;
	unsigned long clock_realtime;
	unsigned long clock_ml;
	bool running = false;
	bool on_early_reg = false;
	bool on_late_reg = false;
	bool lagging = false;
	void (*on_early)();
	void (*on_late)();
};

std::vector<Mainloop> loops;

unsigned long getMillis()
{
	#ifdef _WIN32
	return clock();
	#endif

	#ifdef __linux__

	timeval t;
	gettimeofday(&t, NULL);
	return (t.tv_sec*1000 + t.tv_usec/1000.0)+0.5;

	#endif
}

void mainloopResetTiming(int id)
{
	// Get the start time
	loops[id].clock_start = getMillis()-1;
	loops[id].clock_realtime = getMillis()-1;
}

int mainloopGen(int id)
{
	// Is the id unset
	if(id == -1)
	{
		// Create a new loop
		loops.push_back(*new Mainloop);

		// Get the new id
		id = loops.size()-1;
	}

	else
	{
		// Is the forced id not in range
		if(id >= loops.size())
		{
			// Resize the vector
			loops.resize(id+1);
		}

		// Set the forced id
		loops[id] = *new Mainloop;
	}

	// Set to zero
	memset(&loops[id], 0, sizeof(Mainloop));

	// Reset the timing
	mainloopResetTiming(id);

	// Return the id
	return id;
}

void mainloopDoTick(int id)
{
	// Calculate the latency
	unsigned long clock_now = getMillis();
	loops[id].clock_ml += 1;

	// Late
	if((clock_now - loops[id].clock_start) > (loops[id].clock_ml + 1))
	{
		// Call the on_late handler
		if(loops[id].on_late_reg) loops[id].on_late();
		loops[id].lagging = true;

		// Is this really late (10 seconds late)
		if(clock_now - loops[id].clock_start > (loops[id].clock_ml + 10000))
		{
			// Reset the timer
			loops[id].clock_start = getMillis();
			loops[id].clock_realtime = getMillis();
			loops[id].clock_ml = 0;
		}
	}

	// Early
	else if((clock_now - loops[id].clock_start) < (loops[id].clock_ml + 1))
	{
		// Call the on_early handler
		if(loops[id].on_early_reg) loops[id].on_early();

		// Wait until its the next millisecond
		while((clock_now - loops[id].clock_start) < (loops[id].clock_ml + 1))
		{
			// Wait until the clock is in sync
			clock_now = getMillis();

			// Sleep windows
			#ifdef _WIN32
			Sleep(0.1);
			#endif

			// Sleep linux
			#ifdef __linux__
			usleep(100);
			#endif
		}
	}

	// Loop through all the registered actions
	for(int i=0;i<loops[id].reg_actions.size();i++)
	{
		// Is this function even active
		if(loops[id].reg_actions[i].active)
		{
			// Is it time for execution
			if(loops[id].reg_actions[i].millis == 1)
			{
				// Execute the registered function
				loops[id].reg_actions[i].reg(loops[id].reg_actions[i].args);

				// Is the function set to repeat
				if(loops[id].reg_actions[i].repeat)
				{
					// Reset the actions timer
					loops[id].reg_actions[i].millis = loops[id].reg_actions[i].millis_start;
				}

				else
				{
					// Erase the action
					loops[id].reg_actions.erase(loops[id].reg_actions.begin()+i);
				}
			}

			else
			{
				// Remove time from the registered action
				loops[id].reg_actions[i].millis -= 1;
			}
		}
	}

	// Calculate the latency
	loops[id].clock_realtime = getMillis();
}

void mainloopDoMainloop(int id)
{
	// Register this as true
	loops[id].running = true;

	// Reset the timing
	mainloopResetTiming(id);

	// Start the mainloop
	while(loops[id].running)
	{
		// Do a tick
		mainloopDoTick(id);
	}
}

void mainloopOnEarly(int id, void reg())
{
	// Setup the register
	loops[id].on_early = reg;
	loops[id].on_early_reg = true;
}

void mainloopOnLate(int id, void reg())
{
	// Setup the register
	loops[id].on_late = reg;
	loops[id].on_late_reg = true;
}

void mainloopRegAction(int id, void reg(int*), unsigned long millis, int *args, bool repeat)
{
	// Setup the registered function
	ActionReg r;
	r.reg = reg;
	r.millis = millis;
	r.millis_start = millis;
	r.args = args;
	r.active = true;
	r.repeat = repeat;

	// Add this to the list
	loops[id].reg_actions.push_back(r);
}

void mainloopStop(int id)
{
	// Stop the mainloop
	loops[id].running = false;
}

void mainloopFree(int id)
{
	// Stop the mainloop
	mainloopStop(id);

	// Free the mainloop
	//free(&loops[id]);
}

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

struct ActionReg
{
	void (*reg)(int*);
	unsigned long millis;
	unsigned long millis_start;
	bool active = false;
	bool repeat;
	int *args;
};

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

void mainloopInit()
{
	// Get the start time
	clock_start = getMillis();
	clock_realtime = getMillis();
}

void mainloopDoTick()
{
	// Calculate the latency
	unsigned long clock_now = getMillis();
	clock_ml += 1;

	// Late
	if((clock_now - clock_start) > (clock_ml + 1))
	{
		// Call the on_late handler
		if(on_late_reg) on_late();
		lagging = true;

		// Is this really late (10 seconds late)
		if(clock_now - clock_start > (clock_ml + 10000))
		{
			// Reset the timer
			clock_start = getMillis();
			clock_realtime = getMillis();
			clock_ml = 0;
		}
	}

	// Early
	else if((clock_now - clock_start) < (clock_ml + 1))
	{
		// Call the on_early handler
		if(on_early_reg) on_early();

		// Wait until its the next millisecond
		while((clock_now - clock_start) < (clock_ml + 1))
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
	for(int i=0;i<reg_actions.size();i++)
	{
		// Is this function even active
		if(reg_actions[i].active)
		{
			// Is it time for execution
			if(reg_actions[i].millis == 1)
			{
				// Execute the registered function
				reg_actions[i].reg(reg_actions[i].args);

				// Is the function set to repeat
				if(reg_actions[i].repeat)
				{
					// Reset the actions timer
					reg_actions[i].millis = reg_actions[i].millis_start;
				}

				else
				{
					// Erase the action
					reg_actions.erase(reg_actions.begin()+i);
				}
			}

			else
			{
				// Remove time from the registered action
				reg_actions[i].millis -= 1;
			}
		}
	}

	// Calculate the latency
	clock_realtime = getMillis();
}

void mainloopDoMainloop()
{
	// Register this as true
	running = true;

	// Start the mainloop
	while(running)
	{
		// Do a tick
		mainloopDoTick();
	}
}

void mainloopOnEarly(void reg())
{
	// Setup the register
	on_early = reg;
	on_early_reg = true;
}

void mainloopOnLate(void reg())
{
	// Setup the register
	on_late = reg;
	on_late_reg = true;
}

void mainloopRegAction(void reg(int*), unsigned long millis, int *args, bool repeat)
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
	reg_actions.push_back(r);
}

void mainloopStop()
{
	// Stop the mainloop
	running = false;
}

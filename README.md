# Mainloop API
A simple, lightweight, cross-platform, open source C++ mainloop library for scheduling tasks quickly.

# On Time
The api will make sure the code is executed when it should, even when there is lag in the system.
The code uses mainloopOnEarly() and mainloopOnLate() functions to tell you when there is free time
in the system and when a process is hogging process time.

# Fast
The API can schedule tasks instantly, and make them execute in milliseconds of accuracy.

# Cross-platform
The code has been designed to work on Linux and Windows. Haven't tested on Mac yet (because I
don't own a Mac).

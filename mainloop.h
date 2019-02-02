void mainloopInit();
void mainloopDoTick();
void mainloopDoMainloop();
void mainloopOnEarly(void reg());
void mainloopOnLate(void reg());
void mainloopRegAction(void reg(int*), unsigned long millis, int *args, bool repeat=false);
void mainloopStop();

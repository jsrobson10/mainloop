int mainloopGen();
void mainloopDoTick(int id);
void mainloopDoMainloop(int id);
void mainloopOnEarly(int id, void reg());
void mainloopOnLate(int id, void reg());
void mainloopRegAction(int id, void reg(int*), unsigned long millis, int *args, bool repeat=false);
void mainloopStop(int id);

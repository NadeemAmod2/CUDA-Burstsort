
enum TimerState {Stopped, Started};

class Timer
{
protected:
	wstring* identifier;
	TimerState state;
	clock_t starttime;
	clock_t endtime;
public:
	Timer();
	~Timer();
	void start();
	void start(wchar_t*);
	void stop();
};
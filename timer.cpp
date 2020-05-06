
#include "main.h"

Timer::Timer() 
{
	state = Stopped;
	identifier = 0;
}

Timer::~Timer()
{
	if (identifier)
		delete identifier;
}


void Timer::start(wchar_t* string)
{
	if (state == Stopped) {
		if (identifier)
			delete identifier;
		identifier = new wstring((wchar_t*) string);
		//*identifier = (wchar_t*) string;
		start();
	}
}

void Timer::start()
{
	if (state == Stopped) {
		starttime = clock();
		state = Started;
	}
}

void Timer::stop()
{
	if (state == Started) {
		state = Stopped;
		endtime = clock();
		if (identifier)
			wcout << *identifier << L" took " << endtime - starttime << L" clock ticks" << endl;
		else
			wcout << L"Timer took " << endtime - starttime << L" to run" << endl;
	}
}
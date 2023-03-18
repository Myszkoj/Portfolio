#pragma once


#include <dpl_Timer.h>
#include <dpl_ReadOnly.h>


namespace complex
{
	class TimeManager
	{
	public: // data
		dpl::ReadOnly<dpl::Timer,			TimeManager> timer;
		dpl::ReadOnly<dpl::Timer::Seconds,	TimeManager> lastTick;	// Last measured time point
		dpl::ReadOnly<float,				TimeManager> dt;
		dpl::ReadOnly<float,				TimeManager> T;			// Time since last tick
		dpl::ReadOnly<uint64_t,				TimeManager> cycle;
		dpl::ReadOnly<uint32_t,				TimeManager> fpsCounter;// Frames since last tick
		dpl::ReadOnly<uint32_t,				TimeManager> fps;		// Last recorded fps
		dpl::ReadOnly<bool,					TimeManager> bTick;		// true after each second
		
	public: // lifecycle
		CLASS_CTOR		TimeManager();

	protected: // functions
		void			update();

		void			reset();
	};
}
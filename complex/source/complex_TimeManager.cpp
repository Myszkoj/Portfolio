#include "..//include/complex_TimeManager.h"


namespace complex
{
//=====> TimeManager public: // lifecycle
	CLASS_CTOR	TimeManager::TimeManager()
		: lastTick(0)
		, dt(0.f)
		, T(0.f)
		, cycle(0)
		, fpsCounter(0)
		, fps(0)
		, bTick(true)
	{
		
	}

//=====> TimeManager protected: // functions
	void		TimeManager::reset()
	{
		timer->stop();
		lastTick	= 0.0;
		dt			= 0.f;
		T			= 0.f;
		cycle		= 0;
		fpsCounter	= 0;
		fps			= 0;	
		bTick		= false;		
	}

	void		TimeManager::update()
	{
		if(timer->is_started())
		{
			auto currentTick = timer->seconds();

			dt = static_cast<float>((currentTick - lastTick()).count());
			T = T + dt;
			if(T >= 1.0)
			{			
				fps			= fpsCounter;
				fpsCounter	= 0;
				T			= 0.f;
				bTick		= true;
			}
			else
			{
				bTick		= false;
			}

			lastTick = currentTick;
		}
		else
		{
			timer->start();
			lastTick = 0.0;
			dt	= 0.f;
			T	= 0.f;
		}
		
		++(*cycle);
		++(*fpsCounter);
	}
}
/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <chrono>

class Timer {
public:
		Timer(void) {
		
		}
		
		void Restart(void) {
			fBegin = Clock::now();
			fEnd = fBegin;
			fIsRunning = true;
		}
		
		void Stop (void) {
			if (fIsRunning == false)
				return;
				
			fEnd = Clock::now();
			fIsRunning = false;
		}
		
		double MS(void) {
			if (fIsRunning) {
				auto ms = std::chrono::duration_cast<milliseconds>(Clock::now() - fBegin);
				return ms.count();
			} else {
				auto ms = std::chrono::duration_cast<milliseconds>(fEnd - fBegin);
				return ms.count();
			}
		}
		
		double Seconds(void) {
			if (fIsRunning) {
				auto ms = std::chrono::duration_cast<seconds>(Clock::now() - fBegin);
				return ms.count();
			} else {
				auto ms = std::chrono::duration_cast<seconds>(fEnd - fBegin);
				return ms.count();
			}
		}
								
private:
	
	typedef	std::chrono::system_clock 	Clock;
	typedef std::chrono::milliseconds	milliseconds;
	typedef std::chrono::milliseconds	seconds;
	
	Clock::time_point					fBegin;
	Clock::time_point					fEnd;
	
	bool								fIsRunning;
};

#endif // TIMER_H

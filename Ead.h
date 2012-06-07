/*
 * Ead.h
 *
 * Adapted from ead~.c puredata external (creb library)
 * Copyright (c) 2000-2003 by Tom Schouten
 *
 * Copyright 2012 Tim Barrass unbackwards@gmail.com, 2000-2003 Tom Schouten
 *
 * This file is part of Cuttlefish.
 *
 * Cuttlefish is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cuttlefish is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cuttlefish.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EAD_H_
#define EAD_H_

#include "math.h"
#include <fixedMath.h>


/** Exponential attack decay envelope. This produces a natural sounding
envelope. It calculates a new value each time next() is called, which can be
used to change the amplitude or timbre of a sound.  The equations used are:
 attack: y = a*pow((1+r),x)
 decay: y = a*pow((1-r),x)
*/

class Ead
{

public:

	/** Constructor
	@param update_rate
	Usually this will be CONTROL_RATE or AUDIO_RATE, unless you
	design another scheme for updating. One such alternative scheme could take turns
	for various control changes in a rotating schedule to spread out calculations
	made in successive updateControl() routines. */

	Ead(unsigned int update_rate);


	/** Set the attack time in milliseconds.
	@param attack_ms The time taken for values returned by successive calls of
	the next() method to change from 0 to 255. */
	inline
	void setAttack(unsigned int attack_ms)
	{
		Q8n8attack = Q8n8_float2fix(millisToOneMinusRealPole(attack_ms));
	}


	/** Set the decay time in milliseconds.
	@param decay_ms The time taken for values returned by successive calls of
	the next() method to change from 255 to 0. */
	inline
	void setDecay(unsigned int decay_ms)
	{
		Q8n8decay = Q8n8_float2fix(millisToOneMinusRealPole(decay_ms));
	}


	/** Set attack and decay times in milliseconds
	@param attack_ms The time taken for values returned by successive calls of
	the next() method to change from 0 to 255.
	@param decay_ms The time taken for values returned by successive calls of
	the next() method to change from 255 to 0.
	*/
	inline
	void set(unsigned int attack_ms, unsigned int decay_ms)
	{
		setAttack(attack_ms);
		setDecay(decay_ms);
	}


	/** Start the envelope from the beginning. This can be used at any
	time, even if the previous envelope is not finished. */
	inline
	void start()
	{
		Q8n24state = 0;
		attack_phase = true;
	}


	/** Calculate and return the next envelope value, in the range -128 to 127 */

	// 5us
	inline
	unsigned char next()
	{
		switch(attack_phase)
		{
		case true: /* attack phase */
			// signed multiply A(a1,b1) * A(a2,b2)=A(a1 +a2 +1,b1 +b2)
			Q8n24state += ((Q8n24_FIX1 - Q8n24state) * Q8n8attack) >> 8; // Q8n24, shifts all back into n24
			if (Q8n24state >= Q8n24_FIX1-256)
			{
				Q8n24state = Q8n24_FIX1-256;
				attack_phase = false;
			}
			break;
		default: /* decay phase */
			Q8n24state -= (Q8n24state * Q8n8decay)>>8;
			break;
		}
		return Q8n24_to_Q0n8(Q8n24state);
	}

private:

	Q8n8 Q8n8attack;
	Q8n8 Q8n8decay;
	Q8n24 Q8n24state;
	bool attack_phase;
	const unsigned int UPDATE_RATE;


	/* convert milliseconds to 1-p, with p a real pole */
	inline
	float millisToOneMinusRealPole(unsigned int milliseconds)
	{
		static const float NUMERATOR = 1000.0f * log(0.001f);
		return  -expm1(NUMERATOR / ((float)UPDATE_RATE * milliseconds));
	}


	// Compute exp(x) - 1 without loss of precision for small values of x.
	inline
	float expm1(float x)
	{
		if (fabs(x) < 1e-5)
		{
			return x + 0.5*x*x;
		}
		else
		{
			return exp(x) - 1.0;
		}
	}

};

#endif /* EAD_H_ */

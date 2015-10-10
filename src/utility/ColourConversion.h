#pragma once
#include <glm/glm.hpp>

typedef struct 
{
	double r;       // percent
	double g;       // percent
	double b;       // percent
} rgb;

typedef struct 
{
	float h;       // angle in degrees
	float s;       // percent
	float v;       // percent
} hsv;

glm::vec3 RGBtoHSV( glm::vec3 in )
{
	hsv out;
	float min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min  < in.b ? min  : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max  > in.b ? max  : in.b;

	out.v = max;                                // v
	delta = max - min;
	if( max > 0.0f ) { // NOTE: if Max is == 0, this divide would cause a crash
		out.s = (delta / max);                  // s
	} else {
		// if max is 0, then r = g = b = 0              
			// s = 0, v is undefined
		out.s = 0.0;
		out.h = NAN;                            // its now undefined
		return glm::vec3( out.h, out.s, out.v );
	}
	if( in.r >= max )                           // > is bogus, just keeps compilor happy
		out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
	else
	if( in.g >= max )
		out.h = 2.0f + ( in.b - in.r ) / delta;  // between cyan & yellow
	else
		out.h = 4.0f + ( in.r - in.g ) / delta;  // between magenta & cyan

	out.h *= 60.0f;                              // degrees

	if( out.h < 0.0f )
		out.h += 360.0f;

	if( out.s == 0 )
		out.h = 180;

	return glm::vec3( out.h, out.s, out.v );
}


glm::vec3 HSVtoRGB( glm::vec3 _hsv )
{
	hsv in;
	in.h = _hsv.x;
	in.s = _hsv.y;
	in.v = _hsv.z;

	float hh, p, q, t, ff;
	long i;
	glm::vec3	out;

	if(in.s <= 0.0f) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if(hh >= 360.0f) hh = 0.0f;
	hh /= 60.0f;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0f - in.s);
	q = in.v * (1.0f - (in.s * ff));
	t = in.v * (1.0f - (in.s * (1.0f - ff)));

	switch(i) 
	{
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;     
}
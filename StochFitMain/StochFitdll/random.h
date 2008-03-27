/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

namespace Random{

inline double uniform_deviate ( int seed )
 {
   return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
 }

inline unsigned time_seed()
  {
    time_t now = time ( 0 );
    unsigned char *p = (unsigned char *)&now;
    unsigned seed = 0;
	size_t i;
  
    for ( i = 0; i < sizeof now; i++ )
      seed = seed * ( UCHAR_MAX + 2U ) + p[i];
 
  return seed;
 }

inline int random()
{
	return uniform_deviate(rand());
}

inline  int random( int max, int min)
  {
    max++;
	int r = min + uniform_deviate ( rand() ) * ( max - min );
	return r;
  }

inline  int plusminus()
  {
	  int r = random(1,0);

	  if(r == 1)
		  return 1;
	  else
		  return -1;
  }

inline double random(double max, double min)
  {
	double r = min + uniform_deviate ( rand() ) * ( max - min );
	return r;

  }
}
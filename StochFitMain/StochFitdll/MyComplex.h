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



namespace MyComplexNumber{

	//Complex Class definition

	__declspec(align(16)) struct MyComplex
	{
	  public:
	    double complexholder[2];
		double re;
		double im;

		MyComplex(double real = 0.0, double imaginary = 0.0)
		{
			re = real;
			im = imaginary;
		}
		//overload addition, subtraction, multiplication, division
		MyComplex& operator=(const MyComplex& comp) 
		{
			re = comp.re;
			im = comp.im;

			return *this;
		} 

		MyComplex& operator=(const float comp) 
		{
			re = comp;
			im = 0.0;

			return *this;
		} 

		

		const MyComplex operator*(const double s) 
		{
			
			return MyComplex(s*re,s*im);
		} 

		friend const MyComplex operator*(const MyComplex& lhs, const MyComplex& rhs)
		{
			return MyComplex((lhs.re*rhs.re-lhs.im*rhs.im),(lhs.re*rhs.im+lhs.im*rhs.re));

		}

		friend const MyComplex operator/(const MyComplex& lhs,const MyComplex& rhs)
		{
			double denom = 1.0/(rhs.re*rhs.re+rhs.im*rhs.im);
			
			return MyComplex((lhs.re*rhs.re+lhs.im*rhs.im)*denom,(lhs.im*rhs.re-lhs.re*rhs.im)*denom);
		}

		
		friend MyComplex operator+(const MyComplex& lhs,const MyComplex& rhs)
		{
			return MyComplex(lhs.re+rhs.re,lhs.im+rhs.im);
		}

		friend const MyComplex operator-(const MyComplex& lhs, const MyComplex& rhs)
		{
			return MyComplex(lhs.re-rhs.re,lhs.im-rhs.im);
		}

		friend bool operator==(const MyComplex& lhs,const MyComplex& rhs)
		{
			return (lhs.re == rhs.re && lhs.im == rhs.im);
		}
		
		friend bool operator!=(const MyComplex& lhs,const MyComplex& rhs)
		{
			return (lhs.re == rhs.re && lhs.im == rhs.im);
		}
	};

	//Complex functions

	inline MyComplex compexp(const MyComplex& comp)
	{
		double exponent = exp(comp.re);
		return MyComplex(exponent*cos(comp.im),exponent*sin(comp.im));
	}

	inline MyComplex compcos(const MyComplex& comp)
	{
		return MyComplex(cos(comp.re)*cosh(comp.im),0.0-(sin(comp.re)*sinh(comp.im)));
	}

	inline MyComplex compsin(const MyComplex& comp)
	{
		return MyComplex(sin(comp.re)*cosh(comp.im),cos(comp.re)*sinh(comp.im));
	}

	inline float comparg(const MyComplex& comp)
	{
		return atan2(comp.im, comp.re);
	}

	inline MyComplex compsqrt(const MyComplex& comp)
	{
		//Use the half angle relation to calculate the square root. Prevents buffer
		//over/under flows. Don't change this code. This problem actually occurs
		double mag, theta, sqrtholder;
		MyComplex holder;
		if(comp.im != 0.0)
		{
			mag = sqrt(comp.re*comp.re+comp.im*comp.im);
			theta = atan2(comp.im,(comp.re+mag));
			sqrtholder = sqrt(mag);
			holder = MyComplex(sqrtholder*cos(theta),sqrtholder*sin(theta));
		}
		else if(comp.re < 0.0)
			holder = MyComplex(0.0, sqrt(abs(comp.re)));
		else 
			holder =  MyComplex(sqrt(comp.re),0.0);
		
		
		return holder;
	};

	inline double compabs(const MyComplex& comp)
	{
		//While inefficient, this prevents buffer under/overflows - from Numerical Recipes in C++
		double placeholder=0.0;
		
		if(comp.re >= comp.im)
		{
			
			if(comp.re == 0.0)
				return fabs(comp.im);

			placeholder = comp.im/comp.re;
			return fabs(comp.re)*sqrt(1.0f+placeholder*placeholder);
		}
		else
		{
			if(comp.im == 0.0)
			{
				return abs(comp.re);
			}

			placeholder = comp.re/comp.im;

			return abs(comp.im)*sqrt(1.0+placeholder*placeholder);
		}
	};
	
	inline MyComplex cln(const MyComplex& comp)
	{
		return MyComplex(log(compabs(comp)),comparg(comp));
	}
}
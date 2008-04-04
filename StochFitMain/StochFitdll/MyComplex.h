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

	template <class T> struct MyComplex
	{
	  public:
		T re;
		T im;

		MyComplex(T real = 0, T imaginary = 0)
		{
			re = real;
			im = imaginary;
		}
		//overload addition, subtraction, multiplication, division
		MyComplex<T>& operator=(const MyComplex<T>& comp) 
		{
			re = comp.re;
			im = comp.im;

			return *this;
		} 

		MyComplex<T>& operator=(const double comp) 
		{
			re = comp;
			im = 0.0;

			return *this;
		} 

		//template<class D> operator MyComplex<D> () const;

		const MyComplex<T> operator*(const T s) 
		{
			
			return MyComplex<T>(s*re,s*im);
		} 

		friend const MyComplex<T> operator*(MyComplex<T> lhs, MyComplex<T> rhs)
		{
			return MyComplex((lhs.re*rhs.re-lhs.im*rhs.im),(lhs.re*rhs.im+lhs.im*rhs.re));

		}

		friend const MyComplex<T> operator/(MyComplex<T> lhs, MyComplex<T> rhs)
		{
			double denom = rhs.re*rhs.re+rhs.im*rhs.im;
			
			return MyComplex<T>((lhs.re*rhs.re+lhs.im*rhs.im)/denom,(lhs.im*rhs.re-lhs.re*rhs.im)/denom);
		}

		
		friend MyComplex<T> operator+(MyComplex lhs,MyComplex rhs)
		{
			return MyComplex<T>(lhs.re+rhs.re,lhs.im+rhs.im);
		}

		friend const MyComplex<T> operator-(const MyComplex lhs, const MyComplex rhs)
		{
			return MyComplex<T>(lhs.re-rhs.re,lhs.im-rhs.im);
		}
	};

	//Complex functions
	/*template<class T> template<class D> MyComplex<T>::operator MyComplex<D>() const
	{
		return MyComplex<D>(re, im);
	}*/

	template <class T> inline MyComplex<T> compexp(MyComplex<T> comp)
	{
		T exponent = exp(comp.re);
		return MyComplex<T>(exponent*cos(comp.im),exponent*sin(comp.im));
	}

	template <class T> inline MyComplex<T> compcos(MyComplex<T> comp)
	{
		return MyComplex(cos(comp.re)*cosh(comp.im),0-(sin(comp.re)*sinh(comp.im)));
	}

	template <class T> inline MyComplex<T> compsin(MyComplex<T> comp)
	{
		return MyComplex(sin(comp.re)*cosh(comp.im),cos(comp.re)*sinh(comp.im));
	}

	template <class T> inline T comparg(MyComplex<T> comp)
	{
		return atan2(comp.im, comp.re);
	}

	template <class T> inline MyComplex<T> compsqrt(MyComplex<T> comp)
	{
		//Use the half angle relation to calculate the square root. Prevents buffer
		//over/under flows. Don't change this code. This problem actually occurs
		T mag;
		T theta;
		T sqrtholder;

		if(comp.im != 0)
		{
			mag = sqrt(comp.re*comp.re+comp.im*comp.im);
			theta = atan2(comp.im,(comp.re+mag));
			sqrtholder = sqrt(mag);
			return MyComplex<T>(sqrtholder*cos(theta),sqrtholder*sin(theta));
		}
		
		if(comp.re < 0)
			return MyComplex<T>(0, sqrt(abs(comp.re)));

		return MyComplex<T>(sqrt(comp.re),0);
	};

	template <class T> inline T compabs(MyComplex<T> comp)
	{
		//While inefficient, this prevents buffer under/overflows - from Numerical Recipes in C++
		double placeholder=0;
		if(comp.re >= comp.im)
		{
			if(comp.re == 0)
				return fabs(comp.im);

			placeholder = comp.im/comp.re;
			return(fabs(comp.re)*sqrt(1.0+placeholder*placeholder));
		}
		else
		{
			if(comp.im == 0)
				return fabs(comp.re);

			placeholder = comp.re/comp.im;
			return (fabs(comp.im)*sqrt(1.0+placeholder*placeholder));
		}
		
	};
	
	template <class T> inline MyComplex<T> cln(MyComplex<T> comp)
	{
		return MyComplex<T>(log(compabs(comp)),comparg(comp));
	}
}
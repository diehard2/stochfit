#pragma once
#define M_PI	3.1415926535897932384626433 

namespace MyComplexNumber{

	//Complex Class definition
	struct MyComplex
	{
		public:
			double re;
			double im;

			MyComplex(double real = 0, double imaginary = 0)
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

			const MyComplex operator*(const double s) 
			{
				
				return MyComplex(s*re,s*im);
			} 

			friend const MyComplex operator*(MyComplex lhs, MyComplex rhs)
			{
				return MyComplex((lhs.re*rhs.re-lhs.im*rhs.im),(lhs.re*rhs.im+lhs.im*rhs.re));

			}

			friend const MyComplex operator/(MyComplex lhs, MyComplex rhs)
			{
				double denom = rhs.re*rhs.re+rhs.im*rhs.im;
				
				return MyComplex((lhs.re*rhs.re+lhs.im*rhs.im)/denom,(lhs.im*rhs.re-lhs.re*rhs.im)/denom);
			}

			
			friend MyComplex operator+(MyComplex lhs,MyComplex rhs)
			{
				return MyComplex(lhs.re+rhs.re,lhs.im+rhs.im);
			}

			friend const MyComplex operator-(const MyComplex lhs, const MyComplex rhs)
			{
				return MyComplex(lhs.re-rhs.re,lhs.im-rhs.im);
			}
	};

	//Complex functions

	inline MyComplex compconj(MyComplex comp)
	{
		return MyComplex(comp.re,-1*comp.im);
	}

	inline MyComplex compexp(MyComplex comp)
	{
		double exponent = exp(comp.re);
		return MyComplex(exponent*cos(comp.im),exponent*sin(comp.im));
	}

	inline MyComplex compcos(MyComplex comp)
	{
		return MyComplex(cos(comp.re)*cosh(comp.im),0-(sin(comp.re)*sinh(comp.im)));
	}

	inline MyComplex compsin(MyComplex comp)
	{
		return MyComplex(sin(comp.re)*cosh(comp.im),cos(comp.re)*sinh(comp.im));
	}

	inline double comparg(MyComplex comp)
	{
		return atan2(comp.im, comp.re);
	}

	inline MyComplex compsqrt(MyComplex comp)
	{
		//Use the half angle relation to calculate the square root. Prevents buffer
		//over/under flows. Don't change this code. This problem actually occurs
		double mag;
		double theta;
		double sqrtholder;

		if(comp.im != 0)
		{
			mag = sqrt(comp.re*comp.re+comp.im*comp.im);
			theta = atan2(comp.im,(comp.re+mag));
			sqrtholder = sqrt(mag);
			return MyComplex(sqrtholder*cos(theta),sqrtholder*sin(theta));
		}
		
		if(comp.re < 0)
			return MyComplex(0, sqrt(abs(comp.re)));

		return MyComplex(sqrt(comp.re),0);
	};

	// From Hui's rational approximation 1977
	// Evaluate complex error function. Note that this calculation is only good in the 
	// first quadrant and does not include the origin, purely real, or purely complex numbers.
	// The dispatching function is cerf()
	inline MyComplex poscerf(MyComplex comp)
	{
		double a0 = 122.607931777104326;
		double a1 = 214.382388694706425;
		double a2 = 181.928533092181549;
		double a3 = 93.155580458138441;
		double a4 = 30.180142196210589;
		double a5 = 5.912626209773153;
		double a6 = 0.564189583562615;

		double b0 = 122.60793177387535;
		double b1 = 352.730625110963538;
		double b2 = 457.334478783897737;
		double b3 = 348.703917719495792;
		double b4 = 170.354001821091472;
		double b5 = 53.992906912940207;
		double b6 = 10.479857114260399;
		
		MyComplex x(comp.im, -1*comp.re); 
		MyComplex func;

		func = ((((((a6*x+a5)*x+a4)*x+a3)*x+a2)*x+a1)*x+a0)/(((((((x+b6)*x+b5)*x+b4)*x+b3)*x+b2)*x+b1)*x+b0);
		return func;
	}

	inline unsigned factorial(unsigned N)
	{
		return (N <= 1) ? 1 : N*factorial(N-1);
	}

	//Good for large x - use if you don't have the intel compiler. Very bad for small x
	inline long double myerfcseries(long double x)
	{
		long double prefactor = exp(-1.0*x*x)/(x*sqrt(M_PI));
		long double sum = 0;

		for(int i = 0; i < 10; i++)
		{
			sum += pow(-1.0,i)*factorial(2*i)/(factorial(i)*pow(2.0*x,2*i));
		}
		
		sum *= prefactor;
		return sum;
	}

	inline long double myerfc(double x)
	{
		if(x < 100.0)
			return erfc(x);
		else
			return 0;
	}

	
	inline MyComplex cerf(MyComplex comp)
	{
		if(comp.re > 0)
		{
			return poscerf(comp);
		}
		else if(comp.re == 0 && comp.im > 0)
		{
			if(myerfc(comp.im) == 0)
				return 0;
			else
				return exp((comp.im*comp.im))*myerfc(comp.im);
		}
		else if(comp.re == 0 && comp.im == 0)
		{
			return 0;
		}
		else if(comp.im == 0)
		{
			return erf(comp.re);
		}
		else if(comp.im < 0 && comp.re < 0)
		{
			return 2.0*compexp(-1.0*comp*comp)-poscerf(-1*comp);
		}
		return 0;
	}

	

	

	inline double compabs(MyComplex comp)
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
	
	inline MyComplex cln(MyComplex comp)
	{
		return MyComplex(log(compabs(comp)),comparg(comp));
	}
}
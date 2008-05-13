// CUDAtest.cpp : Defines the entry point for the console application.
//



#include "stdafx.h"
#include <vector_types.h>

//
//Function definitions
//
extern "C" void CUDAInit(float** genomearray, float** doublenk, float** cudank, float2** nk, float* dz, float* m_dboxsize, int boxcount, int npts, float** scratcharray);
extern "C" void CUDAFreeArrays(float** genomearray, float** doublenk, float** cudank, float2** nk, float** scratcharray);
extern "C" void CUDAMakeDensity(const int argc, const char** argv, int points, float* genomearray,float* cudank, float2* nk, float* doublenk, int boxcount, bool writenk, float* scratcharray);
void CPPMakeDensity(int points, float* genomearray, float2* nk, float* edspacingarray, float* distarray, float* rhoarray);
void WriteEDFile(string filename, int points, float dz, float2* nk);
void SetUpArrays(float** genomearray, float** distarray, float** edspace, float** rhoarray, float2** nk, float* dz, float* m_dboxsize, int boxcount, int npts);
void FreeArrays(float** genomearray, float** distarray, float** edspacingarray, float** rhoarray, float2** nk);
void DensitytoVec(vector<float>& vec, float2* nk, int nksize);
bool CheckCalcs(vector<float>& vec1, vector<float>& vec2);


int _tmain(int argc, _TCHAR* argv[])
{
	__int64 start, stop, freq;
	float *distarray, *rhoarray, *edspacingarray, *genomearray, dz, m_dboxsize, *xnk, *doublenk, *scratcharray;
	float2 *nk;
	bool writenk = false;
	vector<float> CPUcalc, GPUcalc;
	
	int npts = TOTALLENGTH*RESOLUTION;
	
	
	/// Setup arrays

	SetUpArrays(&genomearray,&distarray,&edspacingarray,&rhoarray, &nk,&dz,  &m_dboxsize,BOXCOUNT,  npts);

	///
	/// Perform ED calc using OpenMP
	///
	
	QueryPerformanceCounter((LARGE_INTEGER*) &start);

	for(int i = 0; i < ITERATIONS ; i++)
	{
		CPPMakeDensity(npts,genomearray, nk, edspacingarray, distarray, rhoarray);
	}
	

	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
   DensitytoVec(CPUcalc, nk, npts);
   cout << "Time to perform " << ITERATIONS << " ED calculations \non the CPU for " << npts << " points in usecs/calc - " << (((float)(stop-start)/(float)freq)/((float)ITERATIONS))*1e6 << endl << endl << endl;
   WriteEDFile("CPUcalc.txt", npts, dz, nk);
   FreeArrays(&genomearray, &distarray, &edspacingarray, &rhoarray, &nk);


   //
   //Try it with CUDA
   //

	

	CUDAInit(&genomearray,&doublenk, &xnk, &nk, &dz,  &m_dboxsize,BOXCOUNT,  npts, &scratcharray);

	QueryPerformanceCounter((LARGE_INTEGER*) &start);

	for(int i = 0; i < ITERATIONS; i++)
	{
		if(i == ITERATIONS - 1)
			writenk = true;

		CUDAMakeDensity(argc,(const char**)argv,npts,genomearray, xnk, nk, doublenk, BOXCOUNT, writenk, scratcharray);
	}
	
	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
    cout << "Time to perform " << ITERATIONS << " ED calculations \non the GPU for " << npts << " points in usecs/calc - " << (((float)(stop-start)/(float)freq)/((float)ITERATIONS))*1e6 << endl << endl << endl;

	DensitytoVec(GPUcalc, nk, npts);
	
	if(CheckCalcs(CPUcalc, GPUcalc) == false)
		cout << "RESULT ---  The CPU and GPU calculations are not equivalent\n\n";
	else
		cout << "RESULT ---  The CPU and GPU calculation are within machine precision\n\n";

	WriteEDFile("CUDAcalc.txt", npts, dz, nk);
	

	CUDAFreeArrays(&genomearray, &doublenk, &xnk, &nk, &scratcharray);

   cout << "Press any key to exit" << endl;
   getch();

  
}

void CPPMakeDensity(int pts, float* genomearray, float2* nk, float* edspacingarray, float* distarray, float* rhoarray)
{
	int refllayers = BOXCOUNT+1;
	int reflpoints = pts;
	float roughness = 1.0f/(ROUGHNESS * sqrtf(2.0f));
	float supersld = genomearray[0]*RHO_A;
	
	for(int k = 0; k < refllayers; k++)
	{
		rhoarray[k] = RHO_A *(genomearray[k+1]-genomearray[k])/2.0f;
	}


		float dist;
		#pragma omp parallel for private(dist)//schedule(guided)
		for(int i = 0; i < reflpoints; i++)
 		{
			nk[i].x = 0.0f;
			nk[i].y = 0.0f;
			
			
			for(int k = 0; k < refllayers; k++)
			{
				dist = (edspacingarray[i]-distarray[k] )*roughness;

				if(dist > 4.0f)
					nk[i].x += (rhoarray[k])*(2.0f);
				else if (dist > -4.0f)
					nk[i].x += (rhoarray[k])*(1.0f+erff(dist));
			}
		}
}

///Write the output files
void WriteEDFile(string filename, int points, float dz, float2* nk)
{
  //Write out the files
   float x=0;
   ofstream rhoout(filename.c_str());
   for(int i = 0; i < points; i++)
   {
	   float temp = nk[points-1].x;

	   rhoout << x << ' ' << nk[i].x/nk[points-1].x << endl;
	   x += dz;
   }
   rhoout.close();
}

///Setup the Arrays
void SetUpArrays(float** genomearray, float** distarray, float** edspacingarray, float** rhoarray, float2** nk,float* dz, float* m_dboxsize, int boxcount, int npts)
{
	//Create scratch arrays for the electron density calculation
	*distarray = (float*)malloc((boxcount+2)*sizeof(float));
	*rhoarray = (float*)malloc((boxcount+2)*sizeof(float));
	*edspacingarray = (float*)malloc(npts*sizeof(float));
	*nk = (float2*)malloc(npts*sizeof(float2));
	*genomearray = (float*)malloc((boxcount+2)*sizeof(float));
	
	//Setup the arrays
	*dz = 1.0f/RESOLUTION;
	*m_dboxsize = (LAYERLENGTH+7.0f)/((float)boxcount);

	(*genomearray)[0] = 0.0f;
	for(int i = 1; i < boxcount+1; i++)
	{
		(*genomearray)[i] = 1.2f;
	}
	(*genomearray)[boxcount + 1] = 1.0f;

	for(int i = 0; i < npts; i++)
	{
		float temp = i*(*dz)-LEFTOFFSET;
		(*edspacingarray)[i] = i*(*dz)-LEFTOFFSET;
	}

	for(int k = 0; k < boxcount+2; k++)
	{
		(*distarray)[k] = k*(*m_dboxsize);
	}
}

void FreeArrays(float** genomearray, float** distarray, float** edspacingarray, float** rhoarray, float2** nk)
{
  //
  //Free the arrays
  //
	free(&distarray);
	free(&rhoarray);
	free(&edspacingarray);
	free(&genomearray);
	free(&nk);
}

void DensitytoVec(vector<float>& vec, float2* nk, int nksize)
{
	for(int i = 0; i < nksize; i++)
		vec.push_back(nk[i].x);
}

bool CheckCalcs(vector<float>& vec1, vector<float>& vec2)
{
	if(vec1.size() != vec2.size())
	{
		cout << "Catastrophic error - vector size mismatch" << endl;
		return false;
	}

	if(vec1 != vec2)
	{
		float averagediff = 0;
		int averagediffcount = 0;

		cout << "WARNING!!!!  --- Value difference between CPU calc and GPU calc ---  WARNING!!!!\n\n\n";

		for(int i = 0; i < vec1.size(); i++)
		{		
				averagediff += fabsf(vec1[i] - vec2[i]);
				averagediffcount++;
		}

		averagediff /= (float)averagediffcount*RHO_A;

		if(averagediff < 1e-6)
		{
			cout << "Difference appears to be because of machine precision and is negligible\n\n";
			cout << "The average normalized difference is " << averagediff << " ED units\n\n\n";	
			return true;
		}
		else
		{
			cout << "Large difference in calculation not due to machine precision \n\n\n";
			cout << "The average normalized difference is " << averagediff << " ED units\n\n\n";
			return false;
		}
	}
	else
		return true;

}
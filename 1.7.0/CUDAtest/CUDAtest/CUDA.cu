//defines
#include "defines.h"

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <float.h>
// includes, project
#include <cutil.h>
//#define CUDADEBUG
// includes, kernels
#include <kernel.cu>


extern "C" void CUDAInit(float** genomearray, float** doublednk, float** cudank, float2** nk, float* dz, float* m_dboxsize, int boxcount, int npts, float** scratcharray)
{
	//Initialization
    
    int deviceCount;                                                         
    CUDA_SAFE_CALL_NO_SYNC(cudaGetDeviceCount(&deviceCount));                
    CUDA_SAFE_CALL(cudaSetDevice(deviceCount-1));     
  
    
    *nk = (float2*)malloc(npts*sizeof(float2));
    CUDA_SAFE_CALL(cudaMalloc((void**) genomearray, (boxcount+2)*sizeof(float)));
    CUDA_SAFE_CALL(cudaMalloc((void**) doublednk, npts*sizeof(float)));
    CUDA_SAFE_CALL(cudaMalloc((void**) cudank, npts*sizeof(float)));
     CUDA_SAFE_CALL(cudaMalloc((void**) scratcharray, npts*sizeof(float)*(boxcount+2)));
    
	//Setup the arrays
	*dz = 1.0f/RESOLUTION;
	*m_dboxsize = (LAYERLENGTH+7.0f)/((float)boxcount);


    
    setarrays<<<1, 1 >>>(*genomearray, BOXCOUNT);
}

extern "C" void CUDAFreeArrays(float** genomearray, float** doublednk, float** cudank, float2** nk, float** scratcharray)
{
	//Free memory
	CUDA_SAFE_CALL(cudaFree(*genomearray));
    CUDA_SAFE_CALL(cudaFree(*doublednk));
	CUDA_SAFE_CALL(cudaFree(*cudank));
	CUDA_SAFE_CALL(cudaFree(*scratcharray));
    free(*nk);
}

extern "C" void
CUDAMakeDensity(const int argc, const char** argv, int points, float* genomearray,float* cudank, float2* nk, float* doublenk, int boxcount, bool writenk, float* scratcharray)
{
#ifndef BLANK
   unsigned int num_threads = 128;
   int boxes = BOXCOUNT + 1; 
   
   if(BOXCOUNT+2 > num_threads)
		num_threads = BOXCOUNT+2;
   
  
   int pts = TOTALLENGTH*RESOLUTION;
   float boxsize = (LAYERLENGTH+7.0f)/((float)boxcount);
   float dz = 1.0f/RESOLUTION;
 
   

   float rough = dz/(ROUGHNESS * sqrtf(2.0f));
 
   int neededmemory = boxes*sizeof(float)*3;
	
	dim3 dimBlock (num_threads,1);
    dim3 dimGrid (ceil((float)pts/(float)dimBlock.x),1);
    

   EDCalc<<<dimGrid, dimBlock, neededmemory>>> (genomearray, cudank, doublenk, boxes,rough,  RHO_A*0.5f, pts, boxsize, dz, LEFTOFFSET, scratcharray);
//	FasterEDCalc<<<dimGrid, dimBlock, neededmemory>>> (genomearray, cudank, distarray,boxes,rough, RHO_A*0.5f, pts, boxsize, dz, LEFTOFFSET);
	

    

  	if(writenk)
	{
	    float* temp = (float*)malloc(pts*sizeof(float));
		CUDA_SAFE_CALL(cudaMemcpy(temp, cudank, pts*sizeof(float), cudaMemcpyDeviceToHost));
		
		for(int i = 0; i < pts; i++)
		{
		  nk[i].x = temp[i];
		  
		}
		
		free(temp);	
		  
	}

#else
unsigned int num_threads = 1;
    dim3 grid(1, 1, 1);
    dim3 threads(num_threads, 1, 1);
 
   for(int i = 0; i < ITERATIONS; i++)
   {
		blank<<<grid, threads>>>();
   }
   #endif
	
}

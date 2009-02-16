#ifndef _KERNEL_H_
#define _KERNEL_H_

extern __shared__ float combined[];

__global__ void
EDCalc( float* genome, float* dnk, float* doublednk, int refllayers, float roughness,  float rho, int totalpts, float boxsize, float dz, float leftoffset, float* scratcharray)
{
	int tid = threadIdx.x;
	int index = blockIdx.x*blockDim.x+tid;
	float *srhoarray, *sdrhoarray, *sdistarray;
	

	srhoarray = combined;
	sdrhoarray = combined + refllayers;
	sdistarray = combined + refllayers + refllayers;

	if(tid < refllayers)
	{
		srhoarray[tid] = (genome[tid+1]-genome[tid])*rho;
		sdrhoarray[tid] = 2.0f*srhoarray[tid];
		sdistarray[tid] = (-leftoffset- tid*boxsize)/dz;
	}
	
	
	if(index < totalpts)
	{
	  
		float temp = 0;
		float dist = 0;
		for(int k = 0; k < refllayers; k++)
		{
			
			dist = (index + sdistarray[k] )*roughness;
			
			if(dist > 4.0f)
				temp += sdrhoarray[k];
			else if (dist > -4.0f)
				temp += (srhoarray[k])*(1.0f+erff(dist));
			
		}
		dnk[index] = temp;
		doublednk[index] = temp+temp;
	 }
 
 __syncthreads();
 
 //Start the reflectivity calculation

}

__global__ void blank()
{

}

__global__ void setarrays(float* genomearray, int boxcount)
{
    genomearray[0] = 0.0f;
	for(int i = 1; i < boxcount+1; i++)
	{
		genomearray[i] = 1.2f;
	
	}
	genomearray[boxcount + 1] = 1.0f;
	

   syncthreads();
}


//working
__global__ void
EDCalcWorking( float* genome,float* edspacing,float* rhoarray, float2* dnk, float* distarray, int ptsperthread, int refllayers, float roughness )
{

	 const unsigned int tid = threadIdx.x;
     float2* nk = dnk + tid*ptsperthread;
     float* edspace = edspacing + tid*ptsperthread;

      float temp = 0;
      for(int i = 0; i < ptsperthread; i++)
 		{
			temp = 0;
			for(int k = 0; k < refllayers; k++)
			{
				
				float dist = (edspace[i]-distarray[k] )*roughness;
				if(dist > 6.0f)
				{
					temp += (rhoarray[k])*(2.0f);
				}
				else if (dist > -6.0f)
				{
				//	temp += (rhoarray[k])*(1.0f+erff(dist));
				}
			}
			nk[i].x = temp;
	
		}

}


//Working faster
__global__ void
FasterEDCalc( float* genome, float* dnk, float* distarray, int refllayers, float roughness,  float rho, int totalpts, float boxsize, float dz, float leftoffset)
{

int tid = threadIdx.x;
	int index = blockIdx.x*blockDim.x+tid;
	float *srhoarray, *sdrhoarray, *sdistarray;

	srhoarray = combined;
	sdrhoarray = combined + refllayers;
	sdistarray = combined + 2*refllayers;

	if(tid < refllayers)
	{
		srhoarray[tid] = (genome[tid+1]-genome[tid])*rho;
		sdrhoarray[tid] = 2.0f*srhoarray[tid];
		sdistarray[tid] = (-leftoffset- tid*boxsize)/dz;
	}
	
	
if(index < totalpts)
{
  
    float temp = 0;
    float dist = 0;
	for(int k = 0; k < refllayers; k++)
	{
		
		dist = (index + sdistarray[k] )*roughness;
		
		if(dist > 4.0f)
			temp += sdrhoarray[k];
		else if (dist > -4.0f)
			temp += (srhoarray[k])*(1.0f+erff(dist));
		
	}
	dnk[index] = temp;

 }
 
 __syncthreads();


 
}



#endif //_KERNEL_H_


#include "stdafx.h"
#include "genome.h"
#include "multilayer.h"
#include "stochselection.h"

StochSelection::StochSelection()
{
	//Initialize the random number generator
	srand(static_cast<unsigned>(time( NULL )));
	outtestfile = new ofstream("outfile.txt");
	executioncount = 0;
}

void StochSelection::Init(GARealGenome* genome1, multilayer* ml0)
{
	//Get the initial genome's score
    score0=ml0->objective(genome1);
	m_bthreadstop = false;
	
}
StochSelection::~StochSelection()
{
	outtestfile->close();
	delete outtestfile;
}
 
double StochSelection::StochasticSelection(GARealGenome* genome, multilayer* ml0, double lipidlengthfuzzy, double agene_max)
{
	
	double agene1, holdairwater,score1;
//	GARealGenome genome1=*genome;
	int ii;
	int acc0=0;
	int jmax = 20*4;

	//Change the genome quickly at first, and then slow down significantly
	//if(executioncount == 0)
	//{
	//	mc_step = 0.025;
	//}
	//else if(executioncount == 101)
	//{
	//	mc_step = 0.01;
	//}

	//if(executioncount == 19)
	//	Sleep(10);
	mc_step = 0.025;
	GARealGenome genome1(genome->length, 0.0, 2.0);
	//Only mutate for the actual guessed fuzzy layer length 
	for(unsigned int jj=0; (jj<jmax) && (m_bthreadstop == false);jj++)
		{
			//Pick the gene we're going to mutate
			ii= (int) ( (double) rand()/(RAND_MAX+1)*(genome->size()+1));
			*outtestfile << "Generating new value for " << ii << endl;

			//Copy our best solution to genome1 and holdairwater
			
			genome1 = *genome;
			holdairwater = ml0->m_dairwaterrough;
			
			if(ii <= lipidlengthfuzzy)
			{ 
				//Mutate the rho multiplier (ii != 0) or mutate the roughness
				if(ii < lipidlengthfuzzy && ii != 0)
				{
					if(rand()%2)
						agene1 = genome1.gene(ii) + genome1.gene(ii)*(((double)(rand())/(double)RAND_MAX))*(mc_step);
					else
						agene1 = genome1.gene(ii) - genome1.gene(ii)*(((double)(rand())/(double)RAND_MAX))*(mc_step);

					if(agene1 > agene_max)
						agene1 = agene_max;

					genome1.gene(ii,agene1);
				}
				else if(ii ==0)
				{
			
					if(rand()%2)
						agene1 = genome1.gene(ii) + genome1.gene(ii)*(((double)(rand())/(double)RAND_MAX)*mc_step*4);
					else
						agene1 = genome1.gene(ii) - genome1.gene(ii)*(((double)(rand())/(double)RAND_MAX)*mc_step*4);

					genome1.gene(ii,agene1);
				}
				else if(ii==lipidlengthfuzzy)
				{
						if(rand()%2)
							ml0->m_dairwaterrough = ml0->m_dairwaterrough + ml0->m_dairwaterrough*(((double)(rand())/(double)RAND_MAX)*mc_step*4);
						else
							ml0->m_dairwaterrough = ml0->m_dairwaterrough - ml0->m_dairwaterrough*(((double)(rand())/(double)RAND_MAX)*mc_step*4);

				}
					*outtestfile << agene1 << endl;

				score1=ml0->objective(&genome1);
			}
	

		//See if we've improved the score	
		if( score1< score0) 
		{
				acc0++;
				*genome=genome1;
				score0=score1;
				*outtestfile<<"Accepted"<<endl;
			
				oldscorecount = 0;
				
		}
		else
		{
			ml0->m_dairwaterrough = holdairwater;
		}
	}
	
		double acpt=(double) acc0/jmax;
		
		/*	
		if( acpt< 0.01) mc_step *=0.9;
		else if(acpt>0.05) mc_step *=1.1;

		if( oldscorecount > 4 || mc_step == 0.01 || mc_step == 0.05) 
		{
			mc_step =0.025;
		}
			*/

			
			
			ofstream outfile;
			outfile.open(ml0->fnpop.c_str());
			
			//Write the population file
			int jj=0;
			*outtestfile<<executioncount<<' '<<score0<<' '<<genome->gene(jj)<< ' ' << ml0->m_dairwaterrough<< ' ' << endl;
			outfile<<ii<<' '<<score0<<' '<<genome->gene(jj)<< ' ' << ml0->m_dairwaterrough<< ' ' << endl;
			jj++;
			do{
				outfile<<genome->gene(jj)<< "\n";
				jj++;
			}while(jj<genome->size());

			outfile.close();
 
			if(oldscore == score0 )
			{
				oldscorecount++;
				*outtestfile<<"Incrementing oldscorecount - " << oldscorecount << endl;
			}
	
			oldscore=score0;
			executioncount++;
			return 0;
}


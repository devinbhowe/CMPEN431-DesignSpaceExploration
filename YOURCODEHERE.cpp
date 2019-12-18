#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <math.h>
#include <fcntl.h>

#include "431project.h"

class Configuration
{
	public:
	int 	width;
	int 	fetchSpeed;
	int 	scheduling;
	int 	ruuSize;
	int 	lsqSize;
	int 	memPorts;
	int 	dl1Sets;
	int 	dl1Assoc;
	int 	dl1BlockSize;
	int 	il1Sets;
	int 	il1Assoc;	
	int 	il1BlockSize;
	int 	ul2Sets;
	int 	ul2BlockSize;
	int 	ul2Assoc;
	int 	tlbSets;
	int 	dl1Lat;
	int 	il1Lat;
	int 	ul2Lat;
	int 	bpred;
	
	int dl1Size;
	int il1Size;
	int ul2Size;

	Configuration(std::string configuration, int* configurationDimsAsInts)
	{
		width = pow(2,configurationDimsAsInts[0]);
		fetchSpeed = (2,configurationDimsAsInts[1])+1;	
		scheduling = configurationDimsAsInts[2];	 
		ruuSize = pow(2,configurationDimsAsInts[3]+2);    
		lsqSize = pow(2,configurationDimsAsInts[4]+2);
		memPorts = configurationDimsAsInts[5]+1;
		dl1Sets = pow(2,configurationDimsAsInts[6]+5);
		dl1Assoc = pow(2,configurationDimsAsInts[7]);
		il1Sets = pow(2,configurationDimsAsInts[8]+5);
		il1Assoc = pow(2,configurationDimsAsInts[9]);
		ul2Sets = pow(2,configurationDimsAsInts[10]+8);
		ul2BlockSize = pow(2,configurationDimsAsInts[11]+4);
		ul2Assoc = pow(2,configurationDimsAsInts[12]);
		tlbSets = pow(2,configurationDimsAsInts[13]+2);
		dl1Lat = configurationDimsAsInts[14]+1;
		il1Lat = configurationDimsAsInts[15]+1;
		ul2Lat = configurationDimsAsInts[16]+1;
		bpred = configurationDimsAsInts[17];
		
		il1Size = (int)getil1size(configuration);
		dl1Size = (int)getdl1size(configuration);
		ul2Size = (int)getl2size(configuration);			
		
		il1BlockSize = (int)(il1Size/(il1Sets*il1Assoc));	
		dl1BlockSize = (int)(dl1Size/(dl1Sets*dl1Assoc));
		
	}
	
			
};

class TestLoopInfo
{
	public:
	bool l1setsTestDone;
	int il1sets;
	int dl1sets;
	int il1lat;
	int dl1lat;
	int width;

	bool ul2TestDone;	
	int ul2sets;
	int ul2blocksize;
	int ul2assoc;
	int ul2lat;
	
	int bpred;
	
	bool oOoTestDone;
	int RUUsize;
	int LSQsize;
		
	TestLoopInfo( ) {
		l1setsTestDone = false;
		il1sets = 0;
		dl1sets = 0;
		width = 0;
		il1lat = 0;
		dl1lat = 0;

		ul2TestDone = false;
		ul2sets = 0;
		ul2blocksize = 1;
		ul2assoc = 0;
		ul2lat = 1;

		bpred = 0;
	}
	
	void incrementoOoTest() {
		if (RUUsize < 5) {
			RUUsize++;
		}	
		else if (LSQsize < 3) {
			RUUsize = 0;
			LSQsize++;
		}
		else {
			oOoTestDone = true;	
			std::cout << "oOo Test Complete";
			sleep(3);
		}
	}
	
	void incrementul2Test() {
		if (ul2sets < 9) {
			ul2sets++;
		} 
		else if (ul2blocksize < 3) {
			ul2sets = 0;
			ul2blocksize++;
		}	
		else if (ul2assoc < 4) {
			ul2sets = 0;
			ul2blocksize = 1;
			ul2assoc++;
		}
		else if (ul2lat < 6) {
			ul2sets = 0;
			ul2blocksize = 1;
			ul2assoc = 0;
			ul2lat++;
		}  
		else {
			ul2TestDone = true;
			std::cout << "ul2 Test Complete";
			sleep(3);
		}
	}
	void incrementl1setsTest() {
		if (width < 3) {
			width++;
		} else if (il1sets < 8) {
			width = 0;
			il1sets++;	
		} else if (il1lat < 2) {
			width = 0;
			il1sets = 0;
			il1lat++;
		} else if (dl1sets < 8) {
			width = 0;
			il1sets = 0;
			il1lat = 0;
			dl1sets++;	
		} else if (dl1lat < 2) {
			width = 0;
			il1sets = 0;
			il1lat = 0;
			dl1sets = 0;
			dl1lat++; 
		} else {
			l1setsTestDone = true;
			std::cout << "l1 Test Complete";
			sleep(3);

		}	
	}
};

TestLoopInfo testInfo;

/*
 * Returns 1 if valid, else 0
 */
int validateConfiguration(std::string configuration){
	// Get configurations as ints
	int configurationDims[18];
	if(isan18dimconfiguration(configuration)){ // necessary, but insufficient
		extractConfiguration(configuration, configurationDims); // Configuration parameters now available in array
	} else {
		return 0;
	}
	
	Configuration config(configuration, configurationDims);
		
  	// il1 block size must match ifq size
  	// dl1 should have same block size as il1
 	if (config.il1BlockSize != config.width*8) {
		std::cerr << "I$ block size != width*8\n"; 
		return 0; 	
  	}

	if (config.il1BlockSize != config.dl1BlockSize) {
		std::cerr << "I$ block size != D$ block size\n";
		return 0;
	}

	// ul2 block size must be at least twice the il1 (dl1) block size
  	// with a maximum block size of 128B
  	// ul2 must be at least as large as il1+dl1 in order to be inclusive
  	if (config.ul2BlockSize < 2*config.il1BlockSize) {
		std::cerr << "L2 block size is not at least I$ + D$\n";
		return 0;
	}
	if (config.ul2Size < (config.il1Size+config.dl1Size)) {
		std::cerr << "L2 size is not at least I$ + D$\n";
		return 0; 	
	}
  	// il1 sizes and il1 latencies (same for dl1):
  	//	size	latency
	// 	8KB 	1
  	// 	16KB	2
  	// 	32KB  	3  
  	// 	64KB	4
  	// *For direct mapped caches only. 2-way associative add an additional
  	// cycle, and 4-way associative add an additional two cycles
	
	int assocBoost = 0;
	if (config.il1Assoc == 2) {
		assocBoost = 1;
	}
	else if (config.il1Assoc == 4) {
		assocBoost = 2;
	}

	if (config.il1Size > 64*1024) {
		return 0;
	}

	if ((config.il1Size == 8*1024 and config.il1Lat != 1 + assocBoost)
            or (config.il1Size == 16*1024 and config.il1Lat != 2 + assocBoost)
            or (config.il1Size == 32*1024 and config.il1Lat != 3 + assocBoost)
            or (config.il1Size == 64*1024 and config.il1Lat != 4 + assocBoost)) {
		return 0;
	}

	assocBoost = 0;
	if (config.dl1Assoc == 2) {
		assocBoost = 1;
	}
	else if (config.dl1Assoc == 4) {
		assocBoost = 2;
	}
	if (config.dl1Size > 64*1024) {
		return 0;
	}

	if ((config.dl1Size == 8*1024 and config.dl1Lat != 1 + assocBoost)
            or (config.dl1Size == 16*1024 and config.dl1Lat != 2 + assocBoost)
            or (config.dl1Size == 32*1024 and config.dl1Lat != 3 + assocBoost)
            or (config.dl1Size == 64*1024 and config.dl1Lat != 4 + assocBoost)) {
		return 0;
	} 	
  	// ul2 size and ul2 latencies:
  	//	size	latency
  	//	128KB	7
  	//	256KB	8
  	//	512KB	9
  	//	1MB	10
  	//	2MB	11
  	// *For 4-way set associative caches. For 8-way set associative add one additional
  	// cycle, 16-way associative add 2 cycles, 2-way associative subtract 1 cycle,
  	// direct mapped subtract 2 cycles
  	
	assocBoost = 0;
	if (config.ul2Assoc == 8) {
		assocBoost = 1;
	}
	else if (config.ul2Assoc == 16) {
		assocBoost = 2;
	}
	else if (config.ul2Assoc == 1) {
		assocBoost = -1;
	}
	if (config.il1Size > 64*1024) {
		return 0;
	}
	
	/*if ((config.ul2Size == 128*1024 and config.ul2Lat != 7 + assocBoost)
            or (config.ul2Size == 256*1024 and config.ul2Lat != 8 + assocBoost)
            or (config.ul2Size == 512*1024 and config.ul2Lat != 9 + assocBoost)
            or (config.ul2Size == 1024*1024 and config.ul2Lat != 10 + assocBoost)
	    or (config.ul2Size == 2048*1024 and config.ul2Lat != 11 + assocBoost)) {
		std::cerr << "Conflicting L2 cache size calculation";
		return 0;
	}*/

	// fetch:speed <= 2
	if (config.fetchSpeed > 2) {
		return 0;
	}

	// ifqsize <= 8 words
	if (config.width > 8) {
		return 0;
	}

	// memport <= 2
	if (config.memPorts > 2) {
		return 0;
	}
	
 	// ruu:size <= 128
 	if (config.ruuSize > 128) {
		return 0;
	}

	// lsq:size <= 32
	if (config.lsqSize > 32) {
		return 0;
   	}

  	// mplat is fixed at 3
  	// decode:width and issue:width == fetch:ifqsize
  	// mem:width = 8B (memory bus width)
  	// mem:lat = 51 + 7 cycles for 8 word
  	// tlb:lat == 30
  	// max tlb size of 512 entries for 4-way set associative tlb
  	return 1;

}


/*
 * Given the current best known configuration, the current configuration, and the globally visible map of all previously investigated configurations, suggest a previously unexplored design point. You will only be allowed to investigate 1000 design points in a particular run, so choose wisely.
 */
std::string YourProposalFunction(std::string currentconfiguration_s, 
				std::string bestEXECconfiguration_s, 
				std::string bestEDPconfiguration_s, 
				int optimizeforEXEC, int optimizeforEDP){
  
	int currentConfiguration[18];
	int bestEXECConfiguration[18];
	int bestEDPConfiguration[18];
	int nextConfiguration[18];
	extractConfiguration(currentconfiguration_s, currentConfiguration);  
  	extractConfiguration(bestEXECconfiguration_s, bestEXECConfiguration);
	extractConfiguration(bestEDPconfiguration_s, bestEDPConfiguration);
	
	std::cout << bestEXECconfiguration_s + "\n";
	if (optimizeforEXEC) {
		/* Width 		greater width increases clock cycle time
 		 *			but obviously improves throughput to a certain point 		 
 		 *
		 * Fetchspeed    	I see no reason fetch speed shouldn't be at it's max (2)
		 *
		 * Scheduling		In-order will allow lower clock cycle but may weaken   
 		 *              	overall throughput
 		 *
 		 * RUU Size		Register Update Unit - resposible for register renaming,
 		 *			resolving dependencies, readying insts for issue, holds 
 		 *			instructions until they can commit. A bigger RUU will be useful 
 		 *			to a point (in OoO only) but eventually its size may become
 		 *			a concern.
 		 *
 		 * LSQ Size		Load/Store Queue - Similar to RUU, will only be useful in OoO,
 		 * 			but this will help lower memory access times. If we find that
 		 * 			memory access times are becoming a serious burden, this can
 		 * 			help alleviate that. Again, its size may become an issue if it
 		 * 			becomes too large.
 		 *
 		 * Memports		More memports means more availibility to memory accesses, should
 		 * 			always be at its max (2)
 		 *
 		 * L1 D$ Sets +		We can refer to associativity/sets/cache size vs. miss rate curves
 		 * L1 D$ Ways		for this. Additionally we may need to consider the space that larger
 		 * 			caches will take up which will increase access time.  		 
 		 * 			
 		 * L1 I$ Sets +		(Similar to above)
 		 * L1 I$ Ways		
 		 *
 		 * Unified L2 Sets +	(Similar to above) with all the associated oonstraints 
 		 * Unified L2 Blocksize							
 		 * Unified L2 Ways	
 		 * 
 		 * TLB Sets		Transition lookaside buffer - we can use this to help reduce time taken
 		 * 			to access memory. We know tlb:lat is fixed at 30, so we should probably
 		 * 			just make the TLB as large as possible 
 		 * 			
 		 * L1 D$ Latency	This will essentially adjust our cache size, as this is the only remaining
 		 * 			uncontrolled variable which affects latency. This is something that will
 		 * 			change heavily with the type of work our processor is running
 		 *
 		 * L1 I$ Latency	(Similar to above)
 		 *
 		 * Unified L2 Latency	(Similar to above)
 		 *
 		 * Branch Predictor	We can get more accurate branch predictors and the cost of more space (and
 		 * 			therefore most likely more latency). We'll have to experiment with this
 		 * 			variable a lot as it again is heavily dependent on the workload						
 		*/
		for (int i = 0; i < 18; i++) {
			nextConfiguration[i] = bestEXECConfiguration[i];
		} 
						
		dimensions d;	
		nextConfiguration[d = fetchspeed] = 1;
		nextConfiguration[d = Memports] = 1;
		nextConfiguration[d = tlbsets] = 4;
		  			
		// Finding optimality for L2 Cache
		// Can control sets, blocksize, ways, and latency
		// Cache size is inversely related to our chosen latency
		// We know S*A*K=Cache size, so one of our variables is controlled
		// by the other 3. We'll calculate the latency field based on our
		// other three selections.
		// We're also confined by the fact that a smaller L2 cache may restrict our L1 caches
		// because it needs to be inclusive, so we'll start with a reasonable size L2 and
		// evaluate from there up.
		// We also know block size needs to be >= I$ + D$ blocksize, which will be minimum 32. So we can only
		// test block sizes 32, 64, and 128.
		// We'll evaluate the best options from all sets and block sizes, all ways, and
		// latencies 5-11
		if (!testInfo.ul2TestDone) {
			nextConfiguration[d=il1assoc] = 2;
			nextConfiguration[d=dl1assoc] = 2;

			nextConfiguration[d=ul2sets] = testInfo.ul2sets;
			nextConfiguration[d=ul2lat] = testInfo.ul2lat;
			nextConfiguration[d=ul2assoc] = testInfo.ul2assoc;
			nextConfiguration[d=ul2blocksize] = testInfo.ul2blocksize;  
		 	testInfo.incrementul2Test();
		}
			
		// Finding optimality for L$ and D$
		// We're going to neglect using direct-mapped caches as the improvement from 1 to 4 way
		// associative caches is too large to possibly be detremental (in most workloads).
		// We also know the latency is fixed, so any additional hardware associated with a 4 way
		// cache is negligible. We're going to use the most possible sets for every cache.
		// We know block size will decrease with sets and associativity, which hurts the utilization
		// of spatial locality, so we'll have to take this into consideration when messing with the
		// number of sets.
	
		// Since associativity is now fixed, we know the size:sets ratio of D$ must equal I$ ratio
		// We also know there is a minimum size to the L2. For now we'll make it as extreme as possible
		// then tone it down later  
		// The width must also equal 8* the block size of the L1s, so all these things must be changed
		// with consideration  
		else if (!testInfo.l1setsTestDone) {
			// Just gonna brute force this one and let a bunch fail
			nextConfiguration[d=width] = testInfo.width;
			nextConfiguration[d=il1sets] = testInfo.il1sets;
			nextConfiguration[d=il1lat] = testInfo.il1lat;
			nextConfiguration[d=dl1sets] = testInfo.dl1sets;
			nextConfiguration[d=dl1lat] = testInfo.dl1lat;
			testInfo.incrementl1setsTest();
		} 
		
		// Test each branch predictor type
		else if (testInfo.bpred <= 5) {
			nextConfiguration[d=bpred] = testInfo.bpred;
			testInfo.bpred++; 	
		}
		
		// oOo optimality
		// Out-of-order execution will almost always be faster than in-order, but we need to optimize
		// it's resources. This includes the RUU size and LSQ size.
		// We'll brute force to find the best combination here. I have a feeling the most optimal
		// is just going to be the largest in this case. 
		else if (!testInfo.oOoTestDone) {
			nextConfiguration[d=scheduling] = 1;
			nextConfiguration[d=RUUsize] = testInfo.RUUsize;
			nextConfiguration[d=LSQsize] = testInfo.LSQsize;
			testInfo.incrementoOoTest();		
		}
		else {
			std::cout << "Filling with random vals";
			for (int i = 0; i < 18; i++) {
				nextConfiguration[i] = rand()%GLOB_dimensioncardinality[i];
			}	
		}	 	   
				
	}
	else if (optimizeforEDP) {
		// Do some different shit
	}
	
	std::string nextconfiguration;
	// produces an essentially random proposal
  	std::stringstream ss;
  	for(int dim = 0; dim<17; ++dim){
    		ss << nextConfiguration[dim] << " ";
  	} 
  	ss << nextConfiguration[17];
  	nextconfiguration=ss.str();
  	ss.str("");    
  	return nextconfiguration;
}

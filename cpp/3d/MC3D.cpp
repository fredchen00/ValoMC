#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <utility>
#include <iomanip>

// define USE_OMP to utilize threading with OpenMP 
//#define USE_OMP
// define USE_MPI to utilize MPI in multicomputer cluster
//#define USE_MPI
// define USE_HDF5 to use HDF5 input and output files
//#define USE_HDF5

#ifdef USE_OMP
#include <omp.h>
#endif

#include "Array.hpp"
#include "MC3D.hpp"
#include "../version.h"
#include "../fileio.hpp"

MC3D MC;


int LoadProblem(char *fin);
int SaveProblem(char *fout, int );
bool Progress(double perc);



int main(int argc, char **argv){
<<<<<<< HEAD
=======
  printf("--------------ValoMC-3D--------------\n");
  printf("  Version:  %s\n", build_version);
  printf("  Revision: %s\n", build_revision);
#ifdef USE_OMP
  printf("  OpenMP enabled                     \n");
#endif
  printf("-------------------------------------\n");
  printf("\n");

#ifdef USE_OMP
  printf("Using %d threads\n", omp_get_max_threads());
 //  double tstart = omp_get_wtime();
#endif

  MC.seed = (unsigned long) time(NULL);

>>>>>>> 787f97f77cb924d6f4c392cdac92b169ab29771f
  // Display help
  if ((argc < 3))
  {
    printf("Use syntax:\n");
    printf(" MC2D inputfile outputfile\n");
    printf("\n");
    printf("Authors: Aki Pulkkinen, Aleksi Leino and Tanja Tarvainen (2018).\n");
    printf("The simulation code is originally written by Aki Pulkkinen.\n");
    printf("\n");
    return (0);
  }

  printf("--------------ValoMC-3D--------------\n");
  printf("  Version:  %s\n", build_version);
  printf("  Revision: %s\n", build_revision);
#ifdef USE_OMP
  printf("  OpenMP enabled                     \n");
#else
  printf("  OpenMP disabled, no parallelization\n");
#endif
#ifdef USE_OMP
  printf("  Using %d threads\n", omp_get_max_threads());
  //  double tstart = omp_get_wtime();
#endif
  printf("-------------------------------------\n");
  printf("\n");



  MC.seed = (unsigned long) time(NULL);

  char *fin = argv[1];
  char *fout = argv[2];

  printf("Loading problem %s\n", fin);
  if(LoadProblem(fin)){
    printf("Error while loading!\n");
    return(1);
  }

  printf("Initializing MC3D\n");
  int  tstart = (unsigned long) time(NULL);

  // [AL]
  try {
    MC.ErrorChecks();
    MC.Init();
  } catch (mcerror e) {
    printf("MC Initialization failed!: Reason %s\n", errorstring(e));
    return 1;
  }

  int tend = (unsigned long) time(NULL);
  printf("Initialization took %i seconds\n", tend - tstart);
  printf("Computing...\n");
  tstart = (unsigned long) time(NULL);
  MC.MonteCarlo(Progress);
  tend = (unsigned long) time(NULL);
  
  printf("Saving problem\n");
  if(SaveProblem(fout, tend-tstart)){
    printf("Error while saving!\n");
    return(1);
  }

  if(MC.loss) printf(" %ld photons lost during computation!\n", MC.loss);

#ifdef USE_OMP
  // double tend = omp_get_wtime();
#endif
  printf("Computation took %i seconds\n", tend - tstart);

  return(0);
}


int LoadProblem_TXT(char *fin){
  /*
    File Structure for text input:

      Ne Nb Nr Nphoton
      f
      H
      BH
      r
      mua mus g n
      BCType
      [BCn]
      [BCLNormal] -- if provided, BCn has to be provided as well
      [BCLightDirectionType] // [AL]
      [BCIntensity] // [AL]
      [GaussianSigma] //[AL]
  */

  int_fast64_t ii;
  int_fast64_t Ne, Nb, Nr;
  int fsr; // return value for file open (for avoiding compiler warnings)

  FILE *fp = fopen(fin, "r");
  if(fp == NULL) return(1);

  fsr=fscanf(fp, "%lli %lli %lli %lli\n", &Ne, &Nb, &Nr, &MC.Nphoton);
  fsr=fscanf(fp, "%lf %lf\n", &MC.f, &MC.phase0);

  char tmpline[5012]; // skip a line
  fgets(tmpline, 5011, fp);
   
  // make negative phase0 positive by adding a multiple of 2*pi
  if(MC.phase0 < 0) {
    MC.phase0 += 2*M_PI*ceil(-MC.phase0 / (2*M_PI));
  }
     
  printf("Constants:\n");
  printf("  %10s   (%e)\n", "f", MC.f);
  printf("  %10s   (%e)\n", "phase0", MC.phase0);
  printf("  %10s   (%lli)\n", "Ne", Ne);
  printf("  %10s   (%lli)\n", "Nb", Nb);
  printf("  %10s   (%lli)\n", "Nr", Nr);
  printf("  %10s   (%lli)\n", "Nphoton", MC.Nphoton);
  printf("Arrays:\n");

  // make negative phase0 positive by adding a multiple of 2*pi
  if (MC.phase0 < 0)
  {
    MC.phase0 += 2 * M_PI * ceil(-MC.phase0 / (2 * M_PI));
  }

  readAndResize(fp, (int)Ne, 4, true, &MC.H, "H");
  readAndResize(fp, (int)Nb, 3, true, &MC.BH, "BH");
  readAndResize(fp, (int)Nr, 3, true, &MC.r, "r");
  readAndResize(fp, (int)Ne, 1, true, &MC.mua, &MC.mus, &MC.g, &MC.n, "mua", "mus", "g", "n");
  readAndResize(fp, (int)Nb, 1, true, &MC.BCType, "BCType");
  readAndResize(fp, (int)Nb, 1, false, &MC.BCn, "BCn");
  readAndResize(fp, (int)Nb, 3, false, &MC.BCLNormal, "BCLNormal");
  readAndResize(fp, (int)Nb, 1, false, &MC.BCLightDirectionType, "BCLDirType");
  readAndResize(fp, (int)Nb, 1, false, &MC.BCIntensity, "BCIntensity");

  fclose(fp);

  return(0);
}

int LoadProblem(char *fin){
  return( LoadProblem_TXT(fin) );
}


int SaveProblem_TXT(char *fout, int time){
  int ii;

  FILE *fp = fopen(fout, "w");
  if(fp == NULL) return(1);

  fprintf(fp, "%i 0\n", time);
  
  for(ii = 0; ii < MC.ER.Nx; ii++) fprintf(fp, "%18.10e %18.10e\n", MC.ER[ii], MC.EI[ii]);
  for(ii = 0; ii < MC.EBR.Nx; ii++) fprintf(fp, "%18.10e %18.10e\n", MC.EBR[ii], MC.EBI[ii]);
  
  fclose(fp);


  return(0);
}

int SaveProblem(char *fin, int time){
  return( SaveProblem_TXT(fin, time) );
}



bool Progress(double perc){
  printf("  %f %%\r", perc);
  fflush(stdout);
  return true;
}

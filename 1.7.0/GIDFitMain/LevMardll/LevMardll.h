// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LEVMARDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LEVMARDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LEVMARDLL_EXPORTS
#define LEVMARDLL_API __declspec(dllexport)
#else
#define LEVMARDLL_API __declspec(dllimport)
#endif


extern "C" LEVMARDLL_API double GIDFit(int numberofGID, double parameters[], int paramsize, double QRange[], int QSize, double GIDpoints[], int GIDsize,
										double IndividGraphs[], int IndividGraphslength, double covariance[], int covarsize, double GIDRealPoints[], double GIDErrors[], double info[]);
extern "C" LEVMARDLL_API void GIDGenerate(int numberofGID, double parameters[], int paramsize, double QRange[], int QSize, double GIDpoints[], int GIDsize, double IndividGraphs[], int IndividGraphslength);

/* double precision LM, with & without jacobian */
/* unconstrained minimization */
extern "C" LEVMARDLL_API int dlevmarder(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata);

extern "C" LEVMARDLL_API int dlevmardif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata);

/* box-constrained minimization */
extern "C" LEVMARDLL_API int dlevmar_bcder(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       void (*jacf)(double *p, double *j, int m, int n, void *adata),  
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar,double *adata);

extern "C" LEVMARDLL_API int dlevmar_bcdif(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar, double *adata);

//Need LAPACK for compilation
/* linear equation constrained minimization */

extern "C" LEVMARDLL_API int dlevmar_lecder(void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata);

extern "C" LEVMARDLL_API int dlevmar_lecdif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata);



/* single precision LM, with & without jacobian */
/* unconstrained minimization */
extern "C" LEVMARDLL_API int slevmarder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmardif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata);

/* box-constrained minimization */
extern "C" LEVMARDLL_API int slevmar_bcder(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       void (*jacf)(float *p, float *j, int m, int n, void *adata),  
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmar_bcdif(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata);


/* linear equation constrained minimization */
extern "C" LEVMARDLL_API int slevmar_lecder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmar_lecdif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata);

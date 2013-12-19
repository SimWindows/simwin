/*FORMULC.C 2.2           as of 12/13/94        */
/* definitive  version */
/*A fast interpreter of mathematical functions */

/*Copyright (c) 1995 by Harald Helfgott        */
/* This program must be distributed with its corresponding README.DOC */
/* The full copyright and availability notice is in README.DOC	      */
/* 	This program is provided "as is", without any explicit or */
/* implicit warranty. */


/* Programmer's Address:
	    Harald Helfgott
	    MB 1807, Brandeis University
	    P.O. Box 9110
	    Waltham, MA 02254-9110
	    U.S.A.
		hhelf@cs.brandeis.edu
		   OR
	     (during the summer)
	    2606 Willett Apt. 427
	    Laramie, Wyoming 82070
	    seiere@uwyo.edu */
/* Differences between versions 2.1 and 2.2:
       1) FORMULC now runs in machines which don't like unaligned doubles.
       It was necessary to encapsulate the coded-function type;
       now, you must use formu instead of UCHAR.
       2) FORMULC 2.2 has a main function which can be activated
       by defining STAND_ALONE.
       3) FORMULC 2.2 gives error messages.
           
       Modification 2) is due to Ralf Grosse Kunstleve;
       Modification 1) is due to Mr. Grosse and the author.
       Mr. Grosse's address: ralf@kristall.erdw.ethz.ch
       Thanks, Ralf!                                               

   Differences between the September 1994 and the December 1994 versions:
     read CHANGES.DOC                                     */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "formulc.h"

#undef FORMU_DEBUG_ON
/* Substitute #define for #undef to trace evaluation */
#ifdef FORMU_DEBUG_ON
 #define DBG(anything)  anything
#else
 #define DBG(anything)
  /* nothing */
#endif

static double pi(void);

static double value(formu function);

static const char *i_error; /*pointer to the character in source[]
			that causes an error */
#define Max_ctable 255
   /*maximum number of items in a table of constants */
   /* Max_ctable must be less than 256 */
static int i_pctable; /* number of items in a table of constants -
                         used only by the translating functions */
static double *i_ctable; /*current table of constants -
                           used only by the translating functions */
static UCHAR *i_trans(UCHAR *function, char *begin, char *end);
static char  *my_strtok(char *s);
static UCHAR *comp_time(UCHAR *function, UCHAR *fend, int npars);

static char *errmes = NULL;
static void fset_error(char *);

static double param['z'-'a'+1];
typedef struct {
  char *name;
  Func f;    /* pointer to function*/
  int n_pars; /* number of parameters (0, 1, 2 or 3) */
  int varying; /* Does the result of the function vary
		  even when the parameters stay the same?
		  varying=1 for e.g. random-number generators. */ 
} formu_item;
#define TABLESIZE 256
#define STD_LIB_NUM 13
static formu_item ftable[TABLESIZE]=
{
  {"exp", exp,1,0},
  {"ln",  log,1,0},
  {"sin", sin,1,0},
  {"cos", cos,1,0},
  {"tan", tan,1,0},
  {"asin", asin,1,0},
  {"acos", acos,1,0},
  {"atan", atan,1,0},
  {"atan2",(Func) atan2,2,0},
  {"abs",  fabs,1,0},
  {"sqrt",  sqrt,1,0},
  {"pi", (Func) pi,0,0},
  {"rnd", (Func) rnd, 0, 1}, /*returns a random number from 0 to 1 */
  {NULL,NULL,0}};
/* please, don't use this array directly;
   methods are provided for its manipulation */

/*********************************************************/
/* The following routines manipulate the table of functions */

int read_table(int i, char *name, int *n_pars, int *varying)
/* returns 1 if succesful */
/* returns 0 otherwise */
{
 if(!ftable[i].f) {
  fset_error("index out of bounds"); 
  return 0;
 }
 else {
  strcpy(name,ftable[i].name);
  *n_pars = ftable[i].n_pars;
  *varying = ftable[i].varying;
  fset_error(NULL);
  return 1;
 }
}

int where_table(char *name)
/* If the function exists, where_table() returns the index of its name
    in the table. Otherwise, it returns -1. */
{
 formu_item *table_p;

 for(table_p=ftable; table_p->f != NULL &&
	strcmp(name,table_p->name); table_p++)
   ;
 if(table_p->f == NULL) /*The end of the table has been reached,
		 but name[] is not there. */
  {
    fset_error("function not found");
    return -1;
  }
 else {
   fset_error(NULL);
   return table_p - ftable;
 }
}

int fdel(char *name)
/* If the function exists, it is deleted and a non-negative value
    is returned. */
/* Otherwise, -1 is returned. */
/* Original library functions may not be deleted. */
{
 int place;
 formu_item *scan;

 if((place=where_table(name)) == -1) 
  return -1; /* there is an error message already */
 if(place<STD_LIB_NUM) {
  fset_error("original functions may not be deleted");
  return -1;
 }
 free(ftable[place].name);
 for(scan = &ftable[place]; scan->f!=NULL; scan++) {
  DBG(printf("%s \t",scan->name));
  scan->name  =  (scan+1)->name;
  scan->f     =  (scan+1) -> f;
  scan->n_pars = (scan+1) -> n_pars;
 }
 fset_error(NULL);
 return scan-ftable;
} /*end of fdel */

int fnew(char *name, Func f, int n_pars, int varying)
/* 0 is rendered if there is an error */
/* 1 is rendered otherwise */
{
 formu_item *where;

 if(n_pars<0 || n_pars>3) {
  fset_error("invalid number of parameters"); 
  return 0;
 }
 for(where=ftable; where->f != NULL && strcmp(name,where->name); where++);
 if(where->f != NULL) {
  where->f=f;
  where->varying = varying;
  where->n_pars = n_pars;   /*old function is superseded */
  fset_error(NULL);
  return 1;
 } else if((where-ftable) >= TABLESIZE-1) {
  fset_error("function table full");
  return 0; 
 }
 else {
  where->name = (char *) calloc(strlen(name)+1, sizeof(char));
  if(where->name==NULL) {
    fset_error("no memory");
    return 0;
  }
  strcpy(where->name,name);
  where->f=f;
  where->varying = varying;
  where->n_pars = n_pars;
  fset_error(NULL);
  return 1;
 }
}  /* end of fnew */

/***********************************************************/
/* Error functions                                         */

static void fset_error(char *s)
/* fset_error(NULL) and fset_error("") erase 
   any previous error message */
{
 if (s == NULL || *s == '\0') errmes = NULL; /* an empty error message means
		                		   that there is no error */
 else errmes = s;
}

const char *fget_error(void)
{
 return errmes;
}

/**********************************************************/
/* Evaluating functions                                   */

double fval_at(formu function)
{
  fset_error(NULL);
  return value(function);
}

void make_var(char var, double value)
/*for use with fval_at */
/* make_var('x',3); makes x=3 */
{
  param[var-'a']=value;
}

double f_x_val(formu function, double x)
{
 fset_error(NULL);
 param['x'-'a']=x;
 return value(function);
}

/***********************************************************************************************
Modification made by Dave Winston (winston@barley.colorado.edu)

rewrote fval() to take a pointer to values instead of a variable number of arguments.
Below is the original function.

double fval(formu function, char *args, ...)
{
 va_list ap;
 double result;

 DBG(puts("Enter fval"));
 fset_error(NULL);
 va_start(ap, args);
 while(*args)
  param[(*args++)-'a'] = va_arg(ap, double);
 va_end(ap);
 result=value(function);
 return result;
}
*/
double fval(formu function, char *args, double *values)
{
 double result;

 DBG(puts("Enter fval"));
 fset_error(NULL);
 while(*args)
  param[(*args++)-'a'] = *(values++);
 result=value(function);
 return result;
}
/***********************************************************************************************/

#define BUFSIZE 500
/* bufsize is the size of the stack for double value(formu func) */
static double value(formu func)
{
 double buffer[BUFSIZE];
 register double *bufp = buffer;
	  /* points to the first free space in the buffer */
 double x,y,z;
 register double result;
 register UCHAR *function=func.code;
 register double *ctable=func.ctable;

 DBG(puts("Entering value"));
 if(!function) {
   fset_error("empty coded function");
   return 0; /* non-existent function; result of
				an unsuccesful call to translate */
 }
 for(;;) {
   switch(*function++) {
	case '\0':goto finish; /* there is a reason for this "goto":
				  this function must be as fast as possible */
	case 'D': *bufp++ = ctable[*function++];
		  DBG(printf("%g ",ctable[*(function-1)]));
		  break;
	case 'V': *bufp++ = param[(*function++)-'a'];
		  DBG( printf("(%c = %g)   ",*(function-1),*(bufp-1)) );
		  break;
	case 'M':DBG(printf("(Unary -) "));
		 result = -(*--bufp);
		 *bufp++ = result;
		 break;
	case '+':DBG(printf("+ "));
		 y = *(--bufp);
		 result = y + *(--bufp);
		 *bufp++ = result;
	  break;
	case '-':DBG(printf("- "));
		 y = *--bufp;
		 result= *(--bufp) - y;
		 *bufp++ = result;
		 break;
	case '*':DBG(printf("* "));
		 y = *(--bufp);
		 result = *(--bufp) * y;
		 *bufp++ = result;
		 break;
	case '/':DBG(printf("/ "));
		 y = *--bufp;
		 result = *(--bufp) / y;
		 *bufp++ = result;
		 break;
	case '^':DBG(printf("^ "));
		 y = *--bufp;
		 result = pow(*(--bufp),y);
		 *bufp++ = result;
		 break;
	case 'F':DBG(printf("%s ",ftable[*function].name));
	     switch(ftable[*function].n_pars) {
	       case 0:*bufp++ = ((Func0)ftable[*function++].f)();
		      break;
	       case 1:x = *--bufp;
		      *bufp++ = ftable[*function++].f(x);
		      break;
	    case 2:y = *--bufp;
		   x = *--bufp;
		   *bufp++ = ((Func2)ftable[*function++].f)(x,y);
		      break;
	       case 3:z = *--bufp;
			  y = *--bufp;
		      x = *--bufp;
		      *bufp++ = ((Func3)ftable[*function++].f)(x,y,z);
		      break;
	       default:fset_error("I2: too many parameters\n");
		       return 0;
	      }
	     break;
    default:fset_error("I1: unrecognizable operator");
	 return 0;
   }
 }
 finish: if((bufp-buffer)!=1)
	  {
		DBG(putchar('\n'));
		fset_error("I3: corrupted buffer");
		DBG(printf("Buffer: "));
		DBG(while(bufp-- > buffer))
		  DBG(printf("%g ",*bufp));
	   DBG(putchar('\n'));
	 }
 DBG(else putchar('\n'));
 return buffer[0];
} /* end of value */

/**********************************************************/
/* Manipulation of data of type formu                     */

void destrf(formu old)
{
 fset_error(NULL);
 free(old.code);
 free(old.ctable);
}

/***********************************************************************************************
Modification by David Winston (winston@barley.colorado.edu)

Rewrote the following function to take a pointer to f.
Below is the original function.

void make_empty(formu f)
{
 fset_error(NULL);
 f.code=NULL;
 f.ctable=NULL;
}
*/
void make_empty(formu *f)
{
 fset_error(NULL);
 f->code=NULL;
 f->ctable=NULL;
}
/***********************************************************************************************/


int fnot_empty(formu f)
{
 fset_error(NULL);
 return(f.code!=NULL);
}

/*********************************************************/
/* Interpreting functions                                */

static int isoper(char c)
{
 return ((c == '+') || (c == '-') || (c == '*') || (c == '/')
			|| (c == '^'));
}

static int is_code_oper(UCHAR c)
{
 return ((c == '+') || (c == '-') || (c == '*') || (c == '/')
			|| (c == '^') || (c == 'M'));
}
static int isin_real(char c)
/* + and - are not included */
{
 return (isdigit(c) || c=='.' || c=='E');
}

size_t max_size(const char *source)
/* gives an upper estimate of the size required for
   the coded form of source (including the final '\0') */
/* Take care when modifying: the upper estimate
   returned by max_size must not also accomodate
   *proper* output, but also *improper* output
   which takes place before the translator detects an error. */
{
 int numbers=0;
 int functions=0;
 int operators=0;
 int variables=0;

/* const size_t func_size=2*sizeof(UCHAR); */ /* not needed */
 const size_t var_size=2*sizeof(UCHAR);
 const size_t num_size=sizeof(UCHAR)+sizeof(double);
 const size_t op_size=sizeof(UCHAR);
 const size_t end_size=sizeof('\0');

 const char *scan;

 for(scan=source; *scan; scan++)
  if(isalpha(*scan) && (*scan != 'E'))
  {
	if(isalpha(*(scan+1))) ; /* it is a function name,
				it will be counted later on */
	else
	 if(*(scan+1) == '(')  functions++;
	 else variables++;
  }

 if(isoper(*source)) operators++;
 if(*source != '\0')
  for(scan = source+1; *scan; scan++)
   if(isoper(*scan) && *(scan-1) != 'E') operators++;

 /* counting numbers.. */
 scan=source;
 while(*scan)
  if(isin_real(*scan) || ((*scan == '+' || *scan == '-') &&
			   scan>source && *(scan-1)=='E'))
   {numbers++;
    scan++;
    while(isin_real(*scan) || ((*scan == '+' || *scan == '-') &&
				scan>source && *(scan-1)=='E'))
     scan++;
   }
  else scan++;

 return(numbers*num_size + operators*op_size + functions*num_size
			 + variables*var_size + end_size);
 /*Do you wonder why "function" is multiplied with "num_size"
   and not with func_size? This function calculates an upper-bound
   (i.e. pessimistic) estimate. It supposes that all functions are
   converted into doubles by comp_time. For example, pi() actually
   becomes a double. */
}

/***********************************************************/
/* Interface for interpreting functions                     */

formu translate(const char *sourc, const char *args, int *leng,          
                                                        int *error)
{
 UCHAR *result;
 char *source;
 const char *scan, *scarg;
 UCHAR *function;
 UCHAR *nfunc; /* used to free unused heap space */
 size_t size_estim; /* upper bound for the size of the
					coded function */
 double *ctable;
 formu returned; /*the value to be returned by the function
		   is stored here */


 i_error=NULL;

 source = (char *) malloc(strlen(sourc) + 1);
 if(source==NULL) {
   fset_error("no memory");
   *leng = 0;
   *error = 0; /* first character */
   returned.code = NULL;
   returned.ctable = NULL;
   return(returned);
 }
 strcpy(source,sourc);
 /* FORMULC's routines must have their own copy of sourc
    because the copy could be temporarily modified.
    Modifying a string constant can make some computers crash. */

 /* search for undeclared parameters */
 for(scan=source; *scan != '\0'; scan++) {
  if(islower(*scan) && !isalpha(*(scan+1)) &&
      (scan==source || !isalpha(*(scan-1))) ) {
   for(scarg=args; *scarg != '\0' && *scarg != *scan; scarg++)
     ;
   if(*scarg == '\0') /*parameter not found */
    {
     i_error = scan;

	 fset_error("undeclared parameter");
     *leng = 0;
     *error = i_error - source;
     returned.code=NULL;
     returned.ctable=NULL;
     free(source);
     return(returned);
    }
  }
 }  /* end of search for undeclared... */

 size_estim=max_size(source); /* upper estimate of the size
				 of the coded function,
				 which doesn't exist yet */

 if(!(function = (UCHAR *) malloc(size_estim))) {
  /* out of memory */
  fset_error("no memory");
  *leng = 0;
  *error = -1;
  returned.code=NULL;
  returned.ctable=NULL;
  free(source);
  return (returned);
 }

 /*table of memory is cleaned: */
 i_pctable=0;   
 if(!(i_ctable = (double *) malloc(Max_ctable * sizeof(double)) )) {
  /* out of memory */
  fset_error("no memory"); 
  free(function);
  *leng = 0;
  *error = -1;
  returned.code=NULL;
  returned.ctable=NULL;
  free(source);
  return (returned);
 }
 ctable = i_ctable;

 fset_error(NULL);
 /* THIS IS THE CORE STATEMENT */
 result=i_trans(function,(char *) source,(char *) source+strlen(source));
 
 if(!result || fget_error()) { 
  free(function);
  free(i_ctable);
  *leng = 0;
  if(i_error)
   *error = i_error-source;
  else *error = -1; /* internal error or out of memory */
  returned.code=NULL;
  returned.ctable=NULL;
  free(source);
  return (returned);
 }
 else { /* OK */
  *result = '\0';
  *error = -1;
  *leng = result-function;

  /* free unused heap space.. */
  if(((*leng)+1) * sizeof(UCHAR) > size_estim)
   /* one must use (*leng)+1 instead of *leng because '\0'
      has not been counted */
   { 
	fset_error("I4: size estimate too small");
    returned.code=NULL;
    returned.ctable=NULL;
    free(source);
    return (returned);
   }
  else if(((*leng)+1) * sizeof(UCHAR) < size_estim) {
    nfunc = (UCHAR *) malloc(((*leng)+1) * sizeof(UCHAR));
      if(nfunc) {
	memcpy( nfunc, function, ((*leng)+1) * sizeof(UCHAR) );
	free(function);
	function=nfunc;
      }
  } /* end of if-else stairs */

  /* free heap space hoarded by i_ctable.. */
  if(i_pctable<Max_ctable) {
    ctable = (double *) malloc(i_pctable * sizeof(double));
    if(ctable) {
      memcpy(ctable, i_ctable, i_pctable * sizeof(double));
	  free(i_ctable);
    } else ctable = i_ctable;
  } else ctable = i_ctable;
 
  returned.code=function;
  returned.ctable=ctable;
  fset_error(NULL);
  free(source);
  return(returned);
 } /* end of OK */
}  /* end of translate */

static UCHAR *comp_time(UCHAR *function, UCHAR *fend, int npars)
  /* calculates at "compile time" */
  /* Postconditions: If the coded expression in *function..*(fend-1)
      can be calculated, its value is stored in *function..*(fend-1) */
  /* comp_time returns a pointer to the first character after the
     end of the coded function; if this function cannot be evaluated
     at compile time, comp_time returns fend, of course.  */
  /* Only memory positions from *function to *comp_time are touched. */
{
  UCHAR *scan;
  UCHAR temp;
  double tempd;
  int i;
  formu trans;

  DBG(puts("Entering comp_time"));
  scan=function;
  for(i=0; i<npars; i++) {
   if(*scan++ != 'D') return fend;
   scan++;
  }

  if(!( ( scan == fend - (sizeof((UCHAR) 'F')+sizeof(UCHAR))
	   && *(fend-2) == 'F' && ftable[*(fend-1)].varying == 0) ||
	 ( scan == fend - sizeof(UCHAR)
	   && is_code_oper(*(fend-1)) ) )
	)
    /* compile-time evaluation is done only if 
       1) everything but the ending function consists of doubles
       AND
       2) the function does not vary when its parameters remain the same
          (i.e. random-number generators are not evaluated at compile time)
	  */
   return fend;

  temp = *fend;
  *fend = '\0';
   
  trans.code=function;
  trans.ctable=i_ctable;
  tempd = value(trans);
  *fend = temp;
  *function++ = 'D';
  i_pctable -= npars;
  *function++ = (UCHAR) i_pctable;
  i_ctable[i_pctable++] = tempd;

  DBG(puts("Exiting comp_time succesfully"));
  return function;
} /* end of comp_time */

static char *my_strtok(char *s)
/* a version of strtok that respects parentheses */
/* token delimiter = comma */
{
 int pars;
 static char *token=NULL;
 char *next_token;

 if(s!=NULL) token=s;
 else if(token!=NULL) s=token;
 else return NULL;

 for(pars=0; *s != '\0' && (*s != ',' || pars!=0); s++) {
   if(*s == '(') ++pars;
   if(*s == ')') --pars;
 }
 if(*s=='\0') {
  next_token=NULL;
  s=token;

  token=next_token;
  DBG(printf("The token is: %s\n",s));
  return s;
 } else {
  *s = '\0';
  next_token=s+1;
  s=token;

  token=next_token;
  DBG(printf("The token is: %s\n",s));
  return s;
 }
} /* end of my_strtok */


/************************************************************/
/* Here begins the core of interpretation                   */

#define TWO_OP {                                 \
    if((tempu=i_trans(function,begin,scan)) &&      \
       (temp3=i_trans(tempu,scan+1,end)) ) {       \
    *temp3++ = *scan; /* copies operator */                 \
    temp3 = comp_time(function,temp3,2); /*tries to simplify expression*/ \
   if(fget_error()) return NULL; /* internal error in comp_time */  \
   else return temp3; /* expression has been translated */ \
  } else return NULL; /* something is wrong with the operands */ \
 }

#define ERROR_MEM {    \
   fset_error("no memory"); \
   i_error=NULL;  \
   return NULL;   \
  }
static UCHAR *i_trans(UCHAR *function, char *begin, char *end)
 /* the source is *begin .. *(end-1) */
 /* returns NULL if a normal error or an internal error occured; 
    otherwise, returns a pointer 
    to the first character after the end of function[] */
 /* i_trans() does not write a '\0' at the end of function[], */
 /* but it MAY touch its end (i.e. *i_trans) without changing it.*/
{
 int pars;     /* parentheses */
 char *scan;
 UCHAR *tempu, *temp3;
 char *temps;
 char tempch;
 double tempd;
 char *endf;     /* points to the opening
		    parenthesis of a function (e.g. of sin(x) ) */
 int n_function;
 int space;
 int i;

 char *paramstr[MAXPAR];
 char *par_buf;

 if(begin>=end) {
  fset_error("missing operand"); 
  i_error = begin;
  return NULL;
 }

 DBG(tempch = *end);
 DBG(*end = '\0');
 DBG(puts(begin));
 DBG(*end = tempch);

 /* test paired parentheses */
 for(pars=0, scan=begin; scan<end && pars>=0; scan++) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
 }
 if(pars<0 || pars>0) {
  fset_error("unmatched parentheses"); 
  i_error = scan-1;
  return NULL;
 }

 /* plus and binary minus */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '+' || ((*scan == '-') && scan!=begin))
					  /* recognizes unary
					     minuses */
	     && (scan==begin || *(scan-1)!='E') )
	  /* be wary of misunderstanding exponential notation */
   break;
 }

 if(scan >= begin) TWO_OP

 /* multiply and divide */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '*' || *scan == '/' ))
   break;
 }

 if(scan >= begin) TWO_OP

 /* unary minus */
 if(*begin == '-') {
   tempu=i_trans(function,begin+1,end);
   if(tempu) {
     *tempu++ = 'M';
     tempu=comp_time(function,tempu,1); /*tries to simplify
					  expression*/
     if(fget_error()) return NULL; /* internal error in comp_time */
     else return tempu;
   } else return NULL;
 }

 /* power */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '^'))
   break;
 }

 if(scan >= begin) TWO_OP

 /* erase white space */
 while(isspace(*begin))
  begin++;
 while(isspace(*(end-1)))
  end--;

 /* parentheses around the expression */
 if(*begin == '(' && *(end-1) == ')')
  return i_trans(function,begin+1,end-1);

 /* variable */
 if(end == begin+1 && islower(*begin)) {
  *function++ = 'V';
  *function++ = *begin;
  return function;
 }

 /* number */
 tempch = *end;
 *end = '\0';
 tempd=strtod(begin,(char**) &tempu);
 *end = tempch;
 if((char*) tempu == end) {
  *function++ = 'D';
  if (i_pctable < Max_ctable)
	{
      i_ctable[i_pctable] = tempd;
      *function++ = (UCHAR) i_pctable++;
    }
  else
    {
      fset_error("too many constants");
      i_error=begin;
      return NULL;
	}
  return function;
 }

 /*function*/
 if(!isalpha(*begin) && *begin != '_')
			/* underscores are allowed */
 {
  fset_error("syntax error"); 
  i_error=begin;
  return NULL;
 }
 for(endf = begin+1; endf<end && (isalnum(*endf) || *endf=='_');
							   endf++);
 tempch = *endf;
 *endf = '\0';
 if((n_function=where_table(begin)) == -1) {
  *endf = tempch;
  i_error=begin;
  /* error message has already been created */
  return NULL;
 }
 *endf = tempch;
 if(*endf != '(' || *(end-1) != ')') {
  fset_error("improper function syntax"); 
  i_error=endf;
  return NULL;
 }
 if(ftable[n_function].n_pars==0) {
  /*function without parameters (e.g. pi() ) */
   space=1;
   for(scan=endf+1; scan<(end-1); scan++)
    if(!isspace(*scan)) space=0;
   if(space) {
    *function++ = 'F';
    *function++ = n_function;
    function = comp_time(function-2,function,0);
    if(fget_error()) return NULL; /* internal error in comp_time */
    else return function;
   } else {
    i_error=endf+1;
    fset_error("too many parameters");
    return NULL;
   }
 } else {    /*function with parameters*/
	tempch = *(end-1);
    *(end-1) = '\0';
    par_buf = (char *) malloc(strlen(endf+1)+1);
    if(!par_buf)
	 ERROR_MEM;
    strcpy(par_buf, endf+1);
    *(end-1) = tempch;
    /* look at the first parameter */
    for(i=0; i<ftable[n_function].n_pars; i++) {
     if( ( temps=my_strtok((i==0) ? par_buf : NULL) ) == NULL )
      break; /* too few parameters */
     paramstr[i]=temps;
    }
	if(temps==NULL) {
     /* too few parameters */
     free(par_buf);
     i_error=end-2;
     fset_error("too few parameters");
     return NULL;
    }
    if((temps=my_strtok(NULL))!=NULL) {
     /* too many parameters */
     free(par_buf);
     i_error=(temps-par_buf)+(endf+1); /* points to the first character
					  of the first superfluous
					  parameter */
     fset_error("too many parameters");
     return NULL;
    }

    tempu=function;
    for(i=0; i<ftable[n_function].n_pars; i++)
     if(!(tempu=i_trans( tempu, paramstr[i],
				 paramstr[i]+strlen(paramstr[i]) ) ) )
     {
      i_error=(i_error-par_buf)+(endf+1); /* moves i_error to
					   the permanent copy of the
					   parameter */
      free(par_buf);
      return NULL; /* error in one of the parameters */
     }
    /* OK */
	free(par_buf);
    *tempu++ = 'F';
    *tempu++ = n_function;
    tempu = comp_time(function,tempu,ftable[n_function].n_pars);
    if(fget_error()) return NULL; /* internal error in comp_time */
    else return tempu;
 }
}
 
/**************************************************************/
/* Here begins the stand-alone part */ 

 #ifdef STAND_ALONE
  /* this part of FORMULC enables it to work as a UNIX filter. */
  
 static char *progn = "formulc";

 
 #define CtrlZ 0x001A
 
 static int fgetline(FILE *fpin, char s[], int size_s)
 {
   int         last_s, c, i;
 
   last_s = size_s - 1;
 
   i = 0;
 #ifdef __MSDOS__
   while ((c = getc(fpin)) != EOF && c != CtrlZ && c != '\n')
 #else
   while ((c = getc(fpin)) != EOF && c != '\n')
 #endif
     if (i < last_s) s[i++] = c;
 
   s[i] = '\0';
 
 #ifdef __MSDOS__
   if (i == 0 && (c == EOF || c == CtrlZ))
 #else
   if (i == 0 && c == EOF)
 #endif
     return 0;
   else
     return 1;
 }

 static int strarg(char *arg, char *s, int n)
                                       /* - string argument -            */
 {                                     /* return n-th argument of string */
   char *a = arg;                      /* s in arg.                      */
                                       /* n = 0 for the first argument.  */
   while (*s == ' ' || *s == '\t') s++;   /* Blank and Tab are seperators.*/
 
   while (n--)
   { while (*s != '\0' && *s != ' ' && *s != '\t') s++;
     while (*s == ' ' || *s == '\t') s++;
   }
 
   while (*s != '\0' && *s != ' ' && *s != '\t') *a++ = *s++;
   *a = '\0';
 
   return (a != arg);
 }
 
 static int is_legal_c_format(char *cfmt)
 {
   int   count, fmt_on, fmt_width;
   char  *start_digits;


   count = 0;
   start_digits = NULL;

   for (fmt_on = fmt_width = 0; *cfmt; cfmt++)
   {
     if (isdigit(*cfmt))
     {
       if (start_digits == NULL)
		   start_digits = cfmt;
     }

     if (fmt_on == 0)
     {
       if (*cfmt == '%')
       {
         fmt_on = 1;
         fmt_width = 0;
	   }
     }
     else
     {
       if      (*cfmt == '%')
       {
         if (fmt_width) return 0;
         fmt_on = 0;
       }
       else if (*cfmt == '$')
       {
         if (start_digits == NULL) return 0;
         if (cfmt - start_digits != 1) return 0;
         if (*start_digits != '1') return 0;
       }
       else if (*cfmt == '*')
		 return 0;
       else if (isalpha(*cfmt))
       {
         if (strchr("feEgG", *cfmt) == NULL) return 0;
		 fmt_on = 0;
         count++;
       }

       fmt_width++;
     }

     if (isdigit(*cfmt) == 0)
       start_digits = NULL;
   }

   if (fmt_on || count != 1) return 0;

   return 1;
 }


 static void usage(void)
 {
   fprintf(stderr,
     "usage: %s \"function\" \"arguments\" [\"c-format\"]\n",
     progn);
   exit(1);
 }


 int main(int argc, char *argv[])
 {
   int            error, ivar, n, i;
   formu          function;
   char           *args, *e_args, buf[256], svar[sizeof buf], xtrac;
    /* args = table of variables of function;
       this table is an argument of translate and of fval */
   double         var, result;
   long           linec;


   if (argc != 3 && argc != 4) usage();
   if(argc == 4 && is_legal_c_format(argv[3]) == 0)
   {
     fprintf(stderr, "%s: Invalid format string\n", progn);
     exit(1);
   }

   n = strlen(argv[1]);
   if (n == 0)
     usage();   
    
   n = 0;
   e_args = argv[2];
   while (*e_args)
   {
     if (*e_args != ' ' && *e_args != '\t')
     {
	   i = tolower(*e_args) - 'a';
       if (i < 0 || i > 'z' - 'a')
       {
         fprintf(stderr, "%s: Illegal argument character\n", progn);
		 exit(1);
       }
 
       n++;
     }
 
     e_args++;
   }
 
   args = (char *) malloc((n + 1) * sizeof *args);
   if (args == NULL)
   {
     fprintf(stderr, "%s: Not enough core\n", progn);
     exit(1);
   }
 
   n = 0;
   e_args = argv[2];
   while (*e_args)
   {
     if (*e_args != ' ' && *e_args != '\t')
       args[n++] = tolower(*e_args);
 
     e_args++;
   }

   args[n] = '\0';
 

   function = translate(argv[1], args, &n, &error);
   if (n == 0)
   {
     fprintf(stderr, "   %s\n", argv[1]);
     for (i = 0; i < error; i++) putc('-', stderr);
     fprintf(stderr, "---^\n");
     fprintf(stderr, "%s: %s\n", progn, fget_error());
     exit(1);
   }

   rnd_init();
   linec = 0;
 
   while (fgetline(stdin, buf, sizeof buf))
   {
     linec++;
 
     ivar = 0;
            e_args = args;
     while(*e_args)
     {
       if (strarg(svar, buf, ivar++) == 0)
       {
         fprintf(stderr, "%s: Missing variable %d on line %ld\n",
		   progn, ivar, linec);
		 exit(1);
       }
 
           n = sscanf(svar, "%lf%c", &var, &xtrac);
	   if (n != 1)
	   {
		 fprintf(stderr, "%s: Illegal variable %d on line %ld\n",
		   progn, ivar, linec);
	 exit(1);
	   }

	   make_var(*e_args++ , var);
	 }

	 result = fval_at(function);

	 if (argc == 4)
	  fprintf(stdout, argv[3], result);
	 else
	  fprintf(stdout, "%.10g", result);

	 putc('\n', stdout);
   }

   return 0;
 }

#endif

/* Here is the definition of some functions in the FORMULC standard
   library */
static double pi(void)
{
 return 3.14159265358979323846264;
}

/*********************************************************************************************
Added by David Winston (winston@barley.colorado.edu)
*/

#define MY_RND

/*********************************************************************************************/

#ifndef MY_RND

void r250_init(int seed);
double dr250(void);

double rnd(void)
{
  return dr250();
}

void rnd_init(void)
{
  r250_init(time(NULL));
}
/*The following functions were not written by Harald Helfgott. */
/******************************************************************************
*  Module:  r250.c
*  Description: implements R250 random number generator, from S.
*  Kirkpatrick and E.  Stoll, Journal of Computational Physics, 40, p.
*  517 (1981).
*  Written by:    W. L. Maier
*
*	METHOD...
*		16 parallel copies of a linear shift register with
*		period 2^250 - 1.  FAR longer period than the usual
*		linear congruent generator, and commonly faster as
*		well.  (For details see the above paper, and the
*		article in DDJ referenced below.)
*
*	HISTORY...
*		Sep 92: Number returned by dr250() is in range [0,1) instead
*			of [0,1], so for example a random angle in the
*			interval [0, 2*PI) can be calculated
*			conveniently.  (J. R. Van Zandt <jrv@mbunix.mitre.org>)
*		Aug 92: Initialization is optional.  Default condition is
*			equivalent to initializing with the seed 12345,
*			so that the sequence of random numbers begins:
*			1173, 53403, 52352, 35341...  (9996 numbers
*			skipped) ...57769, 14511, 46930, 11942, 7978,
*			56163, 46506, 45768, 21162, 43113...  Using ^=
*			operator rather than two separate statements.
*			Initializing with own linear congruent
*			pseudorandom number generator for portability.
*			Function prototypes moved to a header file.
*			Implemented r250n() to generate numbers
*			uniformly distributed in a specific range
*			[0,n), because the common expedient rand()%n is
*			incorrect.  (J. R. Van Zandt <jrv@mbunix.mitre.org>)
*		May 91: Published by W. L. Maier, "A Fast Pseudo Random Number
*			Generator", Dr. Dobb's Journal #176.
******************************************************************************/

#include <stdlib.h>

/**** Static variables ****/
static int r250_index = 0;
static unsigned int r250_buffer[250] = {
	15301,57764,10921,56345,19316,43154,54727,49252,32360,49582,
	26124,25833,34404,11030,26232,13965,16051,63635,55860,5184,
	15931,39782,16845,11371,38624,10328,9139,1684,48668,59388,
	13297,1364,56028,15687,63279,27771,5277,44628,31973,46977,
	16327,23408,36065,52272,33610,61549,58364,3472,21367,56357,
	56345,54035,7712,55884,39774,10241,50164,47995,1718,46887,
	47892,6010,29575,54972,30458,21966,54449,10387,4492,644,
	57031,41607,61820,54588,40849,54052,59875,43128,50370,44691,
	286,12071,3574,61384,15592,45677,9711,23022,35256,45493,
	48913,146,9053,5881,36635,43280,53464,8529,34344,64955,
	38266,12730,101,16208,12607,58921,22036,8221,31337,11984,
	20290,26734,19552,48,31940,43448,34762,53344,60664,12809,
	57318,17436,44730,19375,30,17425,14117,5416,23853,55783,
	57995,32074,26526,2192,11447,11,53446,35152,64610,64883,
	26899,25357,7667,3577,39414,51161,4,58427,57342,58557,
	53233,1066,29237,36808,19370,17493,37568,3,61468,38876,
	17586,64937,21716,56472,58160,44955,55221,63880,1,32200,
	62066,22911,24090,10438,40783,36364,14999,2489,43284,9898,
	39612,9245,593,34857,41054,30162,65497,53340,27209,45417,
	37497,4612,58397,52910,56313,62716,22377,40310,15190,34471,
	64005,18090,11326,50839,62901,59284,5580,15231,9467,13161,
	58500,7259,317,50968,2962,23006,32280,6994,18751,5148,
	52739,49370,51892,18552,52264,54031,2804,17360,1919,19639,
	2323,9448,43821,11022,45500,31509,49180,35598,38883,19754,
	987,11521,55494,38056,20664,2629,50986,31009,54043,59743
	};

static unsigned myrand(void);
static void mysrand(unsigned newseed);

/**** Function: r250_init  
	Description: initializes r250 random number generator. ****/
void r250_init(int seed)
{
/*---------------------------------------------------------------------------*/
    int        j, k;
    unsigned int mask;
    unsigned int msb;
/*---------------------------------------------------------------------------*/
    mysrand(seed);
    r250_index = 0;
    for (j = 0; j < 250; j++)     /* Fill the r250 buffer with 15-bit values */
        r250_buffer[j] = myrand();
    for (j = 0; j < 250; j++)     /* Set some of the MS bits to 1 */
        if (myrand() > 16384)
            r250_buffer[j] |= 0x8000;
    msb = 0x8000;       /* To turn on the diagonal bit   */
    mask = 0xffff;      /* To turn off the leftmost bits */
	for (j = 0; j < 16; j++)
        {
        k = 11 * j + 3;             /* Select a word to operate on        */
        r250_buffer[k] &= mask;     /* Turn off bits left of the diagonal */
        r250_buffer[k] |= msb;      /* Turn on the diagonal bit           */
        mask >>= 1;
        msb >>= 1;
        }
}

/**** Function: dr250 
		Description: returns a random double z in range 0 <= z < 1.  ****/
double dr250(void)
{
/*---------------------------------------------------------------------------*/
    register int    j;
    register unsigned int new_rand;
/*---------------------------------------------------------------------------*/
    if (r250_index >= 147)
        j = r250_index - 147;     /* Wrap pointer around */
    else
        j = r250_index + 103;

    new_rand = r250_buffer[r250_index] ^= r250_buffer[j];

    if (r250_index >= 249)      /* Increment pointer for next time */
        r250_index = 0;
    else
        r250_index++;
	return new_rand / 65536.;   /* Return a number in [0.0 to 1.0) */
}


/***  linear congruent pseudorandom number generator for initialization ***/

static unsigned long seed=1;

	/*	Return a pseudorandom number in the interval 0 <= n < 32768.
		This produces the following sequence of pseudorandom 
		numbers:
		346, 130, 10982, 1090...  (9996 numbers skipped) ...23369,
		2020, 5703, 12762, 10828, 16252, 28648, 27041, 23444, 6604...
	*/ 
static unsigned myrand(void)
{
	seed = seed*0x015a4e35L + 1;
	return (seed>>16)&0x7fff;
}

	/*	Initialize above linear congruent pseudorandom number generator */
static void mysrand(unsigned newseed)
{	seed = newseed;
}

#endif
/* end of random-number generator */




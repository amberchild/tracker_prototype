/*
 * util.h
 *
 *  Created on: 7 Sep 2018
 *      Author: GDR
 */

#ifndef UTIL_H_
#define UTIL_H_

#define MIN(X, Y)   ((int)X) < ((int)Y) ? ((int)X) : ((int)Y)
#define MAX(X, Y)   ((int)X) > ((int)Y) ? ((int)X) : ((int)Y)

#ifndef nelem
#   define _nelem(X) (sizeof ((X))/sizeof ((*X)))
#   define nelem(X) _nelem((X))
#endif

#define inelem(X) (int)nelem(X)

#define APP_ERR_TRAP(a)     if(a) {__asm("BKPT #0\n");}


#endif /* UTIL_H_ */

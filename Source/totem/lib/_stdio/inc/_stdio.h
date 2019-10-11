/*
 * _stdio.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#if DEBUG_PRINT
    #define PRINT(str) swd_print(str);
#else
    #define PRINT(str)
#endif

void setup_swd_print(void);

void swd_print(const char *pcString);

#endif /* _STDIO_H_ */

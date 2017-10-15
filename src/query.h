/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Dong Nai-Jia
 */

#ifndef _QUERY_H_
#define _QUERY_H_


#include "def.h"

#include <stdbool.h>
#include <mysql/mysql.h>


/* API functions */
void query_init(MYSQL*, const char*);
void query_quit(MYSQL*);
void query_item_coord( /* _IN__ */ MYSQL*,
                       /* _OUT_ */ Coord***);
bool query_by_command( /* _IN__ */ MYSQL*, const char*,
                       /* _OUT_ */ Result*);


#endif

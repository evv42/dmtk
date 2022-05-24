/* See LICENSE file for copyright and license details. */
/* dmtk standard input/output
 * Copyright 2022 evv42.
*/
# ifndef __DMTKSIO_H__
# define __DMTKSIO_H__

//Send a message to the user
void mtksiomsg(char* title, char* message);

//Get a characher list from the user
char* mtksiostr(char* title, char* message);

//Get a integer from the user
int mtksioint(char* title, char* message);

# endif //__DMTKSIO_H__

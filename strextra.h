#ifndef _STRETRA_H_
#define _STRETRA_H_

char* strmerge(char* s1, char* s2);
/*
 * Concatena las cadenas en s1 y s2 devolviendo nueva memoria (debe ser
 * liberada por el llamador con free())
 *
 * USAGE:
 *
 * merge = strmerge(s1, s2);
 *
 * REQUIRES:
 *     s1 != NULL &&  s2 != NULL
 *
 * ENSURES:
 *     merge != NULL && strlen(merge) == strlen(s1) + strlen(s2)
 *
 */

char* strmerge_and_free(char* s1, char* s2);
/*
 * Llama a strmerge y libera s1 y s2.
 * 
 * USAGE:
 *
 * merge = strmerge_and_free(s1, s2);
 * 
 * REQUIRES:
 *     s1 != NULL &&  s2 != NULL
 *
 * ENSURES:
 *     merge != NULL && strlen(merge) == strlen(s1) + strlen(s2)
 */

char* strmerge_and_free_first(char* s1, char* s2);
/*
 * Llama a strmerge y libera solo a s1.
 * 
 * USAGE:
 *
 * merge = strmerge_and_free_first(s1, s2);
 * 
 * REQUIRES:
 *     s1 != NULL &&  s2 != NULL
 *
 * ENSURES:
 *     merge != NULL && strlen(merge) == strlen(s1) + strlen(s2)
 */
#endif

/****************************************************************************
 * fs/dirent/lib_versionsort.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <ctype.h>
#include <string.h>
#include "fs/dirent.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: versionsort
 *
 * Description:
 *   The versionsort() function can be used as the comparison function
 *   compar() for scandir().  It sorts directory entries using strverscmp
 *   on the strings (*a)->d_name and (*b)->d_name.
 *
 * Input Parameters:
 *   a - The first direntry to compare
 *   b - The second direntry to compare
 *
 * Returned Value:
 *   An integer less than, equal to, or greater than zero if the first
 *   argument is considered to be respectively less than, equal to, or
 *   greater than the second.
 *
 ****************************************************************************/

static int strverscmp(FAR const char *s1, FAR const char *s2)
{
    FAR const unsigned char *str1 = (FAR const void *)s1;
    FAR const unsigned char *str2 = (FAR const void *)s2;
    size_t i;
    size_t j;
    size_t dp;
    int z = 1;

    /* Find maximal matching prefix and track its maximal digit
     * suffix and whether those digits are all zeros.
     */

    for (dp = i = 0; str1[i] == str2[i]; i++) {
        int c = str1[i];

        if (c == 0) {
            return 0;
        }

        if (!isdigit(c)) {
            dp = i + 1;
            z = 1;
        } else if (c != '0') {
            z = 0;
        }
    }

    if (str1[dp] != '0' && str2[dp] != '0') {
        /* If we're not looking at a digit sequence that began
         * with a zero, longest digit string is greater.
         */

        for (j = i; isdigit(str1[j]); j++) {
            if (!isdigit(str2[j])) {
                return 1;
            }
        }

        if (isdigit(str2[j])) {
            return -1;
        }
    } else if (z && dp < i && (isdigit(str1[i]) || isdigit(str2[i]))) {
        /* Otherwise, if common prefix of digit sequence is
         * all zeros, digits order less than non-digits.
         */

        return (unsigned char)(str1[i] - '0') -
               (unsigned char)(str2[i] - '0');
    }

    return str1[i] - str2[i];
}

int versionsort(FAR const struct dirent **a, FAR const struct dirent **b)
{
    return strverscmp((*a)->d_name, (*b)->d_name);
}

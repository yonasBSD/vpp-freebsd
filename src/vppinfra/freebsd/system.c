/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/cdefs.h>
#define _WANT_FREEBSD_BITSET

#include <sys/param.h>
#include <sys/types.h>
#include <sys/cpuset.h>
#include <sys/domainset.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <vppinfra/clib.h>
#include <vppinfra/clib_error.h>
#include <vppinfra/format.h>
#include <vppinfra/bitmap.h>

__clib_export clib_error_t *
clib_sysfs_write (char *file_name, char *fmt, ...)
{
  /* Not implemented */
  return NULL;
}

__clib_export clib_error_t *
clib_sysfs_read (char *file_name, char *fmt, ...)
{
  /* Not implemented */
  return NULL;
}

clib_error_t *
clib_sysfs_set_nr_hugepages (int numa_node, int log2_page_size, int nr)
{
  /* Not implemented */
  return NULL;
}

clib_error_t *
clib_sysfs_get_free_hugepages (int numa_node, int log2_page_size, int *v)
{
  /* Not implemented */
  return NULL;
}

clib_error_t *
clib_sysfs_get_nr_hugepages (int numa_node, int log2_page_size, int *v)
{
  /* Not implemented */
  return NULL;
}

clib_error_t *
clib_sysfs_get_surplus_hugepages (int numa_node, int log2_page_size, int *v)
{
  /* Not implemented */
  return NULL;
}

clib_error_t *
clib_sysfs_prealloc_hugepages (int numa_node, int log2_page_size, int nr)
{
  /* Not implemented */
  return NULL;
}

__clib_export uword *
clib_sysfs_list_to_bitmap (char *filename)
{
  /* Not implemented */
  return NULL;
}

__clib_export uword *
clib_system_get_cpu_bitmap (void)
{
  cpuset_t mask;
  uword *r = NULL;

  clib_bitmap_alloc (r, CPU_SETSIZE);

  if (cpuset_getaffinity (CPU_LEVEL_CPUSET, CPU_WHICH_CPUSET, -1,
			  sizeof (mask), &mask) != 0)
    {
      clib_bitmap_free (r);
      return NULL;
    }

  for (int bit = 0; bit < CPU_SETSIZE; bit++)
    clib_bitmap_set (r, bit, CPU_ISSET (bit, (struct bitset *) &mask));

  return r;
}

__clib_export uword *
clib_system_get_domain_bitmap (void)
{
  domainset_t domain;
  uword *r = NULL;
  int policy;

  clib_bitmap_alloc (r, CPU_SETSIZE);

  if (cpuset_getdomain (CPU_LEVEL_CPUSET, CPU_WHICH_CPUSET, -1,
			sizeof (domain), &domain, &policy) != 0)
    {
      clib_bitmap_free (r);
      return NULL;
    }

  for (int bit = 0; bit < CPU_SETSIZE; bit++)
    clib_bitmap_set (r, bit, CPU_ISSET (bit, (struct bitset *) &domain));
  return r;
}

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */

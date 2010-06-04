/***************************************************************************
 *            rtel4i_gnuc_extension.h
 *
 *  Thu Feb  4 17:15:07 2010
 *  Copyright  2010  os4i
 *  <rtel4i@os4i.com>
 ****************************************************************************/

#ifndef   GNUC_EXTENSION_H 
#define   GNUC_EXTENSION_H

/*!
  * \file   gnuc_extension.h
  * \author frederic ferrandis
  */

#ifndef  ARRAY_SIZE
# define ARRAY_SIZE(x)  (sizeof((x))/sizeof(*(x)))
#endif

#ifdef __GNUC__
# define __is_array(arr)    \
            !__builtin_types_compatible_p( typeof(arr), typeof(&arr[0]))
#endif

#if __GNUC__ >= 3
# define __noinline     __attribute__((noinline))
# define __pure         __attribute__((pure))
# define __const        __attribute__((const))
# define __noreturn     __attribute__((noreturn))
# define __malloc       __attribute__((malloc))
# define __must_check   __attribute__((warn_unused_result))
# define __deprecated   __attribute__((deprecated))
# define __used         __attribute__((used))
# define __unused       __attribute__((unused))
# define __packed       __attribute__((packed))
# define __align(x)     __attribute__((aligned (x)))
# define __align_max    __attribute__((aligned))
# define likely(x)      __builtin_expect(!!(x), 1)
# define unlikely(x)    __builtin_expect(!!(x), 0)
# define __hidden       __attribute__((visibility("hidden"))) 
# define __default		__attribute__((visibility("default"))) 
# define __weak         __attribute__((weak))
# define __constructor  __attribute__((constructor))
# define __destructor   __attribute__((destructor))
#else
# define __noinline   
# define __pure       
# define __const      
# define __noreturn  
# define __malloc     
# define __must_check 
# define __deprecated 
# define __used       
# define __unused     
# define __packed    
# define __align(x)   
# define __align_max 
# define likely(x)     (x)
# define unlikely(x)   (x)
# define __hidden
# define __default
# define __constructor
# define __destructor 
# define __weak
#endif

#endif



#ifndef MF_EXPORTS_H
#define MF_EXPORTS_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define MF_EXPORTS
#  define MF_NO_EXPORT
#else
#  ifndef MF_EXPORTS
#    ifdef mf_EXPORTS
        /* We are building this library */
#      define MF_EXPORTS 
#    else
        /* We are using this library */
#      define MF_EXPORTS 
#    endif
#  endif

#  ifndef MF_NO_EXPORT
#    define MF_NO_EXPORT 
#  endif
#endif

#ifndef MF_DEPRECATED
#  define MF_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MF_DEPRECATED_EXPORT
#  define MF_DEPRECATED_EXPORT MF_EXPORTS MF_DEPRECATED
#endif

#ifndef MF_DEPRECATED_NO_EXPORT
#  define MF_DEPRECATED_NO_EXPORT MF_NO_EXPORT MF_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define MF_NO_DEPRECATED
#endif

#endif

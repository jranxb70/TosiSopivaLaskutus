
#ifndef DBEngineLibraryExports_h
#define DBEngineLibraryExports_h

#ifdef _WIN32
    #ifdef engine_EXPORTS
        #define  DB_ENGINE_LIBRARY_EXPORT __declspec( dllexport )
    #else
        #define  DB_ENGINE_LIBRARY_EXPORT __declspec( dllimport )
    #endif
#else
    #define    DB_ENGINE_LIBRARY_EXPORT
#endif

#endif // DBEngineLibraryExports_h

#include "Core/Config.h"

#if !SH_SERVER
	#ifdef ShellEngineUser_EXPORTS
		#define SH_USER_API SH_API_EXPORT
	#else
		#define SH_USER_API SH_API_IMPORT
	#endif
#else
	#ifdef ShellEngineUserServer_EXPORTS
		#define SH_USER_API SH_API_EXPORT
	#else
		#define SH_USER_API SH_API_IMPORT
	#endif
#endif
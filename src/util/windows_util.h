#pragma once

#define DEBUG_BREAK()							(__debugbreak())

// Function type macros.
// Functions with variable arguments
#define VARARGS									__cdecl							
// Standard C function
#define CDECL									__cdecl							
// Standard calling convention
#define STDCALL									__stdcall						
// Force code to be inline
#define FORCEINLINE								__forceinline					
// Force code to NOT be inline
#define FORCENOINLINE							__declspec(noinline)			
// Indicate that the function never returns nullptr.
#define FUNCTION_NON_NULL_RETURN_START			_Ret_notnull_					

// Optimization macros (uses __pragma to enable inside a #define).
#if !defined(__clang__)
	#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL	__pragma(optimize("",off))
	#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL	__pragma(optimize("",on))
#elif defined(_MSC_VER)		// Clang only supports __pragma with -fms-extensions
	#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL	__pragma(clang optimize off)
	#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL	__pragma(clang optimize on)
#endif

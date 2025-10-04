#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

// Include ATL headers first
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

// Then other COM headers
#include "resource.h"

using namespace ATL;

// COM class headers (required for OBJECT_ENTRY_AUTO registration)
#include "FileManagement.h"
#include "ShellExtHandler.h"

// dllmain.h : Declaration of module class.
//
// This header declares the ATL module object used by the COM server DLL.
// The ATL module class provides DLL entry points, module-level registration
// support and lifetime management for the COM objects implemented in this
// DLL. The global module instance (_AtlModule) is used by ATL infrastructure
// to implement DllMain, registration helpers and class factory lifetime.

class CTaskAutomationCOMServerModule : public ATL::CAtlDllModuleT< CTaskAutomationCOMServerModule >
{
public :
	// DECLARE_LIBID associates a type library GUID (LIBID) with this module.
	// ATL uses the LIBID when exporting or registering type libraries for the
	// COM classes implemented in the DLL.
	DECLARE_LIBID(LIBID_TaskAutomationCOMServerLib)

	// DECLARE_REGISTRY_APPID_RESOURCEID links a resource ID (typically an
	// .rgs registry script resource) and an application ID (APPID GUID) with
	// this module. The APPID is used for COM registration under the
	// AppID key and can control COM activation/identity policies.
	//
	// - IDR_TASKAUTOMATIONCOMSERVER: resource identifier for registry script
	//   resource that ATL uses when registering/unregistering the server.
	// - GUID string: the APPID that will be written into the registry.
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TASKAUTOMATIONCOMSERVER, "{48f8d34d-0ea5-46c7-98ab-ddc985161f4f}")
};

// _AtlModule is the single, process-global instance of the ATL module class
// that ATL expects to be defined in a DLL project. It provides the standard
// ATL entry points (DllMain, DllRegisterServer, DllUnregisterServer, etc.)
// and manages module-level state such as object/class counts used for
// controlling DLL lifetime.
extern class CTaskAutomationCOMServerModule _AtlModule;

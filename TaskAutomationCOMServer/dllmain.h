// dllmain.h : Declaration of module class.

class CTaskAutomationCOMServerModule : public ATL::CAtlDllModuleT< CTaskAutomationCOMServerModule >
{
public :
	DECLARE_LIBID(LIBID_TaskAutomationCOMServerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TASKAUTOMATIONCOMSERVER, "{48f8d34d-0ea5-46c7-98ab-ddc985161f4f}")
};

extern class CTaskAutomationCOMServerModule _AtlModule;

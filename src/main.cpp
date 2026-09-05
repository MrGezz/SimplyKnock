// Simply Knock - Special Edition SKSE plugin.
//
// Chesko's original targeted Legendary Edition (32-bit, RUNTIME_VERSION_1_9_32_0) and used the
// old SKSEPlugin_Query handshake. Special Edition plugins declare themselves through
// SKSEPluginVersionData instead, which is what SKSE 2.3.1 reads.
//
// Not marked version independent: the addresses come from skse64's own RelocAddr table, which
// is compiled for one runtime, so the compatible-version list is the honest declaration.

#include "skse64/PluginAPI.h"
#include "skse64_common/skse_version.h"

#include <shlobj.h>

#include "SimplyKnock.h"

static PluginHandle          g_pluginHandle = kPluginHandle_Invalid;
static SKSEPapyrusInterface* g_papyrus      = NULL;

extern "C" {

__declspec(dllexport) SKSEPluginVersionData SKSEPlugin_Version =
{
	SKSEPluginVersionData::kVersion,

	1,
	"SimplyKnock",

	"Chesko",
	"",

	0,	// not version independent - see the note above
	0,
	{ RUNTIME_VERSION_1_7_104, 0 },

	0,	// no minimum SKSE version
};

__declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface* skse)
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS,
		"\\My Games\\Skyrim Special Edition\\SKSE\\SimplyKnock.log");
	gLog.SetPrintLevel(IDebugLog::kLevel_Error);
	gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

	_MESSAGE("SimplyKnock SE loaded (runtime %08X)", skse->runtimeVersion);

	g_pluginHandle = skse->GetPluginHandle();

	if (skse->isEditor)
	{
		_MESSAGE("loaded in the editor, doing nothing");
		return true;
	}

	g_papyrus = (SKSEPapyrusInterface*)skse->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus)
	{
		_ERROR("could not get the papyrus interface");
		return false;
	}

	if (!g_papyrus->Register(SimplyKnockNamespace::RegisterFuncs))
	{
		_ERROR("could not register SimplyKnockSKSE");
		return false;
	}

	_MESSAGE("SimplyKnockSKSE registered");
	return true;
}

};

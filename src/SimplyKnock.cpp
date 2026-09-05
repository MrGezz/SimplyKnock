// Simply Knock's one native: give Papyrus the door a load door teleports to.
//
// Ported from Chesko's 32-bit Legendary Edition plugin. The logic is unchanged - read the
// reference's ExtraTeleport and resolve its stored handle - but the structure is 64-bit and
// the handle lookup now takes an NiPointer.
//
// Layout is taken from CommonLibSSE (RE/E/ExtraTeleport.h), which is the reference for the
// Special Edition classes:
//     DoorTeleportData : ObjectRefHandle linkedDoor @ 0x00, sizeof 0x20
//     ExtraTeleport    : BSExtraData, DoorTeleportData* teleportData @ 0x10, sizeof 0x18
// skse64's BSExtraData is vtable (0x08) + next (0x08), so the first derived member does land
// at 0x10 - asserted below rather than assumed.

#include "SimplyKnock.h"

#include "skse64/GameReferences.h"
#include "skse64/GameBSExtraData.h"
#include "skse64/GameRTTI.h"
#include "skse64/PapyrusNativeFunctions.h"

namespace SimplyKnockNamespace
{
	class ExtraTeleport : public BSExtraData
	{
	public:
		virtual ~ExtraTeleport();

		struct Data
		{
			UInt32	linkedDoor;		// 00 - ObjectRefHandle
			float	posX;			// 04
			float	posY;			// 08
			float	posZ;			// 0C
			float	rotX;			// 10
			float	rotY;			// 14
			float	rotZ;			// 18
			SInt8	flags;			// 1C
			UInt8	pad1D;			// 1D
			UInt16	pad1E;			// 1E
		};

		Data* teleportData;			// 10
	};

	STATIC_ASSERT(sizeof(ExtraTeleport::Data) == 0x20);
	STATIC_ASSERT(offsetof(ExtraTeleport, teleportData) == 0x10);

	TESObjectREFR* GetLinkedDoor(StaticFunctionTag* base, TESObjectREFR* object)
	{
		if (!object)
			return NULL;

		ExtraTeleport* teleport = DYNAMIC_CAST(
			object->extraData.GetByType(kExtraData_Teleport), BSExtraData, ExtraTeleport);
		if (!teleport || !teleport->teleportData)
			return NULL;

		UInt32 handle = teleport->teleportData->linkedDoor;
		if (handle == *g_invalidRefHandle || handle == 0)
			return NULL;

		NiPointer<TESObjectREFR> reference;
		if (!LookupREFRByHandle(handle, reference))
			return NULL;

		return reference;
	}

	bool RegisterFuncs(VMClassRegistry* registry)
	{
		registry->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, TESObjectREFR*, TESObjectREFR*>(
				"GetLinkedDoor", "SimplyKnockSKSE", SimplyKnockNamespace::GetLinkedDoor, registry));

		registry->SetFunctionFlags("SimplyKnockSKSE", "GetLinkedDoor",
			VMClassRegistry::kFunctionFlag_NoWait);

		return true;
	}
}


#pragma once

#include "RedstoneHandler.h"
#include "BoundingBox.h"





class cPressurePlateHandler : public cRedstoneHandler
{
	typedef cRedstoneHandler super;
public:

	virtual unsigned char GetPowerDeliveredToPosition(cWorld & a_World, Vector3i a_Position, BLOCKTYPE a_BlockType, NIBBLETYPE a_Meta, Vector3i a_QueryPosition, BLOCKTYPE a_QueryBlockType) const override
	{
		UNUSED(a_BlockType);
		UNUSED(a_Meta);
		UNUSED(a_QueryPosition);
		UNUSED(a_QueryBlockType);

		return static_cast<cIncrementalRedstoneSimulator *>(a_World.GetRedstoneSimulator())->GetChunkData()->GetCachedPowerData(a_Position).PowerLevel;
	}

	virtual unsigned char GetPowerLevel(cWorld & a_World, Vector3i a_Position, BLOCKTYPE a_BlockType, NIBBLETYPE a_Meta) const override
	{
		UNUSED(a_Meta);

		class cPressurePlateCallback :
			public cEntityCallback
		{
		public:
			cPressurePlateCallback(void) :
				m_NumberOfEntities(0),
				m_FoundPlayer(false)
			{
			}

			virtual bool Item(cEntity * a_Entity) override
			{
				if (a_Entity->IsPlayer())
				{
					m_FoundPlayer = true;
				}

				m_NumberOfEntities++;
				return false;
			}

			unsigned int m_NumberOfEntities;
			bool m_FoundPlayer;
		} PressurePlateCallback;
		a_World.ForEachEntityInBox(cBoundingBox(Vector3d(0.5, 0, 0.5) + a_Position, 0.5, 0.5), PressurePlateCallback);

		switch (a_BlockType)
		{
			case E_BLOCK_STONE_PRESSURE_PLATE:
			{
				return (PressurePlateCallback.m_FoundPlayer ? 15 : 0);
			}
			case E_BLOCK_WOODEN_PRESSURE_PLATE:
			{
				return (PressurePlateCallback.m_NumberOfEntities != 0 ? 15 : 0);
			}
			case E_BLOCK_HEAVY_WEIGHTED_PRESSURE_PLATE:
			{
				return std::min(static_cast<unsigned char>(CeilC(PressurePlateCallback.m_NumberOfEntities / 10.f)), static_cast<unsigned char>(15));
			}
			case E_BLOCK_LIGHT_WEIGHTED_PRESSURE_PLATE:
			{
				return std::min(static_cast<unsigned char>(PressurePlateCallback.m_NumberOfEntities), static_cast<unsigned char>(15));
			}
			default:
			{
				ASSERT(!"Unhandled/unimplemented block in pressure plate handler!");
				return 0;
			}
		}
	}

	virtual cVector3iArray Update(cWorld & a_World, Vector3i a_Position, BLOCKTYPE a_BlockType, NIBBLETYPE a_Meta, PoweringData a_PoweringData) const override
	{
		UNUSED(a_PoweringData.PowerLevel);
		// LOGD("Evaluating clicky the pressure plate (%d %d %d)", a_Position.x, a_Position.y, a_Position.z);

		auto Power = GetPowerLevel(a_World, a_Position, a_BlockType, a_Meta);
		auto PreviousPower = static_cast<cIncrementalRedstoneSimulator *>(a_World.GetRedstoneSimulator())->GetChunkData()->ExchangeUpdateOncePowerData(a_Position, PoweringData(a_BlockType, Power));

		if (Power != PreviousPower.PowerLevel)
		{
			a_World.SetBlockMeta(a_Position, (Power == 0) ? E_META_PRESSURE_PLATE_RAISED : E_META_PRESSURE_PLATE_DEPRESSED);
			return GetAdjustedRelatives(a_Position, StaticAppend(GetRelativeLaterals(), cVector3iArray{ OffsetYM() }));
		}

		return {};
	}

	virtual cVector3iArray GetValidSourcePositions(cWorld & a_World, Vector3i a_Position, BLOCKTYPE a_BlockType, NIBBLETYPE a_Meta) const override
	{
		UNUSED(a_World);
		UNUSED(a_Position);
		UNUSED(a_BlockType);
		UNUSED(a_Meta);
		return {};
	}
};

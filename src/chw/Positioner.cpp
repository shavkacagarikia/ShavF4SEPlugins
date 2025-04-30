#include "Positioner.h"
#include "PapyrusFunctions.h"
#include "VisibleHolster.h"
#include "F4NVSerialization.h"


UInt32 PositionerKeyToggle = 110;
UInt32 PositionerKeyDimensionX = 103;
UInt32 PositionerKeyDimensionY = 104;
UInt32 PositionerKeyDimensionZ = 105;
UInt32 PositionerKeyOperationMove = 97;
UInt32 PositionerKeyOperationRotate = 98;
UInt32 PositionerKeyAdd = 107;
UInt32 PositionerKeySubtract = 109;
UInt32 PositionerKeyMarkForSave = 106;
UInt32 PositionerKeyShowCurrent = 96;
UInt32 PositionerKeyReset = 111;

namespace Positioner {



	NiNode* SkeletonNode = nullptr;

	NiNode* ActiveNode = nullptr;

	VisibleHolster::WeaponPosition* ActiveWeapPos = nullptr;

	double OffsetAmount = 1;
	double OffsetAmountRotate = 0.1;

	bool IsPositioningEnabled = false;

	void SetPositioningEnabled(bool enable) {
		IsPositioningEnabled = enable;
		if (enable) {
			/*SetSkeletonNode(nullptr);
			std::string s = Positioner::GetActiveBoneName();
			if (!s.empty()) {
				Positioner::SetActiveAttachBone(s.c_str());
			}*/
			ShowMessagebox("Positioner enabled");
		}
		else {
			ShowMessagebox("Positioner disabled");
		}
	}
	bool GetPositioningEnabled() {
		return IsPositioningEnabled;
	}

	const char * ActiveBoneName = "";

	std::string GetActiveBoneName() {
		return std::string(ActiveBoneName);
	}

	//0 - move
	//1 - rotate
	//2 -scale
	int OperationType = 0;

	//0 - x
	//1 - y
	//2 - z
	int ActiveDimension = 0;

	void SetSkeletonNode(NiNode* seletonNode) {
		//SkeletonNode = nullptr;
		if (!(*g_player) || !(*g_player)->GetActorRootNode(false) || !(*g_player)->GetActorRootNode(false)->GetAsNiNode())
			return;
		//SkeletonNode = seletonNode;
		SkeletonNode = (*g_player)->GetActorRootNode(false)->GetAsNiNode();
		//SetActiveAttachBone(ActiveBoneName.c_str());
	}
	NiNode* GetSkeletonNode() {
		if (!(*g_player) || !(*g_player)->GetActorRootNode(false) || !(*g_player)->GetActorRootNode(false)->GetAsNiNode())
			return nullptr;

		return (*g_player)->GetActorRootNode(false)->GetAsNiNode();
	}


	void ResetAllChanges() {
		VisibleHolster::ResetAllWeaponPos(SkeletonNode);
	}

	void SetActiveAttachBone(const char* boneName) {
		ActiveBoneName = boneName;
		if (SkeletonNode) {
			//NiAVObject* obj = (SkeletonNode->m_uiRefCount > 0 || SkeletonNode->m_children.m_size < 10000) ? SkeletonNode->GetObjectByName(&BSFixedString(boneName)) : nullptr;
			NiAVObject* obj = (GetSkeletonNode() && GetSkeletonNode()->m_uiRefCount > 0) ? GetSkeletonNode()->GetObjectByName(&BSFixedString(boneName)) : nullptr;
			if (obj) {
				ActiveNode = obj->GetAsNiNode();
				if (ActiveNode) {
					std::vector<VisibleHolster::WeaponPosition>* vec = VisibleHolster::GetAttachPosVec();
					for (size_t i = 0; i < vec->size(); i++)
					{
						VisibleHolster::WeaponPosition* wp = &vec->at(i);
						if (wp) {
							wp->ResetDataOnNode((*g_player)->GetActorRootNode(false)->GetAsNiNode());
							if (!_strcmpi(boneName, wp->name)) {
								ActiveWeapPos = wp;
							}
						}
					}
				}
			}
		}
	}

	void MoveX(bool add) {
		if (add) {
			ActiveNode->m_localTransform.pos.x += OffsetAmount;

		}
		else {
			ActiveNode->m_localTransform.pos.x -= OffsetAmount;
		}
		ActiveWeapPos->posX = ActiveNode->m_localTransform.pos.x;
	}

	void MoveY(bool add) {
		if (add) {
			ActiveNode->m_localTransform.pos.y += OffsetAmount;
		}
		else {
			ActiveNode->m_localTransform.pos.y -= OffsetAmount;
		}
		ActiveWeapPos->posY = ActiveNode->m_localTransform.pos.y;
	}

	void MoveZ(bool add) {
		if (add) {
			ActiveNode->m_localTransform.pos.z += OffsetAmount;
		}
		else {
			ActiveNode->m_localTransform.pos.z -= OffsetAmount;
		}
		ActiveWeapPos->posZ = ActiveNode->m_localTransform.pos.z;
	}

	//NOT USED ANYMORE
	void Scale(bool add) {
		if (add) {
			ActiveNode->m_localTransform.scale += OffsetAmount;
		}
		else {
			ActiveNode->m_localTransform.scale -= OffsetAmount;
		}
	}




	void RotateH(bool add) {
		float a = 0;
		float b = 0;
		float c = 0;
		ActiveNode->m_localTransform.rot.GetEulerAngles(&a, &b, &c);

		float a1 = add ? OffsetAmountRotate : -OffsetAmountRotate;

		NiMatrix43 m;
		m.SetEulerAngles(a1, 0, 0);

		NiMatrix43 finalM;

		finalM = m * ActiveNode->m_localTransform.rot;

		ActiveNode->m_localTransform.rot = finalM;
		ActiveWeapPos->eulerHeading = a;

		_MESSAGE("heading:");
		_MESSAGE(std::to_string(a).c_str());
	}
	void RotateA(bool add) {
		float a = 0;
		float b = 0;
		float c = 0;

		ActiveNode->m_localTransform.rot.GetEulerAngles(&a, &b, &c);

		float b1 = add ? OffsetAmountRotate : -OffsetAmountRotate;

		NiMatrix43 m;
		m.SetEulerAngles(0, b1, 0);

		NiMatrix43 finalM;

		finalM = m * ActiveNode->m_localTransform.rot;

		ActiveNode->m_localTransform.rot = finalM;
		ActiveWeapPos->eulerAttitude = b;
		_MESSAGE("attitude:");
		_MESSAGE(std::to_string(b).c_str());
	}
	void RotateB(bool add) {
		float a = 0;
		float b = 0;
		float c = 0;
		ActiveNode->m_localTransform.rot.GetEulerAngles(&a, &b, &c);


		float c1 = add ? OffsetAmountRotate : -OffsetAmountRotate;

		NiMatrix43 m;
		m.SetEulerAngles(0, 0, c1);

		NiMatrix43 finalM;

		finalM = m * ActiveNode->m_localTransform.rot;

		ActiveNode->m_localTransform.rot = finalM;
		ActiveWeapPos->eulerBank = c;

		_MESSAGE("bank:");
		_MESSAGE(std::to_string(c).c_str());
	}

	void DoOperation(bool add) {
		if (!ActiveNode || !GetSkeletonNode()) {
			ShowNotification("No active holster bone selected");
			_MESSAGE("No active holster bone selected");
			return;
		}
		if (!ActiveWeapPos) {
			ShowNotification("No active weap position");
			_MESSAGE("No active weap position");
		}
		switch (OperationType)
		{
		case 0:
			if (ActiveDimension == 0) {
				MoveX(add);
			}
			else if (ActiveDimension == 1) {
				MoveY(add);
			}
			else if (ActiveDimension == 2) {
				MoveZ(add);
			}
			break;
		case 1:
			if (ActiveDimension == 0) {
				RotateH(add);
			}
			else if (ActiveDimension == 1) {
				RotateA(add);
			}
			else if (ActiveDimension == 2) {
				RotateB(add);
			}
			break;
			/*case 2:
				Scale(add);
				break;*/
		default:
			break;
		}
	}

	void SetActiveDimension(UInt32 keycode) {

		if (keycode == PositionerKeyDimensionX) {
			ActiveDimension = 0;
			ShowNotification("Active Dimenison: X");
		}
		else if (keycode == PositionerKeyDimensionY) {
			ActiveDimension = 1;
			ShowNotification("Active Dimenison: Y");
		}
		else if (keycode == PositionerKeyDimensionZ) {
			ActiveDimension = 2;
			ShowNotification("Active Dimenison: Z");
		}
	}

	void SetOperationType(UInt32 keycode) {

		if (keycode == PositionerKeyOperationMove) {
			OperationType = 0;
			ShowNotification("Operation Type: Move");
		}
		else if (keycode == PositionerKeyOperationRotate) {
			OperationType = 1;
			ShowNotification("Operation Type: Rotate");
		}
	}

	void ShowCurrentState() {
		std::string op = "";
		std::string dimension = "";

		switch (ActiveDimension)
		{
		case 0:
			dimension = "|X|";
			break;
		case 1:
			dimension = "|Y|";
			break;
		case 2:
			dimension = "|Z|";
			break;
		default:
			break;
		}

		switch (OperationType)
		{
		case 0:
			op = "|MOVE|";
			break;
		case 1:
			op = "|ROTATE|";
			break;
			/*case 2:
				op = "|SCALE|";
				break;*/
		default:
			break;
		}

		std::string fin = "Current Operation: " + op + ". Current Dimenison: " + dimension + ". Current Offset" + std::to_string(OffsetAmount);

		ShowMessagebox(fin);

	}

	void LogCurrentPositions() {
		float heading = 0;
		float attitude = 0;
		float bank = 0;
		float x = ActiveNode->m_localTransform.pos.x;
		float y = ActiveNode->m_localTransform.pos.y;
		float z = ActiveNode->m_localTransform.pos.z;
		ActiveNode->m_localTransform.rot.GetEulerAngles(&heading, &attitude, &bank);
		_MESSAGE("--------------------------------");
		_MESSAGE(ActiveBoneName);
		_MESSAGE(std::to_string(heading).c_str());
		_MESSAGE(std::to_string(attitude).c_str());
		_MESSAGE(std::to_string(bank).c_str());
		_MESSAGE(std::to_string(x).c_str());
		_MESSAGE(std::to_string(y).c_str());
		_MESSAGE(std::to_string(z).c_str());
		_MESSAGE("--------------------------------");
	}

	void SaveCurrentPositions() {
		if (!ActiveNode) {
			ShowNotification("No active holster bone selected");
			_MESSAGE("No active holster bone selected");
		}
		if (!GetSkeletonNode()) {
			ShowNotification("No active skeleton");
			_MESSAGE("No active skeleton");
		}


		/*float heading = 0;
		float attitude = 0;
		float bank = 0;
		float x = ActiveNode->m_localTransform.pos.x;
		float y = ActiveNode->m_localTransform.pos.y;
		float z = ActiveNode->m_localTransform.pos.z;
		ActiveNode->m_localTransform.rot.GetEulerAngles(&heading, &attitude, &bank);*/
		std::vector<VisibleHolster::WeaponPosition>* AttachVec = VisibleHolster::GetAttachPosVec();

		HolsterType currType = HolsterType::NotSet;
		for (size_t i = 0; i < AttachVec->size(); i++)
		{
			VisibleHolster::WeaponPosition* wp = &AttachVec->at(i);
			/*if (!_strcmpi(ActiveNode->m_name, wp.name)) {
				currType = AttachVec[i].type;
			}*/
			if (!wp) {
				return;
			}

			float attitude = 0;
			float bank = 0;
			float heading = 0;
			auto currNode = (GetSkeletonNode() && GetSkeletonNode()->m_uiRefCount > 0) ? GetSkeletonNode()->GetObjectByName(&BSFixedString(wp->name))->GetAsNiNode() : nullptr;
			if (currNode) {
				currNode->m_localTransform.rot.GetEulerAngles(&heading, &attitude, &bank);
				F4NVSerialization::UpsertWeaponPosition(wp->type, currNode->m_localTransform.pos.x, currNode->m_localTransform.pos.y, currNode->m_localTransform.pos.z, attitude, bank, heading);
				wp->posX = currNode->m_localTransform.pos.x;
				wp->posY = currNode->m_localTransform.pos.y;
				wp->posZ = currNode->m_localTransform.pos.z;
				wp->eulerAttitude = attitude;
				wp->eulerBank = bank;
				wp->eulerHeading = heading;
			}
		}

		ShowNotification("Positions are marked for save");
		//HolsterType currType = 

		//F4NVSerialization::UpsertWeaponPosition(currType, x, y, z, attitude, bank, heading);

	}

}
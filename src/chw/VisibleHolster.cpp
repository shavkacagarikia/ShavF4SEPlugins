#include "VisibleHolster.h"
#include "f4se\GameThreads.h"
#include <unordered_set>

std::vector<std::string> AdditonalSkeletonPaths;


namespace VisibleHolster {

	static std::vector<std::string> WeaponPaths = {};
	static std::vector<UInt32> AvailableRaces = {};

	enum AdditionalItemType {
		CHW_Always = 1,
		CHW_OnDraw,
		CHW_OnHolster
	};

	VisibleHolster::WeaponPosition DefaultWeaponRifleBoneAttachPos =
	{
		HolsterType::Rifle,
		"WeaponRifleBone",
		-1.467466,
		-1.231682,
		1.663497,
		17.894257,
		-12.836738,
		6.571038,
		/*-1.467466,
		- 1.231682,
		1.663497,
		- 21.105743,
		6.163261,
		62.571037,*/
		true,
		"Chest",
		X
	};



	VisibleHolster::WeaponPosition DefaultWeaponMelee2HAttachPos =
	{
		HolsterType::TwoHMelee,
		"WeaponMelee2H",
		3.050061,
		-1.185978,
		-1.655665,
		25.776085,
		-12.672893,
		-5.732063,
		true,
		"Chest",
		X
	};

	VisibleHolster::WeaponPosition DefaultWeaponPistolUpAttachPos =
	{
		HolsterType::Pistol,
		"WeaponPistolUp",
		1.857521,
		1.570796,
		0.000000,
		8.335701,
		-0.659389,
		9.937208,
		false,
		//"RLeg_Calf"
		//"Pelvis",
		"RLeg_Thigh",
		X
	};

	VisibleHolster::WeaponPosition DefaultWeaponMelee1HAttachPos =
	{
		HolsterType::OneHMelee,
		"WeaponMelee1H",
		0.031314,
		-0.449990,
		3.137365,
		-1.815956,
		3.442890,
		14.893623,
		false,
		"Pelvis",
		MinusZ
	};




	//WeaponPosition RifleAttachPos = {
	//	"WeaponRifleBone",
	//	-1.46746552,
	//	-1.23168182,
	//	1.66349709,
	//	17.894257,
	//	-12.836738,
	//	6.571038
	//};



	WeaponPosition WeaponRifleBoneAttachPos =
	{
		HolsterType::Rifle,
		"WeaponRifleBone",
		-1.467466,
		-1.231682,
		1.663497,
		17.894257,
		-12.836738,
		6.571038,
		/*-1.467466,
		- 1.231682,
		1.663497,
		- 21.105743,
		6.163261,
		62.571037,*/
		true,
		"Chest",
		X
	};



	WeaponPosition WeaponMelee2HAttachPos =
	{
		HolsterType::TwoHMelee,
		"WeaponMelee2H",
		3.050061,
		-1.185978,
		-1.655665,
		25.776085,
		-12.672893,
		-5.732063,
		true,
		"Chest",
		X
	};

	WeaponPosition WeaponPistolUpAttachPos =
	{
		HolsterType::Pistol,
		"WeaponPistolUp",
		1.857521,
		1.570796,
		0.000000,
		8.335701,
		-0.659389,
		9.937208,
		false,
		//"RLeg_Calf"
		//"Pelvis",
		"RLeg_Thigh",
		X
	};




	WeaponPosition WeaponMelee1HAttachPos =
	{
		HolsterType::OneHMelee,
		"WeaponMelee1H",
		0.031314,
		-0.449990,
		3.137365,
		-1.815956,
		3.442890,
		14.893623,
		false,
		"Pelvis",
		MinusZ
	};

	std::vector<WeaponPosition> val1{
		WeaponRifleBoneAttachPos,WeaponMelee2HAttachPos
	};

	std::unordered_map<UInt32, std::vector< WeaponPosition>> PosByIndex = {
		{6,{ WeaponRifleBoneAttachPos,WeaponMelee2HAttachPos }},
		{ 11,{ WeaponRifleBoneAttachPos,WeaponMelee2HAttachPos } },
		{ 10,{ WeaponPistolUpAttachPos} },
		{ 15,{ WeaponPistolUpAttachPos} },
		{ 9,{ WeaponMelee1HAttachPos} },
		{ 14,{ WeaponMelee1HAttachPos} }
	};


	std::unordered_map<BGSKeyword*, WeaponPosition> AttachPosMap = {
	};
	std::vector<BGSKeyword*> KeywordsVec = {
	};
	std::vector<WeaponPosition> AttachPosVec = {
		WeaponRifleBoneAttachPos,
		WeaponPistolUpAttachPos,
		WeaponMelee2HAttachPos,
		WeaponMelee1HAttachPos
	};

	std::vector<WeaponPosition> DefaultAttachPosVec = {
		WeaponRifleBoneAttachPos,
		WeaponPistolUpAttachPos,
		WeaponMelee2HAttachPos,
		WeaponMelee1HAttachPos
	};
	std::vector<WeaponPosition>* GetAttachPosVec() {
		return &AttachPosVec;
	}

	void ResetAllWeaponPos(NiNode* src) {
		for (size_t i = 0; i < AttachPosVec.size(); i++)
		{
			WeaponPosition* curr = &AttachPosVec[i];
			WeaponPosition def = DefaultAttachPosVec[i];
			if (curr) {

				curr->posX = def.posX;
				curr->posY = def.posY;
				curr->posZ = def.posZ;
				curr->eulerAttitude = def.eulerAttitude;
				curr->eulerBank = def.eulerBank;
				curr->eulerHeading = def.eulerHeading;
				curr->ResetDataOnNode(src);
			}
		}
	}

	void printVHWLg(const char* name, NiAVObject* o, float a, float b, float c) {
		std::string r = "WeaponPosition " + std::string(name) + "AttachPos = ";
		_MESSAGE(r.c_str());
		_MESSAGE("{");

		r = "\"" + std::string(name) + "\"" + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(a) + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(b) + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(c) + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(o->m_localTransform.pos.x) + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(o->m_localTransform.pos.y) + ",";
		_MESSAGE(r.c_str());

		r = std::to_string(o->m_localTransform.pos.z);
		_MESSAGE(r.c_str());
		_MESSAGE("};");
	}

	void attachCustomNodeOnRootNode(NiAVObject* rootNode) {
		if (rootNode) {

			//if (rr2 && rr3) {
			for each (WeaponPosition var in AttachPosVec)
			{
				NiAVObject* par = rootNode->GetObjectByName(&BSFixedString(var.potentialParentNode));
				if (par) {
					var.SetDataOnNode(par->GetAsNiNode());
				}
				/*if (var.isTop) {
				var.SetDataOnNode(rr2->GetAsNiNode());
				}
				else {
				var.SetDataOnNode(rr3->GetAsNiNode());
				}*/
			}
			//WeaponPistolUpAttachPos.SetDataOnNode(rr3->GetAsNiNode());
			//}

		}
	}

	void AddWeaponAttachSkinsOnPlayer() {
		if ((*g_player)) {
			NiNode* rr = (*g_player)->GetActorRootNode(false);

			attachCustomNodeOnRootNode(rr);

		}
	}

	void findLargestBound(NiBound currentBound, NiBound* initialBound) {
		if (currentBound.m_kCenter.x > initialBound->m_kCenter.x) {
			initialBound->m_kCenter.x = currentBound.m_kCenter.x;
		}
		if (currentBound.m_kCenter.y > initialBound->m_kCenter.y) {
			initialBound->m_kCenter.y = currentBound.m_kCenter.y;
		}
		if (currentBound.m_kCenter.z > initialBound->m_kCenter.z) {
			initialBound->m_kCenter.z = currentBound.m_kCenter.z;
		}
	}
	//NiBound additionalOffset;
	//corrects every weapon part position since its not exclusive to moving parts
	float ToMoveOnX = 0;
	NiNode* orig = nullptr;
	void handleMovingPartsTransformCorrection(NiAVObject* root, std::vector<const char *>* skinnedNodeNames) {
		std::vector<NiAVObject*> toDec = {};
		float biggestX = -100;
		float smallestX = 100;

		//additionalOffset = {};

		//if (root->GetObjectByName(&BSFixedString(VHWMark))) {
		root->Visit([&](NiAVObject* object)
			{
				std::string str = IsValidBSString(object->m_name) ? object->m_name.c_str() : "";

				if (object && object->GetRTTI()) {
					const char* nam = object->GetRTTI()->name;
					/*if (!_strcmpi(nam, "BSFadeNode")) {
					_MESSAGE("fade");
					}*/


					if (!object->GetAsNiNode() && !object->GetAsBSTriShape() && !object->GetAsBSSubIndexTriShape() && !object->GetAsBSFadeNode()) {
						toDec.push_back(object);

					}

					auto shape = object->GetAsBSTriShape();
					if (shape) {
						//_MESSAGE(std::to_string(shape->vertexDesc).c_str());
						/*if (shape->vertexDesc & shape->kFlag_Unk1) {
							_MESSAGE("kFlag_Unk1");
						}
						if (shape->vertexDesc & shape->kFlag_Unk3) {
							_MESSAGE("kFlag_Unk3");
						}
						if (shape->vertexDesc & shape->kFlag_Unk4) {
							_MESSAGE("kFlag_Unk4");
						}
						if (shape->vertexDesc & shape->kFlag_Vertex) {
							_MESSAGE("kFlag_Vertex");
						}
						if (shape->vertexDesc & shape->kFlag_UVs) {
							_MESSAGE("kFlag_UVs");
						}
						if (shape->vertexDesc & shape->kFlag_Unk5) {
							_MESSAGE("kFlag_Unk5");
						}
						if (shape->vertexDesc & shape->kFlag_Normals) {
							_MESSAGE("kFlag_Normals");
						}
						if (shape->vertexDesc & shape->kFlag_Tangents) {
							_MESSAGE("kFlag_Tangents");
						}
						if (shape->vertexDesc & shape->kFlag_VertexColors) {
							_MESSAGE("kFlag_VertexColors");
						}
						if (shape->vertexDesc & shape->kFlag_Unk6) {
							_MESSAGE("kFlag_Unk6");
						}
						if (shape->vertexDesc & shape->kFlag_MaleEyes) {
							_MESSAGE("kFlag_MaleEyes");
						}
						if (shape->vertexDesc & shape->kFlag_FullPrecision) {
							_MESSAGE("kFlag_FullPrecision");
						}
						if (shape->vertexDesc & shape->kFlag_Unk8) {
							_MESSAGE("kFlag_Unk8");
						}*/
						if (shape->vertexDesc & shape->kFlag_VertexColors) {
							//toDec.push_back(object);
							shape->vertexDesc &= ~shape->kFlag_VertexColors;
						}

						if (skinnedNodeNames) {

							for (size_t i = 0; i < skinnedNodeNames->size(); i++)
							{
								if (!strcmpi(shape->m_name.c_str(),skinnedNodeNames->at(i))) {
									toDec.push_back(object);
								}
							}
						}

						//OLD STYLE SKIN CHECKING
						//if (shape->vertexDesc & shape->kFlag_Skinned) {
						//	toDec.push_back(object);

						//	//shape->vertexDesc &= ~shape->kFlag_Skinned;

						//	//shape->skinInstance.m_pObject->IncRef();

						//	//SKIN CAN BE APPLIED LIKE THIS BUT CANNOT FIX WRONG TRANSOFRM
						//	/*auto ref = skin.get();
						//	if (ref && ref->boneData) {
						//		NiCloningProcess cp = {};

						//		cp.unk60 = 1;
						//		auto clone = ref->CreateClone(&cp);
						//		auto skinClone = (BSSkin::Instance*)clone;
						//		shape->skinInstance = skinClone;
						//	}
						//	shape->m_name = "VHWAppliedSkin";*/


						//}
					}
					//if (str.find("ChainRedo") != std::string::npos) {
					//	/*auto ff = object->GetAsBSTriShape() ? object->GetAsBSTriShape(): nullptr;
					//	if (ff) {
					//		ff->skinInstance = nullptr;
					//	}*/
					//	//_MESSAGE("extra part:");
					//	//_MESSAGE(object->m_name.c_str());
					//	toDec.push_back(object);
					//}
					if (!_strcmpi(nam, "NiNode") && ((object->flags & 7) == 7)) {
						toDec.push_back(object);
					}

					if (!_strcmpi(str.c_str(), "[CHWIgnored]")) {
						if (object->m_parent) {

							toDec.push_back(object->m_parent);
						}
					}

					//if (!str.empty() && (str.find("Weapon") != std::string::npos || (str.find("Receiver") != std::string::npos)) && str.find("VHW") == std::string::npos) {
					if (!str.empty() && (!_strcmpi("NiNode", nam)) && str.find("VHW") == std::string::npos) {
						/*if (!_strcmpi("WeaponExtra2", str.c_str())) {
						_MESSAGE("here");
						}*/
						auto objNode = object->GetAsNiNode();
						if (objNode) {
							if (object->m_localTransform.pos.x < smallestX) {
								smallestX = object->m_localTransform.pos.x;
							}
							std::string strInner = mAbbrv + str;
							const char* finStr = strInner.c_str();
							auto VHWTransformSourceEl = root->GetObjectByName(&BSFixedString(finStr));
							if (VHWTransformSourceEl) {

								/*if (objNode->m_localTransform.pos.x == 0) {
									objNode->m_localTransform.pos.x == VHWTransformSourceEl->m_localTransform.pos.x;
								}
								if (objNode->m_localTransform.pos.y == 0) {
									objNode->m_localTransform.pos.y == VHWTransformSourceEl->m_localTransform.pos.y;
								}
								if (objNode->m_localTransform.pos.z == 0) {
									objNode->m_localTransform.pos.z == VHWTransformSourceEl->m_localTransform.pos.z;
								}

								float heading = 0;
								float attitude = 0;
								float bank = 0;
								VHWTransformSourceEl->m_localTransform.rot.GetEulerAngles(&heading, &attitude, &bank);
								objNode->m_localTransform.rot.SetEulerAngles(heading, attitude, bank);*/
								//if (!_strcmpi(str.c_str(), "CFrontSight") || !_strcmpi(str.c_str(), "C-Rotor") || !_strcmpi(str.c_str(), "CMuzzle") || !_strcmpi(str.c_str(), "CSlide")) {
								//	//_MESSAGE(str.c_str());

								//	_MESSAGE("par that nono transform");
								//	_MESSAGE(str.c_str());
								//}
								auto ed = object->GetExtraData(BSFixedString("ChildOrigin"));
								if (ed) {

								}
								else {
									objNode->m_localTransform = VHWTransformSourceEl->m_localTransform;

								}

							}
						}

					}
				}
				if (object) {
					object->m_spCollisionObject = nullptr;
					//object->m_spCollisionObject = nullptr;
					//auto extradatalist = object->m_extraData;
					//if (extradatalist)
					//{
					//	extradatalist->lock.Lock();
					//	for (UInt32 i = 0; i < extradatalist->count; i++)
					//	{
					//		NiExtraData* ed = extradatalist->entries[i];
					//		ed->DecRef();
					//		//ed->Release();
					//	}
					//	extradatalist->Clear();
					//	extradatalist->lock.Release();
					//}
				}


				return false;
			});
		for each (NiAVObject * var in toDec)
		{
			if (var && var->m_parent) {
				//var->Release();
				var->m_parent->RemoveChild(var);
			}
		}
		skinnedNodeNames->clear();

		ToMoveOnX = smallestX + biggestX;
		toDec.clear();

		//}
	}


	AffectedDimension DimensionToAffect = {};
	//Get the name of node where specific instance should be attached
	std::string GetAttachNodeFromWeapon(TESObjectWEAP::InstanceData* instance, TESObjectWEAP* weap) {
		std::string toRet = "";
		if (instance && instance->refCount > 0 && instance->refCount < INT32_MAX && instance->weight > 0) {
			for (size_t i = 0; i < KeywordsVec.size(); i++)
			{
				BGSKeyword* currKwd = KeywordsVec.at(i);

				if (InstWEAPHasKeyword(instance, weap, currKwd)) {

					std::unordered_map<BGSKeyword*, WeaponPosition>::iterator it;
					it = AttachPosMap.find(currKwd);

					if (it != AttachPosMap.end()) {
						toRet = AttachPosMap[currKwd].name;
						DimensionToAffect = AttachPosMap[currKwd].dimmensionToAffect;
						break;
					}
				};
			}
		}
		else if (weap) {
			for (size_t i = 0; i < KeywordsVec.size(); i++)
			{
				BGSKeyword* currKwd = KeywordsVec.at(i);
				if (HasKeyword(weap, currKwd)) {

					std::unordered_map<BGSKeyword*, WeaponPosition>::iterator it;

					it = AttachPosMap.find(currKwd);
					if (it != AttachPosMap.end()) {
						toRet = AttachPosMap[currKwd].name;
						DimensionToAffect = AttachPosMap[currKwd].dimmensionToAffect;
						break;
					}
				};
			}
		}
		return toRet;
	}

	void ClearAllHolsters(Actor* actor, bool removeAdditionalItems) {
		/*for each (WeaponPosition var in AttachPosVec)
		{
		NiNode* nd = actor->GetActorRootNode(false);
		if (nd) {
		NiAVObject* attachObj = nd->GetObjectByName(&BSFixedString(var.name));
		if (attachObj) {
		NiNode* attachNode = attachObj->GetAsNiNode();
		if (attachNode) {
		for (size_t i = 0; i < attachNode->m_children.m_size; i++)
		{
		attachNode->RemoveChildAt(i);
		}
		}
		}
		}

		}*/
		NiNode* nd = actor->GetActorRootNode(false);
		if (nd) {

			//NiAVObject* skin = nd->GetObjectByName(&BSFixedString("VHWAppliedSkin"));
			//if (skin) {
			//	//skin->DecRef();
			//}

			NiAVObject* f = nd->GetObjectByName(&BSFixedString("VHWWeapon"));
			if (f) {
				//f->Release();
				if (f->m_parent) {
					NiPointer<NiAVObject> oo = nullptr;
					//f->m_parent->RemoveChild(f);
					f->m_parent->DetachChild(f, oo);
					if (oo) {
						oo->Release();
					}
				}
			}


		}

	}

	int slotMaskToSlot(UInt32 x) {

		int j = 0;
		if (x == 0) {
			return -1;
		}
		if (x == 1) {
			return 0;
		}
		while (x != 1) {
			if (x % 2 != 0)
			{
				return -1;
			}
			j++;
			x /= 2;
		}
		return j;

	}


	///NOT YET FINISHED
	float CollectAdjustWeapon(Actor* actor) {
		/*if (actor != (*g_player)) {
			return 0;
		}*/
		return 0;
		//float adjustCollector = 0;
		//if (actor) {
		//	ActorEquipData* ad = actor->equipData;
		//	if (ad) {
		//		for (size_t i = 0; i < ad->kMaxSlots; i++)
		//		{
		//			auto curr = (TESObjectARMO*)ad->slots[i].item;
		//			if (curr && curr->formType == kFormType_ARMO) {
		//				/*const char* nam = curr->fullName.name.c_str();
		//				UInt32 wClass = curr->bipedObject.data.weightClass;*/
		//				//adjustCollector += wClass;

		//				/*TBO_InstanceData* inst = ad->slots[i].instanceData;
		//				if (inst) {
		//				TESObjectARMO::InstanceData* armInst = (TESObjectARMO::InstanceData*)inst;

		//				}*/

		//				if (curr->addons.count > 0) {
		//					for (size_t j = 0; j < curr->addons.count; j++)
		//					{
		//						TESObjectARMA* currArma = curr->addons.entries[j].armorAddon;
		//						if (currArma) {
		//							/*char n = 'a';
		//							TESNPC* npc = (TESNPC*)actor->baseForm;
		//							currArma->GetNodeName(&n, npc, curr);
		//							auto aa = currArma->bipedObject.GetSlotMask();
		//							int bb = slotMaskToSlot(aa);
		//							if ((aa & currArma->bipedObject.kPart_Body)) {
		//								_MESSAGE("body");
		//							}
		//							int b = slotMaskToSlot(aa);*/
		//							if (currArma->weaponAdjust > adjustCollector) {
		//								adjustCollector = currArma->weaponAdjust;
		//							}
		//							//adjustCollector += currArma->weaponAdjust;
		//							//sizeof(float) == sizeof(UInt64)
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}
		//}
		//return adjustCollector;
	}

	bool CheckIsExcludedWeapon(TESObjectWEAP* weap) {
		//return false;
		/*if (weap && !ExcludedForms.empty() && std::find(ExcludedForms.begin(), ExcludedForms.end(), weap->formID) != ExcludedForms.end()) {
			return true;
		}*/
		if (weap && weap->weapData.damageResist == CheckAvif) {
			return true;
		}
		return false;
	}


	void HandleAdditionalParts(NiNode* skeletonNode, Actor* actor, ActorValueInfo* avifToCheck, AdditionalItemType type) {
		if (!actor || actor->actorValueOwner.GetBase(avifToCheck) > 0) {


			return;
		}
		float base = actor->actorValueOwner.GetBase(avifToCheck);
		_MESSAGE(std::to_string(base).c_str());
		_MESSAGE("herecomesjony");
		//auto weaponOutter = skeletonNode->GetObjectByName(&BSFixedString("Weapon"));
		//if (!weaponOutter) {
		//	return;
		//}

		////NiAVObject* inner2 = nullptr;
		//auto weaponOutterNode = weaponOutter->GetAsNiNode();


		//NiAVObject * weapon = (weaponOutterNode && weaponOutterNode->m_children.m_size > 0) ? weaponOutterNode->m_children.m_data[0] : weapon;
		//NiNode* weaponNode = nullptr;
		//if (weapon) {
		//	weaponNode = weapon->GetAsNiNode();
		//}

		//DoAdditionalPartsAttachent(skeletonNode, actor, avifToCheck, type);

	}


	void HandleWeaponDisplay(bool isWeaponDrawn, NiAVObject* weaponNodeRef, Actor* actor, TESObjectWEAP* weap, TESObjectWEAP::InstanceData* inst, bool isManual) {
		bool ovDef = false;
		if (!actor) {
			actor = (*g_player);
		}
		//if (actor->baseForm->formType == kFormType_NPC_) {
		//	TESNPC* npc = (TESNPC*)actor->baseForm;
		//	/*const char* n = CALL_MEMBER_FN(actor, GetReferenceName)();
		//	_MESSAGE(n);*/
		//	if (npc->nativeTerminalForm.terminal == CheckTerminal) {
		//		return;
		//	}
		//}



		if (actor->baseForm->formType == kFormType_NPC_) {

			if (!actor->race || actor->race == VertibirdRace) return;


			/*if (actor->race == HumanRace || actor->race == GhoulRace) {

			}
			else if (std::find(AvailableRaces.begin(), AvailableRaces.end(), actor->race->formID) == AvailableRaces.end()) return;*/

			if (actor->baseForm->formID == (*g_player)->baseForm->formID) {
				float val = actor->actorValueOwner.GetBase(CheckAvif);
				/*_MESSAGE("player, now get check value");

				_MESSAGE(std::to_string(val).c_str());*/

			}
			float val = actor->actorValueOwner.GetBase(CheckAvif);
			if (val == 1) {
				return;
			}
		}
		NiNode* actorRootNode3rd = actor->GetActorRootNode(false);
		//NiNode* actorRootNode3rd = actor->GetObjectRootNode;


		NiAVObject* m = nullptr;

		TESObjectWEAP::InstanceData* currweapInst = nullptr;
		TESObjectWEAP* currweap = nullptr;

		if (!weap && !inst) {
			if (!actor || !actor->middleProcess || !actor->middleProcess->unk08 || actor->middleProcess->unk08->equipData.count == 0 || !actor->middleProcess->unk08->equipData.entries[0].data || actor->middleProcess->unk08->equipData.entries[0].data->m_uiRefCount == 0) {
				return;
			}
			//actor->middleProcess->unk08->lock.Lock();
			TBO_InstanceData* mm = actor->middleProcess->unk08->equipData.entries[0].instanceData;
			TESForm* mmm = actor->middleProcess->unk08->equipData.entries[0].object;




			//currweapInst = (TESObjectWEAP::InstanceData*)mm;
			if (mm && mm->refCount > 0) {

				//currweapInst = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(mm, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				currweapInst = reinterpret_cast<TESObjectWEAP::InstanceData*>(mm);

			}
			//currweapInst = (TESObjectWEAP::InstanceData*)mm;

			currweap = (TESObjectWEAP*)mmm;
			//currweap = (TESObjectWEAP*)Runtime_DynamicCast(mmm, RTTI_TESForm, RTTI_TESObjectWEAP);
			if (actor != (*g_player) && CheckIsExcludedWeapon(currweap)) {
				return;
			}
			//actor->middleProcess->unk08->lock.Release();
		}
		else {
			if (weap) {
				currweap = weap;
			}
			if (inst) {
				currweapInst = inst;
			}
		}

		//HandleAdditionalParts(actorRootNode3rd, actor, TypeAlwaysChecker, AdditionalItemType::CHW_Always);

		if (!isWeaponDrawn) {
			std::string nodeName = GetAttachNodeFromWeapon(currweapInst, currweap);

			if (!nodeName.empty() && actorRootNode3rd) {



				//auto chest = actorRootNode3rd->GetObjectByName(&BSFixedString("Chest"));
				auto weaponOutter = actorRootNode3rd->GetObjectByName(&BSFixedString("Weapon"));
				if (!weaponOutter) {
					return;
				}

				//NiAVObject* inner2 = nullptr;
				auto weaponOutterNode = weaponOutter->GetAsNiNode();


				m = (weaponOutterNode && weaponOutterNode->m_children.m_size > 0) ? weaponOutterNode->m_children.m_data[0] : m;

				/*if (!m) {
				ClearAllHolsters(actor);
				}*/


				NiAVObject* rr2 = actorRootNode3rd->GetObjectByName(&BSFixedString(nodeName.c_str()));

				if (rr2) {
					NiNode* rr3 = rr2->GetAsNiNode();
					if (rr3) {
						if (actor == (*g_player) && (GWeaponInstance != currweapInst || GWeaponForm != currweap)) {
							ClearAllHolsters(actor);
							if (CheckIsExcludedWeapon(currweap)) {
								return;
							}

							GWeaponInstance = currweapInst;
							GWeaponForm = currweap;
						}
						if (actor == (*g_player) && isManual && CheckIsExcludedWeapon(currweap)) {
							return;
						}
						/*if (rr3->m_children.m_size > 0) {
						NiAVObject* potentialUnload = rr3->m_children.m_data[0];
						if (!potentialUnload) {
						potentialUnload.mode
						ClearAllHolsters(actor);
						}
						}*/
						//BowStaggerBonus is used to store holster state for actors
						bool passAnyway = false;
						if ((actor != (*g_player) && rr3->m_children.m_size >= 1)) {
							auto d = rr3->m_children.m_data[0];
							if (d) {
								auto dd = d->GetAsNiNode();
								if (dd) {
									if (dd->m_children.m_size == 1) {
										passAnyway = true;
									}
								}
							}
						}

						if (rr3->m_children.m_size == 0 || passAnyway) {

							/*NiCloningProcess cp;
							memset(&cp, 0, sizeof(NiCloningProcess));
							cp.unk60 = 0;*/
							/*cp.unk38 = 2;
							cp.unk40 = 2;
							cp.unk58 = 2;
							cp.unk64 = 2;*/
							if (actor == (*g_player)) {

								Positioner::SetActiveAttachBone(nodeName.c_str());
							}
							if (m) {


								/*_MESSAGE("----------------------------------");
								_MESSAGE("Attaching for");
								const char* n = CALL_MEMBER_FN(actor, GetReferenceName)();
								_MESSAGE(n);
								_MESSAGE(std::to_string(actor->formID).c_str());
								if (isWeaponDrawn) {
									_MESSAGE("drawn");

								}
								else {
									_MESSAGE("holstered");
								}
								_MESSAGE("And weapon:");
								_MESSAGE(currweap ? currweap->fullName.name.c_str(): "fucked weapon");*/
								ClearAllHolsters(actor);
								//_MESSAGE("Others cleared");
								NiCloningProcess cp = {};
								//memset(&cp, 0, sizeof(NiCloningProcess));
								//tHashSet<void*, void*> hs = {};
								//hs.FreeCount();
								//hs.Clear();
								//sizeof(tHashSet<void*, void*>);
								////memset(&hs, 0, sizeof(tHashSet<void*, void*>));
								//cp.unk00 = hs;
								/*cp.unk30 = 0;
								cp.unk38 = 0;
								cp.unk40 = 0;
								cp.unk48 = 0;
								cp.unk50 = 0;
								cp.unk58 = 0;
								cp.unk64 = 0;*/

								cp.unk60 = 1;

								//cp.m_eCopyType = NiObjectNET::CopyType::COPY_EXACT;
								/*cp.m_eAffectedNodeRelationBehavior = NiCloningProcess::CLONE_RELATION_CLONEDONLY;
								cp.m_eDynamicEffectRelationBehavior = NiCloningProcess::CLONE_RELATION_CLONEDONLY;*/
								//cp.m_pkCloneMap = {};
								std::vector<const char *> skinnedNodeNames = {};
								m->Visit([&](NiAVObject* obj) {
									if (obj && obj->GetRTTI()) {

										auto shape = obj->GetAsBSTriShape();
										if (shape && shape->skinInstance) {
											//if (shape->vertexDesc & shape->kFlag_Skinned) {
											skinnedNodeNames.push_back(shape->m_name.c_str());
											//}
										}
									}
									return false;
									});

								//_MESSAGE("clone begin");
								NiObject* clone = m->CreateClone(&cp);

								//_MESSAGE("clone end");
								NiNode* clone2 = clone->GetAsNiNode();

								clone2->m_name = "VHWWeapon";
								clone2->m_spCollisionObject = nullptr;
								rr3->AttachChild(clone2, true);


								//_MESSAGE("attached");
								//orig = m->GetAsNiNode();
								handleMovingPartsTransformCorrection(clone2, &skinnedNodeNames);
								//_MESSAGE("parts corrected");

								/*clone2->m_localTransform.pos.x += additionalOffset.m_kCenter.x;
								clone2->m_localTransform.pos.y += additionalOffset.m_kCenter.y;
								clone2->m_localTransform.pos.z += additionalOffset.m_kCenter.z;*/

								float adjustVal = CollectAdjustWeapon(actor);
								adjustVal = 0;

								/*WeaponPosition temp = AdjustPositioning(actor, nodeName);
								rr3->m_localTransform.pos.x = temp.posX;
								rr3->m_localTransform.pos.y = temp.posY;
								rr3->m_localTransform.pos.z = temp.posZ;*/

								double thin = 0;
								float muscular = 0;
								float large = 0;
								TESNPC* b = (TESNPC*)actor->baseForm;
								if (b) {
									thin = (b->weightThin < FLT_MAX&& b->weightThin > FLT_MIN) ? b->weightThin : 0;
									muscular = (b->weightMuscular < FLT_MAX&& b->weightMuscular > FLT_MIN) ? b->weightMuscular : 0;
									large = (b->weightLarge < FLT_MAX&& b->weightLarge > FLT_MIN) ? b->weightLarge : 0;
								}

								float bodyWeightFormula = (large * 0.8) - (thin * 0.8) + ((muscular / 2) * 0.8);


								/*_MESSAGE("----------------------------------");
								const char* n = CALL_MEMBER_FN(actor, GetReferenceName)();
								_MESSAGE(n);
								_MESSAGE(std::to_string(actor->formID).c_str());
								if (isWeaponDrawn) {
									_MESSAGE("drawn");

								}
								else {
									_MESSAGE("holstered");
								}*/
								switch (DimensionToAffect)
								{
								case X:
								{

									float pX = bodyWeightFormula > 0 ? max(clone2->m_localTransform.pos.x, bodyWeightFormula) : min(clone2->m_localTransform.pos.x, bodyWeightFormula);


									float fX = bodyWeightFormula > 0 ? (pX * bodyWeightFormula) + pX : pX - (pX * bodyWeightFormula);
									fX = fX + (fX + adjustVal);
									clone2->m_localTransform.pos.x = fX;
								}
								break;
								case MinusZ:
								{

									float pMinusZ = bodyWeightFormula > 0 ? max(clone2->m_localTransform.pos.z, bodyWeightFormula) : min(clone2->m_localTransform.pos.z, bodyWeightFormula);

									float fMinusZ = bodyWeightFormula > 0 ? (pMinusZ * bodyWeightFormula) + pMinusZ : pMinusZ - (pMinusZ * bodyWeightFormula);
									fMinusZ = fMinusZ + (fMinusZ + adjustVal);
									clone2->m_localTransform.pos.z = -fMinusZ;
									//clone2->m_localTransform.pos.z = 20;

								}
								break;
								default:
									break;
								}

								/*if (actor == (*g_player)) {
								_MESSAGE(std::to_string(ToMoveOnX).c_str());
								if (ToMoveOnX > 0) {

								clone2->m_localTransform.pos.x -= ToMoveOnX;
								}
								else {
								clone2->m_localTransform.pos.x += ToMoveOnX;
								}
								}*/

								//NiAVObject::NiUpdateData ctx = {};
								//clone2->UpdateWorldBound();
								//clone2->UpdateWorldData(&ctx);
								//clone2->UpdateControllers();
								////clone2->UpdateDownwardPass();
								//clone2->UpdateTransformAndBounds();
								/*_MESSAGE("all good");
								_MESSAGE("----------------------------------");*/
							}
						}

					}
				}
			}
		}
		else {
			ClearAllHolsters(actor);
		}
	}

	//Clone transforms of moving parts from base nif as sometimes they are cleared afterwards from equip data
	void cloneMovingPartTransforms(NiAVObject* root, const char* modelName) {
		if (root) {
			bool marked = false;
			std::vector<NiNode*> arrOfEls = {};
			NiNode* markElement = nullptr;
			if (!root->GetObjectByName(&BSFixedString(VHWMark))) {

				root->Visit([&](NiAVObject* object)
					{
						if (!marked) {
							marked = true;
							NiNode* firstNode = object->GetAsNiNode();
							if (firstNode) {
								NiNode* mark = firstNode->Create(0);
								if (mark) {
									//firstNode->InsertChildAt(0, mark);
									mark->m_name = VHWMark;
									markElement = mark;
								}
							}
						}
						std::string str = IsValidBSString(object->m_name) ? object->m_name.c_str() : "";
						auto rtti = object->GetRTTI();
						if (!rtti) {
							return false;
						}
						/*if (!_strcmpi("WeaponExtra2", str.c_str())) {
						_MESSAGE("here");
						}*/
						//if (!str.empty() && (str.find("Weapon") != std::string::npos || (str.find("Receiver") != std::string::npos)) && str.find("VHW") == std::string::npos && (_strcmpi("Weapon", str.c_str()) > 0 || _strcmpi("Weapon", str.c_str()) < 0)) {
						if (!str.empty() && (!_strcmpi("NiNode", rtti->name)) && (_strcmpi("Weapon", str.c_str()) > 0 || _strcmpi("Weapon", str.c_str()) < 0) && str.find("VHW") == std::string::npos) {

							NiNode* parent = object->m_parent;
							if (parent) {
								NiNode* created = parent->Create(0);
								if (created) {
									std::string fin = mAbbrv + std::string(object->m_name.c_str());
									created->m_parent = parent;
									//parent->InsertChildAt(0, created);
									created->m_name = fin.c_str();
									created->m_localTransform = object->m_localTransform;
									created->m_worldTransform = object->m_worldTransform;
									created->m_worldBound = object->m_worldBound;
									arrOfEls.push_back(created);
									//created->IncRef();
								}
							}

						}
						//if (str.find("WeaponBolt") != std::string::npos && str.find("VHW") == std::string::npos) {
						//	NiNode* parent = object->m_parent;
						//	if (parent) {
						//		NiNode* created = parent->Create(0);
						//		if (created) {
						//			std::string fin = mAbbrv + std::string(object->m_name.c_str());
						//			created->m_parent = parent;
						//			//parent->InsertChildAt(0, created);
						//			created->m_name = fin.c_str();
						//			created->m_localTransform = object->m_localTransform;
						//			created->m_worldTransform = object->m_worldTransform;
						//			created->m_worldBound = object->m_worldBound;
						//			arrOfEls.push_back(created);
						//			//created->IncRef();
						//		}
						//	}

						//}
						return false;
					});

				NiNode* rootAsNode = root->GetAsNiNode();
				if (rootAsNode) {
					rootAsNode->AttachChild(markElement, true);

					for each (NiNode * var in arrOfEls)
					{
						rootAsNode->AttachChild(var, true);
					}
				}


				arrOfEls.clear();

			}
		}
	}

	bool once = false;


	//NOT USED
	void CollectRacesForSkeleton() {
		auto races = (*g_dataHandler)->arrRACE;

		auto loweredActorSkeletonPath = std::string(ActorSkeletonPath);

		std::transform(loweredActorSkeletonPath.begin(), loweredActorSkeletonPath.end(), loweredActorSkeletonPath.begin(), ::tolower);

		std::string ext("meshes\\");

		size_t pos = loweredActorSkeletonPath.find(ext);
		if (pos != std::string::npos)
		{

			loweredActorSkeletonPath.erase(pos, ext.length());
		}

		for (size_t i = 0; i < races.count; i++)
		{
			auto race = races[i];
			if (!race) continue;

			auto n1 = std::string(race->models[0].GetModelName());
			std::transform(n1.begin(), n1.end(), n1.begin(), ::tolower);

			auto n2 = std::string(race->models[1].GetModelName());
			std::transform(n2.begin(), n2.end(), n2.begin(), ::tolower);



			if (n1 == loweredActorSkeletonPath || n2 == loweredActorSkeletonPath) {
				if (std::find(AvailableRaces.begin(), AvailableRaces.end(), race->formID) == AvailableRaces.end())
				{

					auto name = race->fullName.name;

					auto editorId = std::string(race->editorId.c_str());

					std::transform(editorId.begin(), editorId.end(), editorId.begin(), ::tolower);

					size_t found = editorId.find("subgraph");
					size_t found2 = editorId.find("additive");


					if (found == std::string::npos) {
						AvailableRaces.push_back(race->formID);
						_MESSAGE("Handling race:");
						_MESSAGE(name);
						_MESSAGE("Handling race editor id:");
						_MESSAGE(race->editorId);
						_MESSAGE("_______________________");
					}


				}
			}



		}

	}

	void CollectWeaponOMODModels() {
		if (once) return;

		auto omods = (*g_dataHandler)->arrOMOD;

		for (size_t i = 0; i < omods.count; i++)
		{
			auto omod = omods[i];
			if (!omod) continue;
			if (omod->targetType == omod->kTargetType_Weapon) {
				const char* model = omod->materialSwap.GetModelName();
				if (model && model[0]) {
					auto modelStr = std::string(model);
					std::transform(modelStr.begin(), modelStr.end(), modelStr.begin(), ::tolower);
					size_t lastindex = modelStr.find_last_of(".");
					std::string rawname = modelStr.substr(0, lastindex);
					std::replace(rawname.begin(), rawname.end(), '/', '\\');
					if (std::find(WeaponPaths.begin(), WeaponPaths.end(), rawname) == WeaponPaths.end())
					{
						WeaponPaths.push_back(rawname);
					}
				}
			}
		}

		auto weapons = (*g_dataHandler)->arrWEAP;

		for (size_t i = 0; i < weapons.count; i++)
		{
			auto weapon = weapons[i];
			if (!weapon) continue;
			const char* model = weapon->materialSwap.GetModelName();
			if (model && model[0]) {
				auto modelStr = std::string(model);
				std::transform(modelStr.begin(), modelStr.end(), modelStr.begin(), ::tolower);
				size_t lastindex = modelStr.find_last_of(".");
				std::string rawname = modelStr.substr(0, lastindex);
				std::replace(rawname.begin(), rawname.end(), '/', '\\');
				if (std::find(WeaponPaths.begin(), WeaponPaths.end(), rawname) == WeaponPaths.end())
				{
					WeaponPaths.push_back(rawname);
				}
			}

		}
		once = true;
	}

	class CustomModelProcessor : public BSModelDB::BSModelProcessor
	{
	public:
		CustomModelProcessor(BSModelDB::BSModelProcessor* oldProcessor) : m_oldProcessor(oldProcessor) { }

		virtual void Process(BSModelDB::ModelData* modelData, const char* modelName, NiAVObject** root, UInt32* typeOut);

		DEFINE_STATIC_HEAP(Heap_Allocate, Heap_Free)

	protected:
		BSModelDB::BSModelProcessor* m_oldProcessor;
	};

	void SetModelProcessor()
	{
		(*g_TESProcessor) = new CustomModelProcessor(*g_TESProcessor);
	}


	bool GetExtraString(NiAVObject* rootPointer) {
		//NiAVObject* wp = rootPointer->GetObjectByName(&BSFixedString("WEAPON"))
		NiExtraData* ed = rootPointer->GetExtraData("Prn");
		if (ed) {
			NiStringExtraData* stra = (NiStringExtraData*)ed;
			if (stra) {
				std::string str = stra->m_string.c_str();
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				if (!_strcmpi(str.c_str(), "weapon")) {
					return true;
				}
			}
		}
		/*auto extraDataList = rootPointer->m_extraData;
		if (extraDataList) {
		if (extraDataList)
		{
		extraDataList->lock.Lock();
		for (UInt32 i = 0; i < extraDataList->count; i++)
		{
		std::string str = extraDataList->entries[i]->m_name;
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		if (!_strcmpi(str.c_str(), "weapon")) {
		return true;
		}
		}
		extraDataList->lock.Release();
		}

		}*/
		return false;
	}
	void CustomModelProcessor::Process(BSModelDB::ModelData* modelData, const char* modelName, NiAVObject** root, UInt32* typeOut) {
		if (root && modelName) {
			NiAVObject* rootPointer = (*root);
			if (rootPointer) {
				std::string str = std::string(modelName);
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);


				if (str.find(ActorSkeletonPath) != std::string::npos) {
					/*Positioner::SetSkeletonNode((*root)->GetAsNiNode());
					std::string s = Positioner::GetActiveBoneName();
					if (!s.empty()) {

					Positioner::SetActiveAttachBone(s.c_str());
					}*/
					if (!rootPointer->GetObjectByName(&BSFixedString(WeaponRifleBoneAttachPos.name))) {
						attachCustomNodeOnRootNode(rootPointer);
					}

				}

				if (AdditonalSkeletonPaths.size() > 0) {
					//_MESSAGE(str.c_str());
					if (std::find(AdditonalSkeletonPaths.begin(), AdditonalSkeletonPaths.end(), str) != AdditonalSkeletonPaths.end()) {
						if (!rootPointer->GetObjectByName(&BSFixedString(WeaponRifleBoneAttachPos.name))) {
							attachCustomNodeOnRootNode(rootPointer);
						}
					}
				}

				if (rootPointer->GetObjectByName(&BSFixedString("CHWHandledSkeleton")) && !rootPointer->GetObjectByName(&BSFixedString(WeaponRifleBoneAttachPos.name))) {
					attachCustomNodeOnRootNode(rootPointer);
				}


				//std::string rootName = rootPointer->m_name.c_str();
				//bool skipThis = false;
				/*if (str.find("jetpack") != std::string::npos) {
					_MESSAGE(str.c_str());
				}*/

				if (str.find(WeaponsPath) != std::string::npos) {
					std::string ext("meshes\\");
					std::string ext2("meshes/");

					bool skipSecond = false;
					size_t pos = str.find(ext);
					if (pos != std::string::npos)
					{

						str.erase(pos, ext.length());
						skipSecond = true;
					}

					if (!skipSecond) {
						size_t pos = str.find(ext2);
						if (pos != std::string::npos)
						{
							// If found then erase it from string
							str.erase(pos, ext2.length());
						}
					}

					std::replace(str.begin(), str.end(), '/', '\\');

					//_MESSAGE(str.c_str());

					size_t lastindex = str.find_last_of(".");
					std::string rawname = str.substr(0, lastindex);

					for (const auto& elem : WeaponPaths) {

						std::size_t found = rawname.find(elem);
						if (found != std::string::npos) {
							cloneMovingPartTransforms(rootPointer, modelName);
							//_MESSAGE(str.c_str());

						}
						/*else {
							std::size_t found2 = elem.find(str);
							if (found2 != std::string::npos) {
								cloneMovingPartTransforms(rootPointer, modelName);
								_MESSAGE(str.c_str());

							}
						}*/
					}

					/*for (size_t i = 0; i < WeaponPaths.size(); i++)
					{

					}*/

					/*if (std::find(WeaponPaths.begin(), WeaponPaths.end(), str) != WeaponPaths.end())
					{
					}*/
				}


				//OLD WAY OF HANDLING PATHS
				/*if (str.find(WeaponsPath) != std::string::npos) {
					for (size_t i = 0; i < ExcludedFolderPaths.size(); i++)
					{
						const char * path = ExcludedFolderPaths[i];
						if (str.find(path) != std::string::npos) {
							skipThis = true;
							break;
						}
					}

					if (!skipThis) {
						if (str.find("marker") == std::string::npos) {
							if (IsValidBSString(rootPointer->m_name)) {


								cloneMovingPartTransforms(rootPointer, modelName);
							}
							else {
								if (GetExtraString(rootPointer)) {
									cloneMovingPartTransforms(rootPointer, modelName);
								}
							}
						}
					}

				}*/

			}
		}
		if (m_oldProcessor)
			m_oldProcessor->Process(modelData, modelName, root, typeOut);
	}

	void SetArraysAfterFormsLoad() {

		if (IsRiflesDisplayed) {
			AttachPosMap.insert({ WeaponTypeRifle, WeaponRifleBoneAttachPos });
			KeywordsVec.push_back(WeaponTypeRifle);
		}
		if (IsPistolsDisplayed) {
			AttachPosMap.insert({ WeaponTypePistol, WeaponPistolUpAttachPos });
			KeywordsVec.push_back(WeaponTypePistol);
		}
		if (Is1HMeleeDisplayed) {
			AttachPosMap.insert({ WeaponType1HMelee, WeaponMelee1HAttachPos });
			KeywordsVec.push_back(WeaponType1HMelee);
		}
		if (Is2HMeleeDisplayed) {
			AttachPosMap.insert({ WeaponType2HMelee, WeaponMelee2HAttachPos });
			KeywordsVec.push_back(WeaponType2HMelee);
		}
		if (IsBigGunsDisplayed) {
			AttachPosMap.insert({ WeaponTypeBigGun, WeaponRifleBoneAttachPos });
			KeywordsVec.push_back(WeaponTypeBigGun);
		}


		/*AttachPosMap = {
		{ WeaponTypeRifle, WeaponRifleBoneAttachPos },
		{ WeaponTypePistol, WeaponPistolUpAttachPos },
		{ WeaponType2HMelee, WeaponMelee2HAttachPos },
		{ WeaponType1HMelee, WeaponMelee1HAttachPos },

		};

		KeywordsVec = {
		WeaponTypeRifle,
		WeaponTypePistol,
		WeaponType2HMelee,
		WeaponType1HMelee
		};*/

	}

	void InitMainVec() {

	}

}


//WeaponPosition AdjustPositioning(Actor* actor, std::string attachNodeName) {

//	NiAVObject* neededParent = nullptr;
//	WeaponPosition temp = {};

//	auto ac = actor->GetActorRootNode(false);
//	if (ac) {
//		auto ob = ac->GetObjectByName(&BSFixedString(attachNodeName.c_str()));
//		if (ob) {
//			neededParent = ob->m_parent;
//		}
//	}
//	if (!neededParent) {
//		return temp;
//	}

//	
//	int adjustCollector = 0;
//	if (actor) {
//		ActorEquipData* ad = actor->equipData;
//		if (ad) {
//			for (size_t i = 0; i < ad->kMaxSlots; i++)
//			{
//				auto curr = (TESObjectARMO*)ad->slots[i].item;
//				if (curr && curr->formType == kFormType_ARMO) {
//					const char* nam = curr->fullName.name.c_str();
//					UInt32 wClass = curr->bipedObject.data.weightClass;
//					adjustCollector += wClass;

//					NiAVObject* armorNode = ad->slots[i].node;
//					if (armorNode && false) {
//						armorNode->Visit([&](NiAVObject* obj) {
//							if (obj) {
//								auto subTri = obj->GetAsBSSubIndexTriShape();
//								if (subTri) {
//									BSSkin::Instance* skin = subTri->skinInstance;
//									if (skin) {
//										int index = -1;
//										for (size_t i = 0; i < skin->bones.count; i++)
//										{
//											std::string comb = std::string( neededParent->m_name.c_str()) + "_skin";
//											//std::transform(attachNodeName.begin(), attachNodeName.end(), attachNodeName.begin(), ::tolower);
//											//std::string transformed = skin->bones[i]->m_name;
//											//std::transform(transformed.begin(), transformed.end(), transformed.begin(), ::tolower);
//											NiNode* bn = skin->bones[i];
//											if (bn) {
//												if (!_strcmpi(comb.c_str(), bn->m_name.c_str())) {
//													index = i;
//													break;
//												}
//											}
//										}
//										if (index > -1) {
//											if (skin->boneData) {
//												auto transforms = skin->boneData->transforms;
//												auto neededT = transforms.entries[index];
//												temp.posX = neededT.transform.pos.x;
//												temp.posY = neededT.transform.pos.y;
//												temp.posZ = neededT.transform.pos.z;
//												return true;
//											}
//										}
//									}
//								}
//							}
//							return false;
//						});
//					}

//					/*TBO_InstanceData* inst = ad->slots[i].instanceData;
//					if (inst) {
//						TESObjectARMO::InstanceData* armInst = (TESObjectARMO::InstanceData*)inst;

//					}*/
//					return temp;
//				}
//			}
//		}
//	}
//}
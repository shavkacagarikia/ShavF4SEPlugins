
#pragma once
#include "f4se\GameForms.h"
#include "Globals.h"
#include "Utils.h"

bool areFormsValid = true;


#define VERIFY_FORM(name, formId) verifier(name, #name, formId)
template <typename T>
T verifier(T form, const char* name, const char* formId) {

	std::string toFind = formId;


	auto f = reinterpret_cast<T>(GetFormFromIdentifier(toFind));
	if (f) {
		form = f;
		if (f->formType == f->kTypeID) {
			return form;
		}
	}
	logMessage("Unable to get valid form - [" + std::string(name) + "] with formid: [" + std::string(formId) + "], you are lacking some file/files");
	areFormsValid = false;
	return nullptr;
}

bool GetForms() {

	AffectingAvif = VERIFY_FORM(AffectingAvif,sAffectingAvifFormId.c_str());
	HasImprovedDetection = VERIFY_FORM(HasImprovedDetection, "Fallout4.esm|309");
	Invisibility = VERIFY_FORM(Invisibility, "Fallout4.esm|2F3");


	return areFormsValid;
}
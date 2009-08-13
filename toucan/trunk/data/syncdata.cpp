/////////////////////////////////////////////////////////////////////////////////
// Author:      Steven Lamerton
// Copyright:   Copyright (C) 2007-2009 Steven Lamerton
// License:     GNU GPL 2 (See readme for more info)
/////////////////////////////////////////////////////////////////////////////////

#include "syncdata.h"
#include "../rules.h"
#include "../toucan.h"
#include "../variables.h"
#include "../forms/frmmain.h"
#include "../controls/vdtc.h"
#include <wx/variant.h>
#include <wx/fileconf.h>

bool SyncData::TransferFromFile(){
	bool error = false;
	bool btemp;
	wxString stemp;

	if(!wxGetApp().m_Jobs_Config->Exists(GetName())){
		return false;
	}

	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/Source"), &stemp)) SetSource(stemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/Dest"), &stemp))  SetDest(stemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/Function"), &stemp))  SetFunction(stemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/TimeStamps"), &btemp))  SetTimeStamps(btemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/Attributes"), &btemp))  SetAttributes(btemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/IgnoreReadOnly"), &btemp))  SetIgnoreRO(btemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/IgnoreDaylightSavings"), &btemp))  SetIgnoreDLS(btemp);
		else error = true;
	if(wxGetApp().m_Jobs_Config->Read(GetName() + wxT("/Rules"), &stemp))  SetRules(new Rules(stemp, true));
		else error = true;

	if(error){
		wxMessageBox(_("There was an error reading from the jobs file"), _("Error"), wxICON_ERROR);
		return false;
	}
	return true;
}

bool SyncData::TransferToFile(){
	bool error = false;

	wxGetApp().m_Jobs_Config->DeleteGroup(GetName());

	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Source"),  GetSource())) error = true;	
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Dest"), GetDest())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Function"), GetFunction())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/TimeStamps"), GetTimeStamps())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Attributes"), GetAttributes())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/IgnoreReadOnly"), GetIgnoreRO())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/IgnoreDaylightSavings"), GetIgnoreDLS())) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Rules"), GetRules() ? GetRules()->GetName() : wxT(""))) error = true;
	if(!wxGetApp().m_Jobs_Config->Write(GetName() + wxT("/Type"),  wxT("Sync"))) error = true;

	wxGetApp().m_Jobs_Config->Flush();

	if(error){
		wxMessageBox(_("There was an error saving to the jobs file, \nplease check it is not set as read only or in use"), _("Error"), wxICON_ERROR);
		return false;
	}
	return true;
}

bool SyncData::TransferToForm(frmMain *window){
	if(!window){
		return false;
	}

	window->m_Sync_Source_Txt->SetValue(GetSource());
	window->m_Sync_Source_Tree->DeleteChildren(window->m_Sync_Source_Tree->GetRootItem());
	//ATTN : Move the wxEmptyString checks into the DirCtrl code 
	if(GetSource() != wxEmptyString){
		window->m_Sync_Source_Tree->AddNewPath(Normalise(GetSource()));		
	}

	window->m_Sync_Dest_Txt->SetValue(GetDest());
	window->m_Sync_Dest_Tree->DeleteChildren(window->m_Sync_Dest_Tree->GetRootItem());
	if(GetDest() != wxEmptyString){
		window->m_Sync_Dest_Tree->AddNewPath(Normalise(GetDest()));		
	}

	window->m_Sync_Function->SetStringSelection(GetFunction());
	window->m_Sync_Timestamp->SetValue(GetTimeStamps());
	window->m_Sync_Attributes->SetValue(GetAttributes());
	window->m_Sync_Ignore_Readonly->SetValue(GetIgnoreRO());
	window->m_Sync_Ignore_DaylightS->SetValue(GetIgnoreDLS());
	window->m_Sync_Rules->SetStringSelection(GetRules()->GetName());
	return true;
}

bool SyncData::TransferFromForm(frmMain *window){
	if(!window){
		return false;
	}
	SetSource(window->m_Sync_Source_Txt->GetValue());
	SetDest(window->m_Sync_Dest_Txt->GetValue());
	SetFunction(window->m_Sync_Function->GetStringSelection());
	SetTimeStamps(window->m_Sync_Timestamp->GetValue());
	SetAttributes(window->m_Sync_Attributes->GetValue());
	SetIgnoreRO(window->m_Sync_Ignore_Readonly->GetValue());
	SetIgnoreDLS(window->m_Sync_Ignore_DaylightS->GetValue());
	SetRules(new Rules(window->m_Sync_Rules->GetStringSelection(), true));
	return true;	
}

void SyncData::Output(){
	wxMessageBox (GetSource(), wxT("Source"));
	wxMessageBox(GetDest(), wxT("Destination"));
	wxMessageBox(GetRules()->GetName(), wxT("Rules"));
	wxMessageBox(GetFunction(), wxT("Function"));
	wxVariant temp = GetTimeStamps();
	wxMessageBox(temp.GetString(), wxT("Timestamps?"));
	temp = GetAttributes();
	wxMessageBox(temp.GetString(), wxT("Attributes?"));
	temp = GetIgnoreRO();
	wxMessageBox(temp.GetString(), wxT("Ignore Readonly?"));
	temp = GetIgnoreDLS();
	wxMessageBox(temp.GetString(), wxT("Ignore Daylight Savings?"));
}

bool SyncData::IsReady(){
	if(GetSource() == wxEmptyString || GetDest() == wxEmptyString || GetFunction() == wxEmptyString){
		return false;
	}
	else{
		return true;
	}
}
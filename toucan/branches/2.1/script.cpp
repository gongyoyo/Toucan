/////////////////////////////////////////////////////////////////////////////////
// Author:      Steven Lamerton
// Copyright:   Copyright (C) 2007-2009 Steven Lamerton
// License:     GNU GPL 2 (See readme for more info)
/////////////////////////////////////////////////////////////////////////////////

#include <wx/arrstr.h>
#include <wx/tokenzr.h>
#include <wx/fileconf.h>

#include "script.h"
#include "waitthread.h"
#include "syncdata.h"
#include "sync.h"
#include "backupdata.h"
#include "backupprocess.h"
#include "securedata.h"
#include "secure.h"
#include "variables.h"
#include "basicfunctions.h"
#include "rootdata.h"
#include "loglistctrl.h"

void ScriptManager::SetCommand(int i){
	m_Command = i;
}

int ScriptManager::GetCommand(){
	return m_Command;
}

int ScriptManager::GetCount(){
	return m_Script.GetCount();
}

void ScriptManager::SetScript(wxArrayString script){
	m_Script.Clear();
	m_Script = script;
}

wxDateTime ScriptManager::GetTime(){
	return startTime;
}

wxArrayString ScriptManager::GetScript(){
	return m_Script;
}

bool ScriptManager::Validate(){
	//First check that the whole script is valid (basic number of parameters check)
	wxString strLine, strTemp;
	bool blParseError = false;
	bool blPassNeeded = false;
	for(unsigned int i = 0; i < m_Script.Count(); i++){
		strLine = m_Script.Item(i); 
		wxStringTokenizer tkz(strLine, wxT("\""), wxTOKEN_STRTOK);
		wxString strToken = tkz.GetNextToken();
		strToken.Trim();
		if(strToken == wxT("Sync") || strToken == wxT("Secure") || strToken == _("Delete") || strToken == _("Execute") || strToken == wxT("Backup") || strToken == wxT("Restore")){
			if(tkz.CountTokens() != 1){
				strTemp.Printf(_("Line %d has an incorrect number of parameters"), i+1);
				OutputProgress(strTemp);
				blParseError = true;
			}
		}
		else if(strToken == _("Move") || strToken == _("Copy") || strToken == _("Rename")){
			if(tkz.CountTokens() != 3){
				strTemp.Printf(_("Line %d has an incorrect number of parameters"), i+1);
				OutputProgress(strTemp);
				blParseError = true;
			}
		}
		else{
			strTemp.Printf(strToken + _(" not recognised on line %d"), i+1);
			OutputProgress(strTemp);
			blParseError = true;
		}
		if(strToken == wxT("Secure")){
			blPassNeeded = true;
		}
		if(strToken == wxT("Backup")){
			BackupData data;
			wxString strJob = tkz.GetNextToken();
			data.SetName(strJob);
			if(data.TransferFromFile()){
				if(data.IsPassword == true){
					blPassNeeded = true;
				}
			}
		}
	}
	if(blPassNeeded){
		wxString strPass = InputPassword();
		if(strPass == wxEmptyString){
			CleanUp();
			return false;
		}
		else{
			m_Password = strPass;
		}
	}	
	if(blParseError){
		return false;
	}
	return true;
}

bool ScriptManager::ParseCommand(int i){	
	wxDateTime now = wxDateTime::Now();
	
	wxString strLine = m_Script.Item(i);
	
	wxStringTokenizer tkz(strLine, wxT("\""), wxTOKEN_STRTOK);
	wxString strToken = tkz.GetNextToken();
	
	strToken.Trim();
	
	RootData* data;
	
	if(strToken == wxT("Sync")){
		data = new SyncData();
		strToken = tkz.GetNextToken();
		data->SetName(strToken);
	}	
	else if(strToken == wxT("Backup")){
		data = new BackupData();
		strToken = tkz.GetNextToken();
		data->SetName(strToken);
	}
	else if(strToken == wxT("Secure")){
		data = new SecureData();
		strToken = tkz.GetNextToken();
		data->SetName(strToken);
	}
	else if(strToken == _("Delete")){
		wxString strSource = Normalise(Normalise(tkz.GetNextToken()));
		if(wxRemoveFile(strSource)){
			OutputProgress(_("Deleted ") +strSource + wxT("\n"));	
		}
		else{
			OutputProgress(_("Failed to delete ") +strSource + wxT("\n"));				
		}
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, ID_SCRIPTFINISH);
		wxPostEvent(wxGetApp().ProgressWindow, event);	
		return true;
	}
	else if(strToken == _("Move")){
		wxString strSource = Normalise(Normalise(tkz.GetNextToken()));
		tkz.GetNextToken();
		wxString strDest = Normalise(Normalise(tkz.GetNextToken()));
		if(wxCopyFile(strSource, strDest, true)){
			if(wxRemoveFile(strSource)){
				OutputProgress(_("Moved") +strSource + wxT("\n"));	
			}
			else{
				OutputProgress(_("Failed to move ") + strSource + wxT("\n"));
			}
		}
		else{
			OutputProgress(_("Failed to move ") + strSource + wxT("\n"));		
		}
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, ID_SCRIPTFINISH);
		wxPostEvent(wxGetApp().ProgressWindow, event);	
		return true;
	}
	else if(strToken == _("Copy")){
		wxString strSource = Normalise(Normalise(tkz.GetNextToken()));
		tkz.GetNextToken();
		wxString strDest = Normalise(Normalise(tkz.GetNextToken()));
		if(wxCopyFile(strSource, strDest, true)){
			OutputProgress(_("Copied ") +strSource + wxT("\n"));	
		}
		else{
			OutputProgress(_("Failed to copy ") +strSource + wxT("\n"));
		}
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, ID_SCRIPTFINISH);
		wxPostEvent(wxGetApp().ProgressWindow, event);	
		return true;
	}
	else if(strToken == _("Rename")){
		wxString strSource = Normalise(Normalise(tkz.GetNextToken()));
		tkz.GetNextToken();
		wxString strDest = Normalise(Normalise(tkz.GetNextToken()));
		if(wxRenameFile(strSource, strDest, true)){
			OutputProgress(_("Renamed ") +strSource + wxT("\n"));	
		}
		else{
			OutputProgress(_("Failed to rename ") +strSource + wxT("\n"));
		}
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, ID_SCRIPTFINISH);
		wxPostEvent(wxGetApp().ProgressWindow, event);	
		return true;
	}
	else if(strToken == _("Execute")){
		wxString strExecute = Normalise(Normalise(tkz.GetNextToken()));
		wxExecute(strExecute, wxEXEC_SYNC|wxEXEC_NODISABLE);
		OutputProgress(_("Executed ") + strExecute + wxT("\n"));
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, ID_SCRIPTFINISH);
		wxPostEvent(wxGetApp().ProgressWindow, event);	
		return true;
	}
	else{
		return false;
	}
	
	if(!data->TransferFromFile()){
		CleanUp();
	}
	if(!data->NeededFieldsFilled()){
		ErrorBox(_("Not all of the required fields are filled"));
		CleanUp();
	}
	if(data->NeedsPassword()){
		data->SetPassword(m_Password);
	}
	
	Rules rules;
	if (wxGetApp().m_Jobs_Config->Read(data->GetName() + wxT("/Rules")) != wxEmptyString) {
		rules.TransferFromFile(wxGetApp().m_Jobs_Config->Read(data->GetName() + wxT("/Rules")));
	}
	
	if(!data->Execute(rules)){
		CleanUp();
	}
	
	return true;	
}

bool ScriptManager::Execute(){
	StartUp();
	if(!Validate()){
		CleanUp();
	}
	if(GetCount() != 0){
		SetCommand(1);
		ParseCommand(0);
	}
	return true;
}

bool ScriptManager::CleanUp(){
	m_ProgressWindow->m_OK->Enable(true);
	m_ProgressWindow->m_Save->Enable(true);
	m_ProgressWindow->m_Cancel->Enable(false);
	wxDateTime now = wxDateTime::Now();
	OutputBlank();
	OutputProgress(now.Subtract(startTime).Format(), _("Elapsed"));
	OutputProgress(now.FormatTime(), _("Finished"));
	
	//Resize the second column to show all of the text
	m_ProgressWindow->m_List->SetColumnWidth(1, -1);
	
	//Remove finished jobs
	if (wxGetApp().m_Jobs_Config->HasGroup(wxT("LastSyncJob"))){
		wxGetApp().m_Jobs_Config->DeleteGroup(wxT("LastSyncJob"));
	}
	if (wxGetApp().m_Jobs_Config->HasGroup(wxT("LastBackupJob"))){
		wxGetApp().m_Jobs_Config->DeleteGroup(wxT("LastBackupJob"));
	}
	if (wxGetApp().m_Jobs_Config->HasGroup(wxT("LastSecureJob"))){
		wxGetApp().m_Jobs_Config->DeleteGroup(wxT("LastSecureJob"));
	}
	if (wxGetApp().m_Jobs_Config->HasGroup(wxT("LastRestoreJob"))){
		wxGetApp().m_Jobs_Config->DeleteGroup(wxT("LastRestoreJob"));
	}
	wxGetApp().m_Jobs_Config->Flush();
	wxGetApp().MainWindow->m_Notebook->Enable();
	return true;	
}

bool ScriptManager::StartUp(){
	//Set up all of the form related stuff
	m_ProgressWindow = wxGetApp().ProgressWindow;
	m_ProgressWindow->m_List->DeleteAllItems();
	wxGetApp().MainWindow->m_Notebook->Disable();
	//Send all errors to the list control
	LogListCtrl* logList = new LogListCtrl(m_ProgressWindow->m_List);
	delete wxLog::SetActiveTarget(logList);
	//Set up the buttons on the progress box
	m_ProgressWindow->m_OK->Enable(false);
	m_ProgressWindow->m_Save->Enable(false);
	m_ProgressWindow->m_Cancel->Enable(true);
	
	//Send a blank item to get the item count up
	OutputBlank();
	startTime = wxDateTime::Now();
	OutputProgress(startTime.FormatTime(), _("Starting"));
	OutputBlank();
	//Show the window
	if(wxGetApp().blGUI){
		m_ProgressWindow->Refresh();
		m_ProgressWindow->Update();
		m_ProgressWindow->Show();
	}
	m_ProgressWindow->m_List->SetColumnWidth(1, m_ProgressWindow->m_List->GetClientSize().GetWidth() - m_ProgressWindow->m_List->GetColumnWidth(0));
	return true;
}
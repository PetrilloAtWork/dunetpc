////////////////////////////////////////////////////////////////////////
// \file SpaceChargeProtoDUNE.cxx
//
// \brief implementation of class for storing/accessing space charge distortions for ProtoDUNE
//
// \author mrmooney@colostate.edu
// 
////////////////////////////////////////////////////////////////////////
// C++ language includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "math.h"
#include "stdio.h"
// LArSoft includes
#include "dune/SpaceCharge/SpaceChargeProtoDUNE.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
// Framework includes
#include "cetlib_except/exception.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
// ROOT includes
#include "TFile.h"
#include "TH3.h"
#include "TTree.h"
#include "TLeaf.h"
//-----------------------------------------------
spacecharge::SpaceChargeProtoDUNE::SpaceChargeProtoDUNE(
  fhicl::ParameterSet const& pset
)
{
  //Configure(pset);
}
//------------------------------------------------
bool spacecharge::SpaceChargeProtoDUNE::Configure(fhicl::ParameterSet const& pset, 
							detinfo::DetectorProperties const* detprop)
{  

  fEnableSimSpatialSCE = pset.get<bool>("EnableSimSpatialSCE");
  fEnableSimEfieldSCE = pset.get<bool>("EnableSimEfieldSCE");
  fEnableCalSpatialSCE = pset.get<bool>("EnableCalSpatialSCE");
  fEnableCalEfieldSCE = pset.get<bool>("EnableCalEfieldSCE");
  
  //auto const *detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
  fEfield = detprop->Efield();
  
  if((fEnableSimSpatialSCE == true) || (fEnableSimEfieldSCE == true))
  {
    fRepresentationType = pset.get<std::string>("RepresentationType");
    fInputFilename = pset.get<std::string>("InputFilename");
    
    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(fInputFilename,fname);
    
    std::unique_ptr<TFile> infile(new TFile(fname.c_str(), "READ"));
    if(!infile->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname << "'!\n";
    
   if (fRepresentationType == "Voxelized_TH3") { 
        
      //Load in files
      TH3F* hDx_sim_pos_orig = (TH3F*)infile->Get("RecoFwd_Displacement_X_Pos");
      TH3F* hDy_sim_pos_orig = (TH3F*)infile->Get("RecoFwd_Displacement_Y_Pos");
      TH3F* hDz_sim_pos_orig = (TH3F*)infile->Get("RecoFwd_Displacement_Z_Pos");
      TH3F* hEx_sim_pos_orig = (TH3F*)infile->Get("Reco_ElecField_X_Pos");
      TH3F* hEy_sim_pos_orig = (TH3F*)infile->Get("Reco_ElecField_Y_Pos");
      TH3F* hEz_sim_pos_orig = (TH3F*)infile->Get("Reco_ElecField_Z_Pos");
      
      TH3F* hDx_sim_neg_orig = (TH3F*)infile->Get("RecoFwd_Displacement_X_Neg");
      TH3F* hDy_sim_neg_orig = (TH3F*)infile->Get("RecoFwd_Displacement_Y_Neg");
      TH3F* hDz_sim_neg_orig = (TH3F*)infile->Get("RecoFwd_Displacement_Z_Neg");
      TH3F* hEx_sim_neg_orig = (TH3F*)infile->Get("Reco_ElecField_X_Neg");
      TH3F* hEy_sim_neg_orig = (TH3F*)infile->Get("Reco_ElecField_Y_Neg");
      TH3F* hEz_sim_neg_orig = (TH3F*)infile->Get("Reco_ElecField_Z_Neg");
        
      TH3F* hDx_sim_pos = (TH3F*)hDx_sim_pos_orig->Clone("hDx_pos");
      TH3F* hDy_sim_pos = (TH3F*)hDy_sim_pos_orig->Clone("hDy_pos");
      TH3F* hDz_sim_pos = (TH3F*)hDz_sim_pos_orig->Clone("hDz_pos");
      TH3F* hEx_sim_pos = (TH3F*)hEx_sim_pos_orig->Clone("hEx_pos");
      TH3F* hEy_sim_pos = (TH3F*)hEy_sim_pos_orig->Clone("hEy_pos");
      TH3F* hEz_sim_pos = (TH3F*)hEz_sim_pos_orig->Clone("hEz_pos");
      
      TH3F* hDx_sim_neg = (TH3F*)hDx_sim_neg_orig->Clone("hDx_neg");
      TH3F* hDy_sim_neg = (TH3F*)hDy_sim_neg_orig->Clone("hDy_neg");
      TH3F* hDz_sim_neg = (TH3F*)hDz_sim_neg_orig->Clone("hDz_neg");
      TH3F* hEx_sim_neg = (TH3F*)hEx_sim_neg_orig->Clone("hEx_neg");
      TH3F* hEy_sim_neg = (TH3F*)hEy_sim_neg_orig->Clone("hEy_neg");
      TH3F* hEz_sim_neg = (TH3F*)hEz_sim_neg_orig->Clone("hEz_neg");
        
      hDx_sim_pos->SetDirectory(0);
      hDy_sim_pos->SetDirectory(0);
      hDz_sim_pos->SetDirectory(0);
      hEx_sim_pos->SetDirectory(0);
      hEy_sim_pos->SetDirectory(0);
      hEz_sim_pos->SetDirectory(0);
      
      hDx_sim_neg->SetDirectory(0);
      hDy_sim_neg->SetDirectory(0);
      hDz_sim_neg->SetDirectory(0);
      hEx_sim_neg->SetDirectory(0);
      hEy_sim_neg->SetDirectory(0);
      hEz_sim_neg->SetDirectory(0);
        
      SCEhistograms = {hDx_sim_pos, hDy_sim_pos, hDz_sim_pos, hEx_sim_pos, hEy_sim_pos, hEz_sim_pos, hDx_sim_neg, hDy_sim_neg, hDz_sim_neg, hEx_sim_neg, hEy_sim_neg, hEz_sim_neg};
                  
   } else if (fRepresentationType == "Voxelized") {
    
      //Load files and read in trees
      if (fInputFilename.find("NegX")<fInputFilename.length()){
        
        TTree* treeD_negX = (TTree*)infile->Get("SpaCEtree_fwdDisp");
        TTree* treeE_negX = (TTree*)infile->Get("SpaCEtree");
        
        fInputFilename.replace(fInputFilename.find("NegX"),3,"Pos");
        
        std::string fname2;
        sp.find_file(fInputFilename,fname2);
        std::unique_ptr<TFile> infile2(new TFile(fname2.c_str(), "READ"));
        if(!infile2->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname2 << "'!\n";
        
        TTree* treeD_posX = (TTree*)infile2->Get("SpaCEtree_fwdDisp");
        TTree* treeE_posX = (TTree*)infile2->Get("SpaCEtree");
        
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX, treeE_negX, "x_true", "y_true", "z_true", "fwd");
        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	SCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	SCEhistograms.at(ii)->SetDirectory(0);
        }   
        
        
        infile2->Close();
      
      }else if (fInputFilename.find("PosX")<fInputFilename.length()){
      
        TTree* treeD_posX = (TTree*)infile->Get("SpaCEtree_fwdDisp");
        TTree* treeE_posX = (TTree*)infile->Get("SpaCEtree");
        
        fInputFilename.replace(fInputFilename.find("PosX"),3,"Neg");
        
        std::string fname2;
        sp.find_file(fInputFilename,fname2);
        std::unique_ptr<TFile> infile2(new TFile(fname2.c_str(), "READ"));
        if(!infile2->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname2 << "'!\n";
        
        TTree* treeD_negX = (TTree*)infile2->Get("SpaCEtree_fwdDisp");
        TTree* treeE_negX = (TTree*)infile2->Get("SpaCEtree");
      
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX, treeE_negX, "x_true", "y_true", "z_true", "fwd");
        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	SCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	SCEhistograms.at(ii)->SetDirectory(0);
        }   
        
        infile2->Close();
      
      }else{
      
        TTree* treeD_negX = (TTree*)infile->Get("SpaCEtree_fwdDisp");
        TTree* treeE_negX = (TTree*)infile->Get("SpaCEtree");
               
        TTree* treeD_posX = (TTree*)infile->Get("SpaCEtree_fwdDisp");
        TTree* treeE_posX = (TTree*)infile->Get("SpaCEtree");
        
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX, treeE_negX, "x_true", "y_true", "z_true", "fwd");
        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	SCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	SCEhistograms.at(ii)->SetDirectory(0);
        }           
            
      }
      
    } else if(fRepresentationType == "Parametric")
    {      
    
      for(int i = 0; i < 5; i++)
      {
        g1_x[i] = (TGraph*)infile->Get(Form("deltaX/g1_%d",i));
        g2_x[i] = (TGraph*)infile->Get(Form("deltaX/g2_%d",i));
        g3_x[i] = (TGraph*)infile->Get(Form("deltaX/g3_%d",i));   
        g4_x[i] = (TGraph*)infile->Get(Form("deltaX/g4_%d",i));
        g5_x[i] = (TGraph*)infile->Get(Form("deltaX/g5_%d",i));
        g1_y[i] = (TGraph*)infile->Get(Form("deltaY/g1_%d",i));
        g2_y[i] = (TGraph*)infile->Get(Form("deltaY/g2_%d",i));
        g3_y[i] = (TGraph*)infile->Get(Form("deltaY/g3_%d",i));   
        g4_y[i] = (TGraph*)infile->Get(Form("deltaY/g4_%d",i));
        g5_y[i] = (TGraph*)infile->Get(Form("deltaY/g5_%d",i));
        g6_y[i] = (TGraph*)infile->Get(Form("deltaY/g6_%d",i));
        g1_z[i] = (TGraph*)infile->Get(Form("deltaZ/g1_%d",i));
        g2_z[i] = (TGraph*)infile->Get(Form("deltaZ/g2_%d",i));
        g3_z[i] = (TGraph*)infile->Get(Form("deltaZ/g3_%d",i));   
        g4_z[i] = (TGraph*)infile->Get(Form("deltaZ/g4_%d",i));
        g1_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g1_%d",i));
        g2_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g2_%d",i));
        g3_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g3_%d",i));
        g4_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g4_%d",i));
        g5_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g5_%d",i));
        g1_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g1_%d",i));
        g2_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g2_%d",i));
        g3_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g3_%d",i));
        g4_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g4_%d",i));
        g5_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g5_%d",i));
        g6_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g6_%d",i));
        g1_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g1_%d",i));
        g2_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g2_%d",i));
        g3_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g3_%d",i));
        g4_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g4_%d",i));
      }
      g1_x[5] = (TGraph*)infile->Get("deltaX/g1_5");
      g2_x[5] = (TGraph*)infile->Get("deltaX/g2_5");
      g3_x[5] = (TGraph*)infile->Get("deltaX/g3_5");   
      g4_x[5] = (TGraph*)infile->Get("deltaX/g4_5");
      g5_x[5] = (TGraph*)infile->Get("deltaX/g5_5");
      g1_y[5] = (TGraph*)infile->Get("deltaY/g1_5");
      g2_y[5] = (TGraph*)infile->Get("deltaY/g2_5");
      g3_y[5] = (TGraph*)infile->Get("deltaY/g3_5");   
      g4_y[5] = (TGraph*)infile->Get("deltaY/g4_5");
      g5_y[5] = (TGraph*)infile->Get("deltaY/g5_5");
      g6_y[5] = (TGraph*)infile->Get("deltaY/g6_5");
      
      g1_x[6] = (TGraph*)infile->Get("deltaX/g1_6");
      g2_x[6] = (TGraph*)infile->Get("deltaX/g2_6");
      g3_x[6] = (TGraph*)infile->Get("deltaX/g3_6");
      g4_x[6] = (TGraph*)infile->Get("deltaX/g4_6");
      g5_x[6] = (TGraph*)infile->Get("deltaX/g5_6");
      g1_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g1_5");
      g2_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g2_5");
      g3_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g3_5");
      g4_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g4_5");
      g5_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g5_5");
      g1_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g1_5");
      g2_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g2_5");
      g3_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g3_5");
      g4_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g4_5");
      g5_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g5_5");
      g6_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g6_5");
      g1_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g1_6");
      g2_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g2_6");
      g3_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g3_6");
      g4_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g4_6");
      g5_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g5_6");
    }
    infile->Close();
  }  
  
  if((fEnableCalSpatialSCE == true) || (fEnableCalEfieldSCE == true))
  {
  
    fRepresentationType = pset.get<std::string>("RepresentationType");
    fCalInputFilename = pset.get<std::string>("CalibrationInputFilename");
    
    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(fCalInputFilename,fname);
    
    std::unique_ptr<TFile> infile(new TFile(fname.c_str(), "READ"));
    if(!infile->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname << "'!\n";
  
   if (fRepresentationType == "Voxelized_TH3") { 
   
      //Load in files
      TH3F* hDx_cal_pos_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_X_Pos");
      TH3F* hDy_cal_pos_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_Y_Pos");
      TH3F* hDz_cal_pos_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_Z_Pos");
      TH3F* hEx_cal_pos_orig = (TH3F*)infile->Get("Reco_ElecField_X_Pos");
      TH3F* hEy_cal_pos_orig = (TH3F*)infile->Get("Reco_ElecField_Y_Pos");
      TH3F* hEz_cal_pos_orig = (TH3F*)infile->Get("Reco_ElecField_Z_Pos");
      
      TH3F* hDx_cal_neg_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_X_Neg");
      TH3F* hDy_cal_neg_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_Y_Neg");
      TH3F* hDz_cal_neg_orig = (TH3F*)infile->Get("RecoBkwd_Displacement_Z_Neg");
      TH3F* hEx_cal_neg_orig = (TH3F*)infile->Get("Reco_ElecField_X_Neg");
      TH3F* hEy_cal_neg_orig = (TH3F*)infile->Get("Reco_ElecField_Y_Neg");
      TH3F* hEz_cal_neg_orig = (TH3F*)infile->Get("Reco_ElecField_Z_Neg");
        
      TH3F* hDx_cal_pos = (TH3F*)hDx_cal_pos_orig->Clone("hDx_pos");
      TH3F* hDy_cal_pos = (TH3F*)hDy_cal_pos_orig->Clone("hDy_pos");
      TH3F* hDz_cal_pos = (TH3F*)hDz_cal_pos_orig->Clone("hDz_pos");
      TH3F* hEx_cal_pos = (TH3F*)hEx_cal_pos_orig->Clone("hEx_pos");
      TH3F* hEy_cal_pos = (TH3F*)hEy_cal_pos_orig->Clone("hEy_pos");
      TH3F* hEz_cal_pos = (TH3F*)hEz_cal_pos_orig->Clone("hEz_pos");
      
      TH3F* hDx_cal_neg = (TH3F*)hDx_cal_neg_orig->Clone("hDx_neg");
      TH3F* hDy_cal_neg = (TH3F*)hDy_cal_neg_orig->Clone("hDy_neg");
      TH3F* hDz_cal_neg = (TH3F*)hDz_cal_neg_orig->Clone("hDz_neg");
      TH3F* hEx_cal_neg = (TH3F*)hEx_cal_neg_orig->Clone("hEx_neg");
      TH3F* hEy_cal_neg = (TH3F*)hEy_cal_neg_orig->Clone("hEy_neg");
      TH3F* hEz_cal_neg = (TH3F*)hEz_cal_neg_orig->Clone("hEz_neg");
        
      hDx_cal_pos->SetDirectory(0);
      hDy_cal_pos->SetDirectory(0);
      hDz_cal_pos->SetDirectory(0);
      hEx_cal_pos->SetDirectory(0);
      hEy_cal_pos->SetDirectory(0);
      hEz_cal_pos->SetDirectory(0);
      
      hDx_cal_neg->SetDirectory(0);
      hDy_cal_neg->SetDirectory(0);
      hDz_cal_neg->SetDirectory(0);
      hEx_cal_neg->SetDirectory(0);
      hEy_cal_neg->SetDirectory(0);
      hEz_cal_neg->SetDirectory(0);
        
      CalSCEhistograms = {hDx_cal_pos, hDy_cal_pos, hDz_cal_pos, hEx_cal_pos, hEy_cal_pos, hEz_cal_pos, hDx_cal_neg, hDy_cal_neg, hDz_cal_neg, hEx_cal_neg, hEy_cal_neg, hEz_cal_neg};
      
    } else if (fRepresentationType == "Voxelized") {
    
      //Load files and read in trees
      if (fCalInputFilename.find("NegX")<fCalInputFilename.length()){
        
        TTree* treeD_negX = (TTree*)infile->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_negX = (TTree*)infile->Get("SpaCEtree");
        
        fCalInputFilename.replace(fCalInputFilename.find("NegX"),3,"Pos");
        
        std::string fname2;
        sp.find_file(fCalInputFilename,fname2);
        std::unique_ptr<TFile> infile2(new TFile(fname2.c_str(), "READ"));
        if(!infile2->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname2 << "'!\n";
        
        TTree* treeD_posX = (TTree*)infile2->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_posX = (TTree*)infile2->Get("SpaCEtree");
        
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX,treeE_negX, "x_reco", "y_reco", "z_reco", "bkwd");        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	CalSCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	CalSCEhistograms.at(ii)->SetDirectory(0);
        }   
        
        infile2->Close();
      
      }else if (fCalInputFilename.find("PosX")<fCalInputFilename.length()){
      
        TTree* treeD_posX = (TTree*)infile->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_posX = (TTree*)infile->Get("SpaCEtree");
        
        fCalInputFilename.replace(fCalInputFilename.find("PosX"),3,"Neg");
        
        std::string fname2;
        sp.find_file(fCalInputFilename,fname2);
        std::unique_ptr<TFile> infile2(new TFile(fname2.c_str(), "READ"));
        if(!infile2->IsOpen()) throw cet::exception("SpaceChargeProtoDUNE") << "Could not find the space charge effect file '" << fname2 << "'!\n";
        
        TTree* treeD_negX = (TTree*)infile2->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_negX = (TTree*)infile2->Get("SpaCEtree");
      
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX,treeE_negX, "x_reco", "y_reco", "z_reco", "bkwd");        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	CalSCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	CalSCEhistograms.at(ii)->SetDirectory(0);
        }   
        
        infile2->Close();
      
      }else{
      
        TTree* treeD_negX = (TTree*)infile->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_negX = (TTree*)infile->Get("SpaCEtree");
               
        TTree* treeD_posX = (TTree*)infile->Get("SpaCEtree_bkwdDisp");
        TTree* treeE_posX = (TTree*)infile->Get("SpaCEtree");
        
        std::vector<TH3F*> temp = Build_TH3(treeD_posX, treeE_posX, treeD_negX,treeE_negX, "x_reco", "y_reco", "z_reco", "bkwd");        
        for (size_t ii = 0; ii<temp.size(); ii++){
        	CalSCEhistograms.at(ii) = (TH3F*)temp.at(ii)->Clone(TString::Format("%s",temp.at(ii)->GetName()));
        	CalSCEhistograms.at(ii)->SetDirectory(0);
        }   
      
      }
      
   } else { std::cout << "No space charge representation type chosen." << std::endl;} 
    
    infile->Close();
  }
   
  return true;
}
//------------------------------------------------
bool spacecharge::SpaceChargeProtoDUNE::Update(uint64_t ts) 
{
  if (ts == 0) return false;
  return true;
}
//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to turn simulation of SCE on for
/// spatial distortions
bool spacecharge::SpaceChargeProtoDUNE::EnableSimSpatialSCE() const
{
  return fEnableSimSpatialSCE;
}
//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to turn simulation of SCE on for
/// E field distortions
bool spacecharge::SpaceChargeProtoDUNE::EnableSimEfieldSCE() const
{
  return fEnableSimEfieldSCE;
}
//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to apply SCE corrections
//bool spacecharge::SpaceChargeProtoDUNE::EnableCorrSCE() const
//{
//  return fEnableCorrSCE;
//}

/// Return boolean indicating whether or not to apply SCE corrections
bool spacecharge::SpaceChargeProtoDUNE::EnableCalSpatialSCE() const
{
  return fEnableCalSpatialSCE;
}

/// Return boolean indicating whether or not to apply SCE corrections
bool spacecharge::SpaceChargeProtoDUNE::EnableCalEfieldSCE() const
{
  return fEnableCalEfieldSCE;
}
//----------------------------------------------------------------------------
/// Primary working method of service that provides position offsets to be
/// used in ionization electron drift
geo::Vector_t spacecharge::SpaceChargeProtoDUNE::GetPosOffsets(geo::Point_t const& tmp_point) const
{

  std::vector<double> thePosOffsets;
  geo::Point_t point = tmp_point;
  if(IsTooFarFromBoundaries(point)) {
    thePosOffsets.resize(3,0.0);
    return { -thePosOffsets[0], -thePosOffsets[1], -thePosOffsets[2] };
  }
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  if (fRepresentationType=="Voxelized_TH3"){
    if (point.X() > 0.) {
    	thePosOffsets = GetOffsetsVoxel(point, SCEhistograms.at(0), SCEhistograms.at(1), SCEhistograms.at(2));
    	thePosOffsets[0] = thePosOffsets[0];
    } else {
    	thePosOffsets = GetOffsetsVoxel(point, SCEhistograms.at(6), SCEhistograms.at(7), SCEhistograms.at(8));
    	thePosOffsets[0] = -1.0*thePosOffsets[0];
    }
      
  }else if (fRepresentationType == "Voxelized"){
    if (point.X() > 0.) thePosOffsets = GetOffsetsVoxel(point, SCEhistograms.at(0), SCEhistograms.at(1), SCEhistograms.at(2));
    else thePosOffsets = GetOffsetsVoxel(point, SCEhistograms.at(6), SCEhistograms.at(7), SCEhistograms.at(8));
      
  }else if(fRepresentationType == "Parametric") thePosOffsets = GetPosOffsetsParametric(point.X(), point.Y(), point.Z());
  else thePosOffsets.resize(3,0.0); 
 
  return { thePosOffsets[0], thePosOffsets[1], thePosOffsets[2] };
}

//----------------------------------------------------------------------------
/// Primary working method of service that provides position offsets to be
/// used in calibration of space charge
geo::Vector_t spacecharge::SpaceChargeProtoDUNE::GetCalPosOffsets(geo::Point_t const& tmp_point, int const& TPCid) const
{
	
  std::vector<double> thePosOffsets;
  geo::Point_t point = tmp_point;
  
  if(IsTooFarFromBoundaries(point)) {
    thePosOffsets.resize(3,0.0);
    return { -thePosOffsets[0], -thePosOffsets[1], -thePosOffsets[2] };
  }
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)){ 
  	point = PretendAtBoundary(point); 
  }
  
  if (fRepresentationType == "Voxelized_TH3"){
    if ((TPCid == 2 || TPCid == 6 || TPCid == 10)&&point.X()>-20.){
      if (point.X()<0.) point = {0.00001, point.Y(), point.Z()};
      thePosOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(0), CalSCEhistograms.at(1), CalSCEhistograms.at(2));
      thePosOffsets[0] = -1.0*thePosOffsets[0];
    } else if((TPCid == 1 || TPCid == 5 || TPCid == 9)&&point.X()<20.) {
    	if (point.X()>0.) point= {-0.00001, point.Y(), point.Z()};
      thePosOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(6), CalSCEhistograms.at(7), CalSCEhistograms.at(8));
      thePosOffsets[0] = -1.0*thePosOffsets[0];
    } else thePosOffsets = {0., 0., 0.};
    
  } else if (fRepresentationType=="Voxelized"){
    if ((TPCid == 2 || TPCid == 6 || TPCid == 10)&&point.X()>-20.){
      if (point.X()<0.) point = {0.00001, point.Y(), point.Z()};
      thePosOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(0), CalSCEhistograms.at(1), CalSCEhistograms.at(2));
      thePosOffsets[0] = -1.0*thePosOffsets[0];
    } else if((TPCid == 1 || TPCid == 5 || TPCid == 9)&&point.X()<20.) {
    	if (point.X()>0.) point= {-0.00001, point.Y(), point.Z()};
      thePosOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(6), CalSCEhistograms.at(7), CalSCEhistograms.at(8));
    } else thePosOffsets = {0., 0., 0.};
      
  } else thePosOffsets.resize(3,0.0);
  
  return { thePosOffsets[0], thePosOffsets[1], thePosOffsets[2] };
}

//----------------------------------------------------------------------------
/// Provides position offsets using voxelized interpolation
std::vector<double> spacecharge::SpaceChargeProtoDUNE::GetOffsetsVoxel
  (geo::Point_t const& point, TH3F* hX, TH3F* hY, TH3F* hZ) const
{
  if (fRepresentationType == "Voxelized_TH3"){
  
    return {
      hX->Interpolate(point.X(),point.Y(),point.Z()),
      hY->Interpolate(point.X(),point.Y(),point.Z()),
      hZ->Interpolate(point.X(),point.Y(),point.Z())
    };
    
  } else {
    double xnew = TransformX(point.X());
    double ynew = TransformY(point.Y());
    double znew = TransformZ(point.Z());
  
    return {
      hX->Interpolate(xnew,ynew,znew),
      hY->Interpolate(xnew,ynew,znew),
      hZ->Interpolate(xnew,ynew,znew)
    };
  }
}

/// Build 3d histograms for voxelized interpolation
std::vector<TH3F*> spacecharge::SpaceChargeProtoDUNE::Build_TH3
  (TTree* tree_pos, TTree* eTree_pos, TTree* tree_neg, TTree* eTree_neg, std::string xvar, std::string yvar, std::string zvar, std::string posLeaf) const
{

  //Define the protoDUNE detector
  double Lx = 3.6, Ly = 6.0, Lz = 7.2;
  double numDivisions_x = 18.0;
  double cell_size = Lx/numDivisions_x;
  double numDivisions_y = TMath::Nint((Ly/Lx)*((Double_t)numDivisions_x));
  double numDivisions_z = TMath::Nint((Lz/Lx)*((Double_t)numDivisions_x));
  
  double E_numDivisions_x = 18.0;
  double E_cell_size = Lx/E_numDivisions_x;
  double E_numDivisions_y = TMath::Nint((Ly/Lx)*((Double_t)E_numDivisions_x));
  double E_numDivisions_z = TMath::Nint((Lz/Lx)*((Double_t)E_numDivisions_x));
  
  //initialized histograms for Dx, Dy, Dz, and electric field (pos x)
  TH3F* hDx_pos = new TH3F("hDx_pos", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1 ,-0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDy_pos = new TH3F("hDy_pos", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDz_pos = new TH3F("hDz_pos", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  
  TH3F* hEx_pos = new TH3F("hEx_pos", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEy_pos = new TH3F("hEy_pos", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEz_pos = new TH3F("hEz_pos", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  
  //initialized histograms for Dx, Dy, Dz, and electric field (neg x)
  TH3F* hDx_neg = new TH3F("hDx_neg", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1 ,-0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDy_neg = new TH3F("hDy_neg", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDz_neg = new TH3F("hDz_neg", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  
  TH3F* hEx_neg = new TH3F("hEx_neg", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEy_neg = new TH3F("hEy_neg", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEz_neg = new TH3F("hEz_neg", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
 
  //For each event, read the tree and fill each histogram (pos x)
  for (int ii = 0; ii<tree_pos->GetEntries(); ii++){

    //Read the trees
    tree_pos->GetEntry(ii);
    Double_t x = tree_pos->GetBranch(xvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t y = tree_pos->GetBranch(yvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t z = tree_pos->GetBranch(zvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dx = tree_pos->GetBranch("Dx")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dy = tree_pos->GetBranch("Dy")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dz = tree_pos->GetBranch("Dz")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    
    hDx_pos->Fill(x,y,z,100.0*dx);
    hDy_pos->Fill(x,y,z,100.0*dy);
    hDz_pos->Fill(x,y,z,100.0*dz);
  }
  
  for(int ii = 0; ii<eTree_pos->GetEntries(); ii++){
		
    eTree_pos->GetEntry(ii);
    Double_t x = eTree_pos->GetBranch("xpoint")->GetLeaf("data")->GetValue();
    Double_t y = eTree_pos->GetBranch("ypoint")->GetLeaf("data")->GetValue();
    Double_t z = eTree_pos->GetBranch("zpoint")->GetLeaf("data")->GetValue();
    Double_t Ex = eTree_pos->GetBranch("Ex")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ey = eTree_pos->GetBranch("Ey")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ez = eTree_pos->GetBranch("Ez")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
   
    //Fill the histograms		
    hEx_pos->Fill(x,y,z,Ex);
    hEy_pos->Fill(x,y,z,Ey);
    hEz_pos->Fill(x,y,z,Ez);
  }
  
  //For each event, read the tree and fill each histogram (neg x)
  for (int ii = 0; ii<tree_neg->GetEntries(); ii++){

    //Read the trees
    tree_neg->GetEntry(ii);
    Double_t x = tree_neg->GetBranch(xvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t y = tree_neg->GetBranch(yvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t z = tree_neg->GetBranch(zvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dx = tree_neg->GetBranch("Dx")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dy = tree_neg->GetBranch("Dy")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dz = tree_neg->GetBranch("Dz")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    
    hDx_neg->Fill(x,y,z,100.0*dx);
    hDy_neg->Fill(x,y,z,100.0*dy);
    hDz_neg->Fill(x,y,z,100.0*dz);
  }
  
  for(int ii = 0; ii<eTree_neg->GetEntries(); ii++){
		
    eTree_neg->GetEntry(ii);
    Double_t x = eTree_neg->GetBranch("xpoint")->GetLeaf("data")->GetValue();
    Double_t y = eTree_neg->GetBranch("ypoint")->GetLeaf("data")->GetValue();
    Double_t z = eTree_neg->GetBranch("zpoint")->GetLeaf("data")->GetValue();
    Double_t Ex = eTree_neg->GetBranch("Ex")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ey = eTree_neg->GetBranch("Ey")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ez = eTree_neg->GetBranch("Ez")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
   
    //Fill the histograms		
    hEx_neg->Fill(x,y,z,Ex);
    hEy_neg->Fill(x,y,z,Ey);
    hEz_neg->Fill(x,y,z,Ez);
  }
  
  return {hDx_pos, hDy_pos, hDz_pos, hEx_pos, hEy_pos, hEz_pos, hDx_neg, hDy_neg, hDz_neg, hEx_neg, hEy_neg, hEz_neg};

}

//----------------------------------------------------------------------------
/// Provides position offsets using a parametric representation
std::vector<double> spacecharge::SpaceChargeProtoDUNE::GetPosOffsetsParametric(double xVal, double yVal, double zVal) const
{
  std::vector<double> thePosOffsetsParametric;
  double xValNew = TransformX(xVal);
  double yValNew = TransformY(yVal);
  double zValNew = TransformZ(zVal);
  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"X"));
  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"Y"));
  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"Z"));
  return thePosOffsetsParametric;
}
//----------------------------------------------------------------------------
/// Provides one position offset using a parametric representation, for a given
/// axis
double spacecharge::SpaceChargeProtoDUNE::GetOnePosOffsetParametric(double xValNew, double yValNew, double zValNew, std::string axis) const
{      
  double parA[6][7];
  double parB[6];
  
  xValNew -= 1.8;
  yValNew -= 3.0;
  
  for(int j = 0; j < 6; j++)
  {
    for(int i = 0; i < 7; i++)
      parA[j][i] = 0.0;
  
    parB[j] = 0.0;
  }
  
  if(axis == "X")
  {
    for(int j = 0; j < 7; j++)
    {
      parA[0][j] = g1_x[j]->Eval(zValNew);
      parA[1][j] = g2_x[j]->Eval(zValNew);
      parA[2][j] = g3_x[j]->Eval(zValNew);
      parA[3][j] = g4_x[j]->Eval(zValNew);
      parA[4][j] = g5_x[j]->Eval(zValNew);
    }
  
    f1_x->SetParameters(parA[0]);
    f2_x->SetParameters(parA[1]);
    f3_x->SetParameters(parA[2]);
    f4_x->SetParameters(parA[3]);
    f5_x->SetParameters(parA[4]);
  }
  else if(axis == "Y")
  {
    for(int j = 0; j < 6; j++)
    {
      parA[0][j] = g1_y[j]->Eval(zValNew);
      parA[1][j] = g2_y[j]->Eval(zValNew);
      parA[2][j] = g3_y[j]->Eval(zValNew);
      parA[3][j] = g4_y[j]->Eval(zValNew);
      parA[4][j] = g5_y[j]->Eval(zValNew);
      parA[5][j] = g6_y[j]->Eval(zValNew);
    }
  
    f1_y->SetParameters(parA[0]);
    f2_y->SetParameters(parA[1]);
    f3_y->SetParameters(parA[2]);
    f4_y->SetParameters(parA[3]);
    f5_y->SetParameters(parA[4]);
    f6_y->SetParameters(parA[5]);
  }
  else if(axis == "Z")
  {
    for(int j = 0; j < 5; j++)
    {
      parA[0][j] = g1_z[j]->Eval(zValNew);
      parA[1][j] = g2_z[j]->Eval(zValNew);
      parA[2][j] = g3_z[j]->Eval(zValNew);
      parA[3][j] = g4_z[j]->Eval(zValNew);
    }
  
    f1_z->SetParameters(parA[0]);
    f2_z->SetParameters(parA[1]);
    f3_z->SetParameters(parA[2]);
    f4_z->SetParameters(parA[3]);
  }
  
  double aValNew;
  double bValNew;
  
  if(axis == "Y")
  {
    aValNew = xValNew;
    bValNew = yValNew;
  }
  else
  {
    aValNew = yValNew;
    bValNew = xValNew;
  }
  
  double offsetValNew = 0.0;
  if(axis == "X")
  {
    parB[0] = f1_x->Eval(aValNew);
    parB[1] = f2_x->Eval(aValNew);
    parB[2] = f3_x->Eval(aValNew);
    parB[3] = f4_x->Eval(aValNew);
    parB[4] = f5_x->Eval(aValNew);
  
    fFinal_x->SetParameters(parB);
    offsetValNew = 100.0*fFinal_x->Eval(bValNew);
  }
  else if(axis == "Y")
  {
    parB[0] = f1_y->Eval(aValNew);
    parB[1] = f2_y->Eval(aValNew);
    parB[2] = f3_y->Eval(aValNew);
    parB[3] = f4_y->Eval(aValNew);
    parB[4] = f5_y->Eval(aValNew);
    parB[5] = f6_y->Eval(aValNew);
  
    fFinal_y->SetParameters(parB);
    offsetValNew = 100.0*fFinal_y->Eval(bValNew);
  }
  else if(axis == "Z")
  {
    parB[0] = f1_z->Eval(aValNew);
    parB[1] = f2_z->Eval(aValNew);
    parB[2] = f3_z->Eval(aValNew);
    parB[3] = f4_z->Eval(aValNew);
  
    fFinal_z->SetParameters(parB);
    offsetValNew = 100.0*fFinal_z->Eval(bValNew);
  }
  
  return offsetValNew;
}
//----------------------------------------------------------------------------
/// Primary working method of service that provides E field offsets to be
/// used in charge/light yield calculation (e.g.)
geo::Vector_t spacecharge::SpaceChargeProtoDUNE::GetEfieldOffsets(geo::Point_t const& tmp_point) const
{

  std::vector<double> theEfieldOffsets;
  geo::Point_t point = tmp_point;
  if(IsTooFarFromBoundaries(point)) {
    theEfieldOffsets.resize(3,0.0);
    return { -theEfieldOffsets[0], -theEfieldOffsets[1], -theEfieldOffsets[2] };
  }
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  if (fRepresentationType=="Voxelized_TH3"){
    if (point.X() > 0.) theEfieldOffsets = GetOffsetsVoxel(point, SCEhistograms.at(3), SCEhistograms.at(4), SCEhistograms.at(5));
    else theEfieldOffsets = GetOffsetsVoxel(point, SCEhistograms.at(9), SCEhistograms.at(10), SCEhistograms.at(11));
    theEfieldOffsets[0] = -1.0*theEfieldOffsets[0];
    theEfieldOffsets[1] = -1.0*theEfieldOffsets[1];
    theEfieldOffsets[2] = -1.0*theEfieldOffsets[2];
  }else if (fRepresentationType == "Voxelized"){
    if (point.X() > 0.) theEfieldOffsets = GetOffsetsVoxel(point, SCEhistograms.at(3), SCEhistograms.at(4), SCEhistograms.at(5));
    else theEfieldOffsets = GetOffsetsVoxel(point, SCEhistograms.at(9), SCEhistograms.at(10), SCEhistograms.at(11));
  }else if(fRepresentationType == "Parametric") theEfieldOffsets = GetEfieldOffsetsParametric(point.X(), point.Y(), point.Z());
  else theEfieldOffsets.resize(3,0.0);
    
   return { -theEfieldOffsets[0], -theEfieldOffsets[1], -theEfieldOffsets[2] };
}
//----------------------------------------------------------------------------
/// Primary working method of service that provides E field offsets to be
/// used in charge/light yield calculation (e.g.) for calibration
geo::Vector_t spacecharge::SpaceChargeProtoDUNE::GetCalEfieldOffsets(geo::Point_t const& tmp_point, int const& TPCid) const
{ 
  std::vector<double> theEfieldOffsets;
  geo::Point_t point = tmp_point;
  if(IsTooFarFromBoundaries(point)) {
    theEfieldOffsets.resize(3,0.0);
    return { -theEfieldOffsets[0], -theEfieldOffsets[1], -theEfieldOffsets[2] };
  }
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  if (fRepresentationType == "Voxelized_TH3"){
    if ((TPCid == 2 || TPCid == 6 || TPCid == 10)&&point.X()>-20.){
      if (point.X()<0.) point = {0.00001, point.Y(), point.Z()};
      theEfieldOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(3), CalSCEhistograms.at(4), CalSCEhistograms.at(5));
    }else if ((TPCid == 1 || TPCid == 5 || TPCid == 9)&&point.X()<20.){
      if (point.X()>0.) point = {-0.00001, point.Y(), point.Z()};
      theEfieldOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(9), CalSCEhistograms.at(10), CalSCEhistograms.at(11));
    } else theEfieldOffsets = {0., 0., 0.};
    theEfieldOffsets[0] = -1.0*theEfieldOffsets[0];
    theEfieldOffsets[1] = -1.0*theEfieldOffsets[1];
    theEfieldOffsets[2] = -1.0*theEfieldOffsets[2];
  }else if (fRepresentationType == "Voxelized"){
    if ((TPCid == 2 || TPCid == 6 || TPCid == 10)&&point.X()>-20.){
      if (point.X()<0.) point = {0.00001, point.Y(), point.Z()};
      theEfieldOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(3), CalSCEhistograms.at(4), CalSCEhistograms.at(5));
    }else if ((TPCid == 1 || TPCid == 5 || TPCid == 9)&&point.X()<20.){
      if (point.X()>0.) point = {-0.00001, point.Y(), point.Z()};
      theEfieldOffsets = GetOffsetsVoxel(point, CalSCEhistograms.at(9), CalSCEhistograms.at(10), CalSCEhistograms.at(11));
    } else theEfieldOffsets = {0., 0., 0.};
  }else
    theEfieldOffsets.resize(3,0.0);
  
  return { -theEfieldOffsets[0], -theEfieldOffsets[1], -theEfieldOffsets[2] };
}

//----------------------------------------------------------------------------
/// Provides E field offsets using a parametric representation
std::vector<double> spacecharge::SpaceChargeProtoDUNE::GetEfieldOffsetsParametric(double xVal, double yVal, double zVal) const
{
  std::vector<double> theEfieldOffsetsParametric;
  double xValNew = TransformX(xVal);
  double yValNew = TransformY(yVal);
  double zValNew = TransformZ(zVal);
  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"X"));
  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"Y"));
  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"Z"));
  return theEfieldOffsetsParametric;
}
//----------------------------------------------------------------------------
/// Provides one E field offset using a parametric representation, for a given
/// axis
double spacecharge::SpaceChargeProtoDUNE::GetOneEfieldOffsetParametric(double xValNew, double yValNew, double zValNew, std::string axis) const
{      
  xValNew -= 1.8;
  yValNew -= 3.0;

  double parA[6][7];
  double parB[6];
  
  for(int j = 0; j < 6; j++)
  {
    for(int i = 0; i < 7; i++)
      parA[j][i] = 0.0;
  
    parB[j] = 0.0;
  }
  
  if(axis == "X")
  {
    for(int j = 0; j < 7; j++)
    {
      parA[0][j] = g1_Ex[j]->Eval(zValNew);
      parA[1][j] = g2_Ex[j]->Eval(zValNew);
      parA[2][j] = g3_Ex[j]->Eval(zValNew);
      parA[3][j] = g4_Ex[j]->Eval(zValNew);
      parA[4][j] = g5_Ex[j]->Eval(zValNew);
    }
  
    f1_Ex->SetParameters(parA[0]);
    f2_Ex->SetParameters(parA[1]);
    f3_Ex->SetParameters(parA[2]);
    f4_Ex->SetParameters(parA[3]);
    f5_Ex->SetParameters(parA[4]);
  }
  else if(axis == "Y")
  {
    for(int j = 0; j < 6; j++)
    {
      parA[0][j] = g1_Ey[j]->Eval(zValNew);
      parA[1][j] = g2_Ey[j]->Eval(zValNew);
      parA[2][j] = g3_Ey[j]->Eval(zValNew);
      parA[3][j] = g4_Ey[j]->Eval(zValNew);
      parA[4][j] = g5_Ey[j]->Eval(zValNew);
      parA[5][j] = g6_Ey[j]->Eval(zValNew);
    }
  
    f1_Ey->SetParameters(parA[0]);
    f2_Ey->SetParameters(parA[1]);
    f3_Ey->SetParameters(parA[2]);
    f4_Ey->SetParameters(parA[3]);
    f5_Ey->SetParameters(parA[4]);
    f6_Ey->SetParameters(parA[5]);
  }
  else if(axis == "Z")
  {
    for(int j = 0; j < 5; j++)
    {
      parA[0][j] = g1_Ez[j]->Eval(zValNew);
      parA[1][j] = g2_Ez[j]->Eval(zValNew);
      parA[2][j] = g3_Ez[j]->Eval(zValNew);
      parA[3][j] = g4_Ez[j]->Eval(zValNew);
    }
  
    f1_Ez->SetParameters(parA[0]);
    f2_Ez->SetParameters(parA[1]);
    f3_Ez->SetParameters(parA[2]);
    f4_Ez->SetParameters(parA[3]);
  }
  
  double aValNew;
  double bValNew;
  
  if(axis == "Y")
  {
    aValNew = xValNew;
    bValNew = yValNew;
  }
  else
  {
    aValNew = yValNew;
    bValNew = xValNew;
  }
  
  double offsetValNew = 0.0;
  if(axis == "X")
  {
    parB[0] = f1_Ex->Eval(aValNew);
    parB[1] = f2_Ex->Eval(aValNew);
    parB[2] = f3_Ex->Eval(aValNew);
    parB[3] = f4_Ex->Eval(aValNew);
    parB[4] = f5_Ex->Eval(aValNew);
  
    fFinal_Ex->SetParameters(parB);
    offsetValNew = fFinal_Ex->Eval(bValNew);
  }
  else if(axis == "Y")
  {
    parB[0] = f1_Ey->Eval(aValNew);
    parB[1] = f2_Ey->Eval(aValNew);
    parB[2] = f3_Ey->Eval(aValNew);
    parB[3] = f4_Ey->Eval(aValNew);
    parB[4] = f5_Ey->Eval(aValNew);
    parB[5] = f6_Ey->Eval(aValNew);
  
    fFinal_Ey->SetParameters(parB);
    offsetValNew = fFinal_Ey->Eval(bValNew);
  }
  else if(axis == "Z")
  {
    parB[0] = f1_Ez->Eval(aValNew);
    parB[1] = f2_Ez->Eval(aValNew);
    parB[2] = f3_Ez->Eval(aValNew);
    parB[3] = f4_Ez->Eval(aValNew);
  
    fFinal_Ez->SetParameters(parB);
    offsetValNew = fFinal_Ez->Eval(bValNew);
  }
  
  return offsetValNew;
}
//----------------------------------------------------------------------------
/// Transform X to SCE X coordinate:  [0.0,3.6] --> [0.0,3.6]
double spacecharge::SpaceChargeProtoDUNE::TransformX(double xVal) const
{
  double xValNew;
  xValNew = (fabs(xVal)/100.0);
  //xValNew -= 1.8;
  return xValNew;
}
//----------------------------------------------------------------------------
/// Transform Y to SCE Y coordinate:  [0.0,6.08] --> [0.0,6.0]
double spacecharge::SpaceChargeProtoDUNE::TransformY(double yVal) const
{
  double yValNew;
  yValNew = (6.00/6.08)*((yVal+0.2)/100.0);
  //yValNew -= 3.0;
  return yValNew;
}
//----------------------------------------------------------------------------
/// Transform Z to SCE Z coordinate:  [0.0,6.97] --> [0.0,7.2]
double spacecharge::SpaceChargeProtoDUNE::TransformZ(double zVal) const
{
  double zValNew;
  zValNew = (7.20/6.97)*((zVal+0.8)/100.0);
  return zValNew;
}
//----------------------------------------------------------------------------
/// Check to see if point is inside boundaries of map (allow to go slightly out of range)
bool spacecharge::SpaceChargeProtoDUNE::IsInsideBoundaries(geo::Point_t const& point) const
{
  if(fRepresentationType=="Voxelized_TH3"){
  	return !(
         (TMath::Abs(point.X()) <= 0.0) || (TMath::Abs(point.X()) >= 360.0)
      || (point.Y()             <= 5.2) || (point.Y()             >= 604.0)
      || (point.Z()             <= -0.5) || (point.Z()             >= 695.3)
    );
  } else{
  	return !(
         (TMath::Abs(point.X()) <=  0.0) || (TMath::Abs(point.X()) >= 360.0)
      || (point.Y()             <= -0.2) || (point.Y()             >= 607.8)
      || (point.Z()             <= -0.8) || (point.Z()             >= 696.2)
    );
  }
} 
  
bool spacecharge::SpaceChargeProtoDUNE::IsTooFarFromBoundaries(geo::Point_t const& point) const
{
  if(fRepresentationType=="Voxelized_TH3"){
    return (
         (TMath::Abs(point.X()) < -20.0) || (TMath::Abs(point.X())  >= 360.0)
      || (point.Y()             < -14.8) || (point.Y()              >  624.0)
      || (point.Z()             < -20.5) || (point.Z()              >  715.3)
    );
  } else {
    return (
         (TMath::Abs(point.X()) < -20.0) || (TMath::Abs(point.X())  >= 360.0)
      || (point.Y()             < -20.2) || (point.Y()              >  627.8)
      || (point.Z()             < -20.8) || (point.Z()              >  716.2)
    );
  }
}

geo::Point_t spacecharge::SpaceChargeProtoDUNE::PretendAtBoundary(geo::Point_t const& point) const
{
  double x = point.X(), y = point.Y(), z = point.Z();
  
  if(fRepresentationType=="Voxelized_TH3"){ 
  
    if      (TMath::Abs(point.X()) ==    0.0    ) x =                           -0.00001;
    else if (TMath::Abs(point.X()) <	 0.00001) x =   TMath::Sign(point.X(),1)*0.00001; 
    else if (TMath::Abs(point.X()) >=    360.0  ) x = TMath::Sign(point.X(),1)*359.99999;
  
    if      (point.Y() <=   5.2) y =   5.20001;
    else if (point.Y() >= 604.0) y = 603.99999;
  
    if      (point.Z() <=   -0.5) z =   -0.49999;
    else if (point.Z() >= 695.3) z = 695.29999;
    
  } else { 
  
    if      (TMath::Abs(point.X()) ==    0.0) x =                           -0.00001;
    else if (TMath::Abs(point.X()) <	 0.0) x =   TMath::Sign(point.X(),1)*0.00001; 
    else if (TMath::Abs(point.X()) >=  360.0) x = TMath::Sign(point.X(),1)*359.99999;
  
    if      (point.Y() <=  -0.2) y =  -0.19999;
    else if (point.Y() >= 607.8) y = 607.79999;
  
    if      (point.Z() <=  -0.8) z =  -0.79999;
    else if (point.Z() >= 696.2) z = 696.19999;
    
  }
  return {x, y, z};
}

// -*- C++ -*-
//
// Package:     PhysicsTools/NanoAODOutput
// Class  :     NanoAODOutputModule
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Christopher Jones
//         Created:  Mon, 07 Aug 2017 14:21:41 GMT
//

// system include files
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "Compression.h"

// user include files
#include "FWCore/Framework/interface/OutputModule.h"
#include "FWCore/Framework/interface/one/OutputModule.h"
#include "FWCore/Framework/interface/RunForOutput.h"
#include "FWCore/Framework/interface/LuminosityBlockForOutput.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/JobReport.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"
#include "FWCore/Utilities/interface/Digest.h"

#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "PhysicsTools/NanoAOD/plugins/TableOutputBranches.h"
#include "PhysicsTools/NanoAOD/plugins/TriggerOutputBranches.h"
#include "PhysicsTools/NanoAOD/plugins/SummaryTableOutputBranches.h"

#include <iostream>

class NanoAODOutputModule : public edm::one::OutputModule<> {
public:
  NanoAODOutputModule(edm::ParameterSet const& pset);
  virtual ~NanoAODOutputModule();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void write(edm::EventForOutput const& e) override;
  virtual void writeLuminosityBlock(edm::LuminosityBlockForOutput const&) override;
  virtual void writeRun(edm::RunForOutput const&) override;
  virtual bool isFileOpen() const override;
  virtual void openFile(edm::FileBlock const&) override;
  virtual void reallyCloseFile() override;

  std::string m_fileName;
  std::string m_logicalFileName;
  int m_compressionLevel;
  std::string m_compressionAlgorithm;
  edm::JobReport::Token m_jrToken;
  std::unique_ptr<TFile> m_file;
  std::unique_ptr<TTree> m_tree, m_lumiTree, m_runTree;

  class CommonEventBranches {
     public:
         void branch(TTree &tree) {
            tree.Branch("run", & m_run, "run/i");
            tree.Branch("luminosityBlock", & m_luminosityBlock, "luminosityBlock/i");
            tree.Branch("event", & m_event, "event/l");
         }
         void fill(const edm::EventID & id) { 
            m_run = id.run(); m_luminosityBlock = id.luminosityBlock(); m_event = id.event(); 
         }
     private:
         UInt_t m_run; UInt_t m_luminosityBlock; ULong64_t m_event;
  } m_commonBranches;

  class CommonLumiBranches {
     public:
         void branch(TTree &tree) {
            tree.Branch("run", & m_run, "run/i");
            tree.Branch("luminosityBlock", & m_luminosityBlock, "luminosityBlock/i");
         }
         void fill(const edm::LuminosityBlockID & id) { 
            m_run = id.run(); 
            m_luminosityBlock = id.value(); 
         }
     private:
         UInt_t m_run; UInt_t m_luminosityBlock;
  } m_commonLumiBranches;

  class CommonRunBranches {
     public:
         void branch(TTree &tree) {
            tree.Branch("run", & m_run, "run/i");
         }
         void fill(const edm::RunID & id) { 
            m_run = id.run(); 
         }
     private:
         UInt_t m_run;
  } m_commonRunBranches;


  std::vector<TableOutputBranches> m_tables;
  std::vector<TriggerOutputBranches> m_triggers;

  std::vector<SummaryTableOutputBranches> m_runTables;
};


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
NanoAODOutputModule::NanoAODOutputModule(edm::ParameterSet const& pset):
  edm::one::OutputModuleBase::OutputModuleBase(pset),
  edm::one::OutputModule<>(pset),
  m_fileName(pset.getUntrackedParameter<std::string>("fileName")),
  m_logicalFileName(pset.getUntrackedParameter<std::string>("logicalFileName")),
  m_compressionLevel(pset.getUntrackedParameter<int>("compressionLevel")),
  m_compressionAlgorithm(pset.getUntrackedParameter<std::string>("compressionAlgorithm"))
{
}

NanoAODOutputModule::~NanoAODOutputModule()
{
}

void 
NanoAODOutputModule::write(edm::EventForOutput const& iEvent) {
  //Get data from 'e' and write it to the file
  edm::Service<edm::JobReport> jr;
  jr->eventWrittenToFile(m_jrToken, iEvent.id().run(), iEvent.id().event());

  m_commonBranches.fill(iEvent.id());
  // fill all tables, starting from main tables and then doing extension tables
  for (unsigned int extensions = 0; extensions <= 1; ++extensions) {
      for (auto & t : m_tables) t.fill(iEvent,*m_tree,extensions);
  }
  // fill triggers
  for (auto & t : m_triggers) t.fill(iEvent,*m_tree);
  m_tree->Fill();
}

void 
NanoAODOutputModule::writeLuminosityBlock(edm::LuminosityBlockForOutput const& iLumi) {
  edm::Service<edm::JobReport> jr;
  jr->reportLumiSection(m_jrToken, iLumi.id().run(), iLumi.id().value());

  m_commonLumiBranches.fill(iLumi.id());
  m_lumiTree->Fill();
}

void 
NanoAODOutputModule::writeRun(edm::RunForOutput const& iRun) {
  edm::Service<edm::JobReport> jr;
  jr->reportRunNumber(m_jrToken, iRun.id().run());

  m_commonRunBranches.fill(iRun.id());

  for (auto & t : m_runTables) t.fill(iRun,*m_runTree);

  m_runTree->Fill();
}

bool 
NanoAODOutputModule::isFileOpen() const {
  return nullptr != m_file.get();
}

void 
NanoAODOutputModule::openFile(edm::FileBlock const&) {
  m_file = std::make_unique<TFile>(m_fileName.c_str(),"RECREATE","",m_compressionLevel);
  edm::Service<edm::JobReport> jr;
  cms::Digest branchHash;
  m_jrToken = jr->outputFileOpened(m_fileName,
                                   m_logicalFileName,
                                   std::string(),
                                   "NanoAODOutputModule",
                                   description().moduleLabel(),
                                   edm::createGlobalIdentifier(),
                                   std::string(),
                                   branchHash.digest().toString(),
                                   std::vector<std::string>()
                                   );

  if (m_compressionAlgorithm == std::string("ZLIB")) {
      m_file->SetCompressionAlgorithm(ROOT::kZLIB);
    } else if (m_compressionAlgorithm == std::string("LZMA")) {
      m_file->SetCompressionAlgorithm(ROOT::kLZMA);
    } else {
      throw cms::Exception("Configuration") << "NanoAODOutputModule configured with unknown compression algorithm '" << m_compressionAlgorithm << "'\n"
					     << "Allowed compression algorithms are ZLIB and LZMA\n";
    }
  /* Setup file structure here */
  m_tables.clear();
  m_triggers.clear();
  m_runTables.clear();
  const auto & keeps = keptProducts();
  for (const auto & keep : keeps[edm::InEvent]) {
      if(keep.first->className() == "FlatTable" )
	      m_tables.emplace_back(keep.first, keep.second);
      else if(keep.first->className() == "edm::TriggerResults" )
	  {
	      m_triggers.emplace_back(keep.first, keep.second);
	  }
      else throw cms::Exception("Configuration", "NanoAODOutputModule cannot handle class " + keep.first->className());     
  }

  for (const auto & keep : keeps[edm::InRun]) {
      if(keep.first->className() == "MergableCounterTable" )
	      m_runTables.push_back(SummaryTableOutputBranches(keep.first, keep.second));
      else throw cms::Exception("Configuration", "NanoAODOutputModule cannot handle class " + keep.first->className() + " in Run branch");     
  }


  // create the trees
  m_tree.reset(new TTree("Events","Events"));
  m_tree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonBranches.branch(*m_tree);

  m_lumiTree.reset(new TTree("LuminosityBlocks","LuminosityBlocks"));
  m_lumiTree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonLumiBranches.branch(*m_lumiTree);

  m_runTree.reset(new TTree("Runs","Runs"));
  m_runTree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonRunBranches.branch(*m_runTree);
}
void 
NanoAODOutputModule::reallyCloseFile() {
  m_file->Write();
  m_file->Close();
  m_file.reset();
  m_tree.release();     // apparently root has ownership
  m_lumiTree.release(); // 
  m_runTree.release(); // 
  edm::Service<edm::JobReport> jr;
  jr->outputFileClosed(m_jrToken);
}

void 
NanoAODOutputModule::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.addUntracked<std::string>("fileName");
  desc.addUntracked<std::string>("logicalFileName","");

  desc.addUntracked<int>("compressionLevel", 9)
        ->setComment("ROOT compression level of output file.");
  desc.addUntracked<std::string>("compressionAlgorithm", "ZLIB")
        ->setComment("Algorithm used to compress data in the ROOT output file, allowed values are ZLIB and LZMA");

  //replace with whatever you want to get from the EDM by default
  const std::vector<std::string> keep = {"drop *", "keep FlatTable_*_*_*"};
  edm::OutputModule::fillDescription(desc, keep);
  
  //Used by Workflow management for their own meta data
  edm::ParameterSetDescription dataSet;
  dataSet.setAllowAnything();
  desc.addUntracked<edm::ParameterSetDescription>("dataset", dataSet)
    ->setComment("PSet is only used by Data Operations and not by this module.");
  
  edm::ParameterSetDescription branchSet;
  branchSet.setAllowAnything();
  desc.add<edm::ParameterSetDescription>("branches", branchSet);



  descriptions.addDefault(desc);

}

DEFINE_FWK_MODULE(NanoAODOutputModule);

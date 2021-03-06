import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################


finalJets = cms.EDFilter("PATJetRefSelector",
    src = cms.InputTag("slimmedJets"),
    cut = cms.string("pt > 15")
)

##################### Tables for final output and docs ##########################


jetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    doc  = cms.string("slimmedJets, i.e. ak4 PFJets CHS with JECs applied, after basic selection (" + finalJets.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs",precision=10),
        nMuons = Var("?hasOverlaps('muons')?overlaps('muons').size():0", int, doc="number of muons in the jet"),
        muon1 = Var("?overlaps('muons').size()>0?overlaps('muons')[0].key():-1", int, doc="index of first matching muon"),
        muon2 = Var("?overlaps('muons').size()>1?overlaps('muons')[1].key():-1", int, doc="index of second matching muon"),
        electron1 = Var("?overlaps('electrons').size()>0?overlaps('electrons')[0].key():-1", int, doc="index of first matching electron"),
        electron2 = Var("?overlaps('electrons').size()>1?overlaps('electrons')[1].key():-1", int, doc="index of second matching electron"),
        nElectrons = Var("?hasOverlaps('electrons')?overlaps('electrons').size():0", int, doc="number of electrons in the jet"),
	btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepC = Var("bDiscriminator('pfDeepCSVJetTags:probc')",float,doc="CMVA V2 btag discriminator",precision=10),
#puIdDisc = Var("userFloat('pileupJetId:fullDiscriminant')",float,doc="Pilup ID discriminant",precision=10),
	puId = Var("userInt('pileupJetId:fullId')",int,doc="Pilup ID flags"),
	qgl = Var("userFloat('QGTagger:qgLikelihood')",float,doc="Quark vs Gluon likelihood discriminator",precision=10),
	nConstituents = Var("numberOfDaughters()",int,doc="Number of particles in the jet"),
	rawFactor = Var("1.-jecFactor('Uncorrected')",float,doc="1 - Factor to get back to raw pT",precision=6),
    )
)
#jets are not as precise as muons
jetTable.variables.pt.precision=10


## BOOSTED STUFF #################
fatJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedJetsAK8"),
    cut = cms.string(" pt > 170"), #probably already applied in miniaod
    name = cms.string("FatJet"),
    doc  = cms.string("slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs",precision=10),
        tau1 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau1')",float, doc="Nsubjettiness (1 axis)",precision=10),
        tau2 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau2')",float, doc="Nsubjettiness (2 axis)",precision=10),
        tau3 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau3')",float, doc="Nsubjettiness (3 axis)",precision=10),
        msoftdrop = Var("userFloat('ak8PFJetsCHSValueMap:ak8PFJetsCHSSoftDropMass')",float, doc="Soft drop mass",precision=10),
        mpruned = Var("userFloat('ak8PFJetsCHSValueMap:ak8PFJetsCHSPrunedMass')", float, doc="Pruned mass",precision=10),


        btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),

        subJet1 = Var("?numberOfSourceCandidatePtrs()>0 && sourceCandidatePtr(0).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(0).key():-1", int,
		     doc="index of first subjet"),
        subJet2 = Var("?numberOfSourceCandidatePtrs()>1 && sourceCandidatePtr(1).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(1).key():-1", int,
		     doc="index of second subjet"),
        subJet3 = Var("?numberOfSourceCandidatePtrs()>2 && sourceCandidatePtr(2).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(2).key():-1", int,
		     doc="index of third subjet"),
	
#        btagDeepC = Var("bDiscriminator('pfDeepCSVJetTags:probc')",float,doc="CMVA V2 btag discriminator",precision=10),
#puIdDisc = Var("userFloat('pileupJetId:fullDiscriminant')",float,doc="Pilup ID discriminant",precision=10),
#        nConstituents = Var("numberOfDaughters()",int,doc="Number of particles in the jet"),
#        rawFactor = Var("1.-jecFactor('Uncorrected')",float,doc="1 - Factor to get back to raw pT",precision=6),
    )
)

subJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedJetsAK8PFPuppiSoftDropPacked","SubJets"),
    cut = cms.string(""), #probably already applied in miniaod
    name = cms.string("SubJet"),
    doc  = cms.string("slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),
    )
)

#jets are not as precise as muons
fatJetTable.variables.pt.precision=10





## MC STUFF ######################
jetMCTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(True), # this is an extension  table for the jets
    variables = cms.PSet(
        partonFlavour = Var("partonFlavour()", int, doc="flavour from parton matching"),
        hadroFlavour = Var("hadronFlavour()", int, doc="flavour from hadron ghost clustering"),
        genJetIdx = Var("?genJetFwdRef().backRef().isNonnull()?genJetFwdRef().backRef().key():-1", int, doc="index of matched gen jet"),
    )
)
genJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedGenJets"),
    cut = cms.string("pt > 10"),
    name = cms.string("GenJet"),
    doc  = cms.string("slimmedGenJets, i.e. ak4 Jets made with visible genparticles"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the genjets
    variables = cms.PSet(P4Vars,
	#anything else?
    )
)



#before cross linking
jetSequence = cms.Sequence(finalJets)
#after cross linkining
jetTables = cms.Sequence( jetTable+fatJetTable+subJetTable)

#MC only producers and tables
jetMC = cms.Sequence(jetMCTable+genJetTable)


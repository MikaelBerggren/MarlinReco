#include "IsolatedLeptonFinderProcessor.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <cmath>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <IMPL/LCCollectionVec.h>

// ----- include for verbosity dependend logging ---------
#include <marlin/VerbosityLevels.h>

#include <TMath.h>
#include <TVector3.h>
#include <TLorentzVector.h>

using namespace lcio ;
using namespace marlin ;

IsolatedLeptonFinderProcessor aIsolatedLeptonFinderProcessor ;

IsolatedLeptonFinderProcessor::IsolatedLeptonFinderProcessor()
	: Processor("IsolatedLeptonFinderProcessor") {

		// Processor description
		_description = "Isolated Lepton Finder Processor" ;

		// register steering parameters: name, description, class-variable, default value
		registerInputCollection( LCIO::RECONSTRUCTEDPARTICLE,
				"InputCollection" ,
				"Input collection of ReconstructedParticles",
				_inputPFOsCollection,
				std::string("PandoraPFOs"));

		registerOutputCollection( LCIO::RECONSTRUCTEDPARTICLE,
				"OutputCollectionWithoutIsolatedLepton",
				"Copy of input collection but without the isolated leptons",
				_outputPFOsRemovedIsoLepCollection,
				std::string("PandoraPFOsWithoutIsoLep") );

		registerOutputCollection( LCIO::RECONSTRUCTEDPARTICLE,
				"OutputCollectionIsolatedLeptons",
				"Output collection of isolated leptons",
				_outputIsoLepCollection,
				std::string("Isolep") );

		registerProcessorParameter( "CosConeAngle",
				"Cosine of the half-angle of the cone used in isolation criteria",
				_cosConeAngle,
				float(0.98));

		registerProcessorParameter( "UsePID",
				"Use primitive particle ID based on calorimeter energy deposits",
				_usePID,
				bool(true));

		registerProcessorParameter( "ElectronMinEnergyDepositByMomentum",
				"Electron ID: Minimum energy deposit divided by momentum",
				_electronMinEnergyDepositByMomentum,
				float(0.7));

		registerProcessorParameter( "ElectronMaxEnergyDepositByMomentum",
				"Electron ID: Maximum energy deposit divided by momentum",
				_electronMaxEnergyDepositByMomentum,
				float(1.4));

		registerProcessorParameter( "ElectronMinEcalToHcalFraction",
				"Electron ID: Minimum Ecal deposit divided by sum of Ecal and Hcal deposits",
				_electronMinEcalToHcalFraction,
				float(0.9));

		registerProcessorParameter( "ElectronMaxEcalToHcalFraction",
				"Electron ID: Maximum Ecal deposit divided by sum of Ecal and Hcal deposits",
				_electronMaxEcalToHcalFraction,
				float(1.0));

		registerProcessorParameter( "MuonMinEnergyDepositByMomentum",
				"Muon ID: Minimum energy deposit divided by momentum",
				_muonMinEnergyDepositByMomentum,
				float(0.0));

		registerProcessorParameter( "MuonMaxEnergyDepositByMomentum",
				"Muon ID: Maximum energy deposit divided by momentum",
				_muonMaxEnergyDepositByMomentum,
				float(0.3));

		registerProcessorParameter( "MuonMinEcalToHcalFraction",
				"Muon ID: Minimum Ecal deposit divided by sum of Ecal and Hcal deposits",
				_muonMinEcalToHcalFraction,
				float(0.0));

		registerProcessorParameter( "MuonMaxEcalToHcalFraction",
				"Muon ID: Maximum Ecal deposit divided by sum of Ecal and Hcal deposits",
				_muonMaxEcalToHcalFraction,
				float(0.4));

		registerProcessorParameter( "UseImpactParameter",
				"Use impact parameter cuts for consistency with primary/secondary track",
				_useImpactParameter,
				bool(true));

		registerProcessorParameter( "ImpactParameterMinD0",
				"Minimum d0 impact parameter",
				_minD0,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMaxD0",
				"Maximum d0 impact parameter",
				_maxD0,
				float(1e20));

		registerProcessorParameter( "ImpactParameterMinZ0",
				"Minimum z0 impact parameter",
				_minZ0,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMaxZ0",
				"Maximum z0 impact parameter",
				_maxZ0,
				float(1e20));

		registerProcessorParameter( "ImpactParameterMin3D",
				"Minimum impact parameter in 3D",
				_minR0,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMax3D",
				"Maximum impact parameter in 3D",
				_maxR0,
				float(0.01));

		registerProcessorParameter( "UseImpactParameterSignificance",
				"Use impact parameter significance cuts for consistency with primary/secondary track",
				_useImpactParameterSignificance,
				bool(false));

		registerProcessorParameter( "ImpactParameterMinD0Significance",
				"Minimum d0 impact parameter significance",
				_minD0Sig,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMaxD0Significance",
				"Maximum d0 impact parameter significance",
				_maxD0Sig,
				float(1e20));

		registerProcessorParameter( "ImpactParameterMinZ0Significance",
				"Minimum z0 impact parameter significance",
				_minZ0Sig,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMaxZ0Significance",
				"Maximum z0 impact parameter significance",
				_maxZ0Sig,
				float(1e20));

		registerProcessorParameter( "ImpactParameterMin3DSignificance",
				"Minimum impact parameter significance in 3D",
				_minR0Sig,
				float(0.0));

		registerProcessorParameter( "ImpactParameterMax3DSignificance",
				"Maximum impact parameter significance in 3D",
				_maxR0Sig,
				float(1e20));

		registerProcessorParameter( "UseRectangularIsolation",
				"Use rectangular cuts on track and cone energy",
				_useRectangularIsolation,
				bool(true));

		registerProcessorParameter( "IsolationMinimumTrackEnergy",
				"Minimum track energy for isolation requirement",
				_isoMinTrackEnergy,
				float(15));

		registerProcessorParameter( "IsolationMaximumTrackEnergy",
				"Maximum track energy for isolation requirement",
				_isoMaxTrackEnergy,
				float(1e20));

		registerProcessorParameter( "IsolationMinimumConeEnergy",
				"Minimum cone energy for isolation requirement",
				_isoMinConeEnergy,
				float(0));

		registerProcessorParameter( "IsolationMaximumConeEnergy",
				"Maximum cone energy for isolation requirement",
				_isoMaxConeEnergy,
				float(1e20));

		registerProcessorParameter( "UsePolynomialIsolation",
				"Use polynomial cuts on track and cone energy",
				_usePolynomialIsolation,
				bool(true));

		registerProcessorParameter( "IsolationPolynomialCutA",
				"Polynomial cut (A) on track energy and cone energy: Econe^2 < A*Etrk^2+B*Etrk+C",
				_isoPolynomialA,
				float(0));

		registerProcessorParameter( "IsolationPolynomialCutB",
				"Polynomial cut (B) on track energy and cone energy: Econe^2 < A*Etrk^2+B*Etrk+C",
				_isoPolynomialB,
				float(20));

		registerProcessorParameter( "IsolationPolynomialCutC",
				"Polynomial cut (C) on track energy and cone energy: Econe^2 < A*Etrk^2+B*Etrk+C",
				_isoPolynomialC,
				float(-300));

		registerProcessorParameter( "UseJetIsolation",
				"Use jet-based isolation",
				_useJetIsolation,
				bool(false));

		registerInputCollection( LCIO::RECONSTRUCTEDPARTICLE,
				"JetCollection" ,
				"Input collection of jets for isolation",
				_jetCollectionName,
				std::string("JetsForIsolation"));

		registerProcessorParameter( "JetIsolationVetoMinimumXt",
				"Minimum Xt in jet-based isolation",
				_jetIsoVetoMinXt,
				float(0.));

		registerProcessorParameter( "JetIsolationVetoMaximumXt",
				"Maximum Xt in jet-based isolation",
				_jetIsoVetoMaxXt,
				float(0.25));

		registerProcessorParameter( "JetIsolationVetoMinimumZ",
				"Mininum Z in jet-based isolation",
				_jetIsoVetoMinZ,
				float(0.));

		registerProcessorParameter( "JetIsolationVetoMaximumZ",
				"Maximum Z in jet-based isolation",
				_jetIsoVetoMaxZ,
				float(0.6));

		registerProcessorParameter( "UseDressedLeptons",
				"Dress leptons with close-by particles",
				_useDressedLeptons,
				bool(false));

		registerProcessorParameter( "MergeCloseElectrons",
				"Merge close-by electrons into higher energy lepton",
				_mergeCloseElectrons,
				bool(false));

		registerProcessorParameter( "DressPhotonConeAngle",
				"Half-angle (in degrees) of the cone used for lepton dressing with photons",
				_dressPhotonConeAngle,
				float(1));

		registerProcessorParameter( "MergeLeptonConeAngle",
				"Half-angle (in degrees) of the cone used for lepton merging",
				_mergeLeptonConeAngle,
				float(2));

		registerProcessorParameter( "UsePandoraIDs",
				"Use Pandora particle IDs for algorithm",
				_usePandoraIDs,
				bool(false));
	}


void IsolatedLeptonFinderProcessor::init() {
	streamlog_out(DEBUG) << "   init called  " << std::endl ;
	printParameters() ;
}

void IsolatedLeptonFinderProcessor::processEvent( LCEvent * evt ) {

	streamlog_out(DEBUG) <<std::endl;
	streamlog_out(DEBUG) << "processing event: " << evt->getEventNumber() << "   in run:  " << evt->getRunNumber() << std::endl ;

	_rpJetMap.clear();
	_workingList.clear();

	_pfoCol = evt->getCollection( _inputPFOsCollection ) ;

	// Output PFOs isolated leptons
	auto outIsoLepCol = std::unique_ptr<LCCollectionVec>(new LCCollectionVec( LCIO::RECONSTRUCTEDPARTICLE ) );
	// Output PFOs removed isolated leptons
	auto outPFOsRemovedIsoLepCol = std::unique_ptr<LCCollectionVec>(new LCCollectionVec( LCIO::RECONSTRUCTEDPARTICLE ) );

	// Prepare jet/recoparticle map for jet-based isolation
	if (_useJetIsolation) {
		LCCollection *colJet = evt->getCollection(_jetCollectionName);
		int njet = colJet->getNumberOfElements();
		for (int i=0; i<njet; ++i) {
			ReconstructedParticle* jet = static_cast<ReconstructedParticle*>( colJet->getElementAt(i) );
			for (ReconstructedParticleVec::const_iterator iter = jet->getParticles().begin();
					iter != jet->getParticles().end(); ++iter) {
				_rpJetMap.insert( std::make_pair( *iter, jet ) );
			}
		}
	}

	// Determine lepton pfos
	std::vector<int> goodLeptonIndices;
	int npfo = _pfoCol->getNumberOfElements();
	for (int i = 0; i < npfo; i++ ) {
		ReconstructedParticle* pfo = static_cast<ReconstructedParticle*>( _pfoCol->getElementAt(i) );
		_workingList.push_back( pfo );

		if ( !IsGoodLepton( pfo )) {
			continue;
		}

		// remember lepton indices for later
		if (_useDressedLeptons){
			if (IsLepton( pfo )) goodLeptonIndices.push_back(i);
		}else{
			goodLeptonIndices.push_back(i);
		}
	}

	// order by energy
	std::sort(goodLeptonIndices.begin(), goodLeptonIndices.end(), [this](const int i, const int j) {return isMoreEnergetic(i, j);});

	// process and save leptons
	for (unsigned int i = 0; i < goodLeptonIndices.size(); ++i)
	{
		ReconstructedParticle* pfo_tmp = static_cast<ReconstructedParticle*>( _pfoCol->getElementAt(goodLeptonIndices.at(i) ));
		ReconstructedParticleImpl* pfo = CopyReconstructedParticle( pfo_tmp );

		if (_useDressedLeptons){
			// don't reprocess merged leptons
			if (std::find(_workingList.begin(), _workingList.end(), pfo_tmp) == _workingList.end()) {
				continue;
			}

			double energy_before = pfo->getEnergy();
			int n_working_list_before = _workingList.size();

			dressLepton(pfo, goodLeptonIndices.at(i));

			streamlog_out(DEBUG) << "dress lepton " << i << " with " << n_working_list_before - _workingList.size() << " particles: energy " << energy_before << " -> " << pfo->getEnergy() << std::endl ;
	  		// printf("dressedMomentum: %.2f -> %.2f, %.2f -> %.2f ,%.2f -> %.2f ,%.2f -> %.2f\n", pfo_tmp->getMomentum()[0], pfo->getMomentum()[0], pfo_tmp->getMomentum()[1], pfo->getMomentum()[1], pfo_tmp->getMomentum()[2], pfo->getMomentum()[2], pfo->getEnergy(), pfo_tmp->getEnergy());
		}

		if  ( IsIsolatedLepton( pfo ) ){
			outIsoLepCol->addElement( pfo );
		}else{
			outPFOsRemovedIsoLepCol->addElement( pfo );
		}
	}

	// filling remaining non-lepton PFOs
	for (unsigned int i = 0; i < _workingList.size(); i++ ) {
		// don't add leptons again
		if (std::find(goodLeptonIndices.begin(), goodLeptonIndices.end(), i) != goodLeptonIndices.end()){
			continue;
		}
		ReconstructedParticle* pfo_tmp = static_cast<ReconstructedParticle*>( _workingList.at(i) );
		ReconstructedParticleImpl* pfo = CopyReconstructedParticle( pfo_tmp );
		outPFOsRemovedIsoLepCol->addElement( pfo );
	}

	// Add PFOs to new collections
	evt->addCollection( outPFOsRemovedIsoLepCol.release(), _outputPFOsRemovedIsoLepCollection.c_str() );
	evt->addCollection( outIsoLepCol.release(), _outputIsoLepCollection.c_str() );
}
void IsolatedLeptonFinderProcessor::dressLepton( ReconstructedParticleImpl* pfo, int PFO_idx ) {
	TVector3 P_lep( pfo->getMomentum() );
	int npfo = _pfoCol->getNumberOfElements();
	std::vector<int> _dressedPFOs {};
	for ( int i = 0; i < npfo; i++ ) {
		ReconstructedParticle* pfo_dress = static_cast<ReconstructedParticle*>( _pfoCol->getElementAt(i) );

		// only add photons and electrons
		bool isPhoton = IsPhoton(pfo_dress);
		bool isElectron = !isPhoton && IsElectron(pfo_dress);  // avoid electrons identified as photons to enter as both
		if (!isPhoton && !isElectron) continue;

		if (!_mergeCloseElectrons && isElectron) continue;

		// don't add itself to itself
		if ( i == PFO_idx ) continue;

		TVector3 Pdress( pfo_dress->getMomentum() );
		float theta = TMath::ACos(P_lep.Dot( Pdress )/(P_lep.Mag()*Pdress.Mag())) * 360 / (2 * TMath::Pi());

		if ( (isPhoton && theta <= _dressPhotonConeAngle) ||
			 (isElectron && theta <= _mergeLeptonConeAngle) ){
			if (std::find(_dressedPFOs.begin(), _dressedPFOs.end(), i) != _dressedPFOs.end()){
				if (isPhoton) {streamlog_out(DEBUG) << "WARNING: photon "<<i<<" with theta "<<theta <<" and type "<<pfo->getType()<<" already close to another lepton!"<<std::endl;}
				else if (isElectron) {streamlog_out(DEBUG) << "WARNING: lepton "<<i<<" with theta "<<theta <<" and type "<<pfo->getType()<<" already close to another lepton!"<<std::endl;}
				// printf(" -- this lep: %.2f, %.2f ,%.2f ,%.2f\n", pfo->getMomentum()[0], pfo->getMomentum()[1], pfo->getMomentum()[2], pfo->getEnergy());
				continue;
			}
			if (isPhoton) {streamlog_out(DEBUG) << "MESSAGE: dressing photon "<<i<<" with theta "<<theta <<" and type "<<pfo->getType()<<std::endl;}
			else if (isElectron) {streamlog_out(DEBUG) << "MESSAGE: merging lepton "<<i<<" with theta "<<theta <<" and type "<<pfo->getType()<<std::endl;}
			_dressedPFOs.push_back(i);
			_workingList.erase(std::remove(_workingList.begin(), _workingList.end(), pfo_dress), _workingList.end());
			double dressedMomentum[3] = {pfo->getMomentum()[0] + pfo_dress->getMomentum()[0],
								  		 pfo->getMomentum()[1] + pfo_dress->getMomentum()[1],
								  		 pfo->getMomentum()[2] + pfo_dress->getMomentum()[2]};
			double dressedE = pfo->getEnergy() + pfo_dress->getEnergy();
			pfo->setMomentum(dressedMomentum);
			pfo->setEnergy(dressedE);
		}
	}
}
void IsolatedLeptonFinderProcessor::end() {
}
ReconstructedParticleImpl* IsolatedLeptonFinderProcessor::CopyReconstructedParticle ( ReconstructedParticle* pfo_orig ) {
	// copy this in an ugly fashion to be modifiable - a versatile copy constructor would be much better!
	ReconstructedParticleImpl* pfo = new ReconstructedParticleImpl();
	pfo->setMomentum(pfo_orig->getMomentum());
	pfo->setEnergy(pfo_orig->getEnergy());
	pfo->setType(pfo_orig->getType());
	pfo->setCovMatrix(pfo_orig->getCovMatrix());
	pfo->setMass(pfo_orig->getMass());
	pfo->setCharge(pfo_orig->getCharge());
	pfo->setParticleIDUsed(pfo_orig->getParticleIDUsed());
	pfo->setGoodnessOfPID(pfo_orig->getGoodnessOfPID());
	pfo->setStartVertex(pfo_orig->getStartVertex());
	for(unsigned int i=0;i<pfo->getTracks().size();i++){
	  pfo->addTrack(pfo_orig->getTracks()[i]);
	}
	for(unsigned int i=0;i<pfo->getClusters().size();i++){
	  pfo->addCluster(pfo_orig->getClusters()[i]);
	}
	return pfo;
}
bool IsolatedLeptonFinderProcessor::IsCharged( ReconstructedParticle* pfo ) {
	if ( pfo->getCharge() == 0 ) return false;
	return true;
}

bool IsolatedLeptonFinderProcessor::IsPhoton( ReconstructedParticle* pfo ) {
	if ( pfo->getType() == 22 ) return true;
	return false;
}
bool IsolatedLeptonFinderProcessor::IsElectron( ReconstructedParticle* pfo ) {

	if (_usePandoraIDs) return (abs(pfo->getType()) == 11);

	float CalE[2];
	getCalEnergy( pfo , CalE );
	double ecale  = CalE[0];
	double hcale  = CalE[1];
	double p      = TVector3( pfo->getMomentum() ).Mag();
	double calByP = p>0 ? (ecale + hcale)/p : 0;
	double calSum = ecale+hcale;
	double ecalFrac = calSum>0 ? ecale / calSum : 0;

	if ( calByP >= _electronMinEnergyDepositByMomentum
			&& calByP <= _electronMaxEnergyDepositByMomentum
			&& ecalFrac >= _electronMinEcalToHcalFraction
			&& ecalFrac <= _electronMaxEcalToHcalFraction )
		return true;

	return false;
}
bool IsolatedLeptonFinderProcessor::IsMuon( ReconstructedParticle* pfo ) {

	if (_usePandoraIDs) return (abs(pfo->getType()) == 13);

	float CalE[2];
	getCalEnergy( pfo , CalE );
	double ecale  = CalE[0];
	double hcale  = CalE[1];
	double p      = TVector3( pfo->getMomentum() ).Mag();
	double calByP = p>0 ? (ecale + hcale)/p : 0;
	double calSum = ecale+hcale;
	double ecalFrac = calSum>0 ? ecale / calSum : 0;

	if ( calByP >= _muonMinEnergyDepositByMomentum
			&& calByP <= _muonMaxEnergyDepositByMomentum
			&& ecalFrac >= _muonMinEcalToHcalFraction
			&& ecalFrac <= _muonMaxEcalToHcalFraction )
		return true;

	return false;
}
bool IsolatedLeptonFinderProcessor::IsLepton( ReconstructedParticle* pfo ) {

	if (IsElectron(pfo) || IsMuon(pfo))
		return true;
	return false;
}

bool IsolatedLeptonFinderProcessor::IsGoodLepton( ReconstructedParticle* pfo ) {

	if ( !IsCharged(pfo) )
		return false;

	if ( _usePID && !IsLepton(pfo) )
		return false;

	if ( _useImpactParameter && !PassesImpactParameterCuts(pfo) )
		return false ;

	if ( _useImpactParameterSignificance && !PassesImpactParameterSignificanceCuts(pfo) )
		return false ;

	return true;
}

bool IsolatedLeptonFinderProcessor::IsIsolatedLepton( ReconstructedParticle* pfo ) {

	if ( _useRectangularIsolation && !IsIsolatedRectangular(pfo) )
		return false;

	if ( _usePolynomialIsolation && !IsIsolatedPolynomial(pfo) )
		return false;

	if ( _useJetIsolation && !IsIsolatedJet(pfo) )
		return false;

	return true;
}

bool IsolatedLeptonFinderProcessor::IsIsolatedRectangular( ReconstructedParticle* pfo ) {
	float E     = pfo->getEnergy() ;
	float coneE = getConeEnergy( pfo );

	if (E < _isoMinTrackEnergy) return false;
	if (E > _isoMaxTrackEnergy) return false;
	if (coneE < _isoMinConeEnergy) return false;
	if (coneE > _isoMaxConeEnergy) return false;

	return true;
}

bool IsolatedLeptonFinderProcessor::IsIsolatedPolynomial( ReconstructedParticle* pfo ) {
	float E     = pfo->getEnergy() ;
	float coneE = getConeEnergy( pfo );

	if ( coneE*coneE <= _isoPolynomialA*E*E + _isoPolynomialB*E + _isoPolynomialC )
		return true ;
	return false;
}

bool IsolatedLeptonFinderProcessor::IsIsolatedJet( ReconstructedParticle* pfo ) {
	// jet-based isolated lepton (LAL algorithm)

	if ( _rpJetMap.find( pfo ) == _rpJetMap.end() ) {
		// this is often the case when jet finding fails e.g. due to too few particles in event
		return false;
	}

	ReconstructedParticle* jet = _rpJetMap[pfo];
	TVector3 vec1( pfo->getMomentum() );
	TVector3 jetmom( jet->getMomentum() );
	TLorentzVector jetmom4( jet->getMomentum(), jet->getEnergy() );

	float jetxt = vec1.Pt( jetmom )/jetmom4.M();
	float jetz = pfo->getEnergy()/jet->getEnergy();

	if (jetxt >= _jetIsoVetoMinXt && jetxt < _jetIsoVetoMaxXt
			&& jetz >= _jetIsoVetoMinZ && jetz < _jetIsoVetoMaxZ) {
		//printf("xt=%f z=%f (not pass)\n",jetxt,jetz);
		return false;
	}

	//printf("xt=%f z=%f (PASS)\n",jetxt,jetz);
	return true;
}

bool IsolatedLeptonFinderProcessor::PassesImpactParameterCuts( ReconstructedParticle* pfo ) {
	const EVENT::TrackVec & trkvec = pfo->getTracks();

	if (trkvec.size()==0) return false;

	// TODO: more sophisticated pfo/track matching
	float d0 = fabs(trkvec[0]->getD0());
	float z0 = fabs(trkvec[0]->getZ0());
	float r0 = sqrt( d0*d0 + z0*z0 );

	if ( d0 < _minD0 ) return false;
	if ( d0 > _maxD0 ) return false;
	if ( z0 < _minZ0 ) return false;
	if ( z0 > _maxZ0 ) return false;
	if ( r0 < _minR0 ) return false;
	if ( r0 > _maxR0 ) return false;

	return true;
}

bool IsolatedLeptonFinderProcessor::PassesImpactParameterSignificanceCuts( ReconstructedParticle* pfo ) {
	const EVENT::TrackVec & trkvec = pfo->getTracks();

	if (trkvec.size()==0) return false;

	// TODO: more sophisticated pfo/track matching
	float d0 = fabs(trkvec[0]->getD0());
	float z0 = fabs(trkvec[0]->getZ0());
	float d0err = sqrt(trkvec[0]->getCovMatrix()[0]);
	float z0err = sqrt(trkvec[0]->getCovMatrix()[9]);

	float d0sig = d0err != 0 ? d0/d0err : 0;
	float z0sig = z0err != 0 ? z0/z0err : 0;
	float r0sig = sqrt( d0sig*d0sig + z0sig*z0sig );

	if ( d0sig < _minD0Sig ) return false;
	if ( d0sig > _maxD0Sig ) return false;
	if ( z0sig < _minZ0Sig ) return false;
	if ( z0sig > _maxZ0Sig ) return false;
	if ( r0sig < _minR0Sig ) return false;
	if ( r0sig > _maxR0Sig ) return false;

	return true;
}

float IsolatedLeptonFinderProcessor::getConeEnergy( ReconstructedParticle* pfo ) {
	float coneE = 0;

	TVector3 P( pfo->getMomentum() );
	int npfo = _workingList.size();
	for ( int i = 0; i < npfo; i++ ) {
		ReconstructedParticle* pfo_i = static_cast<ReconstructedParticle*>( _workingList.at(i) );

		// don't add itself to the cone energy
		if ( pfo == pfo_i ) continue;

		TVector3 P_i( pfo_i->getMomentum() );
		float cosTheta = P.Dot( P_i )/(P.Mag()*P_i.Mag());
		if ( cosTheta >= _cosConeAngle )
			coneE += pfo_i->getEnergy();
	}

	return coneE;
}

void IsolatedLeptonFinderProcessor::getCalEnergy( ReconstructedParticle* pfo , float* cale) {
	float ecal = 0;
	float hcal = 0;
	std::vector<lcio::Cluster*> clusters = pfo->getClusters();
	for ( std::vector<lcio::Cluster*>::const_iterator iCluster=clusters.begin();
			iCluster!=clusters.end();
			++iCluster) {
		ecal += (*iCluster)->getSubdetectorEnergies()[0];
		hcal += (*iCluster)->getSubdetectorEnergies()[1];
	}
	cale[0] = ecal;
	cale[1] = hcal;
}



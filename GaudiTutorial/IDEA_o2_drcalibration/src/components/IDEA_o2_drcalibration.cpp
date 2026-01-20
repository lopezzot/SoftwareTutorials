/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Gaudi
#include "Gaudi/Property.h"
#include <GaudiKernel/SmartIF.h>

// k4FWCore
#include "k4FWCore/Transformer.h"

// k4Interface
#include <k4Interface/IUniqueIDGenSvc.h>

// edm4hep
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"

// dd4hep
#include <DD4hep/DD4hepUnits.h>

// STL
#include <random>
#include <string>
#include <iostream>

struct IDEA_o2_drcalibration final
    : k4FWCore::MultiTransformer<std::tuple<
          edm4hep::CalorimeterHitCollection, // 1. DRBTS_calib
          edm4hep::CalorimeterHitCollection, // 2. DRBTC_calib
          edm4hep::CalorimeterHitCollection, // 3. DRETSL_calib
          edm4hep::CalorimeterHitCollection, // 4. DRETSR_calib
          edm4hep::CalorimeterHitCollection, // 5. DRETCL_calib
          edm4hep::CalorimeterHitCollection, // 6. DRETCR_calib
          edm4hep::CalorimeterHitCollection, // 7. DRCS_calib
          edm4hep::CalorimeterHitCollection  // 8. DRCC_calib
      >(
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
          const edm4hep::SimCalorimeterHitCollection&,
	  const edm4hep::EventHeaderCollection&)> {

public:
  // Constructor
  IDEA_o2_drcalibration(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            // Input collections for the transformer
            {KeyValues("DRBTS_HC", {"DRBTScin"}),
             KeyValues("DRBTC_HC", {"DRBTCher"}),
             KeyValues("DRETSL_HC", {"DRETScinLeft"}),
             KeyValues("DRETSR_HC", {"DRETScinRight"}),
             KeyValues("DRETCL_HC", {"DRETCherLeft"}),
             KeyValues("DRETCR_HC", {"DRETCherRight"}),
             KeyValues("DRCS_HC", {"SCEPCal_MainScounts"}),
             KeyValues("DRCC_HC", {"SCEPCal_MainCcounts"}),
	     KeyValues("HeaderName", {"EventHeader"})},
            // Output collections for the transformer
	    {KeyValues("DRBTS_HC_calib", {"DRBTScin_calib"}),
             KeyValues("DRBTC_HC_calib", {"DRBTCher_calib"}),
             KeyValues("DRETSL_HC_calib", {"DRETScinLeft_calib"}),
             KeyValues("DRETSR_HC_calib", {"DRETScinRight_calib"}),
             KeyValues("DRETCL_HC_calib", {"DRETCherLeft_calib"}),
             KeyValues("DRETCR_HC_calib", {"DRETCherRight_calib"}),
             KeyValues("DRCS_HC_calib", {"DRCS_calib"}),
             KeyValues("DRCC_HC_calib", {"DRCC_calib"})}) {}

  // Initialize
  StatusCode initialize() override {

    m_uniqueIDSvc = serviceLocator()->service(m_uidSvcName);
    if (!m_uniqueIDSvc) {
      error() << "Unable to locate UniqueIDGenSvc with name: " << m_uidSvcName << endmsg;
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  // Function to calibrate (p.e. -> GeV)
  edm4hep::CalorimeterHitCollection calibrate(
      const edm4hep::SimCalorimeterHitCollection& inputHits,
      double calibFactor) const {
    auto outputHits = edm4hep::CalorimeterHitCollection();
    for (const auto& hit : inputHits) {
      auto digihit = outputHits.create();
      digihit.setCellID(hit.getCellID());
      digihit.setEnergy(hit.getEnergy() / calibFactor);
      digihit.setPosition(hit.getPosition());
    }
    return outputHits;
  }

  // Operator: transforms a SimCalorimeterHitCollection into digitized CalorimeterHitCollection
  std::tuple<
      edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection,
      edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection,
      edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection,
      edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection>
  operator()(const edm4hep::SimCalorimeterHitCollection& DRBTS_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRBTC_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRETSL_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRETSR_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRETCL_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRETCR_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRCS_HC,
	     const edm4hep::SimCalorimeterHitCollection& DRCC_HC,
             const edm4hep::EventHeaderCollection& header) const override {

    // Calibrate all the hit collections
    return std::make_tuple(
        calibrate(DRBTS_HC, m_Fiber_S_calibration),
        calibrate(DRBTC_HC, m_Fiber_C_calibration),
        calibrate(DRETSL_HC, m_Fiber_S_calibration),
        calibrate(DRETSR_HC, m_Fiber_S_calibration),
        calibrate(DRETCL_HC, m_Fiber_C_calibration),
        calibrate(DRETCR_HC, m_Fiber_C_calibration),
        calibrate(DRCS_HC, m_Crystal_S_calibration),
        calibrate(DRCC_HC, m_Crystal_C_calibration)
    );
  }

private:
  SmartIF<IUniqueIDGenSvc> m_uniqueIDSvc{nullptr};
  Gaudi::Property<std::string> m_uidSvcName{
      this, "uidSvcName", "UniqueIDGenSvc",
      "The name of the service for generating unique, but reproducable random seeds"};

  Gaudi::Property<double> m_Crystal_S_calibration{this, "CrystalScalibration",1960.92, "Calibration constant for S crystals"};
  Gaudi::Property<double> m_Crystal_C_calibration{this, "CrystalCcalibration",97.531, "Calibration constant for C crystals"};
  Gaudi::Property<double> m_Fiber_S_calibration{this, "FiberScalibration",206.282, "Calibration constant for S fibers"};
  Gaudi::Property<double> m_Fiber_C_calibration{this, "FiberCcalibration",68.069, "Calibration constant for C fibers"};
};

DECLARE_COMPONENT(IDEA_o2_drcalibration)

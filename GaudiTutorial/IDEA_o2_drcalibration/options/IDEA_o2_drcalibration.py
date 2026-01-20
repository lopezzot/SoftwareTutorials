#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from Gaudi.Configuration import INFO
from k4FWCore import IOSvc, ApplicationMgr
from Configurables import EventDataSvc, UniqueIDGenSvc, ChronoAuditor, AuditorSvc

chra = ChronoAuditor()
audsvc = AuditorSvc()
audsvc.Auditors = [chra]

io_svc = IOSvc("IOSvc")
io_svc.Input = "../../data/IDEA_o2_v01_phi0p5_theta0p5_40gev.root"
io_svc.Output = "IDEA_o2_calibrated.root"

from Configurables import IDEA_o2_drcalibration
IDEA_o2_drcalibrator = IDEA_o2_drcalibration("IDEA_o2_drcalibrator",
                                DRBTS_HC=["DRBTScin"],
                                DRBTC_HC=["DRBTCher"],
                                DRETSL_HC=["DRETScinLeft"],
                                DRETSR_HC=["DRETScinRight"],
                                DRETCL_HC=["DRETCherLeft"],
                                DRETCR_HC=["DRETCherRight"],
                                DRCS_HC=["SCEPCal_MainScounts"],
                                DRCC_HC=["SCEPCal_MainCcounts"],
                                DRBTS_HC_calib=["DRBTScin_calib"],
                                DRBTC_HC_calib=["DRBTCher_calib"],
                                DRETSL_HC_calib=["DRETScinLeft_calib"],
                                DRETSR_HC_calib=["DRETScinRight_calib"],
                                DRETCL_HC_calib=["DRETCherLeft_calib"],
                                DRETCR_HC_calib=["DRETCherRight_calib"],
                                DRCS_HC_calib=["SCEPCal_MainScounts_calib"],
                                DRCC_HC_calib=["SCEPCal_MainCcounts_calib"],
                                uidSvcName="uidSvc",
                                CrystalScalibration = 1960.92, # p.e./GeV (calibration at the em scale, scintillation)
                                CrystalCcalibration = 97.531, # p.e./GeV (calibration at the em scale, Cherenkov) 
                                FiberScalibration = 206.282, # p.e./GeV (calibration at the em scale, scintillation)
                                FiberCcalibration = 68.069, # p.e./GeV (calibration at the em scale, Cherenkov)
                                OutputLevel=INFO
                                )

app_mgr = ApplicationMgr(
    TopAlg=[IDEA_o2_drcalibrator],
    EvtSel='NONE',
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc"), UniqueIDGenSvc("uidSvc"), audsvc],
    StopOnSignal=True,
)

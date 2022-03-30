#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#include "bank_containers.h"
#include "constants.h"
#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"
#include "utilities.h"

int run(char *in_filename, bool use_fmt, int nevn, int run_no, double beam_E) {
    // Sampling Fraction. TODO. Temporary code.
    double p1[6] = { 0.25149,  0.25186,  0.24912,  0.24747,  0.24649,  0.25409};
    double p2[6] = { 1.00000,  1.00000,  1.00000,  1.00000,  1.00000,  1.00000};
    double p3[6] = {-0.03427, -0.03771, -0.02627, -0.03163, -0.02723, -0.04000};
    double p4[6] = { 0.00070,  0.00070,  0.00070,  0.00070,  0.00070,  0.00070};

    // Access input file. TODO. Make this input file*s*.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 1;

    // Create and organize histos.
    std::map<const char *, std::map<const char *, TH1 *>> histos;
    histos.insert({PALL, {}});
    histos.insert({PPOS, {}});
    histos.insert({PNEG, {}});
    histos.insert({PPIP, {}});
    histos.insert({PPIM, {}});
    histos.insert({PELC, {}});
    histos.insert({PTRE, {}});

    std::map<const char *, std::map<const char *, TH1 *>>::iterator hmap_it;
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        const char *k1 = hmap_it->first;
        hmap_it->second = {
            {VZ,      new TH1F(Form("%s: %s", k1, VZ),
                    Form("%s;%s", VZ, VZ),                500, -50, 50)},
            {VZPHI,   new TH2F(Form("%s: %s", k1, VZPHI),
                    Form("%s;%s;%s", VZPHI, VZ, PHI),     500, -50, 50, 180, -180, 180)},
            {VZTHETA, new TH2F(Form("%s: %s", k1, VZTHETA),
                    Form("%s;%s;%s", VZTHETA, VZ, THETA), 500, -50, 50, 200,    0,  50)},

            {VP,     new TH1F(Form("%s: %s", k1, VP),
                    Form("%s;%s", VP, VP),              200,    0, 10)},
            {BETA,   new TH1F(Form("%s: %s", k1, BETA),
                    Form("%s;%s", BETA, BETA),          200, 0.94,  1)},
            {VPBETA, new TH2F(Form("%s: %s", k1, VPBETA),
                    Form("%s;%s;%s", VPBETA, VP, BETA), 200,    0, 10, 200, 0.94, 1)},

            {DTOF,  new TH1F(Form("%s: %s", k1, DTOF),
                    Form("%s;%s", DTOF, DTOF),         200, 0, 20)},
            {VPTOF, new TH2F(Form("%s: %s", k1, VPTOF),
                    Form("%s;%s;%s", VPTOF, VP, DTOF), 200, 0, 10, 100, 0, 20)},

            {EEDIVP,   new TH2F(Form("%s: %s", k1, EEDIVP),
                    Form("%s;%s;%s", EEDIVP, E, EDIVP),  200, 0,  3, 200, 0, 0.4)},
            {PEDIVP,   new TH2F(Form("%s: %s", k1, PEDIVP),
                    Form("%s;%s;%s", PEDIVP, EDIVP, VP), 200, 0, 10, 200, 0, 0.4)},
            {PPCALE,   new TH2F(Form("%s: %s", k1, PPCALE),
                    Form("%s;%s;%s", PPCALE, VP, E),     200, 0, 10, 100, 0, 2)},
            {PECINE,   new TH2F(Form("%s: %s", k1, PECINE),
                    Form("%s;%s;%s", PECINE, VP, E),     200, 0, 10, 100, 0, 2)},
            {PECOUE,   new TH2F(Form("%s: %s", k1, PECOUE),
                    Form("%s;%s;%s", PECOUE, VP, E),     200, 0, 10, 100, 0, 2)},
            {ECALPCAL, new TH2F(Form("%s: %s", k1, ECALPCAL),
                    Form("%s;%s;%s", ECALPCAL, E, E),    200, 0, 2, 100, 0, 2)},

            {PCALSF1, new TH1F(Form("%s: %s", k1, PCALSF1), Form("%s;%s", PCALSF1, SF), 200, 0, .5)},
            {PCALSF2, new TH1F(Form("%s: %s", k1, PCALSF2), Form("%s;%s", PCALSF2, SF), 200, 0, .5)},
            {PCALSF3, new TH1F(Form("%s: %s", k1, PCALSF3), Form("%s;%s", PCALSF3, SF), 200, 0, .5)},
            {PCALSF4, new TH1F(Form("%s: %s", k1, PCALSF4), Form("%s;%s", PCALSF4, SF), 200, 0, .5)},
            {PCALSF5, new TH1F(Form("%s: %s", k1, PCALSF5), Form("%s;%s", PCALSF5, SF), 200, 0, .5)},
            {PCALSF6, new TH1F(Form("%s: %s", k1, PCALSF6), Form("%s;%s", PCALSF6, SF), 200, 0, .5)},
            {ECINSF1, new TH1F(Form("%s: %s", k1, ECINSF1), Form("%s;%s", ECINSF1, SF), 200, 0, .5)},
            {ECINSF2, new TH1F(Form("%s: %s", k1, ECINSF2), Form("%s;%s", ECINSF2, SF), 200, 0, .5)},
            {ECINSF3, new TH1F(Form("%s: %s", k1, ECINSF3), Form("%s;%s", ECINSF3, SF), 200, 0, .5)},
            {ECINSF4, new TH1F(Form("%s: %s", k1, ECINSF4), Form("%s;%s", ECINSF4, SF), 200, 0, .5)},
            {ECINSF5, new TH1F(Form("%s: %s", k1, ECINSF5), Form("%s;%s", ECINSF5, SF), 200, 0, .5)},
            {ECINSF6, new TH1F(Form("%s: %s", k1, ECINSF6), Form("%s;%s", ECINSF6, SF), 200, 0, .5)},
            {ECOUSF1, new TH1F(Form("%s: %s", k1, ECOUSF1), Form("%s;%s", ECOUSF1, SF), 200, 0, .5)},
            {ECOUSF2, new TH1F(Form("%s: %s", k1, ECOUSF2), Form("%s;%s", ECOUSF2, SF), 200, 0, .5)},
            {ECOUSF3, new TH1F(Form("%s: %s", k1, ECOUSF3), Form("%s;%s", ECOUSF3, SF), 200, 0, .5)},
            {ECOUSF4, new TH1F(Form("%s: %s", k1, ECOUSF4), Form("%s;%s", ECOUSF4, SF), 200, 0, .5)},
            {ECOUSF5, new TH1F(Form("%s: %s", k1, ECOUSF5), Form("%s;%s", ECOUSF5, SF), 200, 0, .5)},
            {ECOUSF6, new TH1F(Form("%s: %s", k1, ECOUSF6), Form("%s;%s", ECOUSF6, SF), 200, 0, .5)},

            {Q2, new TH1F(Form("%s: %s", k1, Q2), Form("%s;%s", Q2, Q2), 22, 0, 12)},
            {NU, new TH1F(Form("%s: %s", k1, NU), Form("%s;%s", NU, NU), 22, 0, 12)},
            {XB, new TH1F(Form("%s: %s", k1, XB), Form("%s;%s", XB, XB), 20, 0,  2)},
        };
    }

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    REC_Particle     rp(t);
    REC_Track        rt(t);
    REC_Scintillator rs(t);
    REC_Calorimeter  rc(t);
    FMT_Tracks       ft(t);

    // Iterate through input file. Each TTree entry is one event.
    int evn;
    int divcntr = 0;
    int evnsplitter = 0;
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() : nevn, in_filename);
    for (evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
        if (evn >= evnsplitter) {
            if (evn != 0) {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            }
            printf("[");
            for (int i = 0; i <= 50; ++i) {
                if (i <= divcntr/2) printf("=");
                else                printf(" ");
            }
            printf("] %2d%%", divcntr);
            fflush(stdout);
            divcntr++;
            evnsplitter = nevn == -1 ? (t->GetEntries() / 100) * divcntr : (nevn/100) * divcntr;
        }

        rp.get_entries(t, evn);
        rt.get_entries(t, evn);
        rs.get_entries(t, evn);
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0) continue;

        // Find trigger electron's TOF.
        int tre_pindex = rt.pindex->at(0);
        double tre_tof = INFINITY;
        for (UInt_t i = 0; i < rs.pindex->size(); ++i) {
            if (rs.pindex->at(i) == tre_pindex && rs.time->at(i) < tre_tof) tre_tof = rs.time->at(i);
        }

        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index      = rt.index ->at(pos);
            int pindex     = rt.pindex->at(pos);
            int ndf        = rt.ndf   ->at(pos);
            double chi2    = rt.chi2  ->at(pos);

            int charge     = rp.charge ->at(pindex);
            int pid        = rp.pid    ->at(pindex);
            int status     = rp.status ->at(pindex);
            // double chi2pid = rp.chi2pid->at(pindex);

            // General cuts.
            // if ((int) abs(status)/1000 != 2) continue;
            // if (abs(chi2pid) >= 3) continue; // Spurious particle.
            if (pid == 0)       continue; // Non-identified particle.
            if (chi2/ndf >= 15) continue; // Ignore tracks with high chi2.

            // TODO. I should filter already processed pindexes so that I don't count a detector's
            //       data more than once.

            // Figure out which histograms are to be filled.
            // TODO. Choose these particules from cuts, not PID.
            std::map<const char *, bool> truth_map;
            truth_map.insert({PALL, true});
            truth_map.insert({PPOS, charge > 0  ? true : false});
            truth_map.insert({PNEG, charge < 0  ? true : false});
            truth_map.insert({PPIP, pid ==  211 ? true : false});
            truth_map.insert({PPIM, pid == -211 ? true : false});
            truth_map.insert({PELC, pid ==   11 ? true : false});
            truth_map.insert({PTRE, (pid == 11 && status < 0) ? true : false});

            // Get reconstructed particle from either FMT or DC.
            double vx, vy, vz;
            double px, py, pz;
            if (use_fmt) {
                // Apply FMT cuts.
                if (ft.vz->size() < 1)      continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) != 3) continue; // Track crossed 3 FMT layers.
                // if (ft.ndf->at(index) > 0) printf("NDF: %d\n", ft.ndf->at(index));

                vx = ft.vx->at(index); vy = ft.vy->at(index); vz = ft.vz->at(index);
                px = ft.px->at(index); py = ft.py->at(index); pz = ft.pz->at(index);
            }
            else {
                vx = rp.vx->at(pindex); vy = rp.vy->at(pindex); vz = rp.vz->at(pindex);
                px = rp.px->at(pindex); py = rp.py->at(pindex); pz = rp.pz->at(pindex);
            }

            // Geometry cuts.
            if (vx*vx + vy*vy > 4)   continue; // Too far from beamline.
            if (-40 > vz || vz > 40) continue; // Too far from target.

            // Get calorimeters data.
            double pcal_E = 0; // PCAL total deposited energy.
            double ecin_E = 0; // EC inner total deposited energy.
            double ecou_E = 0; // EC outer total deposited energy.
            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) == pindex) {
                    int lyr = (int) rc.layer->at(i);
                    // TODO. Add correction via sampling fraction.

                    if      (lyr == PCAL_LYR) pcal_E += rc.energy->at(i);
                    else if (lyr == ECIN_LYR) ecin_E += rc.energy->at(i);
                    else if (lyr == ECOU_LYR) ecou_E += rc.energy->at(i);
                    else return 2;
                }
            }
            double tot_E = pcal_E + ecin_E + ecou_E;

            // Get scintillators data.
            double tof = INFINITY;
            for (UInt_t i = 0; i < rs.pindex->size(); ++i)
                if (rs.pindex->at(i) == pindex && rs.time->at(i) < tof) tof = rs.time->at(i);

            // Walk through histos
            for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
                if (!truth_map[hmap_it->first]) continue;

                // Vertex z.
                histos[hmap_it->first][VZ]     ->Fill(vz);
                histos[hmap_it->first][VZPHI]  ->Fill(vz, to_deg(calc_phi(px, py)));
                histos[hmap_it->first][VZTHETA]->Fill(vz, to_deg(calc_theta(px, py, pz)));

                // Vertex p.
                histos[hmap_it->first][VP]    ->Fill(pz);
                histos[hmap_it->first][BETA]  ->Fill(rp.beta->at(pindex));
                histos[hmap_it->first][VPBETA]->Fill(pz, rp.beta->at(pindex));

                // TOF. (TODO. Check FTOF resolution).
                double dtof = tof - tre_tof;
                if (tre_tof > 0 && dtof > 0) { // Only fill if trigger electron's TOF was found.
                    histos[hmap_it->first][DTOF] ->Fill(dtof);
                    histos[hmap_it->first][VPTOF]->Fill(calc_P(px,py,pz), dtof);
                }

                // Calorimeters.
                double tot_P = calc_P(px,py,pz);
                if (tot_E > 0) {
                    histos[hmap_it->first][PEDIVP]->Fill(tot_P, tot_E/tot_P);
                    histos[hmap_it->first][EEDIVP]->Fill(tot_E, tot_E/tot_P);
                }
                if (pcal_E > 0) histos[hmap_it->first][PPCALE]->Fill(tot_P, pcal_E);
                if (ecin_E > 0) histos[hmap_it->first][PECINE]->Fill(tot_P, ecin_E);
                if (ecou_E > 0) histos[hmap_it->first][PECOUE]->Fill(tot_P, ecou_E);
                if (pcal_E > 0 && ecin_E > 0 && ecou_E > 0)
                    histos[hmap_it->first][ECALPCAL]->Fill(ecin_E+ecou_E, pcal_E);

                // Sampling Fraction.
                for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                    if (rc.pindex->at(i) == pindex) {
                        int lyr       = (int) rc.layer->at(i);
                        double energy = rc.energy->at(i);
                        int s         = rc.sector->at(i);
                        double sf = p1[s-1] * (p2[s-1] + p3[s-1]/energy + pow(p4[s-1]/energy, 2));
                        if      (lyr == PCAL_LYR) {
                            switch (s) { // NOTE. Bad solution because I'm lazy. I should change this.
                                case  0: break;
                                case  1: histos[hmap_it->first][PCALSF1]->Fill(sf); break;
                                case  2: histos[hmap_it->first][PCALSF2]->Fill(sf); break;
                                case  3: histos[hmap_it->first][PCALSF3]->Fill(sf); break;
                                case  4: histos[hmap_it->first][PCALSF4]->Fill(sf); break;
                                case  5: histos[hmap_it->first][PCALSF5]->Fill(sf); break;
                                case  6: histos[hmap_it->first][PCALSF6]->Fill(sf); break;
                                default: return 3;
                            };
                        }
                        else if (lyr == ECIN_LYR) {
                            switch (s) {
                                case  0: break;
                                case  1: histos[hmap_it->first][ECINSF1]->Fill(sf); break;
                                case  2: histos[hmap_it->first][ECINSF2]->Fill(sf); break;
                                case  3: histos[hmap_it->first][ECINSF3]->Fill(sf); break;
                                case  4: histos[hmap_it->first][ECINSF4]->Fill(sf); break;
                                case  5: histos[hmap_it->first][ECINSF5]->Fill(sf); break;
                                case  6: histos[hmap_it->first][ECINSF6]->Fill(sf); break;
                                default: return 3;
                            };
                        }
                        else if (lyr == ECOU_LYR) {
                            switch (s) {
                                case  0: break;
                                case  1: histos[hmap_it->first][ECOUSF1]->Fill(sf); break;
                                case  2: histos[hmap_it->first][ECOUSF2]->Fill(sf); break;
                                case  3: histos[hmap_it->first][ECOUSF3]->Fill(sf); break;
                                case  4: histos[hmap_it->first][ECOUSF4]->Fill(sf); break;
                                case  5: histos[hmap_it->first][ECOUSF5]->Fill(sf); break;
                                case  6: histos[hmap_it->first][ECOUSF6]->Fill(sf); break;
                                default: return 3;
                            };
                        }
                        else return 2;
                    }
                }

                // SIDIS variables.
                if (pid == 11) {
                    double calc_theta(double px, double py, double pz);
                    histos[hmap_it->first][Q2]->Fill(calc_Q2(beam_E, calc_P(px,py,pz), calc_theta(px,py,pz)));
                    histos[hmap_it->first][NU]->Fill(calc_nu(beam_E, calc_P(px,py,pz)));
                    histos[hmap_it->first][XB]->Fill(calc_Xb(beam_E, calc_P(px,py,pz), calc_theta(px,py,pz)));
                }
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("[==================================================] 100%%\n");

    // Fit histograms.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        // Vz upstream fit.
        TH1 *vz = histos[hmap_it->first][VZ];
        TString vz_fit_name = Form("%s %s", hmap_it->first, "vz fit");
        TF1 *vz_fit = new TF1(vz_fit_name,
                "[0]*TMath::Gaus(x,[1],[2])+[3]*TMath::Gaus(x,[1]-2.4,[2])+[4]+[5]*x+[6]*x*x",
                -36,-30);
        vz->GetXaxis()->SetRange(70,100); // Set range to fitted range.
        vz_fit->SetParameter(0 /* amp1  */, vz->GetBinContent(vz->GetMaximumBin()));
        vz_fit->SetParameter(1 /* mean  */, vz->GetXaxis()->GetBinCenter(vz->GetMaximumBin()));
        vz_fit->SetParameter(2 /* sigma */, 0.5);
        vz_fit->SetParameter(3 /* amp2 */,  0);
        vz_fit->SetParameter(4 /* p0 */,    0);
        vz_fit->SetParameter(5 /* p1 */,    0);
        vz_fit->SetParameter(6 /* p2 */,    0);
        vz->GetXaxis()->SetRange(0,500);
        histos[hmap_it->first][VZ]->Fit(vz_fit_name, "Q", "", -36., -30.);

        // Vp vs beta theoretical curve.
        double mass = 0;
        if      (!strcmp(hmap_it->first, PPIP) || !strcmp(hmap_it->first, PPIM)) mass = PIMASS;
        else if (!strcmp(hmap_it->first, PELC) || !strcmp(hmap_it->first, PTRE)) mass = EMASS;
        else continue;
        TString beta_vp_curve_name = Form("%s %s", hmap_it->first, "beta vs vp curve");
        TF1 *beta_vp_curve =
                new TF1(beta_vp_curve_name, "(x)/(sqrt([m]*[m] + x*x))", 0, 12);
        beta_vp_curve->FixParameter(0, mass);
        histos[hmap_it->first][VPBETA]->Fit(beta_vp_curve_name, "Q", "", 0, 12);
    }

    // Create output file.
    TFile *f_out = TFile::Open("../root_io/out.root", "RECREATE");

    // Write to output file.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        TCanvas *gcvs = new TCanvas();

        TString dir = Form("%s/%s", hmap_it->first, "Vertex Z");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][VZ]     ->Write();
        histos[hmap_it->first][VZPHI]  ->Draw("colz"); gcvs->Write(VZPHI);
        histos[hmap_it->first][VZTHETA]->Draw("colz"); gcvs->Write(VZTHETA);

        dir = Form("%s/%s", hmap_it->first, "Vertex P");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][VP]    ->Write();
        histos[hmap_it->first][BETA]  ->Write();
        histos[hmap_it->first][VPBETA]->Draw("colz"); gcvs->Write(VPBETA);

        dir = Form("%s/%s", hmap_it->first, "DTOF");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][DTOF] ->Write();
        histos[hmap_it->first][VPTOF]->Draw("colz"); gcvs->Write(VPTOF);

        dir = Form("%s/%s", hmap_it->first, "CALs");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][PEDIVP]  ->Draw("colz"); gcvs->Write(PEDIVP);
        histos[hmap_it->first][EEDIVP]  ->Draw("colz"); gcvs->Write(EEDIVP);
        histos[hmap_it->first][PPCALE]  ->Draw("colz"); gcvs->Write(PPCALE);
        histos[hmap_it->first][PECINE]  ->Draw("colz"); gcvs->Write(PECINE);
        histos[hmap_it->first][PECOUE]  ->Draw("colz"); gcvs->Write(PECOUE);
        histos[hmap_it->first][ECALPCAL]->Draw("colz"); gcvs->Write(ECALPCAL);

        dir = Form("%s/%s/%s", hmap_it->first, "CALs", "Sampling Fraction");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][PCALSF1]->Write();
        histos[hmap_it->first][PCALSF2]->Write();
        histos[hmap_it->first][PCALSF3]->Write();
        histos[hmap_it->first][PCALSF4]->Write();
        histos[hmap_it->first][PCALSF5]->Write();
        histos[hmap_it->first][PCALSF6]->Write();
        histos[hmap_it->first][ECINSF1]->Write();
        histos[hmap_it->first][ECINSF2]->Write();
        histos[hmap_it->first][ECINSF3]->Write();
        histos[hmap_it->first][ECINSF4]->Write();
        histos[hmap_it->first][ECINSF5]->Write();
        histos[hmap_it->first][ECINSF6]->Write();
        histos[hmap_it->first][ECOUSF1]->Write();
        histos[hmap_it->first][ECOUSF2]->Write();
        histos[hmap_it->first][ECOUSF3]->Write();
        histos[hmap_it->first][ECOUSF4]->Write();
        histos[hmap_it->first][ECOUSF5]->Write();
        histos[hmap_it->first][ECOUSF6]->Write();

        dir = Form("%s/%s", hmap_it->first, "SIDIS");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[hmap_it->first][Q2]->Write();
        histos[hmap_it->first][NU]->Write();
        histos[hmap_it->first][XB]->Write();
    }

    f_in ->Close();
    f_out->Close();
    free(in_filename);

    return 0;
}

// Execute program from clas12root (`.x src/acceptance.c(filename, use_fmt, nevn)`).
int acceptance(char *in_filename, bool use_fmt, int nevn) {
    int    run_no = -1;
    double beam_E = -1;
    if (acceptance_handle_args_err(handle_root_filename(in_filename, &run_no, &beam_E),
                                   &in_filename, run_no)
    ) return 1;
    return acceptance_err(run(in_filename, use_fmt, nevn, run_no, beam_E), &in_filename);
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool   use_fmt     = false;
    int    nevn     = -1;
    char   *in_filename = NULL;
    int    run_no      = -1;
    double beam_E      = -1;

    if (acceptance_handle_args_err(
            acceptance_handle_args(argc, argv, &use_fmt, &nevn, &in_filename, &run_no, &beam_E),
                                   &in_filename, run_no)
    ) return 1;
    return acceptance_err(run(in_filename, use_fmt, nevn, run_no, beam_E), &in_filename);
}

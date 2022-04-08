#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraph.h>
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

int run(char *in_filename, bool use_fmt, int nevn) {
    // Access input file. TODO. Make this input file*s*.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 1;

    // Create and organize histos and name arrays.
    std::map<const char *, TH1 *> histos;

    const int ncals = sizeof(CALNAME)/sizeof(CALNAME[0]);
    char *sf1D_name_arr[ncals][NSECTORS][(int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)];
    char *sf2D_name_arr[ncals][NSECTORS];
    TGraph *sf_dotgraph_top[ncals][NSECTORS];
    TGraph *sf_dotgraph_bot[ncals][NSECTORS];

    int ci = -1;
    for (const char *cal : SFARR2D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
            oss << cal << si+1 << ")";
            sf2D_name_arr[ci][si] = (char *) malloc(strlen(oss.str().c_str())+1);
            strncpy(sf2D_name_arr[ci][si], oss.str().c_str(), strlen(oss.str().c_str()));
            insert_TH2F(&histos, PALL, sf2D_name_arr[ci][si], VP, EDIVP, 200, 0, 10, 200, 0, 0.4);
            sf_dotgraph_top[ci][si] = new TGraph();
            sf_dotgraph_top[ci][si]->SetMarkerStyle(kFullCircle);
            sf_dotgraph_top[ci][si]->SetMarkerColor(kRed);
            sf_dotgraph_bot[ci][si] = new TGraph();
            sf_dotgraph_bot[ci][si]->SetMarkerStyle(kFullCircle);
            sf_dotgraph_bot[ci][si]->SetMarkerColor(kRed);
        }
    }

    ci = -1;
    for (const char *cal : SFARR1D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            int pi = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                pi++;
                std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
                oss << cal << si+1 << " (" << p << " < P_{tot} < " << p+SF_PSTEP << ")";
                sf1D_name_arr[ci][si][pi] = (char *) malloc(strlen(oss.str().c_str())+1);
                strncpy(sf1D_name_arr[ci][si][pi], oss.str().c_str(), strlen(oss.str().c_str()));
                insert_TH1F(&histos, PALL, sf1D_name_arr[ci][si][pi], EDIVP, 200, 0, 0.4);
            }
        }
    }

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    REC_Particle     rp(t);
    REC_Track        rt(t);
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
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
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
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0 || rc.pindex->size() == 0) continue;

        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index  = rt.index ->at(pos);
            int pindex = rt.pindex->at(pos);

            // Get particle momentum from either FMT or DC.
            double px, py, pz;
            if (use_fmt) {
                // Apply FMT cuts.
                if (ft.pz->size() < 1)      continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) != 3) continue; // Track crossed 3 FMT layers.

                px = ft.px->at(index); py = ft.py->at(index); pz = ft.pz->at(index);
            }
            else {
                px = rp.px->at(pindex); py = rp.py->at(pindex); pz = rp.pz->at(pindex);
            }
            double tot_P = calc_P(px, py, pz);

            // Compute energy deposited in each calorimeter per sector.
            double sf_E[ncals][NSECTORS];
            for (int ci = 0; ci < ncals; ++ci) {
                for (int si = 0; si < NSECTORS; ++si) sf_E[ci][si] = 0;
            }

            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) != pindex) continue;

                // Get sector.
                int si = rc.sector->at(i) - 1;
                if      (si == -1)                   continue;
                else if (si < -1 || si > NSECTORS-1) return 3;

                // Get detector.
                switch(rc.layer->at(i)) {
                    case PCAL_LYR: sf_E[PCAL_IDX][si] += rc.energy->at(i); break;
                    case ECIN_LYR: sf_E[ECIN_IDX][si] += rc.energy->at(i); break;
                    case ECOU_LYR: sf_E[ECOU_IDX][si] += rc.energy->at(i); break;
                    default:       return 2;
                }
            }

            for (int ci = 0; ci < ncals-1; ++ci) {
                for (int si = 0; si < NSECTORS; ++si) sf_E[CALS_IDX][si] += sf_E[ci][si];
            }

            // Get momentum bin.
            if (tot_P < SF_PMIN || tot_P > SF_PMAX) continue;
            int pi = -1;
            for (double p_cnt = SF_PMIN; p_cnt <= SF_PMAX; p_cnt += SF_PSTEP) {
                if (tot_P < p_cnt) break;
                pi++;
            }

            // Write to histograms.
            for (int ci = 0; ci < ncals; ++ci) {
                for (int si = 0; si < NSECTORS; ++si) {
                    if (sf_E[ci][si] <= 0) continue;
                    histos[sf2D_name_arr[ci][si]]->Fill(tot_P, sf_E[ci][si]/tot_P);
                    histos[sf1D_name_arr[ci][si][pi]]->Fill(sf_E[ci][si]/tot_P);
                }
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("[==================================================] 100%%\n");

    // Fit histograms.
    ci = -1;
    for (const char *cal : SFARR1D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            int pi = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                pi++;

                // Get ref to histogram.
                TH1 *EdivP = histos[sf1D_name_arr[ci][si][pi]];

                // Form fit string name.
                std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
                oss << cal << si+1 << " (" << p << " < P_{tot} < " << p+SF_PSTEP << ") fit";

                // Fit.
                TF1 *sf_gaus = new TF1(oss.str().c_str(),
                                       "[0]*TMath::Gaus(x,[1],[2]) + [3]*x*x + [4]*x + [5]", 0.06, 0.25);
                sf_gaus->SetParameter(0 /* amp   */, EdivP->GetBinContent(EdivP->GetMaximumBin()));
                sf_gaus->SetParameter(1 /* mean  */, 0.15);
                sf_gaus->SetParLimits(1, 0.06, 0.25);
                sf_gaus->SetParameter(2 /* sigma */, 0.05);
                sf_gaus->SetParLimits(2, 0., 0.1);
                sf_gaus->SetParameter(3 /* p0 */,    0);
                sf_gaus->SetParameter(4 /* p1 */,    0);
                sf_gaus->SetParameter(5 /* p2 */,    0);
                EdivP->Fit(sf_gaus, "QR", "", 0.06, 0.25);

                // Extract mean and sigma from fit and add it to 2D plots.
                double mean  = sf_gaus->GetParameter(1);
                double sigma = sf_gaus->GetParameter(2);
                sf_dotgraph_top[ci][si]->AddPoint(p + SF_PSTEP/2, mean + 2*sigma);
                sf_dotgraph_bot[ci][si]->AddPoint(p + SF_PSTEP/2, mean - 2*sigma);
            }
        }
    }

    // Create output file.
    TFile *f_out = TFile::Open("../root_io/sf_study.root", "RECREATE");

    // Write to output file.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (ci = 0; ci < ncals; ++ci) {
        dir = Form("%s", CALNAME[ci]);
        f_out->mkdir(dir);
        f_out->cd(dir);
        for (int si = 0; si < NSECTORS; ++si) {
            dir = Form("%s/sector %d", CALNAME[ci], si+1);
            f_out->mkdir(dir);
            f_out->cd(dir);

            histos[sf2D_name_arr[ci][si]]->Draw("colz");
            sf_dotgraph_top[ci][si]->Draw("Psame");
            sf_dotgraph_bot[ci][si]->Draw("Psame");
            gcvs->Write(sf2D_name_arr[ci][si]);
            free(sf2D_name_arr[ci][si]);
            for (int pi = 0; pi < ((int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)); ++pi) {
                histos[sf1D_name_arr[ci][si][pi]]->Write();
                free(sf1D_name_arr[ci][si][pi]);
            }
        }
    }

    f_in ->Close();
    f_out->Close();
    free(in_filename);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool use_fmt      = false;
    int nevn          = -1;
    char *in_filename = NULL;

    if (extractsf_handle_args_err(extractsf_handle_args(argc, argv, &use_fmt, &nevn, &in_filename),
            &in_filename))
        return 1;
    return extractsf_err(run(in_filename, use_fmt, nevn), &in_filename);
}

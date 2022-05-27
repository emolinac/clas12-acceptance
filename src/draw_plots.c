#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>

// #include "../lib/constants.h"
// #include "../lib/err_handler.h"
// #include "../lib/file_handler.h"
// #include "../lib/io_handler.h"
// #include "../lib/particle.h"
#include "../lib/utilities.h"

// TODO. Check and fix theoretical curves. -> Ask Raffa.
// TODO. See why I'm not seeing any neutrals. -> Ask Raffa.
// TODO. Get simulations from RG-F, understand how they're made to do acceptance correction.
//           -> Ask Raffa.

// TODO. Separate in z bins and see what happens.
// TODO. Evaluate acceptance in diferent regions.
// TODO. See simulations with Esteban.
// NOTE. Adding a functionality to be able to request a plot and get it done in one line would be
//       the gold standard for this program.

int run() {
    TFile * f_in  = TFile::Open("../root_io/ntuples.root", "READ");   // NOTE. This path sucks.
    TFile * f_out = TFile::Open("../root_io/plots.root", "RECREATE"); // NOTE. Again. This path sucks.

    if (!f_in || f_in->IsZombie()) return 1;

    // NOTE. This function could receive a few arguments to speed IO up. Pre-configured cuts,
    //       binnings, and corrections would be nice.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).
    // TODO. Prepare binning.

    // === CUTS ====================================================================================
    bool general_cuts  = false;
    bool geometry_cuts = false;
    bool sidis_cuts    = false;
    printf("\nApply all default cuts? [y/n]\n");
    if (!catch_yn()) {
        printf("\nApply general cuts? [y/n]\n");
        general_cuts = catch_yn();
        printf("\nApply geometry cuts? [y/n]\n");
        geometry_cuts = catch_yn();
        printf("\nApply SIDIS cuts? [y/n]\n");
        sidis_cuts = catch_yn();
    }
    else {
        general_cuts  = true;
        geometry_cuts = true;
        sidis_cuts    = true;
    }

    // TODO.
    // printf("\nApply any custom cut? [y/n]\n");
    // bool custom_cuts = catch_yn();

    // === BINNING =================================================================================

    // === PLOTTING ================================================================================
    // Check if we are to make a 1D or 2D plot.
    printf("\nPlot type? [");
    for (int pi = 0; pi < PLOT_LIST_SIZE; ++pi) printf("%s, ", PLOT_LIST[pi]);
    printf("\b\b]:\n");
    int px = catch_string(PLOT_LIST, PLOT_LIST_SIZE);
    int pn = px + 1; // Number of histogram axes.

    // Check variable(s) to be plotted.
    int vx[pn];
    char * vx_tuplename[pn];
    for (int pi = 0; pi < pn; ++pi) {
        printf("\nDefine var to be plotted on the %s axis. Available vars:\n[", DIM_LIST[pi]);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        vx[pi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);
        find_ntuple(&vx_tuplename[pi], vx[pi]);
    }

    // Define ranges.
    double rx[pn][2];
    for (int pi = 0; pi < pn; ++pi) {
        for (int ri = 0; ri < 2; ++ri) {
            printf("\nDefine %s limit for %s axis:\n", RAN_LIST[ri], DIM_LIST[pi]);
            rx[pi][ri] = catch_double();
        }
    }

    // Define number of bins in plot.
    long bx[pn];
    for (int pi = 0; pi < pn; ++pi) {
        printf("\nDefine number of bins for %s axis:\n", DIM_LIST[pi]);
        bx[pi] = catch_long();
    }

    // Plot.
    TCanvas * gcvs = new TCanvas();
    TH1 * plt;
    if (px == 0)
        plt = new TH1F(S_VAR_LIST[vx[0]], S_VAR_LIST[vx[0]], bx[0], rx[0][0], rx[0][1]);
    if (px == 1) {
        TString name = Form("%s vs %s", S_VAR_LIST[vx[0]], S_VAR_LIST[vx[1]]);
        plt = new TH2F(name, name, bx[0], rx[0][0], rx[0][1], bx[1], rx[1][0], rx[1][1]);
    }

    // Apply SIDIS cuts, checking which event numbers should be skipped.
    // int nruns   =  1; // TODO.
    int nevents = -1;
    // Count number of events. NOTE. There's probably a cleaner way to do this.
    TNtuple * cuts = (TNtuple *) f_in->Get(S_CUTS);
    Float_t c_evn; cuts->SetBranchAddress(S_EVENTNO, &c_evn);
    for (int i = 0; i < cuts->GetEntries(); ++i) {
        cuts->GetEntry(i);
        if (c_evn > nevents) nevents = (int) (c_evn+0.5);
    }

    bool valid_event[nevents];
    Float_t c_pid;    cuts->SetBranchAddress(S_PID,     &c_pid);
    Float_t c_status; cuts->SetBranchAddress(S_STATUS,  &c_status);
    Float_t c_q2;     cuts->SetBranchAddress(S_Q2,      &c_q2);
    Float_t c_w2;     cuts->SetBranchAddress(S_W2,      &c_w2);

    Float_t current_evn = -1;
    bool no_tre_pass, Q2_pass, W2_pass;
    for (int i = 0; i < cuts->GetEntries(); ++i) {
        cuts->GetEntry(i);
        if (c_evn != current_evn) {
            current_evn = c_evn;
            valid_event[(int) (c_evn+0.5)] = false;
            no_tre_pass = false;
            Q2_pass     = true;
            W2_pass     = true;
        }

        if (c_pid != 11 || c_status > 0) continue;
        no_tre_pass = true;
        Q2_pass = c_q2 >= Q2CUT;
        W2_pass = c_w2 >= W2CUT;

        valid_event[(int) (c_evn+0.5)] = no_tre_pass && Q2_pass && W2_pass;
    }

    // TNtuples for cuts.
    Float_t c_ndf;  cuts->SetBranchAddress(S_NDF,     &c_ndf);
    Float_t c_chi2; cuts->SetBranchAddress(S_CHI2,    &c_chi2);
    Float_t c_vx;   cuts->SetBranchAddress(S_VX,      &c_vx);
    Float_t c_vy;   cuts->SetBranchAddress(S_VY,      &c_vy);
    Float_t c_vz;   cuts->SetBranchAddress(S_VZ,      &c_vz);

    // TNtuples of plotted variables.
    TNtuple * t[pn];
    Float_t var[pn];
    for (int pi = 0; pi < pn; ++pi) {
        t[pi] = (TNtuple *) f_in->Get(vx_tuplename[pi]);
        t[pi]->SetBranchAddress(S_VAR_LIST[vx[pi]], &var[pi]);
    }
    for (int i = 0; i < t[0]->GetEntries(); ++i) {
        cuts->GetEntry(i);
        for (int pi = 0; pi < pn; ++pi) t[pi]->GetEntry(i);

        // Apply cuts.
        if (general_cuts) {
            if (-0.5 < c_pid && c_pid < 0.5) continue; // Non-identified particle.
            if (c_chi2/c_ndf >= CHI2NDFCUT)  continue; // Ignore tracks with high chi2.
        }

        if (geometry_cuts) {
            if (sqrt(c_vx*c_vx + c_vy*c_vy) > VXVYCUT) continue; // Too far from beamline.
            if (VZLOWCUT > c_vz || c_vz > VZHIGHCUT)   continue; // Too far from target.
        }

        if (sidis_cuts && !valid_event[(int) (c_evn+0.5)]) continue; // Event didn't pass SIDIS cut.

        // SIDIS variables only make sense for some particles.
        bool sidis_pass = true;
        for (int pi = 0; pi < pn; ++pi) {
            for (int li = 0; li < SIDIS_LIST_SIZE; ++li) {
                if (!strcmp(R_VAR_LIST[vx[pi]], SIDIS_LIST[li]) && var[pi] < 1e-6)
                    sidis_pass = false;
            }
        }
        if (!sidis_pass) continue;

        // Fill histogram.
        if (px == 0) plt->Fill(var[0]);
        if (px == 1) plt->Fill(var[0], var[1]);
    }
    if (px == 0) plt->Write();
    if (px == 1) {
        plt->Draw("colz");
        gcvs->Write();
    }

    // === CLEAN-UP ================================================================================
    f_in ->Close();
    f_out->Close();
    for (int pi = 0; pi < pn; ++pi) free(vx_tuplename[pi]);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}

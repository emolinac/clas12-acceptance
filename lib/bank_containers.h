// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU Lesser General Public License as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You can see a copy of the GNU Lesser Public License under the LICENSE file.

#ifndef BANK_CONTAINERS
#define BANK_CONTAINERS

#include <TBranch.h>
#include <TTree.h>
#include "reader.h"

/** Reconstructed particle "final" information. */
class REC_Particle {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Int_t>    *pid;     TBranch *b_pid;     // particle id in LUND conventions.
    std::vector<Float_t>  *px;      TBranch *b_px;      // x component of the momentum (GeV).
    std::vector<Float_t>  *py;      TBranch *b_py;      // y component of the momentum (GeV).
    std::vector<Float_t>  *pz;      TBranch *b_pz;      // z component of the momentum (GeV).
    std::vector<Float_t>  *vx;      TBranch *b_vx;      // x component of the vertex (cm).
    std::vector<Float_t>  *vy;      TBranch *b_vy;      // y component of the vertex (cm).
    std::vector<Float_t>  *vz;      TBranch *b_vz;      // z component of the vertex (cm).
    std::vector<Float_t>  *vt;      TBranch *b_vt;      // RF and z corrected vertex time (ns).
    std::vector<Char_t>   *charge;  TBranch *b_charge;  // particle charge.
    std::vector<Float_t>  *beta;    TBranch *b_beta;    // particle beta measured by TOF.
    std::vector<Float_t>  *chi2pid; TBranch *b_chi2pid; // Chi2 of assigned PID.
    std::vector<Short_t>  *status;  TBranch *b_status;  // Detector collection particle passed.
    REC_Particle();
    REC_Particle(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Track {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Short_t>  *index;   TBranch *b_index;
    std::vector<Short_t>  *pindex;  TBranch *b_pindex;
    std::vector<Short_t>  *sector;  TBranch *b_sector;
    std::vector<Short_t>  *ndf;     TBranch *b_ndf;
    std::vector<Float_t>  *chi2;    TBranch *b_chi2;
    REC_Track();
    REC_Track(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Calorimeter {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Short_t> *pindex; TBranch *b_pindex;
    std::vector<Char_t>  *layer;  TBranch *b_layer;
    std::vector<Char_t>  *sector; TBranch *b_sector;
    std::vector<Float_t> *energy; TBranch *b_energy;
    std::vector<Float_t> *time;   TBranch *b_time;
    REC_Calorimeter();
    REC_Calorimeter(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Scintillator {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Short_t> *pindex;   TBranch *b_pindex;
    std::vector<Float_t> *time;     TBranch *b_time;
    std::vector<Byte_t>  *detector; TBranch *b_detector;
    std::vector<Byte_t>  *layer;    TBranch *b_layer;
    REC_Scintillator();
    REC_Scintillator(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Cherenkov {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Short_t> *pindex;   TBranch *b_pindex;
    std::vector<Byte_t>  *detector; TBranch *b_detector;
    std::vector<Float_t> *nphe;     TBranch *b_nphe;
    REC_Cherenkov();
    REC_Cherenkov(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class FMT_Tracks {
private:
    int nrows;
    int set_nrows(int in_nrows);
public:
    std::vector<Short_t> *index; TBranch *b_index; // index of the track in the DC bank.
    std::vector<Int_t>   *ndf;   TBranch *b_ndf;   // number of degrees of freedom of the fit.
    std::vector<Float_t> *vx;    TBranch *b_vx;    // Vertex x-position to the DOCA to the beam.
    std::vector<Float_t> *vy;    TBranch *b_vy;    // Vertex y-position of the DOCA to the beam.
    std::vector<Float_t> *vz;    TBranch *b_vz;    // Vertex z-position of the DOCA to the beam.
    std::vector<Float_t> *px;    TBranch *b_px;    // 3-momentum x-coordinate to the DOCA.
    std::vector<Float_t> *py;    TBranch *b_py;    // 3-momentum y-coordinate of the DOCA.
    std::vector<Float_t> *pz;    TBranch *b_pz;    // 3-momentum z-coordinate of the DOCA.
    FMT_Tracks();
    FMT_Tracks(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

#endif

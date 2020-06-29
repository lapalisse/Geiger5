//
//  Radioactivity.h
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#ifndef _RADIOACTIVITY_H_

#define _RADIOACTIVITY_H_

// mSv/year
const float EFFECTIVE_DOSE      = 1; 
const float SEARCH_COVER_DOSE   = 10;
const float NUCLEAR_WORKER_DOSE = 20;
const float GO_AWAY_DOSE        = 50; 
const float HIGH_DOSE           = 100;
const float DEADLY_DOSE         = 1000;

const int N_DOSES = 6;

const float doses[N_DOSES] = { 
  EFFECTIVE_DOSE, SEARCH_COVER_DOSE, NUCLEAR_WORKER_DOSE, GO_AWAY_DOSE, HIGH_DOSE, DEADLY_DOSE
};

// 10-character explanations of doses
const char* dose_texts10[N_DOSES] = {
  ">Effective",
  ">SrchCover",
  ">NucWorker",
  "> Go away ",
  ">  High   ",
  "> Deadly  "
};

// 16-character explanations of doses
const char* dose_texts16[N_DOSES] = {
  "> EFFECTIVE DOSE",
  "> SEARCH COVER  ",
  "> NUCLEAR WORKER",
  "> ** GO  AWAY **",
  "> ***  HIGH  ***",
  "> *** DEADLY *** "
};

#endif

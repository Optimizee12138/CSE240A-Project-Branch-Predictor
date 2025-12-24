//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "YANG CHENGGUO";
const char *studentID = "A69041855";
const char *email = "chy092@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 15; // Number of bits used for Global History
int lhistoryBits = 15; // Number of bits used for Local History
int pathHistoryBits = 15; // Number of bits used for Path History
int pcIndexBits = 10;  // Number of bits used for PC index
int chooserBits = 15;  // Number of bits used for chooser table
int bpType;            // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;
// tournament
uint8_t *bht_tournament_local;
uint32_t *lht_tournament_local;
uint8_t *bht_chooser_tournament;
uint8_t *bht_tournament_global;
uint64_t pathHistory;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

// gshare functions
void init_gshare()
{
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++)
  {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
  case WN:
    bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare()
{
  free(bht_gshare);
}

void init_tournament(){
  int bht_entries_local = 1 << lhistoryBits;
  int bht_entries_global = 1 << pathHistoryBits;
  int lht_entries = 1 << pcIndexBits;
  int chooser_entries = 1 << chooserBits;
  bht_tournament_local = (uint8_t *)calloc(bht_entries_local, sizeof(uint8_t));
  lht_tournament_local = (uint32_t *)calloc(lht_entries, sizeof(uint32_t));
  bht_chooser_tournament = (uint8_t *)calloc(chooser_entries, sizeof(uint8_t));
  bht_tournament_global = (uint8_t *)calloc(bht_entries_global, sizeof(uint8_t));
  for (int i = 0; i < bht_entries_local; i++)
  {
    bht_tournament_local[i] = WWN;
  }
  for (int i = 0; i < lht_entries; i++)
  {
    lht_tournament_local[i] = 0;
  }
  for (int i = 0; i < bht_entries_global; i++)
  {
    bht_chooser_tournament[i] = WN;
    bht_tournament_global[i] = WN;
  }
  pathHistory = 0;

}

uint8_t tournament_predict(uint32_t pc){
  int bht_entries_local = 1 << lhistoryBits;
  int bht_entries_global = 1 << pathHistoryBits;
  int lht_entries = 1 << pcIndexBits;
  int chooser_entries = 1 << chooserBits;
  uint32_t pc_lower_bits = pc & (lht_entries - 1);
  uint32_t path_history_lower_bits = pathHistory & (bht_entries_global - 1);
  uint32_t chooser_index = pc & (chooser_entries - 1);  // use lower bits of pc to index chooser table
  uint8_t chooser_prediction = bht_chooser_tournament[chooser_index];
}

void train_tournament(uint32_t pc, uint8_t outcome){
  int bht_entries_local = 1 << lhistoryBits;
  int bht_entries_global = 1 << pathHistoryBits;
  int lht_entries = 1 << pcIndexBits;
  int chooser_entries = 1 << chooserBits;
  uint32_t pc_lower_bits = pc & (lht_entries - 1);
  uint32_t path_history_lower_bits = pathHistory & (bht_entries_global - 1);
  uint32_t chooser_index = pc & (chooser_entries - 1); // use lower bits of pc to index chooser table
  uint8_t chooser_prediction = bht_chooser_tournament[chooser_index];

  uint32_t lht_entry = lht_tournament_local[pc_lower_bits] & (bht_entries_local - 1);
  uint8_t local_prediction = bht_tournament_local[lht_entry];
  uint8_t global_prediction = bht_tournament_global[path_history_lower_bits];

  // global use 2-bit counter, local use 3-bit counter.
  uint8_t local_prediction_dir = (local_prediction == SST || local_prediction == SWT || local_prediction == WST || local_prediction == WWT) ? TAKEN : NOTTAKEN;
  uint8_t global_prediction_dir = (global_prediction == ST || global_prediction == WT) ? TAKEN : NOTTAKEN;
}


void init_predictor()
{
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    break;
  case CUSTOM:
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct)
{

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return NOTTAKEN;
  case CUSTOM:
    return NOTTAKEN;
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct)
{
  if (condition)
  {
    switch (bpType)
    {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return;
    case CUSTOM:
      return;
    default:
      break;
    }
  }
}

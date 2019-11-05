/*-------------------------------------------------------------------*/
/* CBEVI.C        Mohammad Rezaei                                    */
/*                                                                   */
/* External Validity indexes                                         */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "CBEVI"
#define VersionNumber   "Version 0.05"  /* SS */
#define LastUpdated     "17.8.2017"
#define FACTFILE        "cbevi.fac"

/* ------------------------------------------------------------------- */

#include "parametr.c"
#include "cb.h"
#include "file.h"
#include "interfc.h"
#include "memctrl.h"
#include "random.h"
#include "evi.h"


/* ======================== PRINT ROUTINES =========================== */

void PrintInfo(void)
{
  PrintMessage("%s\t%s\t%s\n\n"
        "Console interface for External Validity Indexes.\n"
        "Usage: %s [%coption] <training-set> <paritioining1 or codebook1> <paritioining2 or codebbok2>\n"
        "For example: %s tmp1.pa tmp2.pa\n"
        "             %s bridge.ts tmp1.cb tmp2.cb \n  Options\n",
        ProgName, VersionNumber, LastUpdated, ProgName, OPTION_SYMBOL,
        ProgName, ProgName);
  PrintOptions();
  PrintMessage("\n");
}  /* PrintInfo() */

/* ------------------------------------------------------------------ */

static char* PrintOperatingInfo(int quietLevel, char *InName1, char *InName2, char *InName3, int indexType, int useInitial)
{
  char* str;
  str = EVIInfo();

  if (quietLevel >= 2) {
    PrintMessage("\n%s\n\n", str);
    PrintMessage("Input file 1\t\t = %s\n", InName1);
    PrintMessage("Input file 2\t\t = %s\n", InName2);
    if ( *InName3 )
      PrintMessage("Input file 3\t\t = %s\n", InName3);

    PrintMessage("External validity Index  = ");
    PrintMessage("%s\n", Name(indexType));
  }

  return str;
}  /* PrintInitialData() */

int getSelectedIndex(int pairCounting, int informationTheoretic, int setMatching)
{
  int selectedIndexType = INDEX_CI;

  if ( pairCounting > 0 ) {
    switch (pairCounting) {
      case 1:
        selectedIndexType = INDEX_RI;  // RI
        break;
      case 2:
        selectedIndexType = INDEX_ARI;  // ARI
        break;
      case 3:
        selectedIndexType = INDEX_JACCARD;  // JACCARD
        break;
      case 4:
        selectedIndexType = INDEX_FM;  // Fowlkes and Mallows
        break;
      default:
        selectedIndexType = INDEX_ARI;
        break;
    }

  } else if ( informationTheoretic > 0 ) {
    switch (informationTheoretic) {
      case 1:
        selectedIndexType = INDEX_NMI; // Normalized Mutual Information
        break;
      case 2:
        selectedIndexType = INDEX_NVI; // Normalized Variation of Information
        break;
      case 3:
        selectedIndexType = INDEX_MI; // Normalized Variation of Information
        break;
      case 4:
        selectedIndexType = INDEX_VI; // Normalized Variation of Information
        break;
      default:
        selectedIndexType = INDEX_NMI;
        break;
    }
  } else {
      switch (setMatching) {
        case 1:
          selectedIndexType = INDEX_NVD; // Normalized VanDongen
          break;
        case 2:
          selectedIndexType = INDEX_CH; // critorion H
          break;
        case 3:
          selectedIndexType = INDEX_CI; // CI
          break;
        case 4:
          selectedIndexType = INDEX_CIstar; // CI*
          break;
        case 5:
          selectedIndexType = INDEX_CIpart; // CI based on partitions
          break;
        default:
          selectedIndexType = INDEX_CI;
          break;
      }
  }

  return selectedIndexType;

}

/* ===========================  MAIN  ================================ */

int main(int argc, char* argv[])
{
  PARTITIONING  P1;
  PARTITIONING  P2;
  CODEBOOK      CB1;
  CODEBOOK      CB2;
  TRAININGSET   TS;

  char          InName1[MAXFILENAME] = {'\0'};
  char          InName2[MAXFILENAME] = {'\0'};
  char          InName3[MAXFILENAME] = {'\0'};
  int           useInitial = 0;
  char*         genMethod;
  int           selectedIndexType = 1;
  int           pc, it, st;
  ParameterInfo paraminfo[3] = { { InName1,  "", 0, INFILE },
                                 { InName2,  "", 0, INFILE },
                                 { InName3,  "", 1, INFILE } };

  ParseParameters(argc, argv, 3, paraminfo);
  pc = Value(PairCounting);
  it  = Value(InformationTheoretic);
  st  = Value(SetMatching);

  selectedIndexType = getSelectedIndex(pc, it, st);

  CheckParameters(InName1, InName2, InName3, selectedIndexType);
  if ( (selectedIndexType != INDEX_CI) && (selectedIndexType != INDEX_CIstar) ) // other indexes rather than CI and CI*
    getPartitionsFromFiles(InName1, InName2, InName3, &TS, &P1, &P2);
  else // CI
    getCentroidsFromFiles(InName1, InName2, InName3, &TS, &CB1, &CB2);

  genMethod = PrintOperatingInfo(Value(QuietLevel), InName1, InName2, InName3, selectedIndexType, useInitial);

  PerformEVI(&TS, &P1, &P2,  &CB1, &CB2, selectedIndexType, Value(QuietLevel), useInitial);

/*
  FreePartitioning(&P1);
  FreePartitioning(&P2);
  free(genMethod);
  */
  return EVERYTHING_OK;
}  /* main() */

/* Three personal-best scores, one per difficulty. */

#include "game.h"
#include <stdio.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#define RR_SCORE_FILE "/persist/rift_runner_bests.txt"
#else
#define RR_SCORE_FILE "rift_runner_bests.txt"
#endif

void SaveBestScores(void)
{
    FILE *f = fopen(RR_SCORE_FILE, "w");
    if (!f) return;
    fprintf(f, "%d %d %d\n", g.bestScores[0], g.bestScores[1], g.bestScores[2]);
    fclose(f);
#if defined(__EMSCRIPTEN__)
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(error) { if (error) console.warn('RIFT RUNNER: score save failed', error); });
    }, 0);
#endif
}

void LoadBestScores(void)
{
    FILE *f = fopen(RR_SCORE_FILE, "r");
    if (!f) return;
    if (fscanf(f, "%d %d %d", &g.bestScores[0], &g.bestScores[1], &g.bestScores[2]) != 3)
        for (int i = 0; i < 3; ++i) g.bestScores[i] = -1;
    for (int i = 0; i < 3; ++i) if (g.bestScores[i] < -1) g.bestScores[i] = -1;
    fclose(f);
}

void RecordPersonalBest(void)
{
    int mode = (int)g.difficulty;
    g.newPersonalBest = g.bestScores[mode] < 0 || g.score > g.bestScores[mode];
    if (g.newPersonalBest) {
        g.bestScores[mode] = g.score;
        SaveBestScores();
    }
}

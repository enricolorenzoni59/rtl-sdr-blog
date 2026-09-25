/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtl-sdr.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)

int main(void)
{
 const int nominal[] = {0,9,14,27,37,77,87,125,144,157,166,197,207,229,254,
  280,297,328,338,364,372,386,402,421,434,439,445,480,496};
 rtlsdr_gain_calibration_t entries[30], before[30], selected;
 int profile, i, target, expected, best;
 int64_t delta, best_delta;
 memset(entries, 0x5a, sizeof(entries));
 memcpy(before, entries, sizeof(entries));
 CHECK(rtlsdr_get_gain_calibration(0, entries, 30) == -1);
 CHECK(rtlsdr_get_gain_calibration(1, NULL, 1) == -1);
 CHECK(rtlsdr_get_gain_calibration(1, entries, 28) == -2);
 CHECK(memcmp(entries, before, sizeof(entries)) == 0);
 CHECK(rtlsdr_select_calibrated_gain(0, 0, entries) == -1);
 CHECK(memcmp(entries, before, sizeof(entries)) == 0);
 CHECK(rtlsdr_select_calibrated_gain(1, 0, NULL) == -1);
 for (profile = 1; profile <= 2; ++profile) {
  CHECK(rtlsdr_get_gain_calibration(profile, NULL, 0) == 29);
  CHECK(rtlsdr_get_gain_calibration(profile, entries, 30) == 29);
  CHECK(memcmp(&entries[29], &before[29], sizeof(entries[29])) == 0);
  for (i = 0; i < 29; ++i) CHECK(entries[i].nominal_gain_tenth_db == nominal[i]);
  CHECK(entries[0].relative_gain_hundredth_db == 0);
  CHECK(entries[28].relative_gain_hundredth_db == (profile == 1 ? 4745 : 4994));
  /* Independently enumerate all candidates for every hundredth-dB request. */
  for (target = -100; target <= 5100; ++target) {
   best = -1;
   best_delta = INT64_MAX;
   for (i = 28; i >= 0; --i) {
    delta = llabs((long long)target - entries[i].relative_gain_hundredth_db);
    if (delta <= best_delta) { best = i; best_delta = delta; }
   }
   CHECK(rtlsdr_select_calibrated_gain(profile, target, &selected) == 0);
   CHECK(selected.nominal_gain_tenth_db == nominal[best]);
   CHECK(selected.relative_gain_hundredth_db == entries[best].relative_gain_hundredth_db);
  }
  CHECK(rtlsdr_select_calibrated_gain(profile, INT_MIN, &selected) == 0);
  CHECK(selected.nominal_gain_tenth_db == 0);
  CHECK(rtlsdr_select_calibrated_gain(profile, INT_MAX, &selected) == 0);
  expected = profile == 1 ? 480 : 496;
  CHECK(selected.nominal_gain_tenth_db == expected);
  target = entries[26].relative_gain_hundredth_db;
  CHECK(rtlsdr_select_calibrated_gain(profile, target, &selected) == 0);
  CHECK(selected.nominal_gain_tenth_db == 439);
 }
 puts("gain calibration: all checks passed");
 return 0;
}

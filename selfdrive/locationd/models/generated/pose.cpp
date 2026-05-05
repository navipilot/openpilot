#include "pose.h"

namespace {
#define DIM 18
#define EDIM 18
#define MEDIM 18
typedef void (*Hfun)(double *, double *, double *);
const static double MAHA_THRESH_4 = 7.814727903251177;
const static double MAHA_THRESH_10 = 7.814727903251177;
const static double MAHA_THRESH_13 = 7.814727903251177;
const static double MAHA_THRESH_14 = 7.814727903251177;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_4940663026929446750) {
   out_4940663026929446750[0] = delta_x[0] + nom_x[0];
   out_4940663026929446750[1] = delta_x[1] + nom_x[1];
   out_4940663026929446750[2] = delta_x[2] + nom_x[2];
   out_4940663026929446750[3] = delta_x[3] + nom_x[3];
   out_4940663026929446750[4] = delta_x[4] + nom_x[4];
   out_4940663026929446750[5] = delta_x[5] + nom_x[5];
   out_4940663026929446750[6] = delta_x[6] + nom_x[6];
   out_4940663026929446750[7] = delta_x[7] + nom_x[7];
   out_4940663026929446750[8] = delta_x[8] + nom_x[8];
   out_4940663026929446750[9] = delta_x[9] + nom_x[9];
   out_4940663026929446750[10] = delta_x[10] + nom_x[10];
   out_4940663026929446750[11] = delta_x[11] + nom_x[11];
   out_4940663026929446750[12] = delta_x[12] + nom_x[12];
   out_4940663026929446750[13] = delta_x[13] + nom_x[13];
   out_4940663026929446750[14] = delta_x[14] + nom_x[14];
   out_4940663026929446750[15] = delta_x[15] + nom_x[15];
   out_4940663026929446750[16] = delta_x[16] + nom_x[16];
   out_4940663026929446750[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1207218016783825442) {
   out_1207218016783825442[0] = -nom_x[0] + true_x[0];
   out_1207218016783825442[1] = -nom_x[1] + true_x[1];
   out_1207218016783825442[2] = -nom_x[2] + true_x[2];
   out_1207218016783825442[3] = -nom_x[3] + true_x[3];
   out_1207218016783825442[4] = -nom_x[4] + true_x[4];
   out_1207218016783825442[5] = -nom_x[5] + true_x[5];
   out_1207218016783825442[6] = -nom_x[6] + true_x[6];
   out_1207218016783825442[7] = -nom_x[7] + true_x[7];
   out_1207218016783825442[8] = -nom_x[8] + true_x[8];
   out_1207218016783825442[9] = -nom_x[9] + true_x[9];
   out_1207218016783825442[10] = -nom_x[10] + true_x[10];
   out_1207218016783825442[11] = -nom_x[11] + true_x[11];
   out_1207218016783825442[12] = -nom_x[12] + true_x[12];
   out_1207218016783825442[13] = -nom_x[13] + true_x[13];
   out_1207218016783825442[14] = -nom_x[14] + true_x[14];
   out_1207218016783825442[15] = -nom_x[15] + true_x[15];
   out_1207218016783825442[16] = -nom_x[16] + true_x[16];
   out_1207218016783825442[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_2820660951987973699) {
   out_2820660951987973699[0] = 1.0;
   out_2820660951987973699[1] = 0.0;
   out_2820660951987973699[2] = 0.0;
   out_2820660951987973699[3] = 0.0;
   out_2820660951987973699[4] = 0.0;
   out_2820660951987973699[5] = 0.0;
   out_2820660951987973699[6] = 0.0;
   out_2820660951987973699[7] = 0.0;
   out_2820660951987973699[8] = 0.0;
   out_2820660951987973699[9] = 0.0;
   out_2820660951987973699[10] = 0.0;
   out_2820660951987973699[11] = 0.0;
   out_2820660951987973699[12] = 0.0;
   out_2820660951987973699[13] = 0.0;
   out_2820660951987973699[14] = 0.0;
   out_2820660951987973699[15] = 0.0;
   out_2820660951987973699[16] = 0.0;
   out_2820660951987973699[17] = 0.0;
   out_2820660951987973699[18] = 0.0;
   out_2820660951987973699[19] = 1.0;
   out_2820660951987973699[20] = 0.0;
   out_2820660951987973699[21] = 0.0;
   out_2820660951987973699[22] = 0.0;
   out_2820660951987973699[23] = 0.0;
   out_2820660951987973699[24] = 0.0;
   out_2820660951987973699[25] = 0.0;
   out_2820660951987973699[26] = 0.0;
   out_2820660951987973699[27] = 0.0;
   out_2820660951987973699[28] = 0.0;
   out_2820660951987973699[29] = 0.0;
   out_2820660951987973699[30] = 0.0;
   out_2820660951987973699[31] = 0.0;
   out_2820660951987973699[32] = 0.0;
   out_2820660951987973699[33] = 0.0;
   out_2820660951987973699[34] = 0.0;
   out_2820660951987973699[35] = 0.0;
   out_2820660951987973699[36] = 0.0;
   out_2820660951987973699[37] = 0.0;
   out_2820660951987973699[38] = 1.0;
   out_2820660951987973699[39] = 0.0;
   out_2820660951987973699[40] = 0.0;
   out_2820660951987973699[41] = 0.0;
   out_2820660951987973699[42] = 0.0;
   out_2820660951987973699[43] = 0.0;
   out_2820660951987973699[44] = 0.0;
   out_2820660951987973699[45] = 0.0;
   out_2820660951987973699[46] = 0.0;
   out_2820660951987973699[47] = 0.0;
   out_2820660951987973699[48] = 0.0;
   out_2820660951987973699[49] = 0.0;
   out_2820660951987973699[50] = 0.0;
   out_2820660951987973699[51] = 0.0;
   out_2820660951987973699[52] = 0.0;
   out_2820660951987973699[53] = 0.0;
   out_2820660951987973699[54] = 0.0;
   out_2820660951987973699[55] = 0.0;
   out_2820660951987973699[56] = 0.0;
   out_2820660951987973699[57] = 1.0;
   out_2820660951987973699[58] = 0.0;
   out_2820660951987973699[59] = 0.0;
   out_2820660951987973699[60] = 0.0;
   out_2820660951987973699[61] = 0.0;
   out_2820660951987973699[62] = 0.0;
   out_2820660951987973699[63] = 0.0;
   out_2820660951987973699[64] = 0.0;
   out_2820660951987973699[65] = 0.0;
   out_2820660951987973699[66] = 0.0;
   out_2820660951987973699[67] = 0.0;
   out_2820660951987973699[68] = 0.0;
   out_2820660951987973699[69] = 0.0;
   out_2820660951987973699[70] = 0.0;
   out_2820660951987973699[71] = 0.0;
   out_2820660951987973699[72] = 0.0;
   out_2820660951987973699[73] = 0.0;
   out_2820660951987973699[74] = 0.0;
   out_2820660951987973699[75] = 0.0;
   out_2820660951987973699[76] = 1.0;
   out_2820660951987973699[77] = 0.0;
   out_2820660951987973699[78] = 0.0;
   out_2820660951987973699[79] = 0.0;
   out_2820660951987973699[80] = 0.0;
   out_2820660951987973699[81] = 0.0;
   out_2820660951987973699[82] = 0.0;
   out_2820660951987973699[83] = 0.0;
   out_2820660951987973699[84] = 0.0;
   out_2820660951987973699[85] = 0.0;
   out_2820660951987973699[86] = 0.0;
   out_2820660951987973699[87] = 0.0;
   out_2820660951987973699[88] = 0.0;
   out_2820660951987973699[89] = 0.0;
   out_2820660951987973699[90] = 0.0;
   out_2820660951987973699[91] = 0.0;
   out_2820660951987973699[92] = 0.0;
   out_2820660951987973699[93] = 0.0;
   out_2820660951987973699[94] = 0.0;
   out_2820660951987973699[95] = 1.0;
   out_2820660951987973699[96] = 0.0;
   out_2820660951987973699[97] = 0.0;
   out_2820660951987973699[98] = 0.0;
   out_2820660951987973699[99] = 0.0;
   out_2820660951987973699[100] = 0.0;
   out_2820660951987973699[101] = 0.0;
   out_2820660951987973699[102] = 0.0;
   out_2820660951987973699[103] = 0.0;
   out_2820660951987973699[104] = 0.0;
   out_2820660951987973699[105] = 0.0;
   out_2820660951987973699[106] = 0.0;
   out_2820660951987973699[107] = 0.0;
   out_2820660951987973699[108] = 0.0;
   out_2820660951987973699[109] = 0.0;
   out_2820660951987973699[110] = 0.0;
   out_2820660951987973699[111] = 0.0;
   out_2820660951987973699[112] = 0.0;
   out_2820660951987973699[113] = 0.0;
   out_2820660951987973699[114] = 1.0;
   out_2820660951987973699[115] = 0.0;
   out_2820660951987973699[116] = 0.0;
   out_2820660951987973699[117] = 0.0;
   out_2820660951987973699[118] = 0.0;
   out_2820660951987973699[119] = 0.0;
   out_2820660951987973699[120] = 0.0;
   out_2820660951987973699[121] = 0.0;
   out_2820660951987973699[122] = 0.0;
   out_2820660951987973699[123] = 0.0;
   out_2820660951987973699[124] = 0.0;
   out_2820660951987973699[125] = 0.0;
   out_2820660951987973699[126] = 0.0;
   out_2820660951987973699[127] = 0.0;
   out_2820660951987973699[128] = 0.0;
   out_2820660951987973699[129] = 0.0;
   out_2820660951987973699[130] = 0.0;
   out_2820660951987973699[131] = 0.0;
   out_2820660951987973699[132] = 0.0;
   out_2820660951987973699[133] = 1.0;
   out_2820660951987973699[134] = 0.0;
   out_2820660951987973699[135] = 0.0;
   out_2820660951987973699[136] = 0.0;
   out_2820660951987973699[137] = 0.0;
   out_2820660951987973699[138] = 0.0;
   out_2820660951987973699[139] = 0.0;
   out_2820660951987973699[140] = 0.0;
   out_2820660951987973699[141] = 0.0;
   out_2820660951987973699[142] = 0.0;
   out_2820660951987973699[143] = 0.0;
   out_2820660951987973699[144] = 0.0;
   out_2820660951987973699[145] = 0.0;
   out_2820660951987973699[146] = 0.0;
   out_2820660951987973699[147] = 0.0;
   out_2820660951987973699[148] = 0.0;
   out_2820660951987973699[149] = 0.0;
   out_2820660951987973699[150] = 0.0;
   out_2820660951987973699[151] = 0.0;
   out_2820660951987973699[152] = 1.0;
   out_2820660951987973699[153] = 0.0;
   out_2820660951987973699[154] = 0.0;
   out_2820660951987973699[155] = 0.0;
   out_2820660951987973699[156] = 0.0;
   out_2820660951987973699[157] = 0.0;
   out_2820660951987973699[158] = 0.0;
   out_2820660951987973699[159] = 0.0;
   out_2820660951987973699[160] = 0.0;
   out_2820660951987973699[161] = 0.0;
   out_2820660951987973699[162] = 0.0;
   out_2820660951987973699[163] = 0.0;
   out_2820660951987973699[164] = 0.0;
   out_2820660951987973699[165] = 0.0;
   out_2820660951987973699[166] = 0.0;
   out_2820660951987973699[167] = 0.0;
   out_2820660951987973699[168] = 0.0;
   out_2820660951987973699[169] = 0.0;
   out_2820660951987973699[170] = 0.0;
   out_2820660951987973699[171] = 1.0;
   out_2820660951987973699[172] = 0.0;
   out_2820660951987973699[173] = 0.0;
   out_2820660951987973699[174] = 0.0;
   out_2820660951987973699[175] = 0.0;
   out_2820660951987973699[176] = 0.0;
   out_2820660951987973699[177] = 0.0;
   out_2820660951987973699[178] = 0.0;
   out_2820660951987973699[179] = 0.0;
   out_2820660951987973699[180] = 0.0;
   out_2820660951987973699[181] = 0.0;
   out_2820660951987973699[182] = 0.0;
   out_2820660951987973699[183] = 0.0;
   out_2820660951987973699[184] = 0.0;
   out_2820660951987973699[185] = 0.0;
   out_2820660951987973699[186] = 0.0;
   out_2820660951987973699[187] = 0.0;
   out_2820660951987973699[188] = 0.0;
   out_2820660951987973699[189] = 0.0;
   out_2820660951987973699[190] = 1.0;
   out_2820660951987973699[191] = 0.0;
   out_2820660951987973699[192] = 0.0;
   out_2820660951987973699[193] = 0.0;
   out_2820660951987973699[194] = 0.0;
   out_2820660951987973699[195] = 0.0;
   out_2820660951987973699[196] = 0.0;
   out_2820660951987973699[197] = 0.0;
   out_2820660951987973699[198] = 0.0;
   out_2820660951987973699[199] = 0.0;
   out_2820660951987973699[200] = 0.0;
   out_2820660951987973699[201] = 0.0;
   out_2820660951987973699[202] = 0.0;
   out_2820660951987973699[203] = 0.0;
   out_2820660951987973699[204] = 0.0;
   out_2820660951987973699[205] = 0.0;
   out_2820660951987973699[206] = 0.0;
   out_2820660951987973699[207] = 0.0;
   out_2820660951987973699[208] = 0.0;
   out_2820660951987973699[209] = 1.0;
   out_2820660951987973699[210] = 0.0;
   out_2820660951987973699[211] = 0.0;
   out_2820660951987973699[212] = 0.0;
   out_2820660951987973699[213] = 0.0;
   out_2820660951987973699[214] = 0.0;
   out_2820660951987973699[215] = 0.0;
   out_2820660951987973699[216] = 0.0;
   out_2820660951987973699[217] = 0.0;
   out_2820660951987973699[218] = 0.0;
   out_2820660951987973699[219] = 0.0;
   out_2820660951987973699[220] = 0.0;
   out_2820660951987973699[221] = 0.0;
   out_2820660951987973699[222] = 0.0;
   out_2820660951987973699[223] = 0.0;
   out_2820660951987973699[224] = 0.0;
   out_2820660951987973699[225] = 0.0;
   out_2820660951987973699[226] = 0.0;
   out_2820660951987973699[227] = 0.0;
   out_2820660951987973699[228] = 1.0;
   out_2820660951987973699[229] = 0.0;
   out_2820660951987973699[230] = 0.0;
   out_2820660951987973699[231] = 0.0;
   out_2820660951987973699[232] = 0.0;
   out_2820660951987973699[233] = 0.0;
   out_2820660951987973699[234] = 0.0;
   out_2820660951987973699[235] = 0.0;
   out_2820660951987973699[236] = 0.0;
   out_2820660951987973699[237] = 0.0;
   out_2820660951987973699[238] = 0.0;
   out_2820660951987973699[239] = 0.0;
   out_2820660951987973699[240] = 0.0;
   out_2820660951987973699[241] = 0.0;
   out_2820660951987973699[242] = 0.0;
   out_2820660951987973699[243] = 0.0;
   out_2820660951987973699[244] = 0.0;
   out_2820660951987973699[245] = 0.0;
   out_2820660951987973699[246] = 0.0;
   out_2820660951987973699[247] = 1.0;
   out_2820660951987973699[248] = 0.0;
   out_2820660951987973699[249] = 0.0;
   out_2820660951987973699[250] = 0.0;
   out_2820660951987973699[251] = 0.0;
   out_2820660951987973699[252] = 0.0;
   out_2820660951987973699[253] = 0.0;
   out_2820660951987973699[254] = 0.0;
   out_2820660951987973699[255] = 0.0;
   out_2820660951987973699[256] = 0.0;
   out_2820660951987973699[257] = 0.0;
   out_2820660951987973699[258] = 0.0;
   out_2820660951987973699[259] = 0.0;
   out_2820660951987973699[260] = 0.0;
   out_2820660951987973699[261] = 0.0;
   out_2820660951987973699[262] = 0.0;
   out_2820660951987973699[263] = 0.0;
   out_2820660951987973699[264] = 0.0;
   out_2820660951987973699[265] = 0.0;
   out_2820660951987973699[266] = 1.0;
   out_2820660951987973699[267] = 0.0;
   out_2820660951987973699[268] = 0.0;
   out_2820660951987973699[269] = 0.0;
   out_2820660951987973699[270] = 0.0;
   out_2820660951987973699[271] = 0.0;
   out_2820660951987973699[272] = 0.0;
   out_2820660951987973699[273] = 0.0;
   out_2820660951987973699[274] = 0.0;
   out_2820660951987973699[275] = 0.0;
   out_2820660951987973699[276] = 0.0;
   out_2820660951987973699[277] = 0.0;
   out_2820660951987973699[278] = 0.0;
   out_2820660951987973699[279] = 0.0;
   out_2820660951987973699[280] = 0.0;
   out_2820660951987973699[281] = 0.0;
   out_2820660951987973699[282] = 0.0;
   out_2820660951987973699[283] = 0.0;
   out_2820660951987973699[284] = 0.0;
   out_2820660951987973699[285] = 1.0;
   out_2820660951987973699[286] = 0.0;
   out_2820660951987973699[287] = 0.0;
   out_2820660951987973699[288] = 0.0;
   out_2820660951987973699[289] = 0.0;
   out_2820660951987973699[290] = 0.0;
   out_2820660951987973699[291] = 0.0;
   out_2820660951987973699[292] = 0.0;
   out_2820660951987973699[293] = 0.0;
   out_2820660951987973699[294] = 0.0;
   out_2820660951987973699[295] = 0.0;
   out_2820660951987973699[296] = 0.0;
   out_2820660951987973699[297] = 0.0;
   out_2820660951987973699[298] = 0.0;
   out_2820660951987973699[299] = 0.0;
   out_2820660951987973699[300] = 0.0;
   out_2820660951987973699[301] = 0.0;
   out_2820660951987973699[302] = 0.0;
   out_2820660951987973699[303] = 0.0;
   out_2820660951987973699[304] = 1.0;
   out_2820660951987973699[305] = 0.0;
   out_2820660951987973699[306] = 0.0;
   out_2820660951987973699[307] = 0.0;
   out_2820660951987973699[308] = 0.0;
   out_2820660951987973699[309] = 0.0;
   out_2820660951987973699[310] = 0.0;
   out_2820660951987973699[311] = 0.0;
   out_2820660951987973699[312] = 0.0;
   out_2820660951987973699[313] = 0.0;
   out_2820660951987973699[314] = 0.0;
   out_2820660951987973699[315] = 0.0;
   out_2820660951987973699[316] = 0.0;
   out_2820660951987973699[317] = 0.0;
   out_2820660951987973699[318] = 0.0;
   out_2820660951987973699[319] = 0.0;
   out_2820660951987973699[320] = 0.0;
   out_2820660951987973699[321] = 0.0;
   out_2820660951987973699[322] = 0.0;
   out_2820660951987973699[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_2310584025056258416) {
   out_2310584025056258416[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_2310584025056258416[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_2310584025056258416[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_2310584025056258416[3] = dt*state[12] + state[3];
   out_2310584025056258416[4] = dt*state[13] + state[4];
   out_2310584025056258416[5] = dt*state[14] + state[5];
   out_2310584025056258416[6] = state[6];
   out_2310584025056258416[7] = state[7];
   out_2310584025056258416[8] = state[8];
   out_2310584025056258416[9] = state[9];
   out_2310584025056258416[10] = state[10];
   out_2310584025056258416[11] = state[11];
   out_2310584025056258416[12] = state[12];
   out_2310584025056258416[13] = state[13];
   out_2310584025056258416[14] = state[14];
   out_2310584025056258416[15] = state[15];
   out_2310584025056258416[16] = state[16];
   out_2310584025056258416[17] = state[17];
}
void F_fun(double *state, double dt, double *out_6557190266781458971) {
   out_6557190266781458971[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6557190266781458971[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6557190266781458971[2] = 0;
   out_6557190266781458971[3] = 0;
   out_6557190266781458971[4] = 0;
   out_6557190266781458971[5] = 0;
   out_6557190266781458971[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6557190266781458971[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6557190266781458971[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6557190266781458971[9] = 0;
   out_6557190266781458971[10] = 0;
   out_6557190266781458971[11] = 0;
   out_6557190266781458971[12] = 0;
   out_6557190266781458971[13] = 0;
   out_6557190266781458971[14] = 0;
   out_6557190266781458971[15] = 0;
   out_6557190266781458971[16] = 0;
   out_6557190266781458971[17] = 0;
   out_6557190266781458971[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6557190266781458971[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6557190266781458971[20] = 0;
   out_6557190266781458971[21] = 0;
   out_6557190266781458971[22] = 0;
   out_6557190266781458971[23] = 0;
   out_6557190266781458971[24] = 0;
   out_6557190266781458971[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6557190266781458971[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6557190266781458971[27] = 0;
   out_6557190266781458971[28] = 0;
   out_6557190266781458971[29] = 0;
   out_6557190266781458971[30] = 0;
   out_6557190266781458971[31] = 0;
   out_6557190266781458971[32] = 0;
   out_6557190266781458971[33] = 0;
   out_6557190266781458971[34] = 0;
   out_6557190266781458971[35] = 0;
   out_6557190266781458971[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6557190266781458971[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6557190266781458971[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6557190266781458971[39] = 0;
   out_6557190266781458971[40] = 0;
   out_6557190266781458971[41] = 0;
   out_6557190266781458971[42] = 0;
   out_6557190266781458971[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6557190266781458971[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6557190266781458971[45] = 0;
   out_6557190266781458971[46] = 0;
   out_6557190266781458971[47] = 0;
   out_6557190266781458971[48] = 0;
   out_6557190266781458971[49] = 0;
   out_6557190266781458971[50] = 0;
   out_6557190266781458971[51] = 0;
   out_6557190266781458971[52] = 0;
   out_6557190266781458971[53] = 0;
   out_6557190266781458971[54] = 0;
   out_6557190266781458971[55] = 0;
   out_6557190266781458971[56] = 0;
   out_6557190266781458971[57] = 1;
   out_6557190266781458971[58] = 0;
   out_6557190266781458971[59] = 0;
   out_6557190266781458971[60] = 0;
   out_6557190266781458971[61] = 0;
   out_6557190266781458971[62] = 0;
   out_6557190266781458971[63] = 0;
   out_6557190266781458971[64] = 0;
   out_6557190266781458971[65] = 0;
   out_6557190266781458971[66] = dt;
   out_6557190266781458971[67] = 0;
   out_6557190266781458971[68] = 0;
   out_6557190266781458971[69] = 0;
   out_6557190266781458971[70] = 0;
   out_6557190266781458971[71] = 0;
   out_6557190266781458971[72] = 0;
   out_6557190266781458971[73] = 0;
   out_6557190266781458971[74] = 0;
   out_6557190266781458971[75] = 0;
   out_6557190266781458971[76] = 1;
   out_6557190266781458971[77] = 0;
   out_6557190266781458971[78] = 0;
   out_6557190266781458971[79] = 0;
   out_6557190266781458971[80] = 0;
   out_6557190266781458971[81] = 0;
   out_6557190266781458971[82] = 0;
   out_6557190266781458971[83] = 0;
   out_6557190266781458971[84] = 0;
   out_6557190266781458971[85] = dt;
   out_6557190266781458971[86] = 0;
   out_6557190266781458971[87] = 0;
   out_6557190266781458971[88] = 0;
   out_6557190266781458971[89] = 0;
   out_6557190266781458971[90] = 0;
   out_6557190266781458971[91] = 0;
   out_6557190266781458971[92] = 0;
   out_6557190266781458971[93] = 0;
   out_6557190266781458971[94] = 0;
   out_6557190266781458971[95] = 1;
   out_6557190266781458971[96] = 0;
   out_6557190266781458971[97] = 0;
   out_6557190266781458971[98] = 0;
   out_6557190266781458971[99] = 0;
   out_6557190266781458971[100] = 0;
   out_6557190266781458971[101] = 0;
   out_6557190266781458971[102] = 0;
   out_6557190266781458971[103] = 0;
   out_6557190266781458971[104] = dt;
   out_6557190266781458971[105] = 0;
   out_6557190266781458971[106] = 0;
   out_6557190266781458971[107] = 0;
   out_6557190266781458971[108] = 0;
   out_6557190266781458971[109] = 0;
   out_6557190266781458971[110] = 0;
   out_6557190266781458971[111] = 0;
   out_6557190266781458971[112] = 0;
   out_6557190266781458971[113] = 0;
   out_6557190266781458971[114] = 1;
   out_6557190266781458971[115] = 0;
   out_6557190266781458971[116] = 0;
   out_6557190266781458971[117] = 0;
   out_6557190266781458971[118] = 0;
   out_6557190266781458971[119] = 0;
   out_6557190266781458971[120] = 0;
   out_6557190266781458971[121] = 0;
   out_6557190266781458971[122] = 0;
   out_6557190266781458971[123] = 0;
   out_6557190266781458971[124] = 0;
   out_6557190266781458971[125] = 0;
   out_6557190266781458971[126] = 0;
   out_6557190266781458971[127] = 0;
   out_6557190266781458971[128] = 0;
   out_6557190266781458971[129] = 0;
   out_6557190266781458971[130] = 0;
   out_6557190266781458971[131] = 0;
   out_6557190266781458971[132] = 0;
   out_6557190266781458971[133] = 1;
   out_6557190266781458971[134] = 0;
   out_6557190266781458971[135] = 0;
   out_6557190266781458971[136] = 0;
   out_6557190266781458971[137] = 0;
   out_6557190266781458971[138] = 0;
   out_6557190266781458971[139] = 0;
   out_6557190266781458971[140] = 0;
   out_6557190266781458971[141] = 0;
   out_6557190266781458971[142] = 0;
   out_6557190266781458971[143] = 0;
   out_6557190266781458971[144] = 0;
   out_6557190266781458971[145] = 0;
   out_6557190266781458971[146] = 0;
   out_6557190266781458971[147] = 0;
   out_6557190266781458971[148] = 0;
   out_6557190266781458971[149] = 0;
   out_6557190266781458971[150] = 0;
   out_6557190266781458971[151] = 0;
   out_6557190266781458971[152] = 1;
   out_6557190266781458971[153] = 0;
   out_6557190266781458971[154] = 0;
   out_6557190266781458971[155] = 0;
   out_6557190266781458971[156] = 0;
   out_6557190266781458971[157] = 0;
   out_6557190266781458971[158] = 0;
   out_6557190266781458971[159] = 0;
   out_6557190266781458971[160] = 0;
   out_6557190266781458971[161] = 0;
   out_6557190266781458971[162] = 0;
   out_6557190266781458971[163] = 0;
   out_6557190266781458971[164] = 0;
   out_6557190266781458971[165] = 0;
   out_6557190266781458971[166] = 0;
   out_6557190266781458971[167] = 0;
   out_6557190266781458971[168] = 0;
   out_6557190266781458971[169] = 0;
   out_6557190266781458971[170] = 0;
   out_6557190266781458971[171] = 1;
   out_6557190266781458971[172] = 0;
   out_6557190266781458971[173] = 0;
   out_6557190266781458971[174] = 0;
   out_6557190266781458971[175] = 0;
   out_6557190266781458971[176] = 0;
   out_6557190266781458971[177] = 0;
   out_6557190266781458971[178] = 0;
   out_6557190266781458971[179] = 0;
   out_6557190266781458971[180] = 0;
   out_6557190266781458971[181] = 0;
   out_6557190266781458971[182] = 0;
   out_6557190266781458971[183] = 0;
   out_6557190266781458971[184] = 0;
   out_6557190266781458971[185] = 0;
   out_6557190266781458971[186] = 0;
   out_6557190266781458971[187] = 0;
   out_6557190266781458971[188] = 0;
   out_6557190266781458971[189] = 0;
   out_6557190266781458971[190] = 1;
   out_6557190266781458971[191] = 0;
   out_6557190266781458971[192] = 0;
   out_6557190266781458971[193] = 0;
   out_6557190266781458971[194] = 0;
   out_6557190266781458971[195] = 0;
   out_6557190266781458971[196] = 0;
   out_6557190266781458971[197] = 0;
   out_6557190266781458971[198] = 0;
   out_6557190266781458971[199] = 0;
   out_6557190266781458971[200] = 0;
   out_6557190266781458971[201] = 0;
   out_6557190266781458971[202] = 0;
   out_6557190266781458971[203] = 0;
   out_6557190266781458971[204] = 0;
   out_6557190266781458971[205] = 0;
   out_6557190266781458971[206] = 0;
   out_6557190266781458971[207] = 0;
   out_6557190266781458971[208] = 0;
   out_6557190266781458971[209] = 1;
   out_6557190266781458971[210] = 0;
   out_6557190266781458971[211] = 0;
   out_6557190266781458971[212] = 0;
   out_6557190266781458971[213] = 0;
   out_6557190266781458971[214] = 0;
   out_6557190266781458971[215] = 0;
   out_6557190266781458971[216] = 0;
   out_6557190266781458971[217] = 0;
   out_6557190266781458971[218] = 0;
   out_6557190266781458971[219] = 0;
   out_6557190266781458971[220] = 0;
   out_6557190266781458971[221] = 0;
   out_6557190266781458971[222] = 0;
   out_6557190266781458971[223] = 0;
   out_6557190266781458971[224] = 0;
   out_6557190266781458971[225] = 0;
   out_6557190266781458971[226] = 0;
   out_6557190266781458971[227] = 0;
   out_6557190266781458971[228] = 1;
   out_6557190266781458971[229] = 0;
   out_6557190266781458971[230] = 0;
   out_6557190266781458971[231] = 0;
   out_6557190266781458971[232] = 0;
   out_6557190266781458971[233] = 0;
   out_6557190266781458971[234] = 0;
   out_6557190266781458971[235] = 0;
   out_6557190266781458971[236] = 0;
   out_6557190266781458971[237] = 0;
   out_6557190266781458971[238] = 0;
   out_6557190266781458971[239] = 0;
   out_6557190266781458971[240] = 0;
   out_6557190266781458971[241] = 0;
   out_6557190266781458971[242] = 0;
   out_6557190266781458971[243] = 0;
   out_6557190266781458971[244] = 0;
   out_6557190266781458971[245] = 0;
   out_6557190266781458971[246] = 0;
   out_6557190266781458971[247] = 1;
   out_6557190266781458971[248] = 0;
   out_6557190266781458971[249] = 0;
   out_6557190266781458971[250] = 0;
   out_6557190266781458971[251] = 0;
   out_6557190266781458971[252] = 0;
   out_6557190266781458971[253] = 0;
   out_6557190266781458971[254] = 0;
   out_6557190266781458971[255] = 0;
   out_6557190266781458971[256] = 0;
   out_6557190266781458971[257] = 0;
   out_6557190266781458971[258] = 0;
   out_6557190266781458971[259] = 0;
   out_6557190266781458971[260] = 0;
   out_6557190266781458971[261] = 0;
   out_6557190266781458971[262] = 0;
   out_6557190266781458971[263] = 0;
   out_6557190266781458971[264] = 0;
   out_6557190266781458971[265] = 0;
   out_6557190266781458971[266] = 1;
   out_6557190266781458971[267] = 0;
   out_6557190266781458971[268] = 0;
   out_6557190266781458971[269] = 0;
   out_6557190266781458971[270] = 0;
   out_6557190266781458971[271] = 0;
   out_6557190266781458971[272] = 0;
   out_6557190266781458971[273] = 0;
   out_6557190266781458971[274] = 0;
   out_6557190266781458971[275] = 0;
   out_6557190266781458971[276] = 0;
   out_6557190266781458971[277] = 0;
   out_6557190266781458971[278] = 0;
   out_6557190266781458971[279] = 0;
   out_6557190266781458971[280] = 0;
   out_6557190266781458971[281] = 0;
   out_6557190266781458971[282] = 0;
   out_6557190266781458971[283] = 0;
   out_6557190266781458971[284] = 0;
   out_6557190266781458971[285] = 1;
   out_6557190266781458971[286] = 0;
   out_6557190266781458971[287] = 0;
   out_6557190266781458971[288] = 0;
   out_6557190266781458971[289] = 0;
   out_6557190266781458971[290] = 0;
   out_6557190266781458971[291] = 0;
   out_6557190266781458971[292] = 0;
   out_6557190266781458971[293] = 0;
   out_6557190266781458971[294] = 0;
   out_6557190266781458971[295] = 0;
   out_6557190266781458971[296] = 0;
   out_6557190266781458971[297] = 0;
   out_6557190266781458971[298] = 0;
   out_6557190266781458971[299] = 0;
   out_6557190266781458971[300] = 0;
   out_6557190266781458971[301] = 0;
   out_6557190266781458971[302] = 0;
   out_6557190266781458971[303] = 0;
   out_6557190266781458971[304] = 1;
   out_6557190266781458971[305] = 0;
   out_6557190266781458971[306] = 0;
   out_6557190266781458971[307] = 0;
   out_6557190266781458971[308] = 0;
   out_6557190266781458971[309] = 0;
   out_6557190266781458971[310] = 0;
   out_6557190266781458971[311] = 0;
   out_6557190266781458971[312] = 0;
   out_6557190266781458971[313] = 0;
   out_6557190266781458971[314] = 0;
   out_6557190266781458971[315] = 0;
   out_6557190266781458971[316] = 0;
   out_6557190266781458971[317] = 0;
   out_6557190266781458971[318] = 0;
   out_6557190266781458971[319] = 0;
   out_6557190266781458971[320] = 0;
   out_6557190266781458971[321] = 0;
   out_6557190266781458971[322] = 0;
   out_6557190266781458971[323] = 1;
}
void h_4(double *state, double *unused, double *out_8177424571477984765) {
   out_8177424571477984765[0] = state[6] + state[9];
   out_8177424571477984765[1] = state[7] + state[10];
   out_8177424571477984765[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_6592468161831881271) {
   out_6592468161831881271[0] = 0;
   out_6592468161831881271[1] = 0;
   out_6592468161831881271[2] = 0;
   out_6592468161831881271[3] = 0;
   out_6592468161831881271[4] = 0;
   out_6592468161831881271[5] = 0;
   out_6592468161831881271[6] = 1;
   out_6592468161831881271[7] = 0;
   out_6592468161831881271[8] = 0;
   out_6592468161831881271[9] = 1;
   out_6592468161831881271[10] = 0;
   out_6592468161831881271[11] = 0;
   out_6592468161831881271[12] = 0;
   out_6592468161831881271[13] = 0;
   out_6592468161831881271[14] = 0;
   out_6592468161831881271[15] = 0;
   out_6592468161831881271[16] = 0;
   out_6592468161831881271[17] = 0;
   out_6592468161831881271[18] = 0;
   out_6592468161831881271[19] = 0;
   out_6592468161831881271[20] = 0;
   out_6592468161831881271[21] = 0;
   out_6592468161831881271[22] = 0;
   out_6592468161831881271[23] = 0;
   out_6592468161831881271[24] = 0;
   out_6592468161831881271[25] = 1;
   out_6592468161831881271[26] = 0;
   out_6592468161831881271[27] = 0;
   out_6592468161831881271[28] = 1;
   out_6592468161831881271[29] = 0;
   out_6592468161831881271[30] = 0;
   out_6592468161831881271[31] = 0;
   out_6592468161831881271[32] = 0;
   out_6592468161831881271[33] = 0;
   out_6592468161831881271[34] = 0;
   out_6592468161831881271[35] = 0;
   out_6592468161831881271[36] = 0;
   out_6592468161831881271[37] = 0;
   out_6592468161831881271[38] = 0;
   out_6592468161831881271[39] = 0;
   out_6592468161831881271[40] = 0;
   out_6592468161831881271[41] = 0;
   out_6592468161831881271[42] = 0;
   out_6592468161831881271[43] = 0;
   out_6592468161831881271[44] = 1;
   out_6592468161831881271[45] = 0;
   out_6592468161831881271[46] = 0;
   out_6592468161831881271[47] = 1;
   out_6592468161831881271[48] = 0;
   out_6592468161831881271[49] = 0;
   out_6592468161831881271[50] = 0;
   out_6592468161831881271[51] = 0;
   out_6592468161831881271[52] = 0;
   out_6592468161831881271[53] = 0;
}
void h_10(double *state, double *unused, double *out_4220186743788310408) {
   out_4220186743788310408[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_4220186743788310408[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_4220186743788310408[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_2494414403690571241) {
   out_2494414403690571241[0] = 0;
   out_2494414403690571241[1] = 9.8100000000000005*cos(state[1]);
   out_2494414403690571241[2] = 0;
   out_2494414403690571241[3] = 0;
   out_2494414403690571241[4] = -state[8];
   out_2494414403690571241[5] = state[7];
   out_2494414403690571241[6] = 0;
   out_2494414403690571241[7] = state[5];
   out_2494414403690571241[8] = -state[4];
   out_2494414403690571241[9] = 0;
   out_2494414403690571241[10] = 0;
   out_2494414403690571241[11] = 0;
   out_2494414403690571241[12] = 1;
   out_2494414403690571241[13] = 0;
   out_2494414403690571241[14] = 0;
   out_2494414403690571241[15] = 1;
   out_2494414403690571241[16] = 0;
   out_2494414403690571241[17] = 0;
   out_2494414403690571241[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_2494414403690571241[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_2494414403690571241[20] = 0;
   out_2494414403690571241[21] = state[8];
   out_2494414403690571241[22] = 0;
   out_2494414403690571241[23] = -state[6];
   out_2494414403690571241[24] = -state[5];
   out_2494414403690571241[25] = 0;
   out_2494414403690571241[26] = state[3];
   out_2494414403690571241[27] = 0;
   out_2494414403690571241[28] = 0;
   out_2494414403690571241[29] = 0;
   out_2494414403690571241[30] = 0;
   out_2494414403690571241[31] = 1;
   out_2494414403690571241[32] = 0;
   out_2494414403690571241[33] = 0;
   out_2494414403690571241[34] = 1;
   out_2494414403690571241[35] = 0;
   out_2494414403690571241[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_2494414403690571241[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_2494414403690571241[38] = 0;
   out_2494414403690571241[39] = -state[7];
   out_2494414403690571241[40] = state[6];
   out_2494414403690571241[41] = 0;
   out_2494414403690571241[42] = state[4];
   out_2494414403690571241[43] = -state[3];
   out_2494414403690571241[44] = 0;
   out_2494414403690571241[45] = 0;
   out_2494414403690571241[46] = 0;
   out_2494414403690571241[47] = 0;
   out_2494414403690571241[48] = 0;
   out_2494414403690571241[49] = 0;
   out_2494414403690571241[50] = 1;
   out_2494414403690571241[51] = 0;
   out_2494414403690571241[52] = 0;
   out_2494414403690571241[53] = 1;
}
void h_13(double *state, double *unused, double *out_5780074361933493057) {
   out_5780074361933493057[0] = state[3];
   out_5780074361933493057[1] = state[4];
   out_5780074361933493057[2] = state[5];
}
void H_13(double *state, double *unused, double *out_8642002086545337544) {
   out_8642002086545337544[0] = 0;
   out_8642002086545337544[1] = 0;
   out_8642002086545337544[2] = 0;
   out_8642002086545337544[3] = 1;
   out_8642002086545337544[4] = 0;
   out_8642002086545337544[5] = 0;
   out_8642002086545337544[6] = 0;
   out_8642002086545337544[7] = 0;
   out_8642002086545337544[8] = 0;
   out_8642002086545337544[9] = 0;
   out_8642002086545337544[10] = 0;
   out_8642002086545337544[11] = 0;
   out_8642002086545337544[12] = 0;
   out_8642002086545337544[13] = 0;
   out_8642002086545337544[14] = 0;
   out_8642002086545337544[15] = 0;
   out_8642002086545337544[16] = 0;
   out_8642002086545337544[17] = 0;
   out_8642002086545337544[18] = 0;
   out_8642002086545337544[19] = 0;
   out_8642002086545337544[20] = 0;
   out_8642002086545337544[21] = 0;
   out_8642002086545337544[22] = 1;
   out_8642002086545337544[23] = 0;
   out_8642002086545337544[24] = 0;
   out_8642002086545337544[25] = 0;
   out_8642002086545337544[26] = 0;
   out_8642002086545337544[27] = 0;
   out_8642002086545337544[28] = 0;
   out_8642002086545337544[29] = 0;
   out_8642002086545337544[30] = 0;
   out_8642002086545337544[31] = 0;
   out_8642002086545337544[32] = 0;
   out_8642002086545337544[33] = 0;
   out_8642002086545337544[34] = 0;
   out_8642002086545337544[35] = 0;
   out_8642002086545337544[36] = 0;
   out_8642002086545337544[37] = 0;
   out_8642002086545337544[38] = 0;
   out_8642002086545337544[39] = 0;
   out_8642002086545337544[40] = 0;
   out_8642002086545337544[41] = 1;
   out_8642002086545337544[42] = 0;
   out_8642002086545337544[43] = 0;
   out_8642002086545337544[44] = 0;
   out_8642002086545337544[45] = 0;
   out_8642002086545337544[46] = 0;
   out_8642002086545337544[47] = 0;
   out_8642002086545337544[48] = 0;
   out_8642002086545337544[49] = 0;
   out_8642002086545337544[50] = 0;
   out_8642002086545337544[51] = 0;
   out_8642002086545337544[52] = 0;
   out_8642002086545337544[53] = 0;
}
void h_14(double *state, double *unused, double *out_3473232613379301184) {
   out_3473232613379301184[0] = state[6];
   out_3473232613379301184[1] = state[7];
   out_3473232613379301184[2] = state[8];
}
void H_14(double *state, double *unused, double *out_3509679729536508975) {
   out_3509679729536508975[0] = 0;
   out_3509679729536508975[1] = 0;
   out_3509679729536508975[2] = 0;
   out_3509679729536508975[3] = 0;
   out_3509679729536508975[4] = 0;
   out_3509679729536508975[5] = 0;
   out_3509679729536508975[6] = 1;
   out_3509679729536508975[7] = 0;
   out_3509679729536508975[8] = 0;
   out_3509679729536508975[9] = 0;
   out_3509679729536508975[10] = 0;
   out_3509679729536508975[11] = 0;
   out_3509679729536508975[12] = 0;
   out_3509679729536508975[13] = 0;
   out_3509679729536508975[14] = 0;
   out_3509679729536508975[15] = 0;
   out_3509679729536508975[16] = 0;
   out_3509679729536508975[17] = 0;
   out_3509679729536508975[18] = 0;
   out_3509679729536508975[19] = 0;
   out_3509679729536508975[20] = 0;
   out_3509679729536508975[21] = 0;
   out_3509679729536508975[22] = 0;
   out_3509679729536508975[23] = 0;
   out_3509679729536508975[24] = 0;
   out_3509679729536508975[25] = 1;
   out_3509679729536508975[26] = 0;
   out_3509679729536508975[27] = 0;
   out_3509679729536508975[28] = 0;
   out_3509679729536508975[29] = 0;
   out_3509679729536508975[30] = 0;
   out_3509679729536508975[31] = 0;
   out_3509679729536508975[32] = 0;
   out_3509679729536508975[33] = 0;
   out_3509679729536508975[34] = 0;
   out_3509679729536508975[35] = 0;
   out_3509679729536508975[36] = 0;
   out_3509679729536508975[37] = 0;
   out_3509679729536508975[38] = 0;
   out_3509679729536508975[39] = 0;
   out_3509679729536508975[40] = 0;
   out_3509679729536508975[41] = 0;
   out_3509679729536508975[42] = 0;
   out_3509679729536508975[43] = 0;
   out_3509679729536508975[44] = 1;
   out_3509679729536508975[45] = 0;
   out_3509679729536508975[46] = 0;
   out_3509679729536508975[47] = 0;
   out_3509679729536508975[48] = 0;
   out_3509679729536508975[49] = 0;
   out_3509679729536508975[50] = 0;
   out_3509679729536508975[51] = 0;
   out_3509679729536508975[52] = 0;
   out_3509679729536508975[53] = 0;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_4, H_4, NULL, in_z, in_R, in_ea, MAHA_THRESH_4);
}
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_10, H_10, NULL, in_z, in_R, in_ea, MAHA_THRESH_10);
}
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_13, H_13, NULL, in_z, in_R, in_ea, MAHA_THRESH_13);
}
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_14, H_14, NULL, in_z, in_R, in_ea, MAHA_THRESH_14);
}
void pose_err_fun(double *nom_x, double *delta_x, double *out_4940663026929446750) {
  err_fun(nom_x, delta_x, out_4940663026929446750);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1207218016783825442) {
  inv_err_fun(nom_x, true_x, out_1207218016783825442);
}
void pose_H_mod_fun(double *state, double *out_2820660951987973699) {
  H_mod_fun(state, out_2820660951987973699);
}
void pose_f_fun(double *state, double dt, double *out_2310584025056258416) {
  f_fun(state,  dt, out_2310584025056258416);
}
void pose_F_fun(double *state, double dt, double *out_6557190266781458971) {
  F_fun(state,  dt, out_6557190266781458971);
}
void pose_h_4(double *state, double *unused, double *out_8177424571477984765) {
  h_4(state, unused, out_8177424571477984765);
}
void pose_H_4(double *state, double *unused, double *out_6592468161831881271) {
  H_4(state, unused, out_6592468161831881271);
}
void pose_h_10(double *state, double *unused, double *out_4220186743788310408) {
  h_10(state, unused, out_4220186743788310408);
}
void pose_H_10(double *state, double *unused, double *out_2494414403690571241) {
  H_10(state, unused, out_2494414403690571241);
}
void pose_h_13(double *state, double *unused, double *out_5780074361933493057) {
  h_13(state, unused, out_5780074361933493057);
}
void pose_H_13(double *state, double *unused, double *out_8642002086545337544) {
  H_13(state, unused, out_8642002086545337544);
}
void pose_h_14(double *state, double *unused, double *out_3473232613379301184) {
  h_14(state, unused, out_3473232613379301184);
}
void pose_H_14(double *state, double *unused, double *out_3509679729536508975) {
  H_14(state, unused, out_3509679729536508975);
}
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
}

const EKF pose = {
  .name = "pose",
  .kinds = { 4, 10, 13, 14 },
  .feature_kinds = {  },
  .f_fun = pose_f_fun,
  .F_fun = pose_F_fun,
  .err_fun = pose_err_fun,
  .inv_err_fun = pose_inv_err_fun,
  .H_mod_fun = pose_H_mod_fun,
  .predict = pose_predict,
  .hs = {
    { 4, pose_h_4 },
    { 10, pose_h_10 },
    { 13, pose_h_13 },
    { 14, pose_h_14 },
  },
  .Hs = {
    { 4, pose_H_4 },
    { 10, pose_H_10 },
    { 13, pose_H_13 },
    { 14, pose_H_14 },
  },
  .updates = {
    { 4, pose_update_4 },
    { 10, pose_update_10 },
    { 13, pose_update_13 },
    { 14, pose_update_14 },
  },
  .Hes = {
  },
  .sets = {
  },
  .extra_routines = {
  },
};

ekf_lib_init(pose)

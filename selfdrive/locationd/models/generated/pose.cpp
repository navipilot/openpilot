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
void err_fun(double *nom_x, double *delta_x, double *out_6656022622757342812) {
   out_6656022622757342812[0] = delta_x[0] + nom_x[0];
   out_6656022622757342812[1] = delta_x[1] + nom_x[1];
   out_6656022622757342812[2] = delta_x[2] + nom_x[2];
   out_6656022622757342812[3] = delta_x[3] + nom_x[3];
   out_6656022622757342812[4] = delta_x[4] + nom_x[4];
   out_6656022622757342812[5] = delta_x[5] + nom_x[5];
   out_6656022622757342812[6] = delta_x[6] + nom_x[6];
   out_6656022622757342812[7] = delta_x[7] + nom_x[7];
   out_6656022622757342812[8] = delta_x[8] + nom_x[8];
   out_6656022622757342812[9] = delta_x[9] + nom_x[9];
   out_6656022622757342812[10] = delta_x[10] + nom_x[10];
   out_6656022622757342812[11] = delta_x[11] + nom_x[11];
   out_6656022622757342812[12] = delta_x[12] + nom_x[12];
   out_6656022622757342812[13] = delta_x[13] + nom_x[13];
   out_6656022622757342812[14] = delta_x[14] + nom_x[14];
   out_6656022622757342812[15] = delta_x[15] + nom_x[15];
   out_6656022622757342812[16] = delta_x[16] + nom_x[16];
   out_6656022622757342812[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_5165337953826908445) {
   out_5165337953826908445[0] = -nom_x[0] + true_x[0];
   out_5165337953826908445[1] = -nom_x[1] + true_x[1];
   out_5165337953826908445[2] = -nom_x[2] + true_x[2];
   out_5165337953826908445[3] = -nom_x[3] + true_x[3];
   out_5165337953826908445[4] = -nom_x[4] + true_x[4];
   out_5165337953826908445[5] = -nom_x[5] + true_x[5];
   out_5165337953826908445[6] = -nom_x[6] + true_x[6];
   out_5165337953826908445[7] = -nom_x[7] + true_x[7];
   out_5165337953826908445[8] = -nom_x[8] + true_x[8];
   out_5165337953826908445[9] = -nom_x[9] + true_x[9];
   out_5165337953826908445[10] = -nom_x[10] + true_x[10];
   out_5165337953826908445[11] = -nom_x[11] + true_x[11];
   out_5165337953826908445[12] = -nom_x[12] + true_x[12];
   out_5165337953826908445[13] = -nom_x[13] + true_x[13];
   out_5165337953826908445[14] = -nom_x[14] + true_x[14];
   out_5165337953826908445[15] = -nom_x[15] + true_x[15];
   out_5165337953826908445[16] = -nom_x[16] + true_x[16];
   out_5165337953826908445[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_5190446458913695382) {
   out_5190446458913695382[0] = 1.0;
   out_5190446458913695382[1] = 0.0;
   out_5190446458913695382[2] = 0.0;
   out_5190446458913695382[3] = 0.0;
   out_5190446458913695382[4] = 0.0;
   out_5190446458913695382[5] = 0.0;
   out_5190446458913695382[6] = 0.0;
   out_5190446458913695382[7] = 0.0;
   out_5190446458913695382[8] = 0.0;
   out_5190446458913695382[9] = 0.0;
   out_5190446458913695382[10] = 0.0;
   out_5190446458913695382[11] = 0.0;
   out_5190446458913695382[12] = 0.0;
   out_5190446458913695382[13] = 0.0;
   out_5190446458913695382[14] = 0.0;
   out_5190446458913695382[15] = 0.0;
   out_5190446458913695382[16] = 0.0;
   out_5190446458913695382[17] = 0.0;
   out_5190446458913695382[18] = 0.0;
   out_5190446458913695382[19] = 1.0;
   out_5190446458913695382[20] = 0.0;
   out_5190446458913695382[21] = 0.0;
   out_5190446458913695382[22] = 0.0;
   out_5190446458913695382[23] = 0.0;
   out_5190446458913695382[24] = 0.0;
   out_5190446458913695382[25] = 0.0;
   out_5190446458913695382[26] = 0.0;
   out_5190446458913695382[27] = 0.0;
   out_5190446458913695382[28] = 0.0;
   out_5190446458913695382[29] = 0.0;
   out_5190446458913695382[30] = 0.0;
   out_5190446458913695382[31] = 0.0;
   out_5190446458913695382[32] = 0.0;
   out_5190446458913695382[33] = 0.0;
   out_5190446458913695382[34] = 0.0;
   out_5190446458913695382[35] = 0.0;
   out_5190446458913695382[36] = 0.0;
   out_5190446458913695382[37] = 0.0;
   out_5190446458913695382[38] = 1.0;
   out_5190446458913695382[39] = 0.0;
   out_5190446458913695382[40] = 0.0;
   out_5190446458913695382[41] = 0.0;
   out_5190446458913695382[42] = 0.0;
   out_5190446458913695382[43] = 0.0;
   out_5190446458913695382[44] = 0.0;
   out_5190446458913695382[45] = 0.0;
   out_5190446458913695382[46] = 0.0;
   out_5190446458913695382[47] = 0.0;
   out_5190446458913695382[48] = 0.0;
   out_5190446458913695382[49] = 0.0;
   out_5190446458913695382[50] = 0.0;
   out_5190446458913695382[51] = 0.0;
   out_5190446458913695382[52] = 0.0;
   out_5190446458913695382[53] = 0.0;
   out_5190446458913695382[54] = 0.0;
   out_5190446458913695382[55] = 0.0;
   out_5190446458913695382[56] = 0.0;
   out_5190446458913695382[57] = 1.0;
   out_5190446458913695382[58] = 0.0;
   out_5190446458913695382[59] = 0.0;
   out_5190446458913695382[60] = 0.0;
   out_5190446458913695382[61] = 0.0;
   out_5190446458913695382[62] = 0.0;
   out_5190446458913695382[63] = 0.0;
   out_5190446458913695382[64] = 0.0;
   out_5190446458913695382[65] = 0.0;
   out_5190446458913695382[66] = 0.0;
   out_5190446458913695382[67] = 0.0;
   out_5190446458913695382[68] = 0.0;
   out_5190446458913695382[69] = 0.0;
   out_5190446458913695382[70] = 0.0;
   out_5190446458913695382[71] = 0.0;
   out_5190446458913695382[72] = 0.0;
   out_5190446458913695382[73] = 0.0;
   out_5190446458913695382[74] = 0.0;
   out_5190446458913695382[75] = 0.0;
   out_5190446458913695382[76] = 1.0;
   out_5190446458913695382[77] = 0.0;
   out_5190446458913695382[78] = 0.0;
   out_5190446458913695382[79] = 0.0;
   out_5190446458913695382[80] = 0.0;
   out_5190446458913695382[81] = 0.0;
   out_5190446458913695382[82] = 0.0;
   out_5190446458913695382[83] = 0.0;
   out_5190446458913695382[84] = 0.0;
   out_5190446458913695382[85] = 0.0;
   out_5190446458913695382[86] = 0.0;
   out_5190446458913695382[87] = 0.0;
   out_5190446458913695382[88] = 0.0;
   out_5190446458913695382[89] = 0.0;
   out_5190446458913695382[90] = 0.0;
   out_5190446458913695382[91] = 0.0;
   out_5190446458913695382[92] = 0.0;
   out_5190446458913695382[93] = 0.0;
   out_5190446458913695382[94] = 0.0;
   out_5190446458913695382[95] = 1.0;
   out_5190446458913695382[96] = 0.0;
   out_5190446458913695382[97] = 0.0;
   out_5190446458913695382[98] = 0.0;
   out_5190446458913695382[99] = 0.0;
   out_5190446458913695382[100] = 0.0;
   out_5190446458913695382[101] = 0.0;
   out_5190446458913695382[102] = 0.0;
   out_5190446458913695382[103] = 0.0;
   out_5190446458913695382[104] = 0.0;
   out_5190446458913695382[105] = 0.0;
   out_5190446458913695382[106] = 0.0;
   out_5190446458913695382[107] = 0.0;
   out_5190446458913695382[108] = 0.0;
   out_5190446458913695382[109] = 0.0;
   out_5190446458913695382[110] = 0.0;
   out_5190446458913695382[111] = 0.0;
   out_5190446458913695382[112] = 0.0;
   out_5190446458913695382[113] = 0.0;
   out_5190446458913695382[114] = 1.0;
   out_5190446458913695382[115] = 0.0;
   out_5190446458913695382[116] = 0.0;
   out_5190446458913695382[117] = 0.0;
   out_5190446458913695382[118] = 0.0;
   out_5190446458913695382[119] = 0.0;
   out_5190446458913695382[120] = 0.0;
   out_5190446458913695382[121] = 0.0;
   out_5190446458913695382[122] = 0.0;
   out_5190446458913695382[123] = 0.0;
   out_5190446458913695382[124] = 0.0;
   out_5190446458913695382[125] = 0.0;
   out_5190446458913695382[126] = 0.0;
   out_5190446458913695382[127] = 0.0;
   out_5190446458913695382[128] = 0.0;
   out_5190446458913695382[129] = 0.0;
   out_5190446458913695382[130] = 0.0;
   out_5190446458913695382[131] = 0.0;
   out_5190446458913695382[132] = 0.0;
   out_5190446458913695382[133] = 1.0;
   out_5190446458913695382[134] = 0.0;
   out_5190446458913695382[135] = 0.0;
   out_5190446458913695382[136] = 0.0;
   out_5190446458913695382[137] = 0.0;
   out_5190446458913695382[138] = 0.0;
   out_5190446458913695382[139] = 0.0;
   out_5190446458913695382[140] = 0.0;
   out_5190446458913695382[141] = 0.0;
   out_5190446458913695382[142] = 0.0;
   out_5190446458913695382[143] = 0.0;
   out_5190446458913695382[144] = 0.0;
   out_5190446458913695382[145] = 0.0;
   out_5190446458913695382[146] = 0.0;
   out_5190446458913695382[147] = 0.0;
   out_5190446458913695382[148] = 0.0;
   out_5190446458913695382[149] = 0.0;
   out_5190446458913695382[150] = 0.0;
   out_5190446458913695382[151] = 0.0;
   out_5190446458913695382[152] = 1.0;
   out_5190446458913695382[153] = 0.0;
   out_5190446458913695382[154] = 0.0;
   out_5190446458913695382[155] = 0.0;
   out_5190446458913695382[156] = 0.0;
   out_5190446458913695382[157] = 0.0;
   out_5190446458913695382[158] = 0.0;
   out_5190446458913695382[159] = 0.0;
   out_5190446458913695382[160] = 0.0;
   out_5190446458913695382[161] = 0.0;
   out_5190446458913695382[162] = 0.0;
   out_5190446458913695382[163] = 0.0;
   out_5190446458913695382[164] = 0.0;
   out_5190446458913695382[165] = 0.0;
   out_5190446458913695382[166] = 0.0;
   out_5190446458913695382[167] = 0.0;
   out_5190446458913695382[168] = 0.0;
   out_5190446458913695382[169] = 0.0;
   out_5190446458913695382[170] = 0.0;
   out_5190446458913695382[171] = 1.0;
   out_5190446458913695382[172] = 0.0;
   out_5190446458913695382[173] = 0.0;
   out_5190446458913695382[174] = 0.0;
   out_5190446458913695382[175] = 0.0;
   out_5190446458913695382[176] = 0.0;
   out_5190446458913695382[177] = 0.0;
   out_5190446458913695382[178] = 0.0;
   out_5190446458913695382[179] = 0.0;
   out_5190446458913695382[180] = 0.0;
   out_5190446458913695382[181] = 0.0;
   out_5190446458913695382[182] = 0.0;
   out_5190446458913695382[183] = 0.0;
   out_5190446458913695382[184] = 0.0;
   out_5190446458913695382[185] = 0.0;
   out_5190446458913695382[186] = 0.0;
   out_5190446458913695382[187] = 0.0;
   out_5190446458913695382[188] = 0.0;
   out_5190446458913695382[189] = 0.0;
   out_5190446458913695382[190] = 1.0;
   out_5190446458913695382[191] = 0.0;
   out_5190446458913695382[192] = 0.0;
   out_5190446458913695382[193] = 0.0;
   out_5190446458913695382[194] = 0.0;
   out_5190446458913695382[195] = 0.0;
   out_5190446458913695382[196] = 0.0;
   out_5190446458913695382[197] = 0.0;
   out_5190446458913695382[198] = 0.0;
   out_5190446458913695382[199] = 0.0;
   out_5190446458913695382[200] = 0.0;
   out_5190446458913695382[201] = 0.0;
   out_5190446458913695382[202] = 0.0;
   out_5190446458913695382[203] = 0.0;
   out_5190446458913695382[204] = 0.0;
   out_5190446458913695382[205] = 0.0;
   out_5190446458913695382[206] = 0.0;
   out_5190446458913695382[207] = 0.0;
   out_5190446458913695382[208] = 0.0;
   out_5190446458913695382[209] = 1.0;
   out_5190446458913695382[210] = 0.0;
   out_5190446458913695382[211] = 0.0;
   out_5190446458913695382[212] = 0.0;
   out_5190446458913695382[213] = 0.0;
   out_5190446458913695382[214] = 0.0;
   out_5190446458913695382[215] = 0.0;
   out_5190446458913695382[216] = 0.0;
   out_5190446458913695382[217] = 0.0;
   out_5190446458913695382[218] = 0.0;
   out_5190446458913695382[219] = 0.0;
   out_5190446458913695382[220] = 0.0;
   out_5190446458913695382[221] = 0.0;
   out_5190446458913695382[222] = 0.0;
   out_5190446458913695382[223] = 0.0;
   out_5190446458913695382[224] = 0.0;
   out_5190446458913695382[225] = 0.0;
   out_5190446458913695382[226] = 0.0;
   out_5190446458913695382[227] = 0.0;
   out_5190446458913695382[228] = 1.0;
   out_5190446458913695382[229] = 0.0;
   out_5190446458913695382[230] = 0.0;
   out_5190446458913695382[231] = 0.0;
   out_5190446458913695382[232] = 0.0;
   out_5190446458913695382[233] = 0.0;
   out_5190446458913695382[234] = 0.0;
   out_5190446458913695382[235] = 0.0;
   out_5190446458913695382[236] = 0.0;
   out_5190446458913695382[237] = 0.0;
   out_5190446458913695382[238] = 0.0;
   out_5190446458913695382[239] = 0.0;
   out_5190446458913695382[240] = 0.0;
   out_5190446458913695382[241] = 0.0;
   out_5190446458913695382[242] = 0.0;
   out_5190446458913695382[243] = 0.0;
   out_5190446458913695382[244] = 0.0;
   out_5190446458913695382[245] = 0.0;
   out_5190446458913695382[246] = 0.0;
   out_5190446458913695382[247] = 1.0;
   out_5190446458913695382[248] = 0.0;
   out_5190446458913695382[249] = 0.0;
   out_5190446458913695382[250] = 0.0;
   out_5190446458913695382[251] = 0.0;
   out_5190446458913695382[252] = 0.0;
   out_5190446458913695382[253] = 0.0;
   out_5190446458913695382[254] = 0.0;
   out_5190446458913695382[255] = 0.0;
   out_5190446458913695382[256] = 0.0;
   out_5190446458913695382[257] = 0.0;
   out_5190446458913695382[258] = 0.0;
   out_5190446458913695382[259] = 0.0;
   out_5190446458913695382[260] = 0.0;
   out_5190446458913695382[261] = 0.0;
   out_5190446458913695382[262] = 0.0;
   out_5190446458913695382[263] = 0.0;
   out_5190446458913695382[264] = 0.0;
   out_5190446458913695382[265] = 0.0;
   out_5190446458913695382[266] = 1.0;
   out_5190446458913695382[267] = 0.0;
   out_5190446458913695382[268] = 0.0;
   out_5190446458913695382[269] = 0.0;
   out_5190446458913695382[270] = 0.0;
   out_5190446458913695382[271] = 0.0;
   out_5190446458913695382[272] = 0.0;
   out_5190446458913695382[273] = 0.0;
   out_5190446458913695382[274] = 0.0;
   out_5190446458913695382[275] = 0.0;
   out_5190446458913695382[276] = 0.0;
   out_5190446458913695382[277] = 0.0;
   out_5190446458913695382[278] = 0.0;
   out_5190446458913695382[279] = 0.0;
   out_5190446458913695382[280] = 0.0;
   out_5190446458913695382[281] = 0.0;
   out_5190446458913695382[282] = 0.0;
   out_5190446458913695382[283] = 0.0;
   out_5190446458913695382[284] = 0.0;
   out_5190446458913695382[285] = 1.0;
   out_5190446458913695382[286] = 0.0;
   out_5190446458913695382[287] = 0.0;
   out_5190446458913695382[288] = 0.0;
   out_5190446458913695382[289] = 0.0;
   out_5190446458913695382[290] = 0.0;
   out_5190446458913695382[291] = 0.0;
   out_5190446458913695382[292] = 0.0;
   out_5190446458913695382[293] = 0.0;
   out_5190446458913695382[294] = 0.0;
   out_5190446458913695382[295] = 0.0;
   out_5190446458913695382[296] = 0.0;
   out_5190446458913695382[297] = 0.0;
   out_5190446458913695382[298] = 0.0;
   out_5190446458913695382[299] = 0.0;
   out_5190446458913695382[300] = 0.0;
   out_5190446458913695382[301] = 0.0;
   out_5190446458913695382[302] = 0.0;
   out_5190446458913695382[303] = 0.0;
   out_5190446458913695382[304] = 1.0;
   out_5190446458913695382[305] = 0.0;
   out_5190446458913695382[306] = 0.0;
   out_5190446458913695382[307] = 0.0;
   out_5190446458913695382[308] = 0.0;
   out_5190446458913695382[309] = 0.0;
   out_5190446458913695382[310] = 0.0;
   out_5190446458913695382[311] = 0.0;
   out_5190446458913695382[312] = 0.0;
   out_5190446458913695382[313] = 0.0;
   out_5190446458913695382[314] = 0.0;
   out_5190446458913695382[315] = 0.0;
   out_5190446458913695382[316] = 0.0;
   out_5190446458913695382[317] = 0.0;
   out_5190446458913695382[318] = 0.0;
   out_5190446458913695382[319] = 0.0;
   out_5190446458913695382[320] = 0.0;
   out_5190446458913695382[321] = 0.0;
   out_5190446458913695382[322] = 0.0;
   out_5190446458913695382[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_337750480546902101) {
   out_337750480546902101[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_337750480546902101[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_337750480546902101[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_337750480546902101[3] = dt*state[12] + state[3];
   out_337750480546902101[4] = dt*state[13] + state[4];
   out_337750480546902101[5] = dt*state[14] + state[5];
   out_337750480546902101[6] = state[6];
   out_337750480546902101[7] = state[7];
   out_337750480546902101[8] = state[8];
   out_337750480546902101[9] = state[9];
   out_337750480546902101[10] = state[10];
   out_337750480546902101[11] = state[11];
   out_337750480546902101[12] = state[12];
   out_337750480546902101[13] = state[13];
   out_337750480546902101[14] = state[14];
   out_337750480546902101[15] = state[15];
   out_337750480546902101[16] = state[16];
   out_337750480546902101[17] = state[17];
}
void F_fun(double *state, double dt, double *out_1252525263334667707) {
   out_1252525263334667707[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1252525263334667707[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1252525263334667707[2] = 0;
   out_1252525263334667707[3] = 0;
   out_1252525263334667707[4] = 0;
   out_1252525263334667707[5] = 0;
   out_1252525263334667707[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1252525263334667707[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1252525263334667707[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1252525263334667707[9] = 0;
   out_1252525263334667707[10] = 0;
   out_1252525263334667707[11] = 0;
   out_1252525263334667707[12] = 0;
   out_1252525263334667707[13] = 0;
   out_1252525263334667707[14] = 0;
   out_1252525263334667707[15] = 0;
   out_1252525263334667707[16] = 0;
   out_1252525263334667707[17] = 0;
   out_1252525263334667707[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1252525263334667707[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1252525263334667707[20] = 0;
   out_1252525263334667707[21] = 0;
   out_1252525263334667707[22] = 0;
   out_1252525263334667707[23] = 0;
   out_1252525263334667707[24] = 0;
   out_1252525263334667707[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1252525263334667707[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1252525263334667707[27] = 0;
   out_1252525263334667707[28] = 0;
   out_1252525263334667707[29] = 0;
   out_1252525263334667707[30] = 0;
   out_1252525263334667707[31] = 0;
   out_1252525263334667707[32] = 0;
   out_1252525263334667707[33] = 0;
   out_1252525263334667707[34] = 0;
   out_1252525263334667707[35] = 0;
   out_1252525263334667707[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1252525263334667707[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1252525263334667707[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1252525263334667707[39] = 0;
   out_1252525263334667707[40] = 0;
   out_1252525263334667707[41] = 0;
   out_1252525263334667707[42] = 0;
   out_1252525263334667707[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1252525263334667707[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1252525263334667707[45] = 0;
   out_1252525263334667707[46] = 0;
   out_1252525263334667707[47] = 0;
   out_1252525263334667707[48] = 0;
   out_1252525263334667707[49] = 0;
   out_1252525263334667707[50] = 0;
   out_1252525263334667707[51] = 0;
   out_1252525263334667707[52] = 0;
   out_1252525263334667707[53] = 0;
   out_1252525263334667707[54] = 0;
   out_1252525263334667707[55] = 0;
   out_1252525263334667707[56] = 0;
   out_1252525263334667707[57] = 1;
   out_1252525263334667707[58] = 0;
   out_1252525263334667707[59] = 0;
   out_1252525263334667707[60] = 0;
   out_1252525263334667707[61] = 0;
   out_1252525263334667707[62] = 0;
   out_1252525263334667707[63] = 0;
   out_1252525263334667707[64] = 0;
   out_1252525263334667707[65] = 0;
   out_1252525263334667707[66] = dt;
   out_1252525263334667707[67] = 0;
   out_1252525263334667707[68] = 0;
   out_1252525263334667707[69] = 0;
   out_1252525263334667707[70] = 0;
   out_1252525263334667707[71] = 0;
   out_1252525263334667707[72] = 0;
   out_1252525263334667707[73] = 0;
   out_1252525263334667707[74] = 0;
   out_1252525263334667707[75] = 0;
   out_1252525263334667707[76] = 1;
   out_1252525263334667707[77] = 0;
   out_1252525263334667707[78] = 0;
   out_1252525263334667707[79] = 0;
   out_1252525263334667707[80] = 0;
   out_1252525263334667707[81] = 0;
   out_1252525263334667707[82] = 0;
   out_1252525263334667707[83] = 0;
   out_1252525263334667707[84] = 0;
   out_1252525263334667707[85] = dt;
   out_1252525263334667707[86] = 0;
   out_1252525263334667707[87] = 0;
   out_1252525263334667707[88] = 0;
   out_1252525263334667707[89] = 0;
   out_1252525263334667707[90] = 0;
   out_1252525263334667707[91] = 0;
   out_1252525263334667707[92] = 0;
   out_1252525263334667707[93] = 0;
   out_1252525263334667707[94] = 0;
   out_1252525263334667707[95] = 1;
   out_1252525263334667707[96] = 0;
   out_1252525263334667707[97] = 0;
   out_1252525263334667707[98] = 0;
   out_1252525263334667707[99] = 0;
   out_1252525263334667707[100] = 0;
   out_1252525263334667707[101] = 0;
   out_1252525263334667707[102] = 0;
   out_1252525263334667707[103] = 0;
   out_1252525263334667707[104] = dt;
   out_1252525263334667707[105] = 0;
   out_1252525263334667707[106] = 0;
   out_1252525263334667707[107] = 0;
   out_1252525263334667707[108] = 0;
   out_1252525263334667707[109] = 0;
   out_1252525263334667707[110] = 0;
   out_1252525263334667707[111] = 0;
   out_1252525263334667707[112] = 0;
   out_1252525263334667707[113] = 0;
   out_1252525263334667707[114] = 1;
   out_1252525263334667707[115] = 0;
   out_1252525263334667707[116] = 0;
   out_1252525263334667707[117] = 0;
   out_1252525263334667707[118] = 0;
   out_1252525263334667707[119] = 0;
   out_1252525263334667707[120] = 0;
   out_1252525263334667707[121] = 0;
   out_1252525263334667707[122] = 0;
   out_1252525263334667707[123] = 0;
   out_1252525263334667707[124] = 0;
   out_1252525263334667707[125] = 0;
   out_1252525263334667707[126] = 0;
   out_1252525263334667707[127] = 0;
   out_1252525263334667707[128] = 0;
   out_1252525263334667707[129] = 0;
   out_1252525263334667707[130] = 0;
   out_1252525263334667707[131] = 0;
   out_1252525263334667707[132] = 0;
   out_1252525263334667707[133] = 1;
   out_1252525263334667707[134] = 0;
   out_1252525263334667707[135] = 0;
   out_1252525263334667707[136] = 0;
   out_1252525263334667707[137] = 0;
   out_1252525263334667707[138] = 0;
   out_1252525263334667707[139] = 0;
   out_1252525263334667707[140] = 0;
   out_1252525263334667707[141] = 0;
   out_1252525263334667707[142] = 0;
   out_1252525263334667707[143] = 0;
   out_1252525263334667707[144] = 0;
   out_1252525263334667707[145] = 0;
   out_1252525263334667707[146] = 0;
   out_1252525263334667707[147] = 0;
   out_1252525263334667707[148] = 0;
   out_1252525263334667707[149] = 0;
   out_1252525263334667707[150] = 0;
   out_1252525263334667707[151] = 0;
   out_1252525263334667707[152] = 1;
   out_1252525263334667707[153] = 0;
   out_1252525263334667707[154] = 0;
   out_1252525263334667707[155] = 0;
   out_1252525263334667707[156] = 0;
   out_1252525263334667707[157] = 0;
   out_1252525263334667707[158] = 0;
   out_1252525263334667707[159] = 0;
   out_1252525263334667707[160] = 0;
   out_1252525263334667707[161] = 0;
   out_1252525263334667707[162] = 0;
   out_1252525263334667707[163] = 0;
   out_1252525263334667707[164] = 0;
   out_1252525263334667707[165] = 0;
   out_1252525263334667707[166] = 0;
   out_1252525263334667707[167] = 0;
   out_1252525263334667707[168] = 0;
   out_1252525263334667707[169] = 0;
   out_1252525263334667707[170] = 0;
   out_1252525263334667707[171] = 1;
   out_1252525263334667707[172] = 0;
   out_1252525263334667707[173] = 0;
   out_1252525263334667707[174] = 0;
   out_1252525263334667707[175] = 0;
   out_1252525263334667707[176] = 0;
   out_1252525263334667707[177] = 0;
   out_1252525263334667707[178] = 0;
   out_1252525263334667707[179] = 0;
   out_1252525263334667707[180] = 0;
   out_1252525263334667707[181] = 0;
   out_1252525263334667707[182] = 0;
   out_1252525263334667707[183] = 0;
   out_1252525263334667707[184] = 0;
   out_1252525263334667707[185] = 0;
   out_1252525263334667707[186] = 0;
   out_1252525263334667707[187] = 0;
   out_1252525263334667707[188] = 0;
   out_1252525263334667707[189] = 0;
   out_1252525263334667707[190] = 1;
   out_1252525263334667707[191] = 0;
   out_1252525263334667707[192] = 0;
   out_1252525263334667707[193] = 0;
   out_1252525263334667707[194] = 0;
   out_1252525263334667707[195] = 0;
   out_1252525263334667707[196] = 0;
   out_1252525263334667707[197] = 0;
   out_1252525263334667707[198] = 0;
   out_1252525263334667707[199] = 0;
   out_1252525263334667707[200] = 0;
   out_1252525263334667707[201] = 0;
   out_1252525263334667707[202] = 0;
   out_1252525263334667707[203] = 0;
   out_1252525263334667707[204] = 0;
   out_1252525263334667707[205] = 0;
   out_1252525263334667707[206] = 0;
   out_1252525263334667707[207] = 0;
   out_1252525263334667707[208] = 0;
   out_1252525263334667707[209] = 1;
   out_1252525263334667707[210] = 0;
   out_1252525263334667707[211] = 0;
   out_1252525263334667707[212] = 0;
   out_1252525263334667707[213] = 0;
   out_1252525263334667707[214] = 0;
   out_1252525263334667707[215] = 0;
   out_1252525263334667707[216] = 0;
   out_1252525263334667707[217] = 0;
   out_1252525263334667707[218] = 0;
   out_1252525263334667707[219] = 0;
   out_1252525263334667707[220] = 0;
   out_1252525263334667707[221] = 0;
   out_1252525263334667707[222] = 0;
   out_1252525263334667707[223] = 0;
   out_1252525263334667707[224] = 0;
   out_1252525263334667707[225] = 0;
   out_1252525263334667707[226] = 0;
   out_1252525263334667707[227] = 0;
   out_1252525263334667707[228] = 1;
   out_1252525263334667707[229] = 0;
   out_1252525263334667707[230] = 0;
   out_1252525263334667707[231] = 0;
   out_1252525263334667707[232] = 0;
   out_1252525263334667707[233] = 0;
   out_1252525263334667707[234] = 0;
   out_1252525263334667707[235] = 0;
   out_1252525263334667707[236] = 0;
   out_1252525263334667707[237] = 0;
   out_1252525263334667707[238] = 0;
   out_1252525263334667707[239] = 0;
   out_1252525263334667707[240] = 0;
   out_1252525263334667707[241] = 0;
   out_1252525263334667707[242] = 0;
   out_1252525263334667707[243] = 0;
   out_1252525263334667707[244] = 0;
   out_1252525263334667707[245] = 0;
   out_1252525263334667707[246] = 0;
   out_1252525263334667707[247] = 1;
   out_1252525263334667707[248] = 0;
   out_1252525263334667707[249] = 0;
   out_1252525263334667707[250] = 0;
   out_1252525263334667707[251] = 0;
   out_1252525263334667707[252] = 0;
   out_1252525263334667707[253] = 0;
   out_1252525263334667707[254] = 0;
   out_1252525263334667707[255] = 0;
   out_1252525263334667707[256] = 0;
   out_1252525263334667707[257] = 0;
   out_1252525263334667707[258] = 0;
   out_1252525263334667707[259] = 0;
   out_1252525263334667707[260] = 0;
   out_1252525263334667707[261] = 0;
   out_1252525263334667707[262] = 0;
   out_1252525263334667707[263] = 0;
   out_1252525263334667707[264] = 0;
   out_1252525263334667707[265] = 0;
   out_1252525263334667707[266] = 1;
   out_1252525263334667707[267] = 0;
   out_1252525263334667707[268] = 0;
   out_1252525263334667707[269] = 0;
   out_1252525263334667707[270] = 0;
   out_1252525263334667707[271] = 0;
   out_1252525263334667707[272] = 0;
   out_1252525263334667707[273] = 0;
   out_1252525263334667707[274] = 0;
   out_1252525263334667707[275] = 0;
   out_1252525263334667707[276] = 0;
   out_1252525263334667707[277] = 0;
   out_1252525263334667707[278] = 0;
   out_1252525263334667707[279] = 0;
   out_1252525263334667707[280] = 0;
   out_1252525263334667707[281] = 0;
   out_1252525263334667707[282] = 0;
   out_1252525263334667707[283] = 0;
   out_1252525263334667707[284] = 0;
   out_1252525263334667707[285] = 1;
   out_1252525263334667707[286] = 0;
   out_1252525263334667707[287] = 0;
   out_1252525263334667707[288] = 0;
   out_1252525263334667707[289] = 0;
   out_1252525263334667707[290] = 0;
   out_1252525263334667707[291] = 0;
   out_1252525263334667707[292] = 0;
   out_1252525263334667707[293] = 0;
   out_1252525263334667707[294] = 0;
   out_1252525263334667707[295] = 0;
   out_1252525263334667707[296] = 0;
   out_1252525263334667707[297] = 0;
   out_1252525263334667707[298] = 0;
   out_1252525263334667707[299] = 0;
   out_1252525263334667707[300] = 0;
   out_1252525263334667707[301] = 0;
   out_1252525263334667707[302] = 0;
   out_1252525263334667707[303] = 0;
   out_1252525263334667707[304] = 1;
   out_1252525263334667707[305] = 0;
   out_1252525263334667707[306] = 0;
   out_1252525263334667707[307] = 0;
   out_1252525263334667707[308] = 0;
   out_1252525263334667707[309] = 0;
   out_1252525263334667707[310] = 0;
   out_1252525263334667707[311] = 0;
   out_1252525263334667707[312] = 0;
   out_1252525263334667707[313] = 0;
   out_1252525263334667707[314] = 0;
   out_1252525263334667707[315] = 0;
   out_1252525263334667707[316] = 0;
   out_1252525263334667707[317] = 0;
   out_1252525263334667707[318] = 0;
   out_1252525263334667707[319] = 0;
   out_1252525263334667707[320] = 0;
   out_1252525263334667707[321] = 0;
   out_1252525263334667707[322] = 0;
   out_1252525263334667707[323] = 1;
}
void h_4(double *state, double *unused, double *out_1011967617642247994) {
   out_1011967617642247994[0] = state[6] + state[9];
   out_1011967617642247994[1] = state[7] + state[10];
   out_1011967617642247994[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_7408215808043897513) {
   out_7408215808043897513[0] = 0;
   out_7408215808043897513[1] = 0;
   out_7408215808043897513[2] = 0;
   out_7408215808043897513[3] = 0;
   out_7408215808043897513[4] = 0;
   out_7408215808043897513[5] = 0;
   out_7408215808043897513[6] = 1;
   out_7408215808043897513[7] = 0;
   out_7408215808043897513[8] = 0;
   out_7408215808043897513[9] = 1;
   out_7408215808043897513[10] = 0;
   out_7408215808043897513[11] = 0;
   out_7408215808043897513[12] = 0;
   out_7408215808043897513[13] = 0;
   out_7408215808043897513[14] = 0;
   out_7408215808043897513[15] = 0;
   out_7408215808043897513[16] = 0;
   out_7408215808043897513[17] = 0;
   out_7408215808043897513[18] = 0;
   out_7408215808043897513[19] = 0;
   out_7408215808043897513[20] = 0;
   out_7408215808043897513[21] = 0;
   out_7408215808043897513[22] = 0;
   out_7408215808043897513[23] = 0;
   out_7408215808043897513[24] = 0;
   out_7408215808043897513[25] = 1;
   out_7408215808043897513[26] = 0;
   out_7408215808043897513[27] = 0;
   out_7408215808043897513[28] = 1;
   out_7408215808043897513[29] = 0;
   out_7408215808043897513[30] = 0;
   out_7408215808043897513[31] = 0;
   out_7408215808043897513[32] = 0;
   out_7408215808043897513[33] = 0;
   out_7408215808043897513[34] = 0;
   out_7408215808043897513[35] = 0;
   out_7408215808043897513[36] = 0;
   out_7408215808043897513[37] = 0;
   out_7408215808043897513[38] = 0;
   out_7408215808043897513[39] = 0;
   out_7408215808043897513[40] = 0;
   out_7408215808043897513[41] = 0;
   out_7408215808043897513[42] = 0;
   out_7408215808043897513[43] = 0;
   out_7408215808043897513[44] = 1;
   out_7408215808043897513[45] = 0;
   out_7408215808043897513[46] = 0;
   out_7408215808043897513[47] = 1;
   out_7408215808043897513[48] = 0;
   out_7408215808043897513[49] = 0;
   out_7408215808043897513[50] = 0;
   out_7408215808043897513[51] = 0;
   out_7408215808043897513[52] = 0;
   out_7408215808043897513[53] = 0;
}
void h_10(double *state, double *unused, double *out_511164501586373249) {
   out_511164501586373249[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_511164501586373249[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_511164501586373249[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_8712555441471564813) {
   out_8712555441471564813[0] = 0;
   out_8712555441471564813[1] = 9.8100000000000005*cos(state[1]);
   out_8712555441471564813[2] = 0;
   out_8712555441471564813[3] = 0;
   out_8712555441471564813[4] = -state[8];
   out_8712555441471564813[5] = state[7];
   out_8712555441471564813[6] = 0;
   out_8712555441471564813[7] = state[5];
   out_8712555441471564813[8] = -state[4];
   out_8712555441471564813[9] = 0;
   out_8712555441471564813[10] = 0;
   out_8712555441471564813[11] = 0;
   out_8712555441471564813[12] = 1;
   out_8712555441471564813[13] = 0;
   out_8712555441471564813[14] = 0;
   out_8712555441471564813[15] = 1;
   out_8712555441471564813[16] = 0;
   out_8712555441471564813[17] = 0;
   out_8712555441471564813[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_8712555441471564813[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_8712555441471564813[20] = 0;
   out_8712555441471564813[21] = state[8];
   out_8712555441471564813[22] = 0;
   out_8712555441471564813[23] = -state[6];
   out_8712555441471564813[24] = -state[5];
   out_8712555441471564813[25] = 0;
   out_8712555441471564813[26] = state[3];
   out_8712555441471564813[27] = 0;
   out_8712555441471564813[28] = 0;
   out_8712555441471564813[29] = 0;
   out_8712555441471564813[30] = 0;
   out_8712555441471564813[31] = 1;
   out_8712555441471564813[32] = 0;
   out_8712555441471564813[33] = 0;
   out_8712555441471564813[34] = 1;
   out_8712555441471564813[35] = 0;
   out_8712555441471564813[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_8712555441471564813[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_8712555441471564813[38] = 0;
   out_8712555441471564813[39] = -state[7];
   out_8712555441471564813[40] = state[6];
   out_8712555441471564813[41] = 0;
   out_8712555441471564813[42] = state[4];
   out_8712555441471564813[43] = -state[3];
   out_8712555441471564813[44] = 0;
   out_8712555441471564813[45] = 0;
   out_8712555441471564813[46] = 0;
   out_8712555441471564813[47] = 0;
   out_8712555441471564813[48] = 0;
   out_8712555441471564813[49] = 0;
   out_8712555441471564813[50] = 1;
   out_8712555441471564813[51] = 0;
   out_8712555441471564813[52] = 0;
   out_8712555441471564813[53] = 1;
}
void h_13(double *state, double *unused, double *out_6347656539957505651) {
   out_6347656539957505651[0] = state[3];
   out_6347656539957505651[1] = state[4];
   out_6347656539957505651[2] = state[5];
}
void H_13(double *state, double *unused, double *out_7826254440333321302) {
   out_7826254440333321302[0] = 0;
   out_7826254440333321302[1] = 0;
   out_7826254440333321302[2] = 0;
   out_7826254440333321302[3] = 1;
   out_7826254440333321302[4] = 0;
   out_7826254440333321302[5] = 0;
   out_7826254440333321302[6] = 0;
   out_7826254440333321302[7] = 0;
   out_7826254440333321302[8] = 0;
   out_7826254440333321302[9] = 0;
   out_7826254440333321302[10] = 0;
   out_7826254440333321302[11] = 0;
   out_7826254440333321302[12] = 0;
   out_7826254440333321302[13] = 0;
   out_7826254440333321302[14] = 0;
   out_7826254440333321302[15] = 0;
   out_7826254440333321302[16] = 0;
   out_7826254440333321302[17] = 0;
   out_7826254440333321302[18] = 0;
   out_7826254440333321302[19] = 0;
   out_7826254440333321302[20] = 0;
   out_7826254440333321302[21] = 0;
   out_7826254440333321302[22] = 1;
   out_7826254440333321302[23] = 0;
   out_7826254440333321302[24] = 0;
   out_7826254440333321302[25] = 0;
   out_7826254440333321302[26] = 0;
   out_7826254440333321302[27] = 0;
   out_7826254440333321302[28] = 0;
   out_7826254440333321302[29] = 0;
   out_7826254440333321302[30] = 0;
   out_7826254440333321302[31] = 0;
   out_7826254440333321302[32] = 0;
   out_7826254440333321302[33] = 0;
   out_7826254440333321302[34] = 0;
   out_7826254440333321302[35] = 0;
   out_7826254440333321302[36] = 0;
   out_7826254440333321302[37] = 0;
   out_7826254440333321302[38] = 0;
   out_7826254440333321302[39] = 0;
   out_7826254440333321302[40] = 0;
   out_7826254440333321302[41] = 1;
   out_7826254440333321302[42] = 0;
   out_7826254440333321302[43] = 0;
   out_7826254440333321302[44] = 0;
   out_7826254440333321302[45] = 0;
   out_7826254440333321302[46] = 0;
   out_7826254440333321302[47] = 0;
   out_7826254440333321302[48] = 0;
   out_7826254440333321302[49] = 0;
   out_7826254440333321302[50] = 0;
   out_7826254440333321302[51] = 0;
   out_7826254440333321302[52] = 0;
   out_7826254440333321302[53] = 0;
}
void h_14(double *state, double *unused, double *out_251978956082886222) {
   out_251978956082886222[0] = state[6];
   out_251978956082886222[1] = state[7];
   out_251978956082886222[2] = state[8];
}
void H_14(double *state, double *unused, double *out_7075287409326169574) {
   out_7075287409326169574[0] = 0;
   out_7075287409326169574[1] = 0;
   out_7075287409326169574[2] = 0;
   out_7075287409326169574[3] = 0;
   out_7075287409326169574[4] = 0;
   out_7075287409326169574[5] = 0;
   out_7075287409326169574[6] = 1;
   out_7075287409326169574[7] = 0;
   out_7075287409326169574[8] = 0;
   out_7075287409326169574[9] = 0;
   out_7075287409326169574[10] = 0;
   out_7075287409326169574[11] = 0;
   out_7075287409326169574[12] = 0;
   out_7075287409326169574[13] = 0;
   out_7075287409326169574[14] = 0;
   out_7075287409326169574[15] = 0;
   out_7075287409326169574[16] = 0;
   out_7075287409326169574[17] = 0;
   out_7075287409326169574[18] = 0;
   out_7075287409326169574[19] = 0;
   out_7075287409326169574[20] = 0;
   out_7075287409326169574[21] = 0;
   out_7075287409326169574[22] = 0;
   out_7075287409326169574[23] = 0;
   out_7075287409326169574[24] = 0;
   out_7075287409326169574[25] = 1;
   out_7075287409326169574[26] = 0;
   out_7075287409326169574[27] = 0;
   out_7075287409326169574[28] = 0;
   out_7075287409326169574[29] = 0;
   out_7075287409326169574[30] = 0;
   out_7075287409326169574[31] = 0;
   out_7075287409326169574[32] = 0;
   out_7075287409326169574[33] = 0;
   out_7075287409326169574[34] = 0;
   out_7075287409326169574[35] = 0;
   out_7075287409326169574[36] = 0;
   out_7075287409326169574[37] = 0;
   out_7075287409326169574[38] = 0;
   out_7075287409326169574[39] = 0;
   out_7075287409326169574[40] = 0;
   out_7075287409326169574[41] = 0;
   out_7075287409326169574[42] = 0;
   out_7075287409326169574[43] = 0;
   out_7075287409326169574[44] = 1;
   out_7075287409326169574[45] = 0;
   out_7075287409326169574[46] = 0;
   out_7075287409326169574[47] = 0;
   out_7075287409326169574[48] = 0;
   out_7075287409326169574[49] = 0;
   out_7075287409326169574[50] = 0;
   out_7075287409326169574[51] = 0;
   out_7075287409326169574[52] = 0;
   out_7075287409326169574[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_6656022622757342812) {
  err_fun(nom_x, delta_x, out_6656022622757342812);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5165337953826908445) {
  inv_err_fun(nom_x, true_x, out_5165337953826908445);
}
void pose_H_mod_fun(double *state, double *out_5190446458913695382) {
  H_mod_fun(state, out_5190446458913695382);
}
void pose_f_fun(double *state, double dt, double *out_337750480546902101) {
  f_fun(state,  dt, out_337750480546902101);
}
void pose_F_fun(double *state, double dt, double *out_1252525263334667707) {
  F_fun(state,  dt, out_1252525263334667707);
}
void pose_h_4(double *state, double *unused, double *out_1011967617642247994) {
  h_4(state, unused, out_1011967617642247994);
}
void pose_H_4(double *state, double *unused, double *out_7408215808043897513) {
  H_4(state, unused, out_7408215808043897513);
}
void pose_h_10(double *state, double *unused, double *out_511164501586373249) {
  h_10(state, unused, out_511164501586373249);
}
void pose_H_10(double *state, double *unused, double *out_8712555441471564813) {
  H_10(state, unused, out_8712555441471564813);
}
void pose_h_13(double *state, double *unused, double *out_6347656539957505651) {
  h_13(state, unused, out_6347656539957505651);
}
void pose_H_13(double *state, double *unused, double *out_7826254440333321302) {
  H_13(state, unused, out_7826254440333321302);
}
void pose_h_14(double *state, double *unused, double *out_251978956082886222) {
  h_14(state, unused, out_251978956082886222);
}
void pose_H_14(double *state, double *unused, double *out_7075287409326169574) {
  H_14(state, unused, out_7075287409326169574);
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

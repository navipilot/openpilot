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
void err_fun(double *nom_x, double *delta_x, double *out_7099155186075195158) {
   out_7099155186075195158[0] = delta_x[0] + nom_x[0];
   out_7099155186075195158[1] = delta_x[1] + nom_x[1];
   out_7099155186075195158[2] = delta_x[2] + nom_x[2];
   out_7099155186075195158[3] = delta_x[3] + nom_x[3];
   out_7099155186075195158[4] = delta_x[4] + nom_x[4];
   out_7099155186075195158[5] = delta_x[5] + nom_x[5];
   out_7099155186075195158[6] = delta_x[6] + nom_x[6];
   out_7099155186075195158[7] = delta_x[7] + nom_x[7];
   out_7099155186075195158[8] = delta_x[8] + nom_x[8];
   out_7099155186075195158[9] = delta_x[9] + nom_x[9];
   out_7099155186075195158[10] = delta_x[10] + nom_x[10];
   out_7099155186075195158[11] = delta_x[11] + nom_x[11];
   out_7099155186075195158[12] = delta_x[12] + nom_x[12];
   out_7099155186075195158[13] = delta_x[13] + nom_x[13];
   out_7099155186075195158[14] = delta_x[14] + nom_x[14];
   out_7099155186075195158[15] = delta_x[15] + nom_x[15];
   out_7099155186075195158[16] = delta_x[16] + nom_x[16];
   out_7099155186075195158[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2441097136385557776) {
   out_2441097136385557776[0] = -nom_x[0] + true_x[0];
   out_2441097136385557776[1] = -nom_x[1] + true_x[1];
   out_2441097136385557776[2] = -nom_x[2] + true_x[2];
   out_2441097136385557776[3] = -nom_x[3] + true_x[3];
   out_2441097136385557776[4] = -nom_x[4] + true_x[4];
   out_2441097136385557776[5] = -nom_x[5] + true_x[5];
   out_2441097136385557776[6] = -nom_x[6] + true_x[6];
   out_2441097136385557776[7] = -nom_x[7] + true_x[7];
   out_2441097136385557776[8] = -nom_x[8] + true_x[8];
   out_2441097136385557776[9] = -nom_x[9] + true_x[9];
   out_2441097136385557776[10] = -nom_x[10] + true_x[10];
   out_2441097136385557776[11] = -nom_x[11] + true_x[11];
   out_2441097136385557776[12] = -nom_x[12] + true_x[12];
   out_2441097136385557776[13] = -nom_x[13] + true_x[13];
   out_2441097136385557776[14] = -nom_x[14] + true_x[14];
   out_2441097136385557776[15] = -nom_x[15] + true_x[15];
   out_2441097136385557776[16] = -nom_x[16] + true_x[16];
   out_2441097136385557776[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_3208607372720185681) {
   out_3208607372720185681[0] = 1.0;
   out_3208607372720185681[1] = 0.0;
   out_3208607372720185681[2] = 0.0;
   out_3208607372720185681[3] = 0.0;
   out_3208607372720185681[4] = 0.0;
   out_3208607372720185681[5] = 0.0;
   out_3208607372720185681[6] = 0.0;
   out_3208607372720185681[7] = 0.0;
   out_3208607372720185681[8] = 0.0;
   out_3208607372720185681[9] = 0.0;
   out_3208607372720185681[10] = 0.0;
   out_3208607372720185681[11] = 0.0;
   out_3208607372720185681[12] = 0.0;
   out_3208607372720185681[13] = 0.0;
   out_3208607372720185681[14] = 0.0;
   out_3208607372720185681[15] = 0.0;
   out_3208607372720185681[16] = 0.0;
   out_3208607372720185681[17] = 0.0;
   out_3208607372720185681[18] = 0.0;
   out_3208607372720185681[19] = 1.0;
   out_3208607372720185681[20] = 0.0;
   out_3208607372720185681[21] = 0.0;
   out_3208607372720185681[22] = 0.0;
   out_3208607372720185681[23] = 0.0;
   out_3208607372720185681[24] = 0.0;
   out_3208607372720185681[25] = 0.0;
   out_3208607372720185681[26] = 0.0;
   out_3208607372720185681[27] = 0.0;
   out_3208607372720185681[28] = 0.0;
   out_3208607372720185681[29] = 0.0;
   out_3208607372720185681[30] = 0.0;
   out_3208607372720185681[31] = 0.0;
   out_3208607372720185681[32] = 0.0;
   out_3208607372720185681[33] = 0.0;
   out_3208607372720185681[34] = 0.0;
   out_3208607372720185681[35] = 0.0;
   out_3208607372720185681[36] = 0.0;
   out_3208607372720185681[37] = 0.0;
   out_3208607372720185681[38] = 1.0;
   out_3208607372720185681[39] = 0.0;
   out_3208607372720185681[40] = 0.0;
   out_3208607372720185681[41] = 0.0;
   out_3208607372720185681[42] = 0.0;
   out_3208607372720185681[43] = 0.0;
   out_3208607372720185681[44] = 0.0;
   out_3208607372720185681[45] = 0.0;
   out_3208607372720185681[46] = 0.0;
   out_3208607372720185681[47] = 0.0;
   out_3208607372720185681[48] = 0.0;
   out_3208607372720185681[49] = 0.0;
   out_3208607372720185681[50] = 0.0;
   out_3208607372720185681[51] = 0.0;
   out_3208607372720185681[52] = 0.0;
   out_3208607372720185681[53] = 0.0;
   out_3208607372720185681[54] = 0.0;
   out_3208607372720185681[55] = 0.0;
   out_3208607372720185681[56] = 0.0;
   out_3208607372720185681[57] = 1.0;
   out_3208607372720185681[58] = 0.0;
   out_3208607372720185681[59] = 0.0;
   out_3208607372720185681[60] = 0.0;
   out_3208607372720185681[61] = 0.0;
   out_3208607372720185681[62] = 0.0;
   out_3208607372720185681[63] = 0.0;
   out_3208607372720185681[64] = 0.0;
   out_3208607372720185681[65] = 0.0;
   out_3208607372720185681[66] = 0.0;
   out_3208607372720185681[67] = 0.0;
   out_3208607372720185681[68] = 0.0;
   out_3208607372720185681[69] = 0.0;
   out_3208607372720185681[70] = 0.0;
   out_3208607372720185681[71] = 0.0;
   out_3208607372720185681[72] = 0.0;
   out_3208607372720185681[73] = 0.0;
   out_3208607372720185681[74] = 0.0;
   out_3208607372720185681[75] = 0.0;
   out_3208607372720185681[76] = 1.0;
   out_3208607372720185681[77] = 0.0;
   out_3208607372720185681[78] = 0.0;
   out_3208607372720185681[79] = 0.0;
   out_3208607372720185681[80] = 0.0;
   out_3208607372720185681[81] = 0.0;
   out_3208607372720185681[82] = 0.0;
   out_3208607372720185681[83] = 0.0;
   out_3208607372720185681[84] = 0.0;
   out_3208607372720185681[85] = 0.0;
   out_3208607372720185681[86] = 0.0;
   out_3208607372720185681[87] = 0.0;
   out_3208607372720185681[88] = 0.0;
   out_3208607372720185681[89] = 0.0;
   out_3208607372720185681[90] = 0.0;
   out_3208607372720185681[91] = 0.0;
   out_3208607372720185681[92] = 0.0;
   out_3208607372720185681[93] = 0.0;
   out_3208607372720185681[94] = 0.0;
   out_3208607372720185681[95] = 1.0;
   out_3208607372720185681[96] = 0.0;
   out_3208607372720185681[97] = 0.0;
   out_3208607372720185681[98] = 0.0;
   out_3208607372720185681[99] = 0.0;
   out_3208607372720185681[100] = 0.0;
   out_3208607372720185681[101] = 0.0;
   out_3208607372720185681[102] = 0.0;
   out_3208607372720185681[103] = 0.0;
   out_3208607372720185681[104] = 0.0;
   out_3208607372720185681[105] = 0.0;
   out_3208607372720185681[106] = 0.0;
   out_3208607372720185681[107] = 0.0;
   out_3208607372720185681[108] = 0.0;
   out_3208607372720185681[109] = 0.0;
   out_3208607372720185681[110] = 0.0;
   out_3208607372720185681[111] = 0.0;
   out_3208607372720185681[112] = 0.0;
   out_3208607372720185681[113] = 0.0;
   out_3208607372720185681[114] = 1.0;
   out_3208607372720185681[115] = 0.0;
   out_3208607372720185681[116] = 0.0;
   out_3208607372720185681[117] = 0.0;
   out_3208607372720185681[118] = 0.0;
   out_3208607372720185681[119] = 0.0;
   out_3208607372720185681[120] = 0.0;
   out_3208607372720185681[121] = 0.0;
   out_3208607372720185681[122] = 0.0;
   out_3208607372720185681[123] = 0.0;
   out_3208607372720185681[124] = 0.0;
   out_3208607372720185681[125] = 0.0;
   out_3208607372720185681[126] = 0.0;
   out_3208607372720185681[127] = 0.0;
   out_3208607372720185681[128] = 0.0;
   out_3208607372720185681[129] = 0.0;
   out_3208607372720185681[130] = 0.0;
   out_3208607372720185681[131] = 0.0;
   out_3208607372720185681[132] = 0.0;
   out_3208607372720185681[133] = 1.0;
   out_3208607372720185681[134] = 0.0;
   out_3208607372720185681[135] = 0.0;
   out_3208607372720185681[136] = 0.0;
   out_3208607372720185681[137] = 0.0;
   out_3208607372720185681[138] = 0.0;
   out_3208607372720185681[139] = 0.0;
   out_3208607372720185681[140] = 0.0;
   out_3208607372720185681[141] = 0.0;
   out_3208607372720185681[142] = 0.0;
   out_3208607372720185681[143] = 0.0;
   out_3208607372720185681[144] = 0.0;
   out_3208607372720185681[145] = 0.0;
   out_3208607372720185681[146] = 0.0;
   out_3208607372720185681[147] = 0.0;
   out_3208607372720185681[148] = 0.0;
   out_3208607372720185681[149] = 0.0;
   out_3208607372720185681[150] = 0.0;
   out_3208607372720185681[151] = 0.0;
   out_3208607372720185681[152] = 1.0;
   out_3208607372720185681[153] = 0.0;
   out_3208607372720185681[154] = 0.0;
   out_3208607372720185681[155] = 0.0;
   out_3208607372720185681[156] = 0.0;
   out_3208607372720185681[157] = 0.0;
   out_3208607372720185681[158] = 0.0;
   out_3208607372720185681[159] = 0.0;
   out_3208607372720185681[160] = 0.0;
   out_3208607372720185681[161] = 0.0;
   out_3208607372720185681[162] = 0.0;
   out_3208607372720185681[163] = 0.0;
   out_3208607372720185681[164] = 0.0;
   out_3208607372720185681[165] = 0.0;
   out_3208607372720185681[166] = 0.0;
   out_3208607372720185681[167] = 0.0;
   out_3208607372720185681[168] = 0.0;
   out_3208607372720185681[169] = 0.0;
   out_3208607372720185681[170] = 0.0;
   out_3208607372720185681[171] = 1.0;
   out_3208607372720185681[172] = 0.0;
   out_3208607372720185681[173] = 0.0;
   out_3208607372720185681[174] = 0.0;
   out_3208607372720185681[175] = 0.0;
   out_3208607372720185681[176] = 0.0;
   out_3208607372720185681[177] = 0.0;
   out_3208607372720185681[178] = 0.0;
   out_3208607372720185681[179] = 0.0;
   out_3208607372720185681[180] = 0.0;
   out_3208607372720185681[181] = 0.0;
   out_3208607372720185681[182] = 0.0;
   out_3208607372720185681[183] = 0.0;
   out_3208607372720185681[184] = 0.0;
   out_3208607372720185681[185] = 0.0;
   out_3208607372720185681[186] = 0.0;
   out_3208607372720185681[187] = 0.0;
   out_3208607372720185681[188] = 0.0;
   out_3208607372720185681[189] = 0.0;
   out_3208607372720185681[190] = 1.0;
   out_3208607372720185681[191] = 0.0;
   out_3208607372720185681[192] = 0.0;
   out_3208607372720185681[193] = 0.0;
   out_3208607372720185681[194] = 0.0;
   out_3208607372720185681[195] = 0.0;
   out_3208607372720185681[196] = 0.0;
   out_3208607372720185681[197] = 0.0;
   out_3208607372720185681[198] = 0.0;
   out_3208607372720185681[199] = 0.0;
   out_3208607372720185681[200] = 0.0;
   out_3208607372720185681[201] = 0.0;
   out_3208607372720185681[202] = 0.0;
   out_3208607372720185681[203] = 0.0;
   out_3208607372720185681[204] = 0.0;
   out_3208607372720185681[205] = 0.0;
   out_3208607372720185681[206] = 0.0;
   out_3208607372720185681[207] = 0.0;
   out_3208607372720185681[208] = 0.0;
   out_3208607372720185681[209] = 1.0;
   out_3208607372720185681[210] = 0.0;
   out_3208607372720185681[211] = 0.0;
   out_3208607372720185681[212] = 0.0;
   out_3208607372720185681[213] = 0.0;
   out_3208607372720185681[214] = 0.0;
   out_3208607372720185681[215] = 0.0;
   out_3208607372720185681[216] = 0.0;
   out_3208607372720185681[217] = 0.0;
   out_3208607372720185681[218] = 0.0;
   out_3208607372720185681[219] = 0.0;
   out_3208607372720185681[220] = 0.0;
   out_3208607372720185681[221] = 0.0;
   out_3208607372720185681[222] = 0.0;
   out_3208607372720185681[223] = 0.0;
   out_3208607372720185681[224] = 0.0;
   out_3208607372720185681[225] = 0.0;
   out_3208607372720185681[226] = 0.0;
   out_3208607372720185681[227] = 0.0;
   out_3208607372720185681[228] = 1.0;
   out_3208607372720185681[229] = 0.0;
   out_3208607372720185681[230] = 0.0;
   out_3208607372720185681[231] = 0.0;
   out_3208607372720185681[232] = 0.0;
   out_3208607372720185681[233] = 0.0;
   out_3208607372720185681[234] = 0.0;
   out_3208607372720185681[235] = 0.0;
   out_3208607372720185681[236] = 0.0;
   out_3208607372720185681[237] = 0.0;
   out_3208607372720185681[238] = 0.0;
   out_3208607372720185681[239] = 0.0;
   out_3208607372720185681[240] = 0.0;
   out_3208607372720185681[241] = 0.0;
   out_3208607372720185681[242] = 0.0;
   out_3208607372720185681[243] = 0.0;
   out_3208607372720185681[244] = 0.0;
   out_3208607372720185681[245] = 0.0;
   out_3208607372720185681[246] = 0.0;
   out_3208607372720185681[247] = 1.0;
   out_3208607372720185681[248] = 0.0;
   out_3208607372720185681[249] = 0.0;
   out_3208607372720185681[250] = 0.0;
   out_3208607372720185681[251] = 0.0;
   out_3208607372720185681[252] = 0.0;
   out_3208607372720185681[253] = 0.0;
   out_3208607372720185681[254] = 0.0;
   out_3208607372720185681[255] = 0.0;
   out_3208607372720185681[256] = 0.0;
   out_3208607372720185681[257] = 0.0;
   out_3208607372720185681[258] = 0.0;
   out_3208607372720185681[259] = 0.0;
   out_3208607372720185681[260] = 0.0;
   out_3208607372720185681[261] = 0.0;
   out_3208607372720185681[262] = 0.0;
   out_3208607372720185681[263] = 0.0;
   out_3208607372720185681[264] = 0.0;
   out_3208607372720185681[265] = 0.0;
   out_3208607372720185681[266] = 1.0;
   out_3208607372720185681[267] = 0.0;
   out_3208607372720185681[268] = 0.0;
   out_3208607372720185681[269] = 0.0;
   out_3208607372720185681[270] = 0.0;
   out_3208607372720185681[271] = 0.0;
   out_3208607372720185681[272] = 0.0;
   out_3208607372720185681[273] = 0.0;
   out_3208607372720185681[274] = 0.0;
   out_3208607372720185681[275] = 0.0;
   out_3208607372720185681[276] = 0.0;
   out_3208607372720185681[277] = 0.0;
   out_3208607372720185681[278] = 0.0;
   out_3208607372720185681[279] = 0.0;
   out_3208607372720185681[280] = 0.0;
   out_3208607372720185681[281] = 0.0;
   out_3208607372720185681[282] = 0.0;
   out_3208607372720185681[283] = 0.0;
   out_3208607372720185681[284] = 0.0;
   out_3208607372720185681[285] = 1.0;
   out_3208607372720185681[286] = 0.0;
   out_3208607372720185681[287] = 0.0;
   out_3208607372720185681[288] = 0.0;
   out_3208607372720185681[289] = 0.0;
   out_3208607372720185681[290] = 0.0;
   out_3208607372720185681[291] = 0.0;
   out_3208607372720185681[292] = 0.0;
   out_3208607372720185681[293] = 0.0;
   out_3208607372720185681[294] = 0.0;
   out_3208607372720185681[295] = 0.0;
   out_3208607372720185681[296] = 0.0;
   out_3208607372720185681[297] = 0.0;
   out_3208607372720185681[298] = 0.0;
   out_3208607372720185681[299] = 0.0;
   out_3208607372720185681[300] = 0.0;
   out_3208607372720185681[301] = 0.0;
   out_3208607372720185681[302] = 0.0;
   out_3208607372720185681[303] = 0.0;
   out_3208607372720185681[304] = 1.0;
   out_3208607372720185681[305] = 0.0;
   out_3208607372720185681[306] = 0.0;
   out_3208607372720185681[307] = 0.0;
   out_3208607372720185681[308] = 0.0;
   out_3208607372720185681[309] = 0.0;
   out_3208607372720185681[310] = 0.0;
   out_3208607372720185681[311] = 0.0;
   out_3208607372720185681[312] = 0.0;
   out_3208607372720185681[313] = 0.0;
   out_3208607372720185681[314] = 0.0;
   out_3208607372720185681[315] = 0.0;
   out_3208607372720185681[316] = 0.0;
   out_3208607372720185681[317] = 0.0;
   out_3208607372720185681[318] = 0.0;
   out_3208607372720185681[319] = 0.0;
   out_3208607372720185681[320] = 0.0;
   out_3208607372720185681[321] = 0.0;
   out_3208607372720185681[322] = 0.0;
   out_3208607372720185681[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_6464550598295734542) {
   out_6464550598295734542[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_6464550598295734542[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_6464550598295734542[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_6464550598295734542[3] = dt*state[12] + state[3];
   out_6464550598295734542[4] = dt*state[13] + state[4];
   out_6464550598295734542[5] = dt*state[14] + state[5];
   out_6464550598295734542[6] = state[6];
   out_6464550598295734542[7] = state[7];
   out_6464550598295734542[8] = state[8];
   out_6464550598295734542[9] = state[9];
   out_6464550598295734542[10] = state[10];
   out_6464550598295734542[11] = state[11];
   out_6464550598295734542[12] = state[12];
   out_6464550598295734542[13] = state[13];
   out_6464550598295734542[14] = state[14];
   out_6464550598295734542[15] = state[15];
   out_6464550598295734542[16] = state[16];
   out_6464550598295734542[17] = state[17];
}
void F_fun(double *state, double dt, double *out_4669200903438633107) {
   out_4669200903438633107[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4669200903438633107[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4669200903438633107[2] = 0;
   out_4669200903438633107[3] = 0;
   out_4669200903438633107[4] = 0;
   out_4669200903438633107[5] = 0;
   out_4669200903438633107[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4669200903438633107[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4669200903438633107[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4669200903438633107[9] = 0;
   out_4669200903438633107[10] = 0;
   out_4669200903438633107[11] = 0;
   out_4669200903438633107[12] = 0;
   out_4669200903438633107[13] = 0;
   out_4669200903438633107[14] = 0;
   out_4669200903438633107[15] = 0;
   out_4669200903438633107[16] = 0;
   out_4669200903438633107[17] = 0;
   out_4669200903438633107[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4669200903438633107[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4669200903438633107[20] = 0;
   out_4669200903438633107[21] = 0;
   out_4669200903438633107[22] = 0;
   out_4669200903438633107[23] = 0;
   out_4669200903438633107[24] = 0;
   out_4669200903438633107[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4669200903438633107[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4669200903438633107[27] = 0;
   out_4669200903438633107[28] = 0;
   out_4669200903438633107[29] = 0;
   out_4669200903438633107[30] = 0;
   out_4669200903438633107[31] = 0;
   out_4669200903438633107[32] = 0;
   out_4669200903438633107[33] = 0;
   out_4669200903438633107[34] = 0;
   out_4669200903438633107[35] = 0;
   out_4669200903438633107[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4669200903438633107[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4669200903438633107[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4669200903438633107[39] = 0;
   out_4669200903438633107[40] = 0;
   out_4669200903438633107[41] = 0;
   out_4669200903438633107[42] = 0;
   out_4669200903438633107[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4669200903438633107[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4669200903438633107[45] = 0;
   out_4669200903438633107[46] = 0;
   out_4669200903438633107[47] = 0;
   out_4669200903438633107[48] = 0;
   out_4669200903438633107[49] = 0;
   out_4669200903438633107[50] = 0;
   out_4669200903438633107[51] = 0;
   out_4669200903438633107[52] = 0;
   out_4669200903438633107[53] = 0;
   out_4669200903438633107[54] = 0;
   out_4669200903438633107[55] = 0;
   out_4669200903438633107[56] = 0;
   out_4669200903438633107[57] = 1;
   out_4669200903438633107[58] = 0;
   out_4669200903438633107[59] = 0;
   out_4669200903438633107[60] = 0;
   out_4669200903438633107[61] = 0;
   out_4669200903438633107[62] = 0;
   out_4669200903438633107[63] = 0;
   out_4669200903438633107[64] = 0;
   out_4669200903438633107[65] = 0;
   out_4669200903438633107[66] = dt;
   out_4669200903438633107[67] = 0;
   out_4669200903438633107[68] = 0;
   out_4669200903438633107[69] = 0;
   out_4669200903438633107[70] = 0;
   out_4669200903438633107[71] = 0;
   out_4669200903438633107[72] = 0;
   out_4669200903438633107[73] = 0;
   out_4669200903438633107[74] = 0;
   out_4669200903438633107[75] = 0;
   out_4669200903438633107[76] = 1;
   out_4669200903438633107[77] = 0;
   out_4669200903438633107[78] = 0;
   out_4669200903438633107[79] = 0;
   out_4669200903438633107[80] = 0;
   out_4669200903438633107[81] = 0;
   out_4669200903438633107[82] = 0;
   out_4669200903438633107[83] = 0;
   out_4669200903438633107[84] = 0;
   out_4669200903438633107[85] = dt;
   out_4669200903438633107[86] = 0;
   out_4669200903438633107[87] = 0;
   out_4669200903438633107[88] = 0;
   out_4669200903438633107[89] = 0;
   out_4669200903438633107[90] = 0;
   out_4669200903438633107[91] = 0;
   out_4669200903438633107[92] = 0;
   out_4669200903438633107[93] = 0;
   out_4669200903438633107[94] = 0;
   out_4669200903438633107[95] = 1;
   out_4669200903438633107[96] = 0;
   out_4669200903438633107[97] = 0;
   out_4669200903438633107[98] = 0;
   out_4669200903438633107[99] = 0;
   out_4669200903438633107[100] = 0;
   out_4669200903438633107[101] = 0;
   out_4669200903438633107[102] = 0;
   out_4669200903438633107[103] = 0;
   out_4669200903438633107[104] = dt;
   out_4669200903438633107[105] = 0;
   out_4669200903438633107[106] = 0;
   out_4669200903438633107[107] = 0;
   out_4669200903438633107[108] = 0;
   out_4669200903438633107[109] = 0;
   out_4669200903438633107[110] = 0;
   out_4669200903438633107[111] = 0;
   out_4669200903438633107[112] = 0;
   out_4669200903438633107[113] = 0;
   out_4669200903438633107[114] = 1;
   out_4669200903438633107[115] = 0;
   out_4669200903438633107[116] = 0;
   out_4669200903438633107[117] = 0;
   out_4669200903438633107[118] = 0;
   out_4669200903438633107[119] = 0;
   out_4669200903438633107[120] = 0;
   out_4669200903438633107[121] = 0;
   out_4669200903438633107[122] = 0;
   out_4669200903438633107[123] = 0;
   out_4669200903438633107[124] = 0;
   out_4669200903438633107[125] = 0;
   out_4669200903438633107[126] = 0;
   out_4669200903438633107[127] = 0;
   out_4669200903438633107[128] = 0;
   out_4669200903438633107[129] = 0;
   out_4669200903438633107[130] = 0;
   out_4669200903438633107[131] = 0;
   out_4669200903438633107[132] = 0;
   out_4669200903438633107[133] = 1;
   out_4669200903438633107[134] = 0;
   out_4669200903438633107[135] = 0;
   out_4669200903438633107[136] = 0;
   out_4669200903438633107[137] = 0;
   out_4669200903438633107[138] = 0;
   out_4669200903438633107[139] = 0;
   out_4669200903438633107[140] = 0;
   out_4669200903438633107[141] = 0;
   out_4669200903438633107[142] = 0;
   out_4669200903438633107[143] = 0;
   out_4669200903438633107[144] = 0;
   out_4669200903438633107[145] = 0;
   out_4669200903438633107[146] = 0;
   out_4669200903438633107[147] = 0;
   out_4669200903438633107[148] = 0;
   out_4669200903438633107[149] = 0;
   out_4669200903438633107[150] = 0;
   out_4669200903438633107[151] = 0;
   out_4669200903438633107[152] = 1;
   out_4669200903438633107[153] = 0;
   out_4669200903438633107[154] = 0;
   out_4669200903438633107[155] = 0;
   out_4669200903438633107[156] = 0;
   out_4669200903438633107[157] = 0;
   out_4669200903438633107[158] = 0;
   out_4669200903438633107[159] = 0;
   out_4669200903438633107[160] = 0;
   out_4669200903438633107[161] = 0;
   out_4669200903438633107[162] = 0;
   out_4669200903438633107[163] = 0;
   out_4669200903438633107[164] = 0;
   out_4669200903438633107[165] = 0;
   out_4669200903438633107[166] = 0;
   out_4669200903438633107[167] = 0;
   out_4669200903438633107[168] = 0;
   out_4669200903438633107[169] = 0;
   out_4669200903438633107[170] = 0;
   out_4669200903438633107[171] = 1;
   out_4669200903438633107[172] = 0;
   out_4669200903438633107[173] = 0;
   out_4669200903438633107[174] = 0;
   out_4669200903438633107[175] = 0;
   out_4669200903438633107[176] = 0;
   out_4669200903438633107[177] = 0;
   out_4669200903438633107[178] = 0;
   out_4669200903438633107[179] = 0;
   out_4669200903438633107[180] = 0;
   out_4669200903438633107[181] = 0;
   out_4669200903438633107[182] = 0;
   out_4669200903438633107[183] = 0;
   out_4669200903438633107[184] = 0;
   out_4669200903438633107[185] = 0;
   out_4669200903438633107[186] = 0;
   out_4669200903438633107[187] = 0;
   out_4669200903438633107[188] = 0;
   out_4669200903438633107[189] = 0;
   out_4669200903438633107[190] = 1;
   out_4669200903438633107[191] = 0;
   out_4669200903438633107[192] = 0;
   out_4669200903438633107[193] = 0;
   out_4669200903438633107[194] = 0;
   out_4669200903438633107[195] = 0;
   out_4669200903438633107[196] = 0;
   out_4669200903438633107[197] = 0;
   out_4669200903438633107[198] = 0;
   out_4669200903438633107[199] = 0;
   out_4669200903438633107[200] = 0;
   out_4669200903438633107[201] = 0;
   out_4669200903438633107[202] = 0;
   out_4669200903438633107[203] = 0;
   out_4669200903438633107[204] = 0;
   out_4669200903438633107[205] = 0;
   out_4669200903438633107[206] = 0;
   out_4669200903438633107[207] = 0;
   out_4669200903438633107[208] = 0;
   out_4669200903438633107[209] = 1;
   out_4669200903438633107[210] = 0;
   out_4669200903438633107[211] = 0;
   out_4669200903438633107[212] = 0;
   out_4669200903438633107[213] = 0;
   out_4669200903438633107[214] = 0;
   out_4669200903438633107[215] = 0;
   out_4669200903438633107[216] = 0;
   out_4669200903438633107[217] = 0;
   out_4669200903438633107[218] = 0;
   out_4669200903438633107[219] = 0;
   out_4669200903438633107[220] = 0;
   out_4669200903438633107[221] = 0;
   out_4669200903438633107[222] = 0;
   out_4669200903438633107[223] = 0;
   out_4669200903438633107[224] = 0;
   out_4669200903438633107[225] = 0;
   out_4669200903438633107[226] = 0;
   out_4669200903438633107[227] = 0;
   out_4669200903438633107[228] = 1;
   out_4669200903438633107[229] = 0;
   out_4669200903438633107[230] = 0;
   out_4669200903438633107[231] = 0;
   out_4669200903438633107[232] = 0;
   out_4669200903438633107[233] = 0;
   out_4669200903438633107[234] = 0;
   out_4669200903438633107[235] = 0;
   out_4669200903438633107[236] = 0;
   out_4669200903438633107[237] = 0;
   out_4669200903438633107[238] = 0;
   out_4669200903438633107[239] = 0;
   out_4669200903438633107[240] = 0;
   out_4669200903438633107[241] = 0;
   out_4669200903438633107[242] = 0;
   out_4669200903438633107[243] = 0;
   out_4669200903438633107[244] = 0;
   out_4669200903438633107[245] = 0;
   out_4669200903438633107[246] = 0;
   out_4669200903438633107[247] = 1;
   out_4669200903438633107[248] = 0;
   out_4669200903438633107[249] = 0;
   out_4669200903438633107[250] = 0;
   out_4669200903438633107[251] = 0;
   out_4669200903438633107[252] = 0;
   out_4669200903438633107[253] = 0;
   out_4669200903438633107[254] = 0;
   out_4669200903438633107[255] = 0;
   out_4669200903438633107[256] = 0;
   out_4669200903438633107[257] = 0;
   out_4669200903438633107[258] = 0;
   out_4669200903438633107[259] = 0;
   out_4669200903438633107[260] = 0;
   out_4669200903438633107[261] = 0;
   out_4669200903438633107[262] = 0;
   out_4669200903438633107[263] = 0;
   out_4669200903438633107[264] = 0;
   out_4669200903438633107[265] = 0;
   out_4669200903438633107[266] = 1;
   out_4669200903438633107[267] = 0;
   out_4669200903438633107[268] = 0;
   out_4669200903438633107[269] = 0;
   out_4669200903438633107[270] = 0;
   out_4669200903438633107[271] = 0;
   out_4669200903438633107[272] = 0;
   out_4669200903438633107[273] = 0;
   out_4669200903438633107[274] = 0;
   out_4669200903438633107[275] = 0;
   out_4669200903438633107[276] = 0;
   out_4669200903438633107[277] = 0;
   out_4669200903438633107[278] = 0;
   out_4669200903438633107[279] = 0;
   out_4669200903438633107[280] = 0;
   out_4669200903438633107[281] = 0;
   out_4669200903438633107[282] = 0;
   out_4669200903438633107[283] = 0;
   out_4669200903438633107[284] = 0;
   out_4669200903438633107[285] = 1;
   out_4669200903438633107[286] = 0;
   out_4669200903438633107[287] = 0;
   out_4669200903438633107[288] = 0;
   out_4669200903438633107[289] = 0;
   out_4669200903438633107[290] = 0;
   out_4669200903438633107[291] = 0;
   out_4669200903438633107[292] = 0;
   out_4669200903438633107[293] = 0;
   out_4669200903438633107[294] = 0;
   out_4669200903438633107[295] = 0;
   out_4669200903438633107[296] = 0;
   out_4669200903438633107[297] = 0;
   out_4669200903438633107[298] = 0;
   out_4669200903438633107[299] = 0;
   out_4669200903438633107[300] = 0;
   out_4669200903438633107[301] = 0;
   out_4669200903438633107[302] = 0;
   out_4669200903438633107[303] = 0;
   out_4669200903438633107[304] = 1;
   out_4669200903438633107[305] = 0;
   out_4669200903438633107[306] = 0;
   out_4669200903438633107[307] = 0;
   out_4669200903438633107[308] = 0;
   out_4669200903438633107[309] = 0;
   out_4669200903438633107[310] = 0;
   out_4669200903438633107[311] = 0;
   out_4669200903438633107[312] = 0;
   out_4669200903438633107[313] = 0;
   out_4669200903438633107[314] = 0;
   out_4669200903438633107[315] = 0;
   out_4669200903438633107[316] = 0;
   out_4669200903438633107[317] = 0;
   out_4669200903438633107[318] = 0;
   out_4669200903438633107[319] = 0;
   out_4669200903438633107[320] = 0;
   out_4669200903438633107[321] = 0;
   out_4669200903438633107[322] = 0;
   out_4669200903438633107[323] = 1;
}
void h_4(double *state, double *unused, double *out_5330802774113093904) {
   out_5330802774113093904[0] = state[6] + state[9];
   out_5330802774113093904[1] = state[7] + state[10];
   out_5330802774113093904[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_2454446370665473074) {
   out_2454446370665473074[0] = 0;
   out_2454446370665473074[1] = 0;
   out_2454446370665473074[2] = 0;
   out_2454446370665473074[3] = 0;
   out_2454446370665473074[4] = 0;
   out_2454446370665473074[5] = 0;
   out_2454446370665473074[6] = 1;
   out_2454446370665473074[7] = 0;
   out_2454446370665473074[8] = 0;
   out_2454446370665473074[9] = 1;
   out_2454446370665473074[10] = 0;
   out_2454446370665473074[11] = 0;
   out_2454446370665473074[12] = 0;
   out_2454446370665473074[13] = 0;
   out_2454446370665473074[14] = 0;
   out_2454446370665473074[15] = 0;
   out_2454446370665473074[16] = 0;
   out_2454446370665473074[17] = 0;
   out_2454446370665473074[18] = 0;
   out_2454446370665473074[19] = 0;
   out_2454446370665473074[20] = 0;
   out_2454446370665473074[21] = 0;
   out_2454446370665473074[22] = 0;
   out_2454446370665473074[23] = 0;
   out_2454446370665473074[24] = 0;
   out_2454446370665473074[25] = 1;
   out_2454446370665473074[26] = 0;
   out_2454446370665473074[27] = 0;
   out_2454446370665473074[28] = 1;
   out_2454446370665473074[29] = 0;
   out_2454446370665473074[30] = 0;
   out_2454446370665473074[31] = 0;
   out_2454446370665473074[32] = 0;
   out_2454446370665473074[33] = 0;
   out_2454446370665473074[34] = 0;
   out_2454446370665473074[35] = 0;
   out_2454446370665473074[36] = 0;
   out_2454446370665473074[37] = 0;
   out_2454446370665473074[38] = 0;
   out_2454446370665473074[39] = 0;
   out_2454446370665473074[40] = 0;
   out_2454446370665473074[41] = 0;
   out_2454446370665473074[42] = 0;
   out_2454446370665473074[43] = 0;
   out_2454446370665473074[44] = 1;
   out_2454446370665473074[45] = 0;
   out_2454446370665473074[46] = 0;
   out_2454446370665473074[47] = 1;
   out_2454446370665473074[48] = 0;
   out_2454446370665473074[49] = 0;
   out_2454446370665473074[50] = 0;
   out_2454446370665473074[51] = 0;
   out_2454446370665473074[52] = 0;
   out_2454446370665473074[53] = 0;
}
void h_10(double *state, double *unused, double *out_6487917254077376695) {
   out_6487917254077376695[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_6487917254077376695[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_6487917254077376695[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_63197332932821273) {
   out_63197332932821273[0] = 0;
   out_63197332932821273[1] = 9.8100000000000005*cos(state[1]);
   out_63197332932821273[2] = 0;
   out_63197332932821273[3] = 0;
   out_63197332932821273[4] = -state[8];
   out_63197332932821273[5] = state[7];
   out_63197332932821273[6] = 0;
   out_63197332932821273[7] = state[5];
   out_63197332932821273[8] = -state[4];
   out_63197332932821273[9] = 0;
   out_63197332932821273[10] = 0;
   out_63197332932821273[11] = 0;
   out_63197332932821273[12] = 1;
   out_63197332932821273[13] = 0;
   out_63197332932821273[14] = 0;
   out_63197332932821273[15] = 1;
   out_63197332932821273[16] = 0;
   out_63197332932821273[17] = 0;
   out_63197332932821273[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_63197332932821273[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_63197332932821273[20] = 0;
   out_63197332932821273[21] = state[8];
   out_63197332932821273[22] = 0;
   out_63197332932821273[23] = -state[6];
   out_63197332932821273[24] = -state[5];
   out_63197332932821273[25] = 0;
   out_63197332932821273[26] = state[3];
   out_63197332932821273[27] = 0;
   out_63197332932821273[28] = 0;
   out_63197332932821273[29] = 0;
   out_63197332932821273[30] = 0;
   out_63197332932821273[31] = 1;
   out_63197332932821273[32] = 0;
   out_63197332932821273[33] = 0;
   out_63197332932821273[34] = 1;
   out_63197332932821273[35] = 0;
   out_63197332932821273[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_63197332932821273[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_63197332932821273[38] = 0;
   out_63197332932821273[39] = -state[7];
   out_63197332932821273[40] = state[6];
   out_63197332932821273[41] = 0;
   out_63197332932821273[42] = state[4];
   out_63197332932821273[43] = -state[3];
   out_63197332932821273[44] = 0;
   out_63197332932821273[45] = 0;
   out_63197332932821273[46] = 0;
   out_63197332932821273[47] = 0;
   out_63197332932821273[48] = 0;
   out_63197332932821273[49] = 0;
   out_63197332932821273[50] = 1;
   out_63197332932821273[51] = 0;
   out_63197332932821273[52] = 0;
   out_63197332932821273[53] = 1;
}
void h_13(double *state, double *unused, double *out_7869262712734505763) {
   out_7869262712734505763[0] = state[3];
   out_7869262712734505763[1] = state[4];
   out_7869262712734505763[2] = state[5];
}
void H_13(double *state, double *unused, double *out_5156184837651227855) {
   out_5156184837651227855[0] = 0;
   out_5156184837651227855[1] = 0;
   out_5156184837651227855[2] = 0;
   out_5156184837651227855[3] = 1;
   out_5156184837651227855[4] = 0;
   out_5156184837651227855[5] = 0;
   out_5156184837651227855[6] = 0;
   out_5156184837651227855[7] = 0;
   out_5156184837651227855[8] = 0;
   out_5156184837651227855[9] = 0;
   out_5156184837651227855[10] = 0;
   out_5156184837651227855[11] = 0;
   out_5156184837651227855[12] = 0;
   out_5156184837651227855[13] = 0;
   out_5156184837651227855[14] = 0;
   out_5156184837651227855[15] = 0;
   out_5156184837651227855[16] = 0;
   out_5156184837651227855[17] = 0;
   out_5156184837651227855[18] = 0;
   out_5156184837651227855[19] = 0;
   out_5156184837651227855[20] = 0;
   out_5156184837651227855[21] = 0;
   out_5156184837651227855[22] = 1;
   out_5156184837651227855[23] = 0;
   out_5156184837651227855[24] = 0;
   out_5156184837651227855[25] = 0;
   out_5156184837651227855[26] = 0;
   out_5156184837651227855[27] = 0;
   out_5156184837651227855[28] = 0;
   out_5156184837651227855[29] = 0;
   out_5156184837651227855[30] = 0;
   out_5156184837651227855[31] = 0;
   out_5156184837651227855[32] = 0;
   out_5156184837651227855[33] = 0;
   out_5156184837651227855[34] = 0;
   out_5156184837651227855[35] = 0;
   out_5156184837651227855[36] = 0;
   out_5156184837651227855[37] = 0;
   out_5156184837651227855[38] = 0;
   out_5156184837651227855[39] = 0;
   out_5156184837651227855[40] = 0;
   out_5156184837651227855[41] = 1;
   out_5156184837651227855[42] = 0;
   out_5156184837651227855[43] = 0;
   out_5156184837651227855[44] = 0;
   out_5156184837651227855[45] = 0;
   out_5156184837651227855[46] = 0;
   out_5156184837651227855[47] = 0;
   out_5156184837651227855[48] = 0;
   out_5156184837651227855[49] = 0;
   out_5156184837651227855[50] = 0;
   out_5156184837651227855[51] = 0;
   out_5156184837651227855[52] = 0;
   out_5156184837651227855[53] = 0;
}
void h_14(double *state, double *unused, double *out_3486024826794941099) {
   out_3486024826794941099[0] = state[6];
   out_3486024826794941099[1] = state[7];
   out_3486024826794941099[2] = state[8];
}
void H_14(double *state, double *unused, double *out_5537234802960845370) {
   out_5537234802960845370[0] = 0;
   out_5537234802960845370[1] = 0;
   out_5537234802960845370[2] = 0;
   out_5537234802960845370[3] = 0;
   out_5537234802960845370[4] = 0;
   out_5537234802960845370[5] = 0;
   out_5537234802960845370[6] = 1;
   out_5537234802960845370[7] = 0;
   out_5537234802960845370[8] = 0;
   out_5537234802960845370[9] = 0;
   out_5537234802960845370[10] = 0;
   out_5537234802960845370[11] = 0;
   out_5537234802960845370[12] = 0;
   out_5537234802960845370[13] = 0;
   out_5537234802960845370[14] = 0;
   out_5537234802960845370[15] = 0;
   out_5537234802960845370[16] = 0;
   out_5537234802960845370[17] = 0;
   out_5537234802960845370[18] = 0;
   out_5537234802960845370[19] = 0;
   out_5537234802960845370[20] = 0;
   out_5537234802960845370[21] = 0;
   out_5537234802960845370[22] = 0;
   out_5537234802960845370[23] = 0;
   out_5537234802960845370[24] = 0;
   out_5537234802960845370[25] = 1;
   out_5537234802960845370[26] = 0;
   out_5537234802960845370[27] = 0;
   out_5537234802960845370[28] = 0;
   out_5537234802960845370[29] = 0;
   out_5537234802960845370[30] = 0;
   out_5537234802960845370[31] = 0;
   out_5537234802960845370[32] = 0;
   out_5537234802960845370[33] = 0;
   out_5537234802960845370[34] = 0;
   out_5537234802960845370[35] = 0;
   out_5537234802960845370[36] = 0;
   out_5537234802960845370[37] = 0;
   out_5537234802960845370[38] = 0;
   out_5537234802960845370[39] = 0;
   out_5537234802960845370[40] = 0;
   out_5537234802960845370[41] = 0;
   out_5537234802960845370[42] = 0;
   out_5537234802960845370[43] = 0;
   out_5537234802960845370[44] = 1;
   out_5537234802960845370[45] = 0;
   out_5537234802960845370[46] = 0;
   out_5537234802960845370[47] = 0;
   out_5537234802960845370[48] = 0;
   out_5537234802960845370[49] = 0;
   out_5537234802960845370[50] = 0;
   out_5537234802960845370[51] = 0;
   out_5537234802960845370[52] = 0;
   out_5537234802960845370[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_7099155186075195158) {
  err_fun(nom_x, delta_x, out_7099155186075195158);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2441097136385557776) {
  inv_err_fun(nom_x, true_x, out_2441097136385557776);
}
void pose_H_mod_fun(double *state, double *out_3208607372720185681) {
  H_mod_fun(state, out_3208607372720185681);
}
void pose_f_fun(double *state, double dt, double *out_6464550598295734542) {
  f_fun(state,  dt, out_6464550598295734542);
}
void pose_F_fun(double *state, double dt, double *out_4669200903438633107) {
  F_fun(state,  dt, out_4669200903438633107);
}
void pose_h_4(double *state, double *unused, double *out_5330802774113093904) {
  h_4(state, unused, out_5330802774113093904);
}
void pose_H_4(double *state, double *unused, double *out_2454446370665473074) {
  H_4(state, unused, out_2454446370665473074);
}
void pose_h_10(double *state, double *unused, double *out_6487917254077376695) {
  h_10(state, unused, out_6487917254077376695);
}
void pose_H_10(double *state, double *unused, double *out_63197332932821273) {
  H_10(state, unused, out_63197332932821273);
}
void pose_h_13(double *state, double *unused, double *out_7869262712734505763) {
  h_13(state, unused, out_7869262712734505763);
}
void pose_H_13(double *state, double *unused, double *out_5156184837651227855) {
  H_13(state, unused, out_5156184837651227855);
}
void pose_h_14(double *state, double *unused, double *out_3486024826794941099) {
  h_14(state, unused, out_3486024826794941099);
}
void pose_H_14(double *state, double *unused, double *out_5537234802960845370) {
  H_14(state, unused, out_5537234802960845370);
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

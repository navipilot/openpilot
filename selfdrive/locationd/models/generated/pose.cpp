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
void err_fun(double *nom_x, double *delta_x, double *out_3060982030804230470) {
   out_3060982030804230470[0] = delta_x[0] + nom_x[0];
   out_3060982030804230470[1] = delta_x[1] + nom_x[1];
   out_3060982030804230470[2] = delta_x[2] + nom_x[2];
   out_3060982030804230470[3] = delta_x[3] + nom_x[3];
   out_3060982030804230470[4] = delta_x[4] + nom_x[4];
   out_3060982030804230470[5] = delta_x[5] + nom_x[5];
   out_3060982030804230470[6] = delta_x[6] + nom_x[6];
   out_3060982030804230470[7] = delta_x[7] + nom_x[7];
   out_3060982030804230470[8] = delta_x[8] + nom_x[8];
   out_3060982030804230470[9] = delta_x[9] + nom_x[9];
   out_3060982030804230470[10] = delta_x[10] + nom_x[10];
   out_3060982030804230470[11] = delta_x[11] + nom_x[11];
   out_3060982030804230470[12] = delta_x[12] + nom_x[12];
   out_3060982030804230470[13] = delta_x[13] + nom_x[13];
   out_3060982030804230470[14] = delta_x[14] + nom_x[14];
   out_3060982030804230470[15] = delta_x[15] + nom_x[15];
   out_3060982030804230470[16] = delta_x[16] + nom_x[16];
   out_3060982030804230470[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1893009693429340384) {
   out_1893009693429340384[0] = -nom_x[0] + true_x[0];
   out_1893009693429340384[1] = -nom_x[1] + true_x[1];
   out_1893009693429340384[2] = -nom_x[2] + true_x[2];
   out_1893009693429340384[3] = -nom_x[3] + true_x[3];
   out_1893009693429340384[4] = -nom_x[4] + true_x[4];
   out_1893009693429340384[5] = -nom_x[5] + true_x[5];
   out_1893009693429340384[6] = -nom_x[6] + true_x[6];
   out_1893009693429340384[7] = -nom_x[7] + true_x[7];
   out_1893009693429340384[8] = -nom_x[8] + true_x[8];
   out_1893009693429340384[9] = -nom_x[9] + true_x[9];
   out_1893009693429340384[10] = -nom_x[10] + true_x[10];
   out_1893009693429340384[11] = -nom_x[11] + true_x[11];
   out_1893009693429340384[12] = -nom_x[12] + true_x[12];
   out_1893009693429340384[13] = -nom_x[13] + true_x[13];
   out_1893009693429340384[14] = -nom_x[14] + true_x[14];
   out_1893009693429340384[15] = -nom_x[15] + true_x[15];
   out_1893009693429340384[16] = -nom_x[16] + true_x[16];
   out_1893009693429340384[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_8923486826760505000) {
   out_8923486826760505000[0] = 1.0;
   out_8923486826760505000[1] = 0.0;
   out_8923486826760505000[2] = 0.0;
   out_8923486826760505000[3] = 0.0;
   out_8923486826760505000[4] = 0.0;
   out_8923486826760505000[5] = 0.0;
   out_8923486826760505000[6] = 0.0;
   out_8923486826760505000[7] = 0.0;
   out_8923486826760505000[8] = 0.0;
   out_8923486826760505000[9] = 0.0;
   out_8923486826760505000[10] = 0.0;
   out_8923486826760505000[11] = 0.0;
   out_8923486826760505000[12] = 0.0;
   out_8923486826760505000[13] = 0.0;
   out_8923486826760505000[14] = 0.0;
   out_8923486826760505000[15] = 0.0;
   out_8923486826760505000[16] = 0.0;
   out_8923486826760505000[17] = 0.0;
   out_8923486826760505000[18] = 0.0;
   out_8923486826760505000[19] = 1.0;
   out_8923486826760505000[20] = 0.0;
   out_8923486826760505000[21] = 0.0;
   out_8923486826760505000[22] = 0.0;
   out_8923486826760505000[23] = 0.0;
   out_8923486826760505000[24] = 0.0;
   out_8923486826760505000[25] = 0.0;
   out_8923486826760505000[26] = 0.0;
   out_8923486826760505000[27] = 0.0;
   out_8923486826760505000[28] = 0.0;
   out_8923486826760505000[29] = 0.0;
   out_8923486826760505000[30] = 0.0;
   out_8923486826760505000[31] = 0.0;
   out_8923486826760505000[32] = 0.0;
   out_8923486826760505000[33] = 0.0;
   out_8923486826760505000[34] = 0.0;
   out_8923486826760505000[35] = 0.0;
   out_8923486826760505000[36] = 0.0;
   out_8923486826760505000[37] = 0.0;
   out_8923486826760505000[38] = 1.0;
   out_8923486826760505000[39] = 0.0;
   out_8923486826760505000[40] = 0.0;
   out_8923486826760505000[41] = 0.0;
   out_8923486826760505000[42] = 0.0;
   out_8923486826760505000[43] = 0.0;
   out_8923486826760505000[44] = 0.0;
   out_8923486826760505000[45] = 0.0;
   out_8923486826760505000[46] = 0.0;
   out_8923486826760505000[47] = 0.0;
   out_8923486826760505000[48] = 0.0;
   out_8923486826760505000[49] = 0.0;
   out_8923486826760505000[50] = 0.0;
   out_8923486826760505000[51] = 0.0;
   out_8923486826760505000[52] = 0.0;
   out_8923486826760505000[53] = 0.0;
   out_8923486826760505000[54] = 0.0;
   out_8923486826760505000[55] = 0.0;
   out_8923486826760505000[56] = 0.0;
   out_8923486826760505000[57] = 1.0;
   out_8923486826760505000[58] = 0.0;
   out_8923486826760505000[59] = 0.0;
   out_8923486826760505000[60] = 0.0;
   out_8923486826760505000[61] = 0.0;
   out_8923486826760505000[62] = 0.0;
   out_8923486826760505000[63] = 0.0;
   out_8923486826760505000[64] = 0.0;
   out_8923486826760505000[65] = 0.0;
   out_8923486826760505000[66] = 0.0;
   out_8923486826760505000[67] = 0.0;
   out_8923486826760505000[68] = 0.0;
   out_8923486826760505000[69] = 0.0;
   out_8923486826760505000[70] = 0.0;
   out_8923486826760505000[71] = 0.0;
   out_8923486826760505000[72] = 0.0;
   out_8923486826760505000[73] = 0.0;
   out_8923486826760505000[74] = 0.0;
   out_8923486826760505000[75] = 0.0;
   out_8923486826760505000[76] = 1.0;
   out_8923486826760505000[77] = 0.0;
   out_8923486826760505000[78] = 0.0;
   out_8923486826760505000[79] = 0.0;
   out_8923486826760505000[80] = 0.0;
   out_8923486826760505000[81] = 0.0;
   out_8923486826760505000[82] = 0.0;
   out_8923486826760505000[83] = 0.0;
   out_8923486826760505000[84] = 0.0;
   out_8923486826760505000[85] = 0.0;
   out_8923486826760505000[86] = 0.0;
   out_8923486826760505000[87] = 0.0;
   out_8923486826760505000[88] = 0.0;
   out_8923486826760505000[89] = 0.0;
   out_8923486826760505000[90] = 0.0;
   out_8923486826760505000[91] = 0.0;
   out_8923486826760505000[92] = 0.0;
   out_8923486826760505000[93] = 0.0;
   out_8923486826760505000[94] = 0.0;
   out_8923486826760505000[95] = 1.0;
   out_8923486826760505000[96] = 0.0;
   out_8923486826760505000[97] = 0.0;
   out_8923486826760505000[98] = 0.0;
   out_8923486826760505000[99] = 0.0;
   out_8923486826760505000[100] = 0.0;
   out_8923486826760505000[101] = 0.0;
   out_8923486826760505000[102] = 0.0;
   out_8923486826760505000[103] = 0.0;
   out_8923486826760505000[104] = 0.0;
   out_8923486826760505000[105] = 0.0;
   out_8923486826760505000[106] = 0.0;
   out_8923486826760505000[107] = 0.0;
   out_8923486826760505000[108] = 0.0;
   out_8923486826760505000[109] = 0.0;
   out_8923486826760505000[110] = 0.0;
   out_8923486826760505000[111] = 0.0;
   out_8923486826760505000[112] = 0.0;
   out_8923486826760505000[113] = 0.0;
   out_8923486826760505000[114] = 1.0;
   out_8923486826760505000[115] = 0.0;
   out_8923486826760505000[116] = 0.0;
   out_8923486826760505000[117] = 0.0;
   out_8923486826760505000[118] = 0.0;
   out_8923486826760505000[119] = 0.0;
   out_8923486826760505000[120] = 0.0;
   out_8923486826760505000[121] = 0.0;
   out_8923486826760505000[122] = 0.0;
   out_8923486826760505000[123] = 0.0;
   out_8923486826760505000[124] = 0.0;
   out_8923486826760505000[125] = 0.0;
   out_8923486826760505000[126] = 0.0;
   out_8923486826760505000[127] = 0.0;
   out_8923486826760505000[128] = 0.0;
   out_8923486826760505000[129] = 0.0;
   out_8923486826760505000[130] = 0.0;
   out_8923486826760505000[131] = 0.0;
   out_8923486826760505000[132] = 0.0;
   out_8923486826760505000[133] = 1.0;
   out_8923486826760505000[134] = 0.0;
   out_8923486826760505000[135] = 0.0;
   out_8923486826760505000[136] = 0.0;
   out_8923486826760505000[137] = 0.0;
   out_8923486826760505000[138] = 0.0;
   out_8923486826760505000[139] = 0.0;
   out_8923486826760505000[140] = 0.0;
   out_8923486826760505000[141] = 0.0;
   out_8923486826760505000[142] = 0.0;
   out_8923486826760505000[143] = 0.0;
   out_8923486826760505000[144] = 0.0;
   out_8923486826760505000[145] = 0.0;
   out_8923486826760505000[146] = 0.0;
   out_8923486826760505000[147] = 0.0;
   out_8923486826760505000[148] = 0.0;
   out_8923486826760505000[149] = 0.0;
   out_8923486826760505000[150] = 0.0;
   out_8923486826760505000[151] = 0.0;
   out_8923486826760505000[152] = 1.0;
   out_8923486826760505000[153] = 0.0;
   out_8923486826760505000[154] = 0.0;
   out_8923486826760505000[155] = 0.0;
   out_8923486826760505000[156] = 0.0;
   out_8923486826760505000[157] = 0.0;
   out_8923486826760505000[158] = 0.0;
   out_8923486826760505000[159] = 0.0;
   out_8923486826760505000[160] = 0.0;
   out_8923486826760505000[161] = 0.0;
   out_8923486826760505000[162] = 0.0;
   out_8923486826760505000[163] = 0.0;
   out_8923486826760505000[164] = 0.0;
   out_8923486826760505000[165] = 0.0;
   out_8923486826760505000[166] = 0.0;
   out_8923486826760505000[167] = 0.0;
   out_8923486826760505000[168] = 0.0;
   out_8923486826760505000[169] = 0.0;
   out_8923486826760505000[170] = 0.0;
   out_8923486826760505000[171] = 1.0;
   out_8923486826760505000[172] = 0.0;
   out_8923486826760505000[173] = 0.0;
   out_8923486826760505000[174] = 0.0;
   out_8923486826760505000[175] = 0.0;
   out_8923486826760505000[176] = 0.0;
   out_8923486826760505000[177] = 0.0;
   out_8923486826760505000[178] = 0.0;
   out_8923486826760505000[179] = 0.0;
   out_8923486826760505000[180] = 0.0;
   out_8923486826760505000[181] = 0.0;
   out_8923486826760505000[182] = 0.0;
   out_8923486826760505000[183] = 0.0;
   out_8923486826760505000[184] = 0.0;
   out_8923486826760505000[185] = 0.0;
   out_8923486826760505000[186] = 0.0;
   out_8923486826760505000[187] = 0.0;
   out_8923486826760505000[188] = 0.0;
   out_8923486826760505000[189] = 0.0;
   out_8923486826760505000[190] = 1.0;
   out_8923486826760505000[191] = 0.0;
   out_8923486826760505000[192] = 0.0;
   out_8923486826760505000[193] = 0.0;
   out_8923486826760505000[194] = 0.0;
   out_8923486826760505000[195] = 0.0;
   out_8923486826760505000[196] = 0.0;
   out_8923486826760505000[197] = 0.0;
   out_8923486826760505000[198] = 0.0;
   out_8923486826760505000[199] = 0.0;
   out_8923486826760505000[200] = 0.0;
   out_8923486826760505000[201] = 0.0;
   out_8923486826760505000[202] = 0.0;
   out_8923486826760505000[203] = 0.0;
   out_8923486826760505000[204] = 0.0;
   out_8923486826760505000[205] = 0.0;
   out_8923486826760505000[206] = 0.0;
   out_8923486826760505000[207] = 0.0;
   out_8923486826760505000[208] = 0.0;
   out_8923486826760505000[209] = 1.0;
   out_8923486826760505000[210] = 0.0;
   out_8923486826760505000[211] = 0.0;
   out_8923486826760505000[212] = 0.0;
   out_8923486826760505000[213] = 0.0;
   out_8923486826760505000[214] = 0.0;
   out_8923486826760505000[215] = 0.0;
   out_8923486826760505000[216] = 0.0;
   out_8923486826760505000[217] = 0.0;
   out_8923486826760505000[218] = 0.0;
   out_8923486826760505000[219] = 0.0;
   out_8923486826760505000[220] = 0.0;
   out_8923486826760505000[221] = 0.0;
   out_8923486826760505000[222] = 0.0;
   out_8923486826760505000[223] = 0.0;
   out_8923486826760505000[224] = 0.0;
   out_8923486826760505000[225] = 0.0;
   out_8923486826760505000[226] = 0.0;
   out_8923486826760505000[227] = 0.0;
   out_8923486826760505000[228] = 1.0;
   out_8923486826760505000[229] = 0.0;
   out_8923486826760505000[230] = 0.0;
   out_8923486826760505000[231] = 0.0;
   out_8923486826760505000[232] = 0.0;
   out_8923486826760505000[233] = 0.0;
   out_8923486826760505000[234] = 0.0;
   out_8923486826760505000[235] = 0.0;
   out_8923486826760505000[236] = 0.0;
   out_8923486826760505000[237] = 0.0;
   out_8923486826760505000[238] = 0.0;
   out_8923486826760505000[239] = 0.0;
   out_8923486826760505000[240] = 0.0;
   out_8923486826760505000[241] = 0.0;
   out_8923486826760505000[242] = 0.0;
   out_8923486826760505000[243] = 0.0;
   out_8923486826760505000[244] = 0.0;
   out_8923486826760505000[245] = 0.0;
   out_8923486826760505000[246] = 0.0;
   out_8923486826760505000[247] = 1.0;
   out_8923486826760505000[248] = 0.0;
   out_8923486826760505000[249] = 0.0;
   out_8923486826760505000[250] = 0.0;
   out_8923486826760505000[251] = 0.0;
   out_8923486826760505000[252] = 0.0;
   out_8923486826760505000[253] = 0.0;
   out_8923486826760505000[254] = 0.0;
   out_8923486826760505000[255] = 0.0;
   out_8923486826760505000[256] = 0.0;
   out_8923486826760505000[257] = 0.0;
   out_8923486826760505000[258] = 0.0;
   out_8923486826760505000[259] = 0.0;
   out_8923486826760505000[260] = 0.0;
   out_8923486826760505000[261] = 0.0;
   out_8923486826760505000[262] = 0.0;
   out_8923486826760505000[263] = 0.0;
   out_8923486826760505000[264] = 0.0;
   out_8923486826760505000[265] = 0.0;
   out_8923486826760505000[266] = 1.0;
   out_8923486826760505000[267] = 0.0;
   out_8923486826760505000[268] = 0.0;
   out_8923486826760505000[269] = 0.0;
   out_8923486826760505000[270] = 0.0;
   out_8923486826760505000[271] = 0.0;
   out_8923486826760505000[272] = 0.0;
   out_8923486826760505000[273] = 0.0;
   out_8923486826760505000[274] = 0.0;
   out_8923486826760505000[275] = 0.0;
   out_8923486826760505000[276] = 0.0;
   out_8923486826760505000[277] = 0.0;
   out_8923486826760505000[278] = 0.0;
   out_8923486826760505000[279] = 0.0;
   out_8923486826760505000[280] = 0.0;
   out_8923486826760505000[281] = 0.0;
   out_8923486826760505000[282] = 0.0;
   out_8923486826760505000[283] = 0.0;
   out_8923486826760505000[284] = 0.0;
   out_8923486826760505000[285] = 1.0;
   out_8923486826760505000[286] = 0.0;
   out_8923486826760505000[287] = 0.0;
   out_8923486826760505000[288] = 0.0;
   out_8923486826760505000[289] = 0.0;
   out_8923486826760505000[290] = 0.0;
   out_8923486826760505000[291] = 0.0;
   out_8923486826760505000[292] = 0.0;
   out_8923486826760505000[293] = 0.0;
   out_8923486826760505000[294] = 0.0;
   out_8923486826760505000[295] = 0.0;
   out_8923486826760505000[296] = 0.0;
   out_8923486826760505000[297] = 0.0;
   out_8923486826760505000[298] = 0.0;
   out_8923486826760505000[299] = 0.0;
   out_8923486826760505000[300] = 0.0;
   out_8923486826760505000[301] = 0.0;
   out_8923486826760505000[302] = 0.0;
   out_8923486826760505000[303] = 0.0;
   out_8923486826760505000[304] = 1.0;
   out_8923486826760505000[305] = 0.0;
   out_8923486826760505000[306] = 0.0;
   out_8923486826760505000[307] = 0.0;
   out_8923486826760505000[308] = 0.0;
   out_8923486826760505000[309] = 0.0;
   out_8923486826760505000[310] = 0.0;
   out_8923486826760505000[311] = 0.0;
   out_8923486826760505000[312] = 0.0;
   out_8923486826760505000[313] = 0.0;
   out_8923486826760505000[314] = 0.0;
   out_8923486826760505000[315] = 0.0;
   out_8923486826760505000[316] = 0.0;
   out_8923486826760505000[317] = 0.0;
   out_8923486826760505000[318] = 0.0;
   out_8923486826760505000[319] = 0.0;
   out_8923486826760505000[320] = 0.0;
   out_8923486826760505000[321] = 0.0;
   out_8923486826760505000[322] = 0.0;
   out_8923486826760505000[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_6932327335752377632) {
   out_6932327335752377632[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_6932327335752377632[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_6932327335752377632[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_6932327335752377632[3] = dt*state[12] + state[3];
   out_6932327335752377632[4] = dt*state[13] + state[4];
   out_6932327335752377632[5] = dt*state[14] + state[5];
   out_6932327335752377632[6] = state[6];
   out_6932327335752377632[7] = state[7];
   out_6932327335752377632[8] = state[8];
   out_6932327335752377632[9] = state[9];
   out_6932327335752377632[10] = state[10];
   out_6932327335752377632[11] = state[11];
   out_6932327335752377632[12] = state[12];
   out_6932327335752377632[13] = state[13];
   out_6932327335752377632[14] = state[14];
   out_6932327335752377632[15] = state[15];
   out_6932327335752377632[16] = state[16];
   out_6932327335752377632[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3364331378734993426) {
   out_3364331378734993426[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3364331378734993426[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3364331378734993426[2] = 0;
   out_3364331378734993426[3] = 0;
   out_3364331378734993426[4] = 0;
   out_3364331378734993426[5] = 0;
   out_3364331378734993426[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3364331378734993426[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3364331378734993426[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3364331378734993426[9] = 0;
   out_3364331378734993426[10] = 0;
   out_3364331378734993426[11] = 0;
   out_3364331378734993426[12] = 0;
   out_3364331378734993426[13] = 0;
   out_3364331378734993426[14] = 0;
   out_3364331378734993426[15] = 0;
   out_3364331378734993426[16] = 0;
   out_3364331378734993426[17] = 0;
   out_3364331378734993426[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3364331378734993426[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3364331378734993426[20] = 0;
   out_3364331378734993426[21] = 0;
   out_3364331378734993426[22] = 0;
   out_3364331378734993426[23] = 0;
   out_3364331378734993426[24] = 0;
   out_3364331378734993426[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3364331378734993426[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3364331378734993426[27] = 0;
   out_3364331378734993426[28] = 0;
   out_3364331378734993426[29] = 0;
   out_3364331378734993426[30] = 0;
   out_3364331378734993426[31] = 0;
   out_3364331378734993426[32] = 0;
   out_3364331378734993426[33] = 0;
   out_3364331378734993426[34] = 0;
   out_3364331378734993426[35] = 0;
   out_3364331378734993426[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3364331378734993426[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3364331378734993426[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3364331378734993426[39] = 0;
   out_3364331378734993426[40] = 0;
   out_3364331378734993426[41] = 0;
   out_3364331378734993426[42] = 0;
   out_3364331378734993426[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3364331378734993426[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3364331378734993426[45] = 0;
   out_3364331378734993426[46] = 0;
   out_3364331378734993426[47] = 0;
   out_3364331378734993426[48] = 0;
   out_3364331378734993426[49] = 0;
   out_3364331378734993426[50] = 0;
   out_3364331378734993426[51] = 0;
   out_3364331378734993426[52] = 0;
   out_3364331378734993426[53] = 0;
   out_3364331378734993426[54] = 0;
   out_3364331378734993426[55] = 0;
   out_3364331378734993426[56] = 0;
   out_3364331378734993426[57] = 1;
   out_3364331378734993426[58] = 0;
   out_3364331378734993426[59] = 0;
   out_3364331378734993426[60] = 0;
   out_3364331378734993426[61] = 0;
   out_3364331378734993426[62] = 0;
   out_3364331378734993426[63] = 0;
   out_3364331378734993426[64] = 0;
   out_3364331378734993426[65] = 0;
   out_3364331378734993426[66] = dt;
   out_3364331378734993426[67] = 0;
   out_3364331378734993426[68] = 0;
   out_3364331378734993426[69] = 0;
   out_3364331378734993426[70] = 0;
   out_3364331378734993426[71] = 0;
   out_3364331378734993426[72] = 0;
   out_3364331378734993426[73] = 0;
   out_3364331378734993426[74] = 0;
   out_3364331378734993426[75] = 0;
   out_3364331378734993426[76] = 1;
   out_3364331378734993426[77] = 0;
   out_3364331378734993426[78] = 0;
   out_3364331378734993426[79] = 0;
   out_3364331378734993426[80] = 0;
   out_3364331378734993426[81] = 0;
   out_3364331378734993426[82] = 0;
   out_3364331378734993426[83] = 0;
   out_3364331378734993426[84] = 0;
   out_3364331378734993426[85] = dt;
   out_3364331378734993426[86] = 0;
   out_3364331378734993426[87] = 0;
   out_3364331378734993426[88] = 0;
   out_3364331378734993426[89] = 0;
   out_3364331378734993426[90] = 0;
   out_3364331378734993426[91] = 0;
   out_3364331378734993426[92] = 0;
   out_3364331378734993426[93] = 0;
   out_3364331378734993426[94] = 0;
   out_3364331378734993426[95] = 1;
   out_3364331378734993426[96] = 0;
   out_3364331378734993426[97] = 0;
   out_3364331378734993426[98] = 0;
   out_3364331378734993426[99] = 0;
   out_3364331378734993426[100] = 0;
   out_3364331378734993426[101] = 0;
   out_3364331378734993426[102] = 0;
   out_3364331378734993426[103] = 0;
   out_3364331378734993426[104] = dt;
   out_3364331378734993426[105] = 0;
   out_3364331378734993426[106] = 0;
   out_3364331378734993426[107] = 0;
   out_3364331378734993426[108] = 0;
   out_3364331378734993426[109] = 0;
   out_3364331378734993426[110] = 0;
   out_3364331378734993426[111] = 0;
   out_3364331378734993426[112] = 0;
   out_3364331378734993426[113] = 0;
   out_3364331378734993426[114] = 1;
   out_3364331378734993426[115] = 0;
   out_3364331378734993426[116] = 0;
   out_3364331378734993426[117] = 0;
   out_3364331378734993426[118] = 0;
   out_3364331378734993426[119] = 0;
   out_3364331378734993426[120] = 0;
   out_3364331378734993426[121] = 0;
   out_3364331378734993426[122] = 0;
   out_3364331378734993426[123] = 0;
   out_3364331378734993426[124] = 0;
   out_3364331378734993426[125] = 0;
   out_3364331378734993426[126] = 0;
   out_3364331378734993426[127] = 0;
   out_3364331378734993426[128] = 0;
   out_3364331378734993426[129] = 0;
   out_3364331378734993426[130] = 0;
   out_3364331378734993426[131] = 0;
   out_3364331378734993426[132] = 0;
   out_3364331378734993426[133] = 1;
   out_3364331378734993426[134] = 0;
   out_3364331378734993426[135] = 0;
   out_3364331378734993426[136] = 0;
   out_3364331378734993426[137] = 0;
   out_3364331378734993426[138] = 0;
   out_3364331378734993426[139] = 0;
   out_3364331378734993426[140] = 0;
   out_3364331378734993426[141] = 0;
   out_3364331378734993426[142] = 0;
   out_3364331378734993426[143] = 0;
   out_3364331378734993426[144] = 0;
   out_3364331378734993426[145] = 0;
   out_3364331378734993426[146] = 0;
   out_3364331378734993426[147] = 0;
   out_3364331378734993426[148] = 0;
   out_3364331378734993426[149] = 0;
   out_3364331378734993426[150] = 0;
   out_3364331378734993426[151] = 0;
   out_3364331378734993426[152] = 1;
   out_3364331378734993426[153] = 0;
   out_3364331378734993426[154] = 0;
   out_3364331378734993426[155] = 0;
   out_3364331378734993426[156] = 0;
   out_3364331378734993426[157] = 0;
   out_3364331378734993426[158] = 0;
   out_3364331378734993426[159] = 0;
   out_3364331378734993426[160] = 0;
   out_3364331378734993426[161] = 0;
   out_3364331378734993426[162] = 0;
   out_3364331378734993426[163] = 0;
   out_3364331378734993426[164] = 0;
   out_3364331378734993426[165] = 0;
   out_3364331378734993426[166] = 0;
   out_3364331378734993426[167] = 0;
   out_3364331378734993426[168] = 0;
   out_3364331378734993426[169] = 0;
   out_3364331378734993426[170] = 0;
   out_3364331378734993426[171] = 1;
   out_3364331378734993426[172] = 0;
   out_3364331378734993426[173] = 0;
   out_3364331378734993426[174] = 0;
   out_3364331378734993426[175] = 0;
   out_3364331378734993426[176] = 0;
   out_3364331378734993426[177] = 0;
   out_3364331378734993426[178] = 0;
   out_3364331378734993426[179] = 0;
   out_3364331378734993426[180] = 0;
   out_3364331378734993426[181] = 0;
   out_3364331378734993426[182] = 0;
   out_3364331378734993426[183] = 0;
   out_3364331378734993426[184] = 0;
   out_3364331378734993426[185] = 0;
   out_3364331378734993426[186] = 0;
   out_3364331378734993426[187] = 0;
   out_3364331378734993426[188] = 0;
   out_3364331378734993426[189] = 0;
   out_3364331378734993426[190] = 1;
   out_3364331378734993426[191] = 0;
   out_3364331378734993426[192] = 0;
   out_3364331378734993426[193] = 0;
   out_3364331378734993426[194] = 0;
   out_3364331378734993426[195] = 0;
   out_3364331378734993426[196] = 0;
   out_3364331378734993426[197] = 0;
   out_3364331378734993426[198] = 0;
   out_3364331378734993426[199] = 0;
   out_3364331378734993426[200] = 0;
   out_3364331378734993426[201] = 0;
   out_3364331378734993426[202] = 0;
   out_3364331378734993426[203] = 0;
   out_3364331378734993426[204] = 0;
   out_3364331378734993426[205] = 0;
   out_3364331378734993426[206] = 0;
   out_3364331378734993426[207] = 0;
   out_3364331378734993426[208] = 0;
   out_3364331378734993426[209] = 1;
   out_3364331378734993426[210] = 0;
   out_3364331378734993426[211] = 0;
   out_3364331378734993426[212] = 0;
   out_3364331378734993426[213] = 0;
   out_3364331378734993426[214] = 0;
   out_3364331378734993426[215] = 0;
   out_3364331378734993426[216] = 0;
   out_3364331378734993426[217] = 0;
   out_3364331378734993426[218] = 0;
   out_3364331378734993426[219] = 0;
   out_3364331378734993426[220] = 0;
   out_3364331378734993426[221] = 0;
   out_3364331378734993426[222] = 0;
   out_3364331378734993426[223] = 0;
   out_3364331378734993426[224] = 0;
   out_3364331378734993426[225] = 0;
   out_3364331378734993426[226] = 0;
   out_3364331378734993426[227] = 0;
   out_3364331378734993426[228] = 1;
   out_3364331378734993426[229] = 0;
   out_3364331378734993426[230] = 0;
   out_3364331378734993426[231] = 0;
   out_3364331378734993426[232] = 0;
   out_3364331378734993426[233] = 0;
   out_3364331378734993426[234] = 0;
   out_3364331378734993426[235] = 0;
   out_3364331378734993426[236] = 0;
   out_3364331378734993426[237] = 0;
   out_3364331378734993426[238] = 0;
   out_3364331378734993426[239] = 0;
   out_3364331378734993426[240] = 0;
   out_3364331378734993426[241] = 0;
   out_3364331378734993426[242] = 0;
   out_3364331378734993426[243] = 0;
   out_3364331378734993426[244] = 0;
   out_3364331378734993426[245] = 0;
   out_3364331378734993426[246] = 0;
   out_3364331378734993426[247] = 1;
   out_3364331378734993426[248] = 0;
   out_3364331378734993426[249] = 0;
   out_3364331378734993426[250] = 0;
   out_3364331378734993426[251] = 0;
   out_3364331378734993426[252] = 0;
   out_3364331378734993426[253] = 0;
   out_3364331378734993426[254] = 0;
   out_3364331378734993426[255] = 0;
   out_3364331378734993426[256] = 0;
   out_3364331378734993426[257] = 0;
   out_3364331378734993426[258] = 0;
   out_3364331378734993426[259] = 0;
   out_3364331378734993426[260] = 0;
   out_3364331378734993426[261] = 0;
   out_3364331378734993426[262] = 0;
   out_3364331378734993426[263] = 0;
   out_3364331378734993426[264] = 0;
   out_3364331378734993426[265] = 0;
   out_3364331378734993426[266] = 1;
   out_3364331378734993426[267] = 0;
   out_3364331378734993426[268] = 0;
   out_3364331378734993426[269] = 0;
   out_3364331378734993426[270] = 0;
   out_3364331378734993426[271] = 0;
   out_3364331378734993426[272] = 0;
   out_3364331378734993426[273] = 0;
   out_3364331378734993426[274] = 0;
   out_3364331378734993426[275] = 0;
   out_3364331378734993426[276] = 0;
   out_3364331378734993426[277] = 0;
   out_3364331378734993426[278] = 0;
   out_3364331378734993426[279] = 0;
   out_3364331378734993426[280] = 0;
   out_3364331378734993426[281] = 0;
   out_3364331378734993426[282] = 0;
   out_3364331378734993426[283] = 0;
   out_3364331378734993426[284] = 0;
   out_3364331378734993426[285] = 1;
   out_3364331378734993426[286] = 0;
   out_3364331378734993426[287] = 0;
   out_3364331378734993426[288] = 0;
   out_3364331378734993426[289] = 0;
   out_3364331378734993426[290] = 0;
   out_3364331378734993426[291] = 0;
   out_3364331378734993426[292] = 0;
   out_3364331378734993426[293] = 0;
   out_3364331378734993426[294] = 0;
   out_3364331378734993426[295] = 0;
   out_3364331378734993426[296] = 0;
   out_3364331378734993426[297] = 0;
   out_3364331378734993426[298] = 0;
   out_3364331378734993426[299] = 0;
   out_3364331378734993426[300] = 0;
   out_3364331378734993426[301] = 0;
   out_3364331378734993426[302] = 0;
   out_3364331378734993426[303] = 0;
   out_3364331378734993426[304] = 1;
   out_3364331378734993426[305] = 0;
   out_3364331378734993426[306] = 0;
   out_3364331378734993426[307] = 0;
   out_3364331378734993426[308] = 0;
   out_3364331378734993426[309] = 0;
   out_3364331378734993426[310] = 0;
   out_3364331378734993426[311] = 0;
   out_3364331378734993426[312] = 0;
   out_3364331378734993426[313] = 0;
   out_3364331378734993426[314] = 0;
   out_3364331378734993426[315] = 0;
   out_3364331378734993426[316] = 0;
   out_3364331378734993426[317] = 0;
   out_3364331378734993426[318] = 0;
   out_3364331378734993426[319] = 0;
   out_3364331378734993426[320] = 0;
   out_3364331378734993426[321] = 0;
   out_3364331378734993426[322] = 0;
   out_3364331378734993426[323] = 1;
}
void h_4(double *state, double *unused, double *out_2331867789495253821) {
   out_2331867789495253821[0] = state[6] + state[9];
   out_2331867789495253821[1] = state[7] + state[10];
   out_2331867789495253821[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_7103788100854208139) {
   out_7103788100854208139[0] = 0;
   out_7103788100854208139[1] = 0;
   out_7103788100854208139[2] = 0;
   out_7103788100854208139[3] = 0;
   out_7103788100854208139[4] = 0;
   out_7103788100854208139[5] = 0;
   out_7103788100854208139[6] = 1;
   out_7103788100854208139[7] = 0;
   out_7103788100854208139[8] = 0;
   out_7103788100854208139[9] = 1;
   out_7103788100854208139[10] = 0;
   out_7103788100854208139[11] = 0;
   out_7103788100854208139[12] = 0;
   out_7103788100854208139[13] = 0;
   out_7103788100854208139[14] = 0;
   out_7103788100854208139[15] = 0;
   out_7103788100854208139[16] = 0;
   out_7103788100854208139[17] = 0;
   out_7103788100854208139[18] = 0;
   out_7103788100854208139[19] = 0;
   out_7103788100854208139[20] = 0;
   out_7103788100854208139[21] = 0;
   out_7103788100854208139[22] = 0;
   out_7103788100854208139[23] = 0;
   out_7103788100854208139[24] = 0;
   out_7103788100854208139[25] = 1;
   out_7103788100854208139[26] = 0;
   out_7103788100854208139[27] = 0;
   out_7103788100854208139[28] = 1;
   out_7103788100854208139[29] = 0;
   out_7103788100854208139[30] = 0;
   out_7103788100854208139[31] = 0;
   out_7103788100854208139[32] = 0;
   out_7103788100854208139[33] = 0;
   out_7103788100854208139[34] = 0;
   out_7103788100854208139[35] = 0;
   out_7103788100854208139[36] = 0;
   out_7103788100854208139[37] = 0;
   out_7103788100854208139[38] = 0;
   out_7103788100854208139[39] = 0;
   out_7103788100854208139[40] = 0;
   out_7103788100854208139[41] = 0;
   out_7103788100854208139[42] = 0;
   out_7103788100854208139[43] = 0;
   out_7103788100854208139[44] = 1;
   out_7103788100854208139[45] = 0;
   out_7103788100854208139[46] = 0;
   out_7103788100854208139[47] = 1;
   out_7103788100854208139[48] = 0;
   out_7103788100854208139[49] = 0;
   out_7103788100854208139[50] = 0;
   out_7103788100854208139[51] = 0;
   out_7103788100854208139[52] = 0;
   out_7103788100854208139[53] = 0;
}
void h_10(double *state, double *unused, double *out_6139878679209159095) {
   out_6139878679209159095[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_6139878679209159095[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_6139878679209159095[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_1053793302334783175) {
   out_1053793302334783175[0] = 0;
   out_1053793302334783175[1] = 9.8100000000000005*cos(state[1]);
   out_1053793302334783175[2] = 0;
   out_1053793302334783175[3] = 0;
   out_1053793302334783175[4] = -state[8];
   out_1053793302334783175[5] = state[7];
   out_1053793302334783175[6] = 0;
   out_1053793302334783175[7] = state[5];
   out_1053793302334783175[8] = -state[4];
   out_1053793302334783175[9] = 0;
   out_1053793302334783175[10] = 0;
   out_1053793302334783175[11] = 0;
   out_1053793302334783175[12] = 1;
   out_1053793302334783175[13] = 0;
   out_1053793302334783175[14] = 0;
   out_1053793302334783175[15] = 1;
   out_1053793302334783175[16] = 0;
   out_1053793302334783175[17] = 0;
   out_1053793302334783175[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_1053793302334783175[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_1053793302334783175[20] = 0;
   out_1053793302334783175[21] = state[8];
   out_1053793302334783175[22] = 0;
   out_1053793302334783175[23] = -state[6];
   out_1053793302334783175[24] = -state[5];
   out_1053793302334783175[25] = 0;
   out_1053793302334783175[26] = state[3];
   out_1053793302334783175[27] = 0;
   out_1053793302334783175[28] = 0;
   out_1053793302334783175[29] = 0;
   out_1053793302334783175[30] = 0;
   out_1053793302334783175[31] = 1;
   out_1053793302334783175[32] = 0;
   out_1053793302334783175[33] = 0;
   out_1053793302334783175[34] = 1;
   out_1053793302334783175[35] = 0;
   out_1053793302334783175[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_1053793302334783175[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_1053793302334783175[38] = 0;
   out_1053793302334783175[39] = -state[7];
   out_1053793302334783175[40] = state[6];
   out_1053793302334783175[41] = 0;
   out_1053793302334783175[42] = state[4];
   out_1053793302334783175[43] = -state[3];
   out_1053793302334783175[44] = 0;
   out_1053793302334783175[45] = 0;
   out_1053793302334783175[46] = 0;
   out_1053793302334783175[47] = 0;
   out_1053793302334783175[48] = 0;
   out_1053793302334783175[49] = 0;
   out_1053793302334783175[50] = 1;
   out_1053793302334783175[51] = 0;
   out_1053793302334783175[52] = 0;
   out_1053793302334783175[53] = 1;
}
void h_13(double *state, double *unused, double *out_1682911552899262681) {
   out_1682911552899262681[0] = state[3];
   out_1682911552899262681[1] = state[4];
   out_1682911552899262681[2] = state[5];
}
void H_13(double *state, double *unused, double *out_3732324764538642548) {
   out_3732324764538642548[0] = 0;
   out_3732324764538642548[1] = 0;
   out_3732324764538642548[2] = 0;
   out_3732324764538642548[3] = 1;
   out_3732324764538642548[4] = 0;
   out_3732324764538642548[5] = 0;
   out_3732324764538642548[6] = 0;
   out_3732324764538642548[7] = 0;
   out_3732324764538642548[8] = 0;
   out_3732324764538642548[9] = 0;
   out_3732324764538642548[10] = 0;
   out_3732324764538642548[11] = 0;
   out_3732324764538642548[12] = 0;
   out_3732324764538642548[13] = 0;
   out_3732324764538642548[14] = 0;
   out_3732324764538642548[15] = 0;
   out_3732324764538642548[16] = 0;
   out_3732324764538642548[17] = 0;
   out_3732324764538642548[18] = 0;
   out_3732324764538642548[19] = 0;
   out_3732324764538642548[20] = 0;
   out_3732324764538642548[21] = 0;
   out_3732324764538642548[22] = 1;
   out_3732324764538642548[23] = 0;
   out_3732324764538642548[24] = 0;
   out_3732324764538642548[25] = 0;
   out_3732324764538642548[26] = 0;
   out_3732324764538642548[27] = 0;
   out_3732324764538642548[28] = 0;
   out_3732324764538642548[29] = 0;
   out_3732324764538642548[30] = 0;
   out_3732324764538642548[31] = 0;
   out_3732324764538642548[32] = 0;
   out_3732324764538642548[33] = 0;
   out_3732324764538642548[34] = 0;
   out_3732324764538642548[35] = 0;
   out_3732324764538642548[36] = 0;
   out_3732324764538642548[37] = 0;
   out_3732324764538642548[38] = 0;
   out_3732324764538642548[39] = 0;
   out_3732324764538642548[40] = 0;
   out_3732324764538642548[41] = 1;
   out_3732324764538642548[42] = 0;
   out_3732324764538642548[43] = 0;
   out_3732324764538642548[44] = 0;
   out_3732324764538642548[45] = 0;
   out_3732324764538642548[46] = 0;
   out_3732324764538642548[47] = 0;
   out_3732324764538642548[48] = 0;
   out_3732324764538642548[49] = 0;
   out_3732324764538642548[50] = 0;
   out_3732324764538642548[51] = 0;
   out_3732324764538642548[52] = 0;
   out_3732324764538642548[53] = 0;
}
void h_14(double *state, double *unused, double *out_7957741524853228127) {
   out_7957741524853228127[0] = state[6];
   out_7957741524853228127[1] = state[7];
   out_7957741524853228127[2] = state[8];
}
void H_14(double *state, double *unused, double *out_4020999668558835843) {
   out_4020999668558835843[0] = 0;
   out_4020999668558835843[1] = 0;
   out_4020999668558835843[2] = 0;
   out_4020999668558835843[3] = 0;
   out_4020999668558835843[4] = 0;
   out_4020999668558835843[5] = 0;
   out_4020999668558835843[6] = 1;
   out_4020999668558835843[7] = 0;
   out_4020999668558835843[8] = 0;
   out_4020999668558835843[9] = 0;
   out_4020999668558835843[10] = 0;
   out_4020999668558835843[11] = 0;
   out_4020999668558835843[12] = 0;
   out_4020999668558835843[13] = 0;
   out_4020999668558835843[14] = 0;
   out_4020999668558835843[15] = 0;
   out_4020999668558835843[16] = 0;
   out_4020999668558835843[17] = 0;
   out_4020999668558835843[18] = 0;
   out_4020999668558835843[19] = 0;
   out_4020999668558835843[20] = 0;
   out_4020999668558835843[21] = 0;
   out_4020999668558835843[22] = 0;
   out_4020999668558835843[23] = 0;
   out_4020999668558835843[24] = 0;
   out_4020999668558835843[25] = 1;
   out_4020999668558835843[26] = 0;
   out_4020999668558835843[27] = 0;
   out_4020999668558835843[28] = 0;
   out_4020999668558835843[29] = 0;
   out_4020999668558835843[30] = 0;
   out_4020999668558835843[31] = 0;
   out_4020999668558835843[32] = 0;
   out_4020999668558835843[33] = 0;
   out_4020999668558835843[34] = 0;
   out_4020999668558835843[35] = 0;
   out_4020999668558835843[36] = 0;
   out_4020999668558835843[37] = 0;
   out_4020999668558835843[38] = 0;
   out_4020999668558835843[39] = 0;
   out_4020999668558835843[40] = 0;
   out_4020999668558835843[41] = 0;
   out_4020999668558835843[42] = 0;
   out_4020999668558835843[43] = 0;
   out_4020999668558835843[44] = 1;
   out_4020999668558835843[45] = 0;
   out_4020999668558835843[46] = 0;
   out_4020999668558835843[47] = 0;
   out_4020999668558835843[48] = 0;
   out_4020999668558835843[49] = 0;
   out_4020999668558835843[50] = 0;
   out_4020999668558835843[51] = 0;
   out_4020999668558835843[52] = 0;
   out_4020999668558835843[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_3060982030804230470) {
  err_fun(nom_x, delta_x, out_3060982030804230470);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1893009693429340384) {
  inv_err_fun(nom_x, true_x, out_1893009693429340384);
}
void pose_H_mod_fun(double *state, double *out_8923486826760505000) {
  H_mod_fun(state, out_8923486826760505000);
}
void pose_f_fun(double *state, double dt, double *out_6932327335752377632) {
  f_fun(state,  dt, out_6932327335752377632);
}
void pose_F_fun(double *state, double dt, double *out_3364331378734993426) {
  F_fun(state,  dt, out_3364331378734993426);
}
void pose_h_4(double *state, double *unused, double *out_2331867789495253821) {
  h_4(state, unused, out_2331867789495253821);
}
void pose_H_4(double *state, double *unused, double *out_7103788100854208139) {
  H_4(state, unused, out_7103788100854208139);
}
void pose_h_10(double *state, double *unused, double *out_6139878679209159095) {
  h_10(state, unused, out_6139878679209159095);
}
void pose_H_10(double *state, double *unused, double *out_1053793302334783175) {
  H_10(state, unused, out_1053793302334783175);
}
void pose_h_13(double *state, double *unused, double *out_1682911552899262681) {
  h_13(state, unused, out_1682911552899262681);
}
void pose_H_13(double *state, double *unused, double *out_3732324764538642548) {
  H_13(state, unused, out_3732324764538642548);
}
void pose_h_14(double *state, double *unused, double *out_7957741524853228127) {
  h_14(state, unused, out_7957741524853228127);
}
void pose_H_14(double *state, double *unused, double *out_4020999668558835843) {
  H_14(state, unused, out_4020999668558835843);
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

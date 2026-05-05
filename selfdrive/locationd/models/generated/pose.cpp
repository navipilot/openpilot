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
void err_fun(double *nom_x, double *delta_x, double *out_7352766374165151156) {
   out_7352766374165151156[0] = delta_x[0] + nom_x[0];
   out_7352766374165151156[1] = delta_x[1] + nom_x[1];
   out_7352766374165151156[2] = delta_x[2] + nom_x[2];
   out_7352766374165151156[3] = delta_x[3] + nom_x[3];
   out_7352766374165151156[4] = delta_x[4] + nom_x[4];
   out_7352766374165151156[5] = delta_x[5] + nom_x[5];
   out_7352766374165151156[6] = delta_x[6] + nom_x[6];
   out_7352766374165151156[7] = delta_x[7] + nom_x[7];
   out_7352766374165151156[8] = delta_x[8] + nom_x[8];
   out_7352766374165151156[9] = delta_x[9] + nom_x[9];
   out_7352766374165151156[10] = delta_x[10] + nom_x[10];
   out_7352766374165151156[11] = delta_x[11] + nom_x[11];
   out_7352766374165151156[12] = delta_x[12] + nom_x[12];
   out_7352766374165151156[13] = delta_x[13] + nom_x[13];
   out_7352766374165151156[14] = delta_x[14] + nom_x[14];
   out_7352766374165151156[15] = delta_x[15] + nom_x[15];
   out_7352766374165151156[16] = delta_x[16] + nom_x[16];
   out_7352766374165151156[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_5943722919136625654) {
   out_5943722919136625654[0] = -nom_x[0] + true_x[0];
   out_5943722919136625654[1] = -nom_x[1] + true_x[1];
   out_5943722919136625654[2] = -nom_x[2] + true_x[2];
   out_5943722919136625654[3] = -nom_x[3] + true_x[3];
   out_5943722919136625654[4] = -nom_x[4] + true_x[4];
   out_5943722919136625654[5] = -nom_x[5] + true_x[5];
   out_5943722919136625654[6] = -nom_x[6] + true_x[6];
   out_5943722919136625654[7] = -nom_x[7] + true_x[7];
   out_5943722919136625654[8] = -nom_x[8] + true_x[8];
   out_5943722919136625654[9] = -nom_x[9] + true_x[9];
   out_5943722919136625654[10] = -nom_x[10] + true_x[10];
   out_5943722919136625654[11] = -nom_x[11] + true_x[11];
   out_5943722919136625654[12] = -nom_x[12] + true_x[12];
   out_5943722919136625654[13] = -nom_x[13] + true_x[13];
   out_5943722919136625654[14] = -nom_x[14] + true_x[14];
   out_5943722919136625654[15] = -nom_x[15] + true_x[15];
   out_5943722919136625654[16] = -nom_x[16] + true_x[16];
   out_5943722919136625654[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_2863715751586478773) {
   out_2863715751586478773[0] = 1.0;
   out_2863715751586478773[1] = 0.0;
   out_2863715751586478773[2] = 0.0;
   out_2863715751586478773[3] = 0.0;
   out_2863715751586478773[4] = 0.0;
   out_2863715751586478773[5] = 0.0;
   out_2863715751586478773[6] = 0.0;
   out_2863715751586478773[7] = 0.0;
   out_2863715751586478773[8] = 0.0;
   out_2863715751586478773[9] = 0.0;
   out_2863715751586478773[10] = 0.0;
   out_2863715751586478773[11] = 0.0;
   out_2863715751586478773[12] = 0.0;
   out_2863715751586478773[13] = 0.0;
   out_2863715751586478773[14] = 0.0;
   out_2863715751586478773[15] = 0.0;
   out_2863715751586478773[16] = 0.0;
   out_2863715751586478773[17] = 0.0;
   out_2863715751586478773[18] = 0.0;
   out_2863715751586478773[19] = 1.0;
   out_2863715751586478773[20] = 0.0;
   out_2863715751586478773[21] = 0.0;
   out_2863715751586478773[22] = 0.0;
   out_2863715751586478773[23] = 0.0;
   out_2863715751586478773[24] = 0.0;
   out_2863715751586478773[25] = 0.0;
   out_2863715751586478773[26] = 0.0;
   out_2863715751586478773[27] = 0.0;
   out_2863715751586478773[28] = 0.0;
   out_2863715751586478773[29] = 0.0;
   out_2863715751586478773[30] = 0.0;
   out_2863715751586478773[31] = 0.0;
   out_2863715751586478773[32] = 0.0;
   out_2863715751586478773[33] = 0.0;
   out_2863715751586478773[34] = 0.0;
   out_2863715751586478773[35] = 0.0;
   out_2863715751586478773[36] = 0.0;
   out_2863715751586478773[37] = 0.0;
   out_2863715751586478773[38] = 1.0;
   out_2863715751586478773[39] = 0.0;
   out_2863715751586478773[40] = 0.0;
   out_2863715751586478773[41] = 0.0;
   out_2863715751586478773[42] = 0.0;
   out_2863715751586478773[43] = 0.0;
   out_2863715751586478773[44] = 0.0;
   out_2863715751586478773[45] = 0.0;
   out_2863715751586478773[46] = 0.0;
   out_2863715751586478773[47] = 0.0;
   out_2863715751586478773[48] = 0.0;
   out_2863715751586478773[49] = 0.0;
   out_2863715751586478773[50] = 0.0;
   out_2863715751586478773[51] = 0.0;
   out_2863715751586478773[52] = 0.0;
   out_2863715751586478773[53] = 0.0;
   out_2863715751586478773[54] = 0.0;
   out_2863715751586478773[55] = 0.0;
   out_2863715751586478773[56] = 0.0;
   out_2863715751586478773[57] = 1.0;
   out_2863715751586478773[58] = 0.0;
   out_2863715751586478773[59] = 0.0;
   out_2863715751586478773[60] = 0.0;
   out_2863715751586478773[61] = 0.0;
   out_2863715751586478773[62] = 0.0;
   out_2863715751586478773[63] = 0.0;
   out_2863715751586478773[64] = 0.0;
   out_2863715751586478773[65] = 0.0;
   out_2863715751586478773[66] = 0.0;
   out_2863715751586478773[67] = 0.0;
   out_2863715751586478773[68] = 0.0;
   out_2863715751586478773[69] = 0.0;
   out_2863715751586478773[70] = 0.0;
   out_2863715751586478773[71] = 0.0;
   out_2863715751586478773[72] = 0.0;
   out_2863715751586478773[73] = 0.0;
   out_2863715751586478773[74] = 0.0;
   out_2863715751586478773[75] = 0.0;
   out_2863715751586478773[76] = 1.0;
   out_2863715751586478773[77] = 0.0;
   out_2863715751586478773[78] = 0.0;
   out_2863715751586478773[79] = 0.0;
   out_2863715751586478773[80] = 0.0;
   out_2863715751586478773[81] = 0.0;
   out_2863715751586478773[82] = 0.0;
   out_2863715751586478773[83] = 0.0;
   out_2863715751586478773[84] = 0.0;
   out_2863715751586478773[85] = 0.0;
   out_2863715751586478773[86] = 0.0;
   out_2863715751586478773[87] = 0.0;
   out_2863715751586478773[88] = 0.0;
   out_2863715751586478773[89] = 0.0;
   out_2863715751586478773[90] = 0.0;
   out_2863715751586478773[91] = 0.0;
   out_2863715751586478773[92] = 0.0;
   out_2863715751586478773[93] = 0.0;
   out_2863715751586478773[94] = 0.0;
   out_2863715751586478773[95] = 1.0;
   out_2863715751586478773[96] = 0.0;
   out_2863715751586478773[97] = 0.0;
   out_2863715751586478773[98] = 0.0;
   out_2863715751586478773[99] = 0.0;
   out_2863715751586478773[100] = 0.0;
   out_2863715751586478773[101] = 0.0;
   out_2863715751586478773[102] = 0.0;
   out_2863715751586478773[103] = 0.0;
   out_2863715751586478773[104] = 0.0;
   out_2863715751586478773[105] = 0.0;
   out_2863715751586478773[106] = 0.0;
   out_2863715751586478773[107] = 0.0;
   out_2863715751586478773[108] = 0.0;
   out_2863715751586478773[109] = 0.0;
   out_2863715751586478773[110] = 0.0;
   out_2863715751586478773[111] = 0.0;
   out_2863715751586478773[112] = 0.0;
   out_2863715751586478773[113] = 0.0;
   out_2863715751586478773[114] = 1.0;
   out_2863715751586478773[115] = 0.0;
   out_2863715751586478773[116] = 0.0;
   out_2863715751586478773[117] = 0.0;
   out_2863715751586478773[118] = 0.0;
   out_2863715751586478773[119] = 0.0;
   out_2863715751586478773[120] = 0.0;
   out_2863715751586478773[121] = 0.0;
   out_2863715751586478773[122] = 0.0;
   out_2863715751586478773[123] = 0.0;
   out_2863715751586478773[124] = 0.0;
   out_2863715751586478773[125] = 0.0;
   out_2863715751586478773[126] = 0.0;
   out_2863715751586478773[127] = 0.0;
   out_2863715751586478773[128] = 0.0;
   out_2863715751586478773[129] = 0.0;
   out_2863715751586478773[130] = 0.0;
   out_2863715751586478773[131] = 0.0;
   out_2863715751586478773[132] = 0.0;
   out_2863715751586478773[133] = 1.0;
   out_2863715751586478773[134] = 0.0;
   out_2863715751586478773[135] = 0.0;
   out_2863715751586478773[136] = 0.0;
   out_2863715751586478773[137] = 0.0;
   out_2863715751586478773[138] = 0.0;
   out_2863715751586478773[139] = 0.0;
   out_2863715751586478773[140] = 0.0;
   out_2863715751586478773[141] = 0.0;
   out_2863715751586478773[142] = 0.0;
   out_2863715751586478773[143] = 0.0;
   out_2863715751586478773[144] = 0.0;
   out_2863715751586478773[145] = 0.0;
   out_2863715751586478773[146] = 0.0;
   out_2863715751586478773[147] = 0.0;
   out_2863715751586478773[148] = 0.0;
   out_2863715751586478773[149] = 0.0;
   out_2863715751586478773[150] = 0.0;
   out_2863715751586478773[151] = 0.0;
   out_2863715751586478773[152] = 1.0;
   out_2863715751586478773[153] = 0.0;
   out_2863715751586478773[154] = 0.0;
   out_2863715751586478773[155] = 0.0;
   out_2863715751586478773[156] = 0.0;
   out_2863715751586478773[157] = 0.0;
   out_2863715751586478773[158] = 0.0;
   out_2863715751586478773[159] = 0.0;
   out_2863715751586478773[160] = 0.0;
   out_2863715751586478773[161] = 0.0;
   out_2863715751586478773[162] = 0.0;
   out_2863715751586478773[163] = 0.0;
   out_2863715751586478773[164] = 0.0;
   out_2863715751586478773[165] = 0.0;
   out_2863715751586478773[166] = 0.0;
   out_2863715751586478773[167] = 0.0;
   out_2863715751586478773[168] = 0.0;
   out_2863715751586478773[169] = 0.0;
   out_2863715751586478773[170] = 0.0;
   out_2863715751586478773[171] = 1.0;
   out_2863715751586478773[172] = 0.0;
   out_2863715751586478773[173] = 0.0;
   out_2863715751586478773[174] = 0.0;
   out_2863715751586478773[175] = 0.0;
   out_2863715751586478773[176] = 0.0;
   out_2863715751586478773[177] = 0.0;
   out_2863715751586478773[178] = 0.0;
   out_2863715751586478773[179] = 0.0;
   out_2863715751586478773[180] = 0.0;
   out_2863715751586478773[181] = 0.0;
   out_2863715751586478773[182] = 0.0;
   out_2863715751586478773[183] = 0.0;
   out_2863715751586478773[184] = 0.0;
   out_2863715751586478773[185] = 0.0;
   out_2863715751586478773[186] = 0.0;
   out_2863715751586478773[187] = 0.0;
   out_2863715751586478773[188] = 0.0;
   out_2863715751586478773[189] = 0.0;
   out_2863715751586478773[190] = 1.0;
   out_2863715751586478773[191] = 0.0;
   out_2863715751586478773[192] = 0.0;
   out_2863715751586478773[193] = 0.0;
   out_2863715751586478773[194] = 0.0;
   out_2863715751586478773[195] = 0.0;
   out_2863715751586478773[196] = 0.0;
   out_2863715751586478773[197] = 0.0;
   out_2863715751586478773[198] = 0.0;
   out_2863715751586478773[199] = 0.0;
   out_2863715751586478773[200] = 0.0;
   out_2863715751586478773[201] = 0.0;
   out_2863715751586478773[202] = 0.0;
   out_2863715751586478773[203] = 0.0;
   out_2863715751586478773[204] = 0.0;
   out_2863715751586478773[205] = 0.0;
   out_2863715751586478773[206] = 0.0;
   out_2863715751586478773[207] = 0.0;
   out_2863715751586478773[208] = 0.0;
   out_2863715751586478773[209] = 1.0;
   out_2863715751586478773[210] = 0.0;
   out_2863715751586478773[211] = 0.0;
   out_2863715751586478773[212] = 0.0;
   out_2863715751586478773[213] = 0.0;
   out_2863715751586478773[214] = 0.0;
   out_2863715751586478773[215] = 0.0;
   out_2863715751586478773[216] = 0.0;
   out_2863715751586478773[217] = 0.0;
   out_2863715751586478773[218] = 0.0;
   out_2863715751586478773[219] = 0.0;
   out_2863715751586478773[220] = 0.0;
   out_2863715751586478773[221] = 0.0;
   out_2863715751586478773[222] = 0.0;
   out_2863715751586478773[223] = 0.0;
   out_2863715751586478773[224] = 0.0;
   out_2863715751586478773[225] = 0.0;
   out_2863715751586478773[226] = 0.0;
   out_2863715751586478773[227] = 0.0;
   out_2863715751586478773[228] = 1.0;
   out_2863715751586478773[229] = 0.0;
   out_2863715751586478773[230] = 0.0;
   out_2863715751586478773[231] = 0.0;
   out_2863715751586478773[232] = 0.0;
   out_2863715751586478773[233] = 0.0;
   out_2863715751586478773[234] = 0.0;
   out_2863715751586478773[235] = 0.0;
   out_2863715751586478773[236] = 0.0;
   out_2863715751586478773[237] = 0.0;
   out_2863715751586478773[238] = 0.0;
   out_2863715751586478773[239] = 0.0;
   out_2863715751586478773[240] = 0.0;
   out_2863715751586478773[241] = 0.0;
   out_2863715751586478773[242] = 0.0;
   out_2863715751586478773[243] = 0.0;
   out_2863715751586478773[244] = 0.0;
   out_2863715751586478773[245] = 0.0;
   out_2863715751586478773[246] = 0.0;
   out_2863715751586478773[247] = 1.0;
   out_2863715751586478773[248] = 0.0;
   out_2863715751586478773[249] = 0.0;
   out_2863715751586478773[250] = 0.0;
   out_2863715751586478773[251] = 0.0;
   out_2863715751586478773[252] = 0.0;
   out_2863715751586478773[253] = 0.0;
   out_2863715751586478773[254] = 0.0;
   out_2863715751586478773[255] = 0.0;
   out_2863715751586478773[256] = 0.0;
   out_2863715751586478773[257] = 0.0;
   out_2863715751586478773[258] = 0.0;
   out_2863715751586478773[259] = 0.0;
   out_2863715751586478773[260] = 0.0;
   out_2863715751586478773[261] = 0.0;
   out_2863715751586478773[262] = 0.0;
   out_2863715751586478773[263] = 0.0;
   out_2863715751586478773[264] = 0.0;
   out_2863715751586478773[265] = 0.0;
   out_2863715751586478773[266] = 1.0;
   out_2863715751586478773[267] = 0.0;
   out_2863715751586478773[268] = 0.0;
   out_2863715751586478773[269] = 0.0;
   out_2863715751586478773[270] = 0.0;
   out_2863715751586478773[271] = 0.0;
   out_2863715751586478773[272] = 0.0;
   out_2863715751586478773[273] = 0.0;
   out_2863715751586478773[274] = 0.0;
   out_2863715751586478773[275] = 0.0;
   out_2863715751586478773[276] = 0.0;
   out_2863715751586478773[277] = 0.0;
   out_2863715751586478773[278] = 0.0;
   out_2863715751586478773[279] = 0.0;
   out_2863715751586478773[280] = 0.0;
   out_2863715751586478773[281] = 0.0;
   out_2863715751586478773[282] = 0.0;
   out_2863715751586478773[283] = 0.0;
   out_2863715751586478773[284] = 0.0;
   out_2863715751586478773[285] = 1.0;
   out_2863715751586478773[286] = 0.0;
   out_2863715751586478773[287] = 0.0;
   out_2863715751586478773[288] = 0.0;
   out_2863715751586478773[289] = 0.0;
   out_2863715751586478773[290] = 0.0;
   out_2863715751586478773[291] = 0.0;
   out_2863715751586478773[292] = 0.0;
   out_2863715751586478773[293] = 0.0;
   out_2863715751586478773[294] = 0.0;
   out_2863715751586478773[295] = 0.0;
   out_2863715751586478773[296] = 0.0;
   out_2863715751586478773[297] = 0.0;
   out_2863715751586478773[298] = 0.0;
   out_2863715751586478773[299] = 0.0;
   out_2863715751586478773[300] = 0.0;
   out_2863715751586478773[301] = 0.0;
   out_2863715751586478773[302] = 0.0;
   out_2863715751586478773[303] = 0.0;
   out_2863715751586478773[304] = 1.0;
   out_2863715751586478773[305] = 0.0;
   out_2863715751586478773[306] = 0.0;
   out_2863715751586478773[307] = 0.0;
   out_2863715751586478773[308] = 0.0;
   out_2863715751586478773[309] = 0.0;
   out_2863715751586478773[310] = 0.0;
   out_2863715751586478773[311] = 0.0;
   out_2863715751586478773[312] = 0.0;
   out_2863715751586478773[313] = 0.0;
   out_2863715751586478773[314] = 0.0;
   out_2863715751586478773[315] = 0.0;
   out_2863715751586478773[316] = 0.0;
   out_2863715751586478773[317] = 0.0;
   out_2863715751586478773[318] = 0.0;
   out_2863715751586478773[319] = 0.0;
   out_2863715751586478773[320] = 0.0;
   out_2863715751586478773[321] = 0.0;
   out_2863715751586478773[322] = 0.0;
   out_2863715751586478773[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_4163803070138990822) {
   out_4163803070138990822[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_4163803070138990822[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_4163803070138990822[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_4163803070138990822[3] = dt*state[12] + state[3];
   out_4163803070138990822[4] = dt*state[13] + state[4];
   out_4163803070138990822[5] = dt*state[14] + state[5];
   out_4163803070138990822[6] = state[6];
   out_4163803070138990822[7] = state[7];
   out_4163803070138990822[8] = state[8];
   out_4163803070138990822[9] = state[9];
   out_4163803070138990822[10] = state[10];
   out_4163803070138990822[11] = state[11];
   out_4163803070138990822[12] = state[12];
   out_4163803070138990822[13] = state[13];
   out_4163803070138990822[14] = state[14];
   out_4163803070138990822[15] = state[15];
   out_4163803070138990822[16] = state[16];
   out_4163803070138990822[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3325085581577154625) {
   out_3325085581577154625[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3325085581577154625[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3325085581577154625[2] = 0;
   out_3325085581577154625[3] = 0;
   out_3325085581577154625[4] = 0;
   out_3325085581577154625[5] = 0;
   out_3325085581577154625[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3325085581577154625[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3325085581577154625[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3325085581577154625[9] = 0;
   out_3325085581577154625[10] = 0;
   out_3325085581577154625[11] = 0;
   out_3325085581577154625[12] = 0;
   out_3325085581577154625[13] = 0;
   out_3325085581577154625[14] = 0;
   out_3325085581577154625[15] = 0;
   out_3325085581577154625[16] = 0;
   out_3325085581577154625[17] = 0;
   out_3325085581577154625[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3325085581577154625[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3325085581577154625[20] = 0;
   out_3325085581577154625[21] = 0;
   out_3325085581577154625[22] = 0;
   out_3325085581577154625[23] = 0;
   out_3325085581577154625[24] = 0;
   out_3325085581577154625[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3325085581577154625[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3325085581577154625[27] = 0;
   out_3325085581577154625[28] = 0;
   out_3325085581577154625[29] = 0;
   out_3325085581577154625[30] = 0;
   out_3325085581577154625[31] = 0;
   out_3325085581577154625[32] = 0;
   out_3325085581577154625[33] = 0;
   out_3325085581577154625[34] = 0;
   out_3325085581577154625[35] = 0;
   out_3325085581577154625[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3325085581577154625[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3325085581577154625[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3325085581577154625[39] = 0;
   out_3325085581577154625[40] = 0;
   out_3325085581577154625[41] = 0;
   out_3325085581577154625[42] = 0;
   out_3325085581577154625[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3325085581577154625[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3325085581577154625[45] = 0;
   out_3325085581577154625[46] = 0;
   out_3325085581577154625[47] = 0;
   out_3325085581577154625[48] = 0;
   out_3325085581577154625[49] = 0;
   out_3325085581577154625[50] = 0;
   out_3325085581577154625[51] = 0;
   out_3325085581577154625[52] = 0;
   out_3325085581577154625[53] = 0;
   out_3325085581577154625[54] = 0;
   out_3325085581577154625[55] = 0;
   out_3325085581577154625[56] = 0;
   out_3325085581577154625[57] = 1;
   out_3325085581577154625[58] = 0;
   out_3325085581577154625[59] = 0;
   out_3325085581577154625[60] = 0;
   out_3325085581577154625[61] = 0;
   out_3325085581577154625[62] = 0;
   out_3325085581577154625[63] = 0;
   out_3325085581577154625[64] = 0;
   out_3325085581577154625[65] = 0;
   out_3325085581577154625[66] = dt;
   out_3325085581577154625[67] = 0;
   out_3325085581577154625[68] = 0;
   out_3325085581577154625[69] = 0;
   out_3325085581577154625[70] = 0;
   out_3325085581577154625[71] = 0;
   out_3325085581577154625[72] = 0;
   out_3325085581577154625[73] = 0;
   out_3325085581577154625[74] = 0;
   out_3325085581577154625[75] = 0;
   out_3325085581577154625[76] = 1;
   out_3325085581577154625[77] = 0;
   out_3325085581577154625[78] = 0;
   out_3325085581577154625[79] = 0;
   out_3325085581577154625[80] = 0;
   out_3325085581577154625[81] = 0;
   out_3325085581577154625[82] = 0;
   out_3325085581577154625[83] = 0;
   out_3325085581577154625[84] = 0;
   out_3325085581577154625[85] = dt;
   out_3325085581577154625[86] = 0;
   out_3325085581577154625[87] = 0;
   out_3325085581577154625[88] = 0;
   out_3325085581577154625[89] = 0;
   out_3325085581577154625[90] = 0;
   out_3325085581577154625[91] = 0;
   out_3325085581577154625[92] = 0;
   out_3325085581577154625[93] = 0;
   out_3325085581577154625[94] = 0;
   out_3325085581577154625[95] = 1;
   out_3325085581577154625[96] = 0;
   out_3325085581577154625[97] = 0;
   out_3325085581577154625[98] = 0;
   out_3325085581577154625[99] = 0;
   out_3325085581577154625[100] = 0;
   out_3325085581577154625[101] = 0;
   out_3325085581577154625[102] = 0;
   out_3325085581577154625[103] = 0;
   out_3325085581577154625[104] = dt;
   out_3325085581577154625[105] = 0;
   out_3325085581577154625[106] = 0;
   out_3325085581577154625[107] = 0;
   out_3325085581577154625[108] = 0;
   out_3325085581577154625[109] = 0;
   out_3325085581577154625[110] = 0;
   out_3325085581577154625[111] = 0;
   out_3325085581577154625[112] = 0;
   out_3325085581577154625[113] = 0;
   out_3325085581577154625[114] = 1;
   out_3325085581577154625[115] = 0;
   out_3325085581577154625[116] = 0;
   out_3325085581577154625[117] = 0;
   out_3325085581577154625[118] = 0;
   out_3325085581577154625[119] = 0;
   out_3325085581577154625[120] = 0;
   out_3325085581577154625[121] = 0;
   out_3325085581577154625[122] = 0;
   out_3325085581577154625[123] = 0;
   out_3325085581577154625[124] = 0;
   out_3325085581577154625[125] = 0;
   out_3325085581577154625[126] = 0;
   out_3325085581577154625[127] = 0;
   out_3325085581577154625[128] = 0;
   out_3325085581577154625[129] = 0;
   out_3325085581577154625[130] = 0;
   out_3325085581577154625[131] = 0;
   out_3325085581577154625[132] = 0;
   out_3325085581577154625[133] = 1;
   out_3325085581577154625[134] = 0;
   out_3325085581577154625[135] = 0;
   out_3325085581577154625[136] = 0;
   out_3325085581577154625[137] = 0;
   out_3325085581577154625[138] = 0;
   out_3325085581577154625[139] = 0;
   out_3325085581577154625[140] = 0;
   out_3325085581577154625[141] = 0;
   out_3325085581577154625[142] = 0;
   out_3325085581577154625[143] = 0;
   out_3325085581577154625[144] = 0;
   out_3325085581577154625[145] = 0;
   out_3325085581577154625[146] = 0;
   out_3325085581577154625[147] = 0;
   out_3325085581577154625[148] = 0;
   out_3325085581577154625[149] = 0;
   out_3325085581577154625[150] = 0;
   out_3325085581577154625[151] = 0;
   out_3325085581577154625[152] = 1;
   out_3325085581577154625[153] = 0;
   out_3325085581577154625[154] = 0;
   out_3325085581577154625[155] = 0;
   out_3325085581577154625[156] = 0;
   out_3325085581577154625[157] = 0;
   out_3325085581577154625[158] = 0;
   out_3325085581577154625[159] = 0;
   out_3325085581577154625[160] = 0;
   out_3325085581577154625[161] = 0;
   out_3325085581577154625[162] = 0;
   out_3325085581577154625[163] = 0;
   out_3325085581577154625[164] = 0;
   out_3325085581577154625[165] = 0;
   out_3325085581577154625[166] = 0;
   out_3325085581577154625[167] = 0;
   out_3325085581577154625[168] = 0;
   out_3325085581577154625[169] = 0;
   out_3325085581577154625[170] = 0;
   out_3325085581577154625[171] = 1;
   out_3325085581577154625[172] = 0;
   out_3325085581577154625[173] = 0;
   out_3325085581577154625[174] = 0;
   out_3325085581577154625[175] = 0;
   out_3325085581577154625[176] = 0;
   out_3325085581577154625[177] = 0;
   out_3325085581577154625[178] = 0;
   out_3325085581577154625[179] = 0;
   out_3325085581577154625[180] = 0;
   out_3325085581577154625[181] = 0;
   out_3325085581577154625[182] = 0;
   out_3325085581577154625[183] = 0;
   out_3325085581577154625[184] = 0;
   out_3325085581577154625[185] = 0;
   out_3325085581577154625[186] = 0;
   out_3325085581577154625[187] = 0;
   out_3325085581577154625[188] = 0;
   out_3325085581577154625[189] = 0;
   out_3325085581577154625[190] = 1;
   out_3325085581577154625[191] = 0;
   out_3325085581577154625[192] = 0;
   out_3325085581577154625[193] = 0;
   out_3325085581577154625[194] = 0;
   out_3325085581577154625[195] = 0;
   out_3325085581577154625[196] = 0;
   out_3325085581577154625[197] = 0;
   out_3325085581577154625[198] = 0;
   out_3325085581577154625[199] = 0;
   out_3325085581577154625[200] = 0;
   out_3325085581577154625[201] = 0;
   out_3325085581577154625[202] = 0;
   out_3325085581577154625[203] = 0;
   out_3325085581577154625[204] = 0;
   out_3325085581577154625[205] = 0;
   out_3325085581577154625[206] = 0;
   out_3325085581577154625[207] = 0;
   out_3325085581577154625[208] = 0;
   out_3325085581577154625[209] = 1;
   out_3325085581577154625[210] = 0;
   out_3325085581577154625[211] = 0;
   out_3325085581577154625[212] = 0;
   out_3325085581577154625[213] = 0;
   out_3325085581577154625[214] = 0;
   out_3325085581577154625[215] = 0;
   out_3325085581577154625[216] = 0;
   out_3325085581577154625[217] = 0;
   out_3325085581577154625[218] = 0;
   out_3325085581577154625[219] = 0;
   out_3325085581577154625[220] = 0;
   out_3325085581577154625[221] = 0;
   out_3325085581577154625[222] = 0;
   out_3325085581577154625[223] = 0;
   out_3325085581577154625[224] = 0;
   out_3325085581577154625[225] = 0;
   out_3325085581577154625[226] = 0;
   out_3325085581577154625[227] = 0;
   out_3325085581577154625[228] = 1;
   out_3325085581577154625[229] = 0;
   out_3325085581577154625[230] = 0;
   out_3325085581577154625[231] = 0;
   out_3325085581577154625[232] = 0;
   out_3325085581577154625[233] = 0;
   out_3325085581577154625[234] = 0;
   out_3325085581577154625[235] = 0;
   out_3325085581577154625[236] = 0;
   out_3325085581577154625[237] = 0;
   out_3325085581577154625[238] = 0;
   out_3325085581577154625[239] = 0;
   out_3325085581577154625[240] = 0;
   out_3325085581577154625[241] = 0;
   out_3325085581577154625[242] = 0;
   out_3325085581577154625[243] = 0;
   out_3325085581577154625[244] = 0;
   out_3325085581577154625[245] = 0;
   out_3325085581577154625[246] = 0;
   out_3325085581577154625[247] = 1;
   out_3325085581577154625[248] = 0;
   out_3325085581577154625[249] = 0;
   out_3325085581577154625[250] = 0;
   out_3325085581577154625[251] = 0;
   out_3325085581577154625[252] = 0;
   out_3325085581577154625[253] = 0;
   out_3325085581577154625[254] = 0;
   out_3325085581577154625[255] = 0;
   out_3325085581577154625[256] = 0;
   out_3325085581577154625[257] = 0;
   out_3325085581577154625[258] = 0;
   out_3325085581577154625[259] = 0;
   out_3325085581577154625[260] = 0;
   out_3325085581577154625[261] = 0;
   out_3325085581577154625[262] = 0;
   out_3325085581577154625[263] = 0;
   out_3325085581577154625[264] = 0;
   out_3325085581577154625[265] = 0;
   out_3325085581577154625[266] = 1;
   out_3325085581577154625[267] = 0;
   out_3325085581577154625[268] = 0;
   out_3325085581577154625[269] = 0;
   out_3325085581577154625[270] = 0;
   out_3325085581577154625[271] = 0;
   out_3325085581577154625[272] = 0;
   out_3325085581577154625[273] = 0;
   out_3325085581577154625[274] = 0;
   out_3325085581577154625[275] = 0;
   out_3325085581577154625[276] = 0;
   out_3325085581577154625[277] = 0;
   out_3325085581577154625[278] = 0;
   out_3325085581577154625[279] = 0;
   out_3325085581577154625[280] = 0;
   out_3325085581577154625[281] = 0;
   out_3325085581577154625[282] = 0;
   out_3325085581577154625[283] = 0;
   out_3325085581577154625[284] = 0;
   out_3325085581577154625[285] = 1;
   out_3325085581577154625[286] = 0;
   out_3325085581577154625[287] = 0;
   out_3325085581577154625[288] = 0;
   out_3325085581577154625[289] = 0;
   out_3325085581577154625[290] = 0;
   out_3325085581577154625[291] = 0;
   out_3325085581577154625[292] = 0;
   out_3325085581577154625[293] = 0;
   out_3325085581577154625[294] = 0;
   out_3325085581577154625[295] = 0;
   out_3325085581577154625[296] = 0;
   out_3325085581577154625[297] = 0;
   out_3325085581577154625[298] = 0;
   out_3325085581577154625[299] = 0;
   out_3325085581577154625[300] = 0;
   out_3325085581577154625[301] = 0;
   out_3325085581577154625[302] = 0;
   out_3325085581577154625[303] = 0;
   out_3325085581577154625[304] = 1;
   out_3325085581577154625[305] = 0;
   out_3325085581577154625[306] = 0;
   out_3325085581577154625[307] = 0;
   out_3325085581577154625[308] = 0;
   out_3325085581577154625[309] = 0;
   out_3325085581577154625[310] = 0;
   out_3325085581577154625[311] = 0;
   out_3325085581577154625[312] = 0;
   out_3325085581577154625[313] = 0;
   out_3325085581577154625[314] = 0;
   out_3325085581577154625[315] = 0;
   out_3325085581577154625[316] = 0;
   out_3325085581577154625[317] = 0;
   out_3325085581577154625[318] = 0;
   out_3325085581577154625[319] = 0;
   out_3325085581577154625[320] = 0;
   out_3325085581577154625[321] = 0;
   out_3325085581577154625[322] = 0;
   out_3325085581577154625[323] = 1;
}
void h_4(double *state, double *unused, double *out_1824245269356126422) {
   out_1824245269356126422[0] = state[6] + state[9];
   out_1824245269356126422[1] = state[7] + state[10];
   out_1824245269356126422[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_908091458257428799) {
   out_908091458257428799[0] = 0;
   out_908091458257428799[1] = 0;
   out_908091458257428799[2] = 0;
   out_908091458257428799[3] = 0;
   out_908091458257428799[4] = 0;
   out_908091458257428799[5] = 0;
   out_908091458257428799[6] = 1;
   out_908091458257428799[7] = 0;
   out_908091458257428799[8] = 0;
   out_908091458257428799[9] = 1;
   out_908091458257428799[10] = 0;
   out_908091458257428799[11] = 0;
   out_908091458257428799[12] = 0;
   out_908091458257428799[13] = 0;
   out_908091458257428799[14] = 0;
   out_908091458257428799[15] = 0;
   out_908091458257428799[16] = 0;
   out_908091458257428799[17] = 0;
   out_908091458257428799[18] = 0;
   out_908091458257428799[19] = 0;
   out_908091458257428799[20] = 0;
   out_908091458257428799[21] = 0;
   out_908091458257428799[22] = 0;
   out_908091458257428799[23] = 0;
   out_908091458257428799[24] = 0;
   out_908091458257428799[25] = 1;
   out_908091458257428799[26] = 0;
   out_908091458257428799[27] = 0;
   out_908091458257428799[28] = 1;
   out_908091458257428799[29] = 0;
   out_908091458257428799[30] = 0;
   out_908091458257428799[31] = 0;
   out_908091458257428799[32] = 0;
   out_908091458257428799[33] = 0;
   out_908091458257428799[34] = 0;
   out_908091458257428799[35] = 0;
   out_908091458257428799[36] = 0;
   out_908091458257428799[37] = 0;
   out_908091458257428799[38] = 0;
   out_908091458257428799[39] = 0;
   out_908091458257428799[40] = 0;
   out_908091458257428799[41] = 0;
   out_908091458257428799[42] = 0;
   out_908091458257428799[43] = 0;
   out_908091458257428799[44] = 1;
   out_908091458257428799[45] = 0;
   out_908091458257428799[46] = 0;
   out_908091458257428799[47] = 1;
   out_908091458257428799[48] = 0;
   out_908091458257428799[49] = 0;
   out_908091458257428799[50] = 0;
   out_908091458257428799[51] = 0;
   out_908091458257428799[52] = 0;
   out_908091458257428799[53] = 0;
}
void h_10(double *state, double *unused, double *out_5992945801677381357) {
   out_5992945801677381357[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_5992945801677381357[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_5992945801677381357[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_1551783764236526040) {
   out_1551783764236526040[0] = 0;
   out_1551783764236526040[1] = 9.8100000000000005*cos(state[1]);
   out_1551783764236526040[2] = 0;
   out_1551783764236526040[3] = 0;
   out_1551783764236526040[4] = -state[8];
   out_1551783764236526040[5] = state[7];
   out_1551783764236526040[6] = 0;
   out_1551783764236526040[7] = state[5];
   out_1551783764236526040[8] = -state[4];
   out_1551783764236526040[9] = 0;
   out_1551783764236526040[10] = 0;
   out_1551783764236526040[11] = 0;
   out_1551783764236526040[12] = 1;
   out_1551783764236526040[13] = 0;
   out_1551783764236526040[14] = 0;
   out_1551783764236526040[15] = 1;
   out_1551783764236526040[16] = 0;
   out_1551783764236526040[17] = 0;
   out_1551783764236526040[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_1551783764236526040[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_1551783764236526040[20] = 0;
   out_1551783764236526040[21] = state[8];
   out_1551783764236526040[22] = 0;
   out_1551783764236526040[23] = -state[6];
   out_1551783764236526040[24] = -state[5];
   out_1551783764236526040[25] = 0;
   out_1551783764236526040[26] = state[3];
   out_1551783764236526040[27] = 0;
   out_1551783764236526040[28] = 0;
   out_1551783764236526040[29] = 0;
   out_1551783764236526040[30] = 0;
   out_1551783764236526040[31] = 1;
   out_1551783764236526040[32] = 0;
   out_1551783764236526040[33] = 0;
   out_1551783764236526040[34] = 1;
   out_1551783764236526040[35] = 0;
   out_1551783764236526040[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_1551783764236526040[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_1551783764236526040[38] = 0;
   out_1551783764236526040[39] = -state[7];
   out_1551783764236526040[40] = state[6];
   out_1551783764236526040[41] = 0;
   out_1551783764236526040[42] = state[4];
   out_1551783764236526040[43] = -state[3];
   out_1551783764236526040[44] = 0;
   out_1551783764236526040[45] = 0;
   out_1551783764236526040[46] = 0;
   out_1551783764236526040[47] = 0;
   out_1551783764236526040[48] = 0;
   out_1551783764236526040[49] = 0;
   out_1551783764236526040[50] = 1;
   out_1551783764236526040[51] = 0;
   out_1551783764236526040[52] = 0;
   out_1551783764236526040[53] = 1;
}
void h_13(double *state, double *unused, double *out_4650572533653388734) {
   out_4650572533653388734[0] = state[3];
   out_4650572533653388734[1] = state[4];
   out_4650572533653388734[2] = state[5];
}
void H_13(double *state, double *unused, double *out_1472693377939272903) {
   out_1472693377939272903[0] = 0;
   out_1472693377939272903[1] = 0;
   out_1472693377939272903[2] = 0;
   out_1472693377939272903[3] = 1;
   out_1472693377939272903[4] = 0;
   out_1472693377939272903[5] = 0;
   out_1472693377939272903[6] = 0;
   out_1472693377939272903[7] = 0;
   out_1472693377939272903[8] = 0;
   out_1472693377939272903[9] = 0;
   out_1472693377939272903[10] = 0;
   out_1472693377939272903[11] = 0;
   out_1472693377939272903[12] = 0;
   out_1472693377939272903[13] = 0;
   out_1472693377939272903[14] = 0;
   out_1472693377939272903[15] = 0;
   out_1472693377939272903[16] = 0;
   out_1472693377939272903[17] = 0;
   out_1472693377939272903[18] = 0;
   out_1472693377939272903[19] = 0;
   out_1472693377939272903[20] = 0;
   out_1472693377939272903[21] = 0;
   out_1472693377939272903[22] = 1;
   out_1472693377939272903[23] = 0;
   out_1472693377939272903[24] = 0;
   out_1472693377939272903[25] = 0;
   out_1472693377939272903[26] = 0;
   out_1472693377939272903[27] = 0;
   out_1472693377939272903[28] = 0;
   out_1472693377939272903[29] = 0;
   out_1472693377939272903[30] = 0;
   out_1472693377939272903[31] = 0;
   out_1472693377939272903[32] = 0;
   out_1472693377939272903[33] = 0;
   out_1472693377939272903[34] = 0;
   out_1472693377939272903[35] = 0;
   out_1472693377939272903[36] = 0;
   out_1472693377939272903[37] = 0;
   out_1472693377939272903[38] = 0;
   out_1472693377939272903[39] = 0;
   out_1472693377939272903[40] = 0;
   out_1472693377939272903[41] = 1;
   out_1472693377939272903[42] = 0;
   out_1472693377939272903[43] = 0;
   out_1472693377939272903[44] = 0;
   out_1472693377939272903[45] = 0;
   out_1472693377939272903[46] = 0;
   out_1472693377939272903[47] = 0;
   out_1472693377939272903[48] = 0;
   out_1472693377939272903[49] = 0;
   out_1472693377939272903[50] = 0;
   out_1472693377939272903[51] = 0;
   out_1472693377939272903[52] = 0;
   out_1472693377939272903[53] = 0;
}
void h_14(double *state, double *unused, double *out_9091688560146400700) {
   out_9091688560146400700[0] = state[6];
   out_9091688560146400700[1] = state[7];
   out_9091688560146400700[2] = state[8];
}
void H_14(double *state, double *unused, double *out_2174696974037943497) {
   out_2174696974037943497[0] = 0;
   out_2174696974037943497[1] = 0;
   out_2174696974037943497[2] = 0;
   out_2174696974037943497[3] = 0;
   out_2174696974037943497[4] = 0;
   out_2174696974037943497[5] = 0;
   out_2174696974037943497[6] = 1;
   out_2174696974037943497[7] = 0;
   out_2174696974037943497[8] = 0;
   out_2174696974037943497[9] = 0;
   out_2174696974037943497[10] = 0;
   out_2174696974037943497[11] = 0;
   out_2174696974037943497[12] = 0;
   out_2174696974037943497[13] = 0;
   out_2174696974037943497[14] = 0;
   out_2174696974037943497[15] = 0;
   out_2174696974037943497[16] = 0;
   out_2174696974037943497[17] = 0;
   out_2174696974037943497[18] = 0;
   out_2174696974037943497[19] = 0;
   out_2174696974037943497[20] = 0;
   out_2174696974037943497[21] = 0;
   out_2174696974037943497[22] = 0;
   out_2174696974037943497[23] = 0;
   out_2174696974037943497[24] = 0;
   out_2174696974037943497[25] = 1;
   out_2174696974037943497[26] = 0;
   out_2174696974037943497[27] = 0;
   out_2174696974037943497[28] = 0;
   out_2174696974037943497[29] = 0;
   out_2174696974037943497[30] = 0;
   out_2174696974037943497[31] = 0;
   out_2174696974037943497[32] = 0;
   out_2174696974037943497[33] = 0;
   out_2174696974037943497[34] = 0;
   out_2174696974037943497[35] = 0;
   out_2174696974037943497[36] = 0;
   out_2174696974037943497[37] = 0;
   out_2174696974037943497[38] = 0;
   out_2174696974037943497[39] = 0;
   out_2174696974037943497[40] = 0;
   out_2174696974037943497[41] = 0;
   out_2174696974037943497[42] = 0;
   out_2174696974037943497[43] = 0;
   out_2174696974037943497[44] = 1;
   out_2174696974037943497[45] = 0;
   out_2174696974037943497[46] = 0;
   out_2174696974037943497[47] = 0;
   out_2174696974037943497[48] = 0;
   out_2174696974037943497[49] = 0;
   out_2174696974037943497[50] = 0;
   out_2174696974037943497[51] = 0;
   out_2174696974037943497[52] = 0;
   out_2174696974037943497[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_7352766374165151156) {
  err_fun(nom_x, delta_x, out_7352766374165151156);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5943722919136625654) {
  inv_err_fun(nom_x, true_x, out_5943722919136625654);
}
void pose_H_mod_fun(double *state, double *out_2863715751586478773) {
  H_mod_fun(state, out_2863715751586478773);
}
void pose_f_fun(double *state, double dt, double *out_4163803070138990822) {
  f_fun(state,  dt, out_4163803070138990822);
}
void pose_F_fun(double *state, double dt, double *out_3325085581577154625) {
  F_fun(state,  dt, out_3325085581577154625);
}
void pose_h_4(double *state, double *unused, double *out_1824245269356126422) {
  h_4(state, unused, out_1824245269356126422);
}
void pose_H_4(double *state, double *unused, double *out_908091458257428799) {
  H_4(state, unused, out_908091458257428799);
}
void pose_h_10(double *state, double *unused, double *out_5992945801677381357) {
  h_10(state, unused, out_5992945801677381357);
}
void pose_H_10(double *state, double *unused, double *out_1551783764236526040) {
  H_10(state, unused, out_1551783764236526040);
}
void pose_h_13(double *state, double *unused, double *out_4650572533653388734) {
  h_13(state, unused, out_4650572533653388734);
}
void pose_H_13(double *state, double *unused, double *out_1472693377939272903) {
  H_13(state, unused, out_1472693377939272903);
}
void pose_h_14(double *state, double *unused, double *out_9091688560146400700) {
  h_14(state, unused, out_9091688560146400700);
}
void pose_H_14(double *state, double *unused, double *out_2174696974037943497) {
  H_14(state, unused, out_2174696974037943497);
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

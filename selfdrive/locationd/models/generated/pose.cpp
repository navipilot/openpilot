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
void err_fun(double *nom_x, double *delta_x, double *out_5792379923597347601) {
   out_5792379923597347601[0] = delta_x[0] + nom_x[0];
   out_5792379923597347601[1] = delta_x[1] + nom_x[1];
   out_5792379923597347601[2] = delta_x[2] + nom_x[2];
   out_5792379923597347601[3] = delta_x[3] + nom_x[3];
   out_5792379923597347601[4] = delta_x[4] + nom_x[4];
   out_5792379923597347601[5] = delta_x[5] + nom_x[5];
   out_5792379923597347601[6] = delta_x[6] + nom_x[6];
   out_5792379923597347601[7] = delta_x[7] + nom_x[7];
   out_5792379923597347601[8] = delta_x[8] + nom_x[8];
   out_5792379923597347601[9] = delta_x[9] + nom_x[9];
   out_5792379923597347601[10] = delta_x[10] + nom_x[10];
   out_5792379923597347601[11] = delta_x[11] + nom_x[11];
   out_5792379923597347601[12] = delta_x[12] + nom_x[12];
   out_5792379923597347601[13] = delta_x[13] + nom_x[13];
   out_5792379923597347601[14] = delta_x[14] + nom_x[14];
   out_5792379923597347601[15] = delta_x[15] + nom_x[15];
   out_5792379923597347601[16] = delta_x[16] + nom_x[16];
   out_5792379923597347601[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3127222841440964837) {
   out_3127222841440964837[0] = -nom_x[0] + true_x[0];
   out_3127222841440964837[1] = -nom_x[1] + true_x[1];
   out_3127222841440964837[2] = -nom_x[2] + true_x[2];
   out_3127222841440964837[3] = -nom_x[3] + true_x[3];
   out_3127222841440964837[4] = -nom_x[4] + true_x[4];
   out_3127222841440964837[5] = -nom_x[5] + true_x[5];
   out_3127222841440964837[6] = -nom_x[6] + true_x[6];
   out_3127222841440964837[7] = -nom_x[7] + true_x[7];
   out_3127222841440964837[8] = -nom_x[8] + true_x[8];
   out_3127222841440964837[9] = -nom_x[9] + true_x[9];
   out_3127222841440964837[10] = -nom_x[10] + true_x[10];
   out_3127222841440964837[11] = -nom_x[11] + true_x[11];
   out_3127222841440964837[12] = -nom_x[12] + true_x[12];
   out_3127222841440964837[13] = -nom_x[13] + true_x[13];
   out_3127222841440964837[14] = -nom_x[14] + true_x[14];
   out_3127222841440964837[15] = -nom_x[15] + true_x[15];
   out_3127222841440964837[16] = -nom_x[16] + true_x[16];
   out_3127222841440964837[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_1474962527938332194) {
   out_1474962527938332194[0] = 1.0;
   out_1474962527938332194[1] = 0.0;
   out_1474962527938332194[2] = 0.0;
   out_1474962527938332194[3] = 0.0;
   out_1474962527938332194[4] = 0.0;
   out_1474962527938332194[5] = 0.0;
   out_1474962527938332194[6] = 0.0;
   out_1474962527938332194[7] = 0.0;
   out_1474962527938332194[8] = 0.0;
   out_1474962527938332194[9] = 0.0;
   out_1474962527938332194[10] = 0.0;
   out_1474962527938332194[11] = 0.0;
   out_1474962527938332194[12] = 0.0;
   out_1474962527938332194[13] = 0.0;
   out_1474962527938332194[14] = 0.0;
   out_1474962527938332194[15] = 0.0;
   out_1474962527938332194[16] = 0.0;
   out_1474962527938332194[17] = 0.0;
   out_1474962527938332194[18] = 0.0;
   out_1474962527938332194[19] = 1.0;
   out_1474962527938332194[20] = 0.0;
   out_1474962527938332194[21] = 0.0;
   out_1474962527938332194[22] = 0.0;
   out_1474962527938332194[23] = 0.0;
   out_1474962527938332194[24] = 0.0;
   out_1474962527938332194[25] = 0.0;
   out_1474962527938332194[26] = 0.0;
   out_1474962527938332194[27] = 0.0;
   out_1474962527938332194[28] = 0.0;
   out_1474962527938332194[29] = 0.0;
   out_1474962527938332194[30] = 0.0;
   out_1474962527938332194[31] = 0.0;
   out_1474962527938332194[32] = 0.0;
   out_1474962527938332194[33] = 0.0;
   out_1474962527938332194[34] = 0.0;
   out_1474962527938332194[35] = 0.0;
   out_1474962527938332194[36] = 0.0;
   out_1474962527938332194[37] = 0.0;
   out_1474962527938332194[38] = 1.0;
   out_1474962527938332194[39] = 0.0;
   out_1474962527938332194[40] = 0.0;
   out_1474962527938332194[41] = 0.0;
   out_1474962527938332194[42] = 0.0;
   out_1474962527938332194[43] = 0.0;
   out_1474962527938332194[44] = 0.0;
   out_1474962527938332194[45] = 0.0;
   out_1474962527938332194[46] = 0.0;
   out_1474962527938332194[47] = 0.0;
   out_1474962527938332194[48] = 0.0;
   out_1474962527938332194[49] = 0.0;
   out_1474962527938332194[50] = 0.0;
   out_1474962527938332194[51] = 0.0;
   out_1474962527938332194[52] = 0.0;
   out_1474962527938332194[53] = 0.0;
   out_1474962527938332194[54] = 0.0;
   out_1474962527938332194[55] = 0.0;
   out_1474962527938332194[56] = 0.0;
   out_1474962527938332194[57] = 1.0;
   out_1474962527938332194[58] = 0.0;
   out_1474962527938332194[59] = 0.0;
   out_1474962527938332194[60] = 0.0;
   out_1474962527938332194[61] = 0.0;
   out_1474962527938332194[62] = 0.0;
   out_1474962527938332194[63] = 0.0;
   out_1474962527938332194[64] = 0.0;
   out_1474962527938332194[65] = 0.0;
   out_1474962527938332194[66] = 0.0;
   out_1474962527938332194[67] = 0.0;
   out_1474962527938332194[68] = 0.0;
   out_1474962527938332194[69] = 0.0;
   out_1474962527938332194[70] = 0.0;
   out_1474962527938332194[71] = 0.0;
   out_1474962527938332194[72] = 0.0;
   out_1474962527938332194[73] = 0.0;
   out_1474962527938332194[74] = 0.0;
   out_1474962527938332194[75] = 0.0;
   out_1474962527938332194[76] = 1.0;
   out_1474962527938332194[77] = 0.0;
   out_1474962527938332194[78] = 0.0;
   out_1474962527938332194[79] = 0.0;
   out_1474962527938332194[80] = 0.0;
   out_1474962527938332194[81] = 0.0;
   out_1474962527938332194[82] = 0.0;
   out_1474962527938332194[83] = 0.0;
   out_1474962527938332194[84] = 0.0;
   out_1474962527938332194[85] = 0.0;
   out_1474962527938332194[86] = 0.0;
   out_1474962527938332194[87] = 0.0;
   out_1474962527938332194[88] = 0.0;
   out_1474962527938332194[89] = 0.0;
   out_1474962527938332194[90] = 0.0;
   out_1474962527938332194[91] = 0.0;
   out_1474962527938332194[92] = 0.0;
   out_1474962527938332194[93] = 0.0;
   out_1474962527938332194[94] = 0.0;
   out_1474962527938332194[95] = 1.0;
   out_1474962527938332194[96] = 0.0;
   out_1474962527938332194[97] = 0.0;
   out_1474962527938332194[98] = 0.0;
   out_1474962527938332194[99] = 0.0;
   out_1474962527938332194[100] = 0.0;
   out_1474962527938332194[101] = 0.0;
   out_1474962527938332194[102] = 0.0;
   out_1474962527938332194[103] = 0.0;
   out_1474962527938332194[104] = 0.0;
   out_1474962527938332194[105] = 0.0;
   out_1474962527938332194[106] = 0.0;
   out_1474962527938332194[107] = 0.0;
   out_1474962527938332194[108] = 0.0;
   out_1474962527938332194[109] = 0.0;
   out_1474962527938332194[110] = 0.0;
   out_1474962527938332194[111] = 0.0;
   out_1474962527938332194[112] = 0.0;
   out_1474962527938332194[113] = 0.0;
   out_1474962527938332194[114] = 1.0;
   out_1474962527938332194[115] = 0.0;
   out_1474962527938332194[116] = 0.0;
   out_1474962527938332194[117] = 0.0;
   out_1474962527938332194[118] = 0.0;
   out_1474962527938332194[119] = 0.0;
   out_1474962527938332194[120] = 0.0;
   out_1474962527938332194[121] = 0.0;
   out_1474962527938332194[122] = 0.0;
   out_1474962527938332194[123] = 0.0;
   out_1474962527938332194[124] = 0.0;
   out_1474962527938332194[125] = 0.0;
   out_1474962527938332194[126] = 0.0;
   out_1474962527938332194[127] = 0.0;
   out_1474962527938332194[128] = 0.0;
   out_1474962527938332194[129] = 0.0;
   out_1474962527938332194[130] = 0.0;
   out_1474962527938332194[131] = 0.0;
   out_1474962527938332194[132] = 0.0;
   out_1474962527938332194[133] = 1.0;
   out_1474962527938332194[134] = 0.0;
   out_1474962527938332194[135] = 0.0;
   out_1474962527938332194[136] = 0.0;
   out_1474962527938332194[137] = 0.0;
   out_1474962527938332194[138] = 0.0;
   out_1474962527938332194[139] = 0.0;
   out_1474962527938332194[140] = 0.0;
   out_1474962527938332194[141] = 0.0;
   out_1474962527938332194[142] = 0.0;
   out_1474962527938332194[143] = 0.0;
   out_1474962527938332194[144] = 0.0;
   out_1474962527938332194[145] = 0.0;
   out_1474962527938332194[146] = 0.0;
   out_1474962527938332194[147] = 0.0;
   out_1474962527938332194[148] = 0.0;
   out_1474962527938332194[149] = 0.0;
   out_1474962527938332194[150] = 0.0;
   out_1474962527938332194[151] = 0.0;
   out_1474962527938332194[152] = 1.0;
   out_1474962527938332194[153] = 0.0;
   out_1474962527938332194[154] = 0.0;
   out_1474962527938332194[155] = 0.0;
   out_1474962527938332194[156] = 0.0;
   out_1474962527938332194[157] = 0.0;
   out_1474962527938332194[158] = 0.0;
   out_1474962527938332194[159] = 0.0;
   out_1474962527938332194[160] = 0.0;
   out_1474962527938332194[161] = 0.0;
   out_1474962527938332194[162] = 0.0;
   out_1474962527938332194[163] = 0.0;
   out_1474962527938332194[164] = 0.0;
   out_1474962527938332194[165] = 0.0;
   out_1474962527938332194[166] = 0.0;
   out_1474962527938332194[167] = 0.0;
   out_1474962527938332194[168] = 0.0;
   out_1474962527938332194[169] = 0.0;
   out_1474962527938332194[170] = 0.0;
   out_1474962527938332194[171] = 1.0;
   out_1474962527938332194[172] = 0.0;
   out_1474962527938332194[173] = 0.0;
   out_1474962527938332194[174] = 0.0;
   out_1474962527938332194[175] = 0.0;
   out_1474962527938332194[176] = 0.0;
   out_1474962527938332194[177] = 0.0;
   out_1474962527938332194[178] = 0.0;
   out_1474962527938332194[179] = 0.0;
   out_1474962527938332194[180] = 0.0;
   out_1474962527938332194[181] = 0.0;
   out_1474962527938332194[182] = 0.0;
   out_1474962527938332194[183] = 0.0;
   out_1474962527938332194[184] = 0.0;
   out_1474962527938332194[185] = 0.0;
   out_1474962527938332194[186] = 0.0;
   out_1474962527938332194[187] = 0.0;
   out_1474962527938332194[188] = 0.0;
   out_1474962527938332194[189] = 0.0;
   out_1474962527938332194[190] = 1.0;
   out_1474962527938332194[191] = 0.0;
   out_1474962527938332194[192] = 0.0;
   out_1474962527938332194[193] = 0.0;
   out_1474962527938332194[194] = 0.0;
   out_1474962527938332194[195] = 0.0;
   out_1474962527938332194[196] = 0.0;
   out_1474962527938332194[197] = 0.0;
   out_1474962527938332194[198] = 0.0;
   out_1474962527938332194[199] = 0.0;
   out_1474962527938332194[200] = 0.0;
   out_1474962527938332194[201] = 0.0;
   out_1474962527938332194[202] = 0.0;
   out_1474962527938332194[203] = 0.0;
   out_1474962527938332194[204] = 0.0;
   out_1474962527938332194[205] = 0.0;
   out_1474962527938332194[206] = 0.0;
   out_1474962527938332194[207] = 0.0;
   out_1474962527938332194[208] = 0.0;
   out_1474962527938332194[209] = 1.0;
   out_1474962527938332194[210] = 0.0;
   out_1474962527938332194[211] = 0.0;
   out_1474962527938332194[212] = 0.0;
   out_1474962527938332194[213] = 0.0;
   out_1474962527938332194[214] = 0.0;
   out_1474962527938332194[215] = 0.0;
   out_1474962527938332194[216] = 0.0;
   out_1474962527938332194[217] = 0.0;
   out_1474962527938332194[218] = 0.0;
   out_1474962527938332194[219] = 0.0;
   out_1474962527938332194[220] = 0.0;
   out_1474962527938332194[221] = 0.0;
   out_1474962527938332194[222] = 0.0;
   out_1474962527938332194[223] = 0.0;
   out_1474962527938332194[224] = 0.0;
   out_1474962527938332194[225] = 0.0;
   out_1474962527938332194[226] = 0.0;
   out_1474962527938332194[227] = 0.0;
   out_1474962527938332194[228] = 1.0;
   out_1474962527938332194[229] = 0.0;
   out_1474962527938332194[230] = 0.0;
   out_1474962527938332194[231] = 0.0;
   out_1474962527938332194[232] = 0.0;
   out_1474962527938332194[233] = 0.0;
   out_1474962527938332194[234] = 0.0;
   out_1474962527938332194[235] = 0.0;
   out_1474962527938332194[236] = 0.0;
   out_1474962527938332194[237] = 0.0;
   out_1474962527938332194[238] = 0.0;
   out_1474962527938332194[239] = 0.0;
   out_1474962527938332194[240] = 0.0;
   out_1474962527938332194[241] = 0.0;
   out_1474962527938332194[242] = 0.0;
   out_1474962527938332194[243] = 0.0;
   out_1474962527938332194[244] = 0.0;
   out_1474962527938332194[245] = 0.0;
   out_1474962527938332194[246] = 0.0;
   out_1474962527938332194[247] = 1.0;
   out_1474962527938332194[248] = 0.0;
   out_1474962527938332194[249] = 0.0;
   out_1474962527938332194[250] = 0.0;
   out_1474962527938332194[251] = 0.0;
   out_1474962527938332194[252] = 0.0;
   out_1474962527938332194[253] = 0.0;
   out_1474962527938332194[254] = 0.0;
   out_1474962527938332194[255] = 0.0;
   out_1474962527938332194[256] = 0.0;
   out_1474962527938332194[257] = 0.0;
   out_1474962527938332194[258] = 0.0;
   out_1474962527938332194[259] = 0.0;
   out_1474962527938332194[260] = 0.0;
   out_1474962527938332194[261] = 0.0;
   out_1474962527938332194[262] = 0.0;
   out_1474962527938332194[263] = 0.0;
   out_1474962527938332194[264] = 0.0;
   out_1474962527938332194[265] = 0.0;
   out_1474962527938332194[266] = 1.0;
   out_1474962527938332194[267] = 0.0;
   out_1474962527938332194[268] = 0.0;
   out_1474962527938332194[269] = 0.0;
   out_1474962527938332194[270] = 0.0;
   out_1474962527938332194[271] = 0.0;
   out_1474962527938332194[272] = 0.0;
   out_1474962527938332194[273] = 0.0;
   out_1474962527938332194[274] = 0.0;
   out_1474962527938332194[275] = 0.0;
   out_1474962527938332194[276] = 0.0;
   out_1474962527938332194[277] = 0.0;
   out_1474962527938332194[278] = 0.0;
   out_1474962527938332194[279] = 0.0;
   out_1474962527938332194[280] = 0.0;
   out_1474962527938332194[281] = 0.0;
   out_1474962527938332194[282] = 0.0;
   out_1474962527938332194[283] = 0.0;
   out_1474962527938332194[284] = 0.0;
   out_1474962527938332194[285] = 1.0;
   out_1474962527938332194[286] = 0.0;
   out_1474962527938332194[287] = 0.0;
   out_1474962527938332194[288] = 0.0;
   out_1474962527938332194[289] = 0.0;
   out_1474962527938332194[290] = 0.0;
   out_1474962527938332194[291] = 0.0;
   out_1474962527938332194[292] = 0.0;
   out_1474962527938332194[293] = 0.0;
   out_1474962527938332194[294] = 0.0;
   out_1474962527938332194[295] = 0.0;
   out_1474962527938332194[296] = 0.0;
   out_1474962527938332194[297] = 0.0;
   out_1474962527938332194[298] = 0.0;
   out_1474962527938332194[299] = 0.0;
   out_1474962527938332194[300] = 0.0;
   out_1474962527938332194[301] = 0.0;
   out_1474962527938332194[302] = 0.0;
   out_1474962527938332194[303] = 0.0;
   out_1474962527938332194[304] = 1.0;
   out_1474962527938332194[305] = 0.0;
   out_1474962527938332194[306] = 0.0;
   out_1474962527938332194[307] = 0.0;
   out_1474962527938332194[308] = 0.0;
   out_1474962527938332194[309] = 0.0;
   out_1474962527938332194[310] = 0.0;
   out_1474962527938332194[311] = 0.0;
   out_1474962527938332194[312] = 0.0;
   out_1474962527938332194[313] = 0.0;
   out_1474962527938332194[314] = 0.0;
   out_1474962527938332194[315] = 0.0;
   out_1474962527938332194[316] = 0.0;
   out_1474962527938332194[317] = 0.0;
   out_1474962527938332194[318] = 0.0;
   out_1474962527938332194[319] = 0.0;
   out_1474962527938332194[320] = 0.0;
   out_1474962527938332194[321] = 0.0;
   out_1474962527938332194[322] = 0.0;
   out_1474962527938332194[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_8107096558916273409) {
   out_8107096558916273409[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_8107096558916273409[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_8107096558916273409[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_8107096558916273409[3] = dt*state[12] + state[3];
   out_8107096558916273409[4] = dt*state[13] + state[4];
   out_8107096558916273409[5] = dt*state[14] + state[5];
   out_8107096558916273409[6] = state[6];
   out_8107096558916273409[7] = state[7];
   out_8107096558916273409[8] = state[8];
   out_8107096558916273409[9] = state[9];
   out_8107096558916273409[10] = state[10];
   out_8107096558916273409[11] = state[11];
   out_8107096558916273409[12] = state[12];
   out_8107096558916273409[13] = state[13];
   out_8107096558916273409[14] = state[14];
   out_8107096558916273409[15] = state[15];
   out_8107096558916273409[16] = state[16];
   out_8107096558916273409[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3803993622350295832) {
   out_3803993622350295832[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3803993622350295832[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3803993622350295832[2] = 0;
   out_3803993622350295832[3] = 0;
   out_3803993622350295832[4] = 0;
   out_3803993622350295832[5] = 0;
   out_3803993622350295832[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3803993622350295832[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3803993622350295832[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3803993622350295832[9] = 0;
   out_3803993622350295832[10] = 0;
   out_3803993622350295832[11] = 0;
   out_3803993622350295832[12] = 0;
   out_3803993622350295832[13] = 0;
   out_3803993622350295832[14] = 0;
   out_3803993622350295832[15] = 0;
   out_3803993622350295832[16] = 0;
   out_3803993622350295832[17] = 0;
   out_3803993622350295832[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3803993622350295832[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3803993622350295832[20] = 0;
   out_3803993622350295832[21] = 0;
   out_3803993622350295832[22] = 0;
   out_3803993622350295832[23] = 0;
   out_3803993622350295832[24] = 0;
   out_3803993622350295832[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3803993622350295832[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3803993622350295832[27] = 0;
   out_3803993622350295832[28] = 0;
   out_3803993622350295832[29] = 0;
   out_3803993622350295832[30] = 0;
   out_3803993622350295832[31] = 0;
   out_3803993622350295832[32] = 0;
   out_3803993622350295832[33] = 0;
   out_3803993622350295832[34] = 0;
   out_3803993622350295832[35] = 0;
   out_3803993622350295832[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3803993622350295832[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3803993622350295832[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3803993622350295832[39] = 0;
   out_3803993622350295832[40] = 0;
   out_3803993622350295832[41] = 0;
   out_3803993622350295832[42] = 0;
   out_3803993622350295832[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3803993622350295832[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3803993622350295832[45] = 0;
   out_3803993622350295832[46] = 0;
   out_3803993622350295832[47] = 0;
   out_3803993622350295832[48] = 0;
   out_3803993622350295832[49] = 0;
   out_3803993622350295832[50] = 0;
   out_3803993622350295832[51] = 0;
   out_3803993622350295832[52] = 0;
   out_3803993622350295832[53] = 0;
   out_3803993622350295832[54] = 0;
   out_3803993622350295832[55] = 0;
   out_3803993622350295832[56] = 0;
   out_3803993622350295832[57] = 1;
   out_3803993622350295832[58] = 0;
   out_3803993622350295832[59] = 0;
   out_3803993622350295832[60] = 0;
   out_3803993622350295832[61] = 0;
   out_3803993622350295832[62] = 0;
   out_3803993622350295832[63] = 0;
   out_3803993622350295832[64] = 0;
   out_3803993622350295832[65] = 0;
   out_3803993622350295832[66] = dt;
   out_3803993622350295832[67] = 0;
   out_3803993622350295832[68] = 0;
   out_3803993622350295832[69] = 0;
   out_3803993622350295832[70] = 0;
   out_3803993622350295832[71] = 0;
   out_3803993622350295832[72] = 0;
   out_3803993622350295832[73] = 0;
   out_3803993622350295832[74] = 0;
   out_3803993622350295832[75] = 0;
   out_3803993622350295832[76] = 1;
   out_3803993622350295832[77] = 0;
   out_3803993622350295832[78] = 0;
   out_3803993622350295832[79] = 0;
   out_3803993622350295832[80] = 0;
   out_3803993622350295832[81] = 0;
   out_3803993622350295832[82] = 0;
   out_3803993622350295832[83] = 0;
   out_3803993622350295832[84] = 0;
   out_3803993622350295832[85] = dt;
   out_3803993622350295832[86] = 0;
   out_3803993622350295832[87] = 0;
   out_3803993622350295832[88] = 0;
   out_3803993622350295832[89] = 0;
   out_3803993622350295832[90] = 0;
   out_3803993622350295832[91] = 0;
   out_3803993622350295832[92] = 0;
   out_3803993622350295832[93] = 0;
   out_3803993622350295832[94] = 0;
   out_3803993622350295832[95] = 1;
   out_3803993622350295832[96] = 0;
   out_3803993622350295832[97] = 0;
   out_3803993622350295832[98] = 0;
   out_3803993622350295832[99] = 0;
   out_3803993622350295832[100] = 0;
   out_3803993622350295832[101] = 0;
   out_3803993622350295832[102] = 0;
   out_3803993622350295832[103] = 0;
   out_3803993622350295832[104] = dt;
   out_3803993622350295832[105] = 0;
   out_3803993622350295832[106] = 0;
   out_3803993622350295832[107] = 0;
   out_3803993622350295832[108] = 0;
   out_3803993622350295832[109] = 0;
   out_3803993622350295832[110] = 0;
   out_3803993622350295832[111] = 0;
   out_3803993622350295832[112] = 0;
   out_3803993622350295832[113] = 0;
   out_3803993622350295832[114] = 1;
   out_3803993622350295832[115] = 0;
   out_3803993622350295832[116] = 0;
   out_3803993622350295832[117] = 0;
   out_3803993622350295832[118] = 0;
   out_3803993622350295832[119] = 0;
   out_3803993622350295832[120] = 0;
   out_3803993622350295832[121] = 0;
   out_3803993622350295832[122] = 0;
   out_3803993622350295832[123] = 0;
   out_3803993622350295832[124] = 0;
   out_3803993622350295832[125] = 0;
   out_3803993622350295832[126] = 0;
   out_3803993622350295832[127] = 0;
   out_3803993622350295832[128] = 0;
   out_3803993622350295832[129] = 0;
   out_3803993622350295832[130] = 0;
   out_3803993622350295832[131] = 0;
   out_3803993622350295832[132] = 0;
   out_3803993622350295832[133] = 1;
   out_3803993622350295832[134] = 0;
   out_3803993622350295832[135] = 0;
   out_3803993622350295832[136] = 0;
   out_3803993622350295832[137] = 0;
   out_3803993622350295832[138] = 0;
   out_3803993622350295832[139] = 0;
   out_3803993622350295832[140] = 0;
   out_3803993622350295832[141] = 0;
   out_3803993622350295832[142] = 0;
   out_3803993622350295832[143] = 0;
   out_3803993622350295832[144] = 0;
   out_3803993622350295832[145] = 0;
   out_3803993622350295832[146] = 0;
   out_3803993622350295832[147] = 0;
   out_3803993622350295832[148] = 0;
   out_3803993622350295832[149] = 0;
   out_3803993622350295832[150] = 0;
   out_3803993622350295832[151] = 0;
   out_3803993622350295832[152] = 1;
   out_3803993622350295832[153] = 0;
   out_3803993622350295832[154] = 0;
   out_3803993622350295832[155] = 0;
   out_3803993622350295832[156] = 0;
   out_3803993622350295832[157] = 0;
   out_3803993622350295832[158] = 0;
   out_3803993622350295832[159] = 0;
   out_3803993622350295832[160] = 0;
   out_3803993622350295832[161] = 0;
   out_3803993622350295832[162] = 0;
   out_3803993622350295832[163] = 0;
   out_3803993622350295832[164] = 0;
   out_3803993622350295832[165] = 0;
   out_3803993622350295832[166] = 0;
   out_3803993622350295832[167] = 0;
   out_3803993622350295832[168] = 0;
   out_3803993622350295832[169] = 0;
   out_3803993622350295832[170] = 0;
   out_3803993622350295832[171] = 1;
   out_3803993622350295832[172] = 0;
   out_3803993622350295832[173] = 0;
   out_3803993622350295832[174] = 0;
   out_3803993622350295832[175] = 0;
   out_3803993622350295832[176] = 0;
   out_3803993622350295832[177] = 0;
   out_3803993622350295832[178] = 0;
   out_3803993622350295832[179] = 0;
   out_3803993622350295832[180] = 0;
   out_3803993622350295832[181] = 0;
   out_3803993622350295832[182] = 0;
   out_3803993622350295832[183] = 0;
   out_3803993622350295832[184] = 0;
   out_3803993622350295832[185] = 0;
   out_3803993622350295832[186] = 0;
   out_3803993622350295832[187] = 0;
   out_3803993622350295832[188] = 0;
   out_3803993622350295832[189] = 0;
   out_3803993622350295832[190] = 1;
   out_3803993622350295832[191] = 0;
   out_3803993622350295832[192] = 0;
   out_3803993622350295832[193] = 0;
   out_3803993622350295832[194] = 0;
   out_3803993622350295832[195] = 0;
   out_3803993622350295832[196] = 0;
   out_3803993622350295832[197] = 0;
   out_3803993622350295832[198] = 0;
   out_3803993622350295832[199] = 0;
   out_3803993622350295832[200] = 0;
   out_3803993622350295832[201] = 0;
   out_3803993622350295832[202] = 0;
   out_3803993622350295832[203] = 0;
   out_3803993622350295832[204] = 0;
   out_3803993622350295832[205] = 0;
   out_3803993622350295832[206] = 0;
   out_3803993622350295832[207] = 0;
   out_3803993622350295832[208] = 0;
   out_3803993622350295832[209] = 1;
   out_3803993622350295832[210] = 0;
   out_3803993622350295832[211] = 0;
   out_3803993622350295832[212] = 0;
   out_3803993622350295832[213] = 0;
   out_3803993622350295832[214] = 0;
   out_3803993622350295832[215] = 0;
   out_3803993622350295832[216] = 0;
   out_3803993622350295832[217] = 0;
   out_3803993622350295832[218] = 0;
   out_3803993622350295832[219] = 0;
   out_3803993622350295832[220] = 0;
   out_3803993622350295832[221] = 0;
   out_3803993622350295832[222] = 0;
   out_3803993622350295832[223] = 0;
   out_3803993622350295832[224] = 0;
   out_3803993622350295832[225] = 0;
   out_3803993622350295832[226] = 0;
   out_3803993622350295832[227] = 0;
   out_3803993622350295832[228] = 1;
   out_3803993622350295832[229] = 0;
   out_3803993622350295832[230] = 0;
   out_3803993622350295832[231] = 0;
   out_3803993622350295832[232] = 0;
   out_3803993622350295832[233] = 0;
   out_3803993622350295832[234] = 0;
   out_3803993622350295832[235] = 0;
   out_3803993622350295832[236] = 0;
   out_3803993622350295832[237] = 0;
   out_3803993622350295832[238] = 0;
   out_3803993622350295832[239] = 0;
   out_3803993622350295832[240] = 0;
   out_3803993622350295832[241] = 0;
   out_3803993622350295832[242] = 0;
   out_3803993622350295832[243] = 0;
   out_3803993622350295832[244] = 0;
   out_3803993622350295832[245] = 0;
   out_3803993622350295832[246] = 0;
   out_3803993622350295832[247] = 1;
   out_3803993622350295832[248] = 0;
   out_3803993622350295832[249] = 0;
   out_3803993622350295832[250] = 0;
   out_3803993622350295832[251] = 0;
   out_3803993622350295832[252] = 0;
   out_3803993622350295832[253] = 0;
   out_3803993622350295832[254] = 0;
   out_3803993622350295832[255] = 0;
   out_3803993622350295832[256] = 0;
   out_3803993622350295832[257] = 0;
   out_3803993622350295832[258] = 0;
   out_3803993622350295832[259] = 0;
   out_3803993622350295832[260] = 0;
   out_3803993622350295832[261] = 0;
   out_3803993622350295832[262] = 0;
   out_3803993622350295832[263] = 0;
   out_3803993622350295832[264] = 0;
   out_3803993622350295832[265] = 0;
   out_3803993622350295832[266] = 1;
   out_3803993622350295832[267] = 0;
   out_3803993622350295832[268] = 0;
   out_3803993622350295832[269] = 0;
   out_3803993622350295832[270] = 0;
   out_3803993622350295832[271] = 0;
   out_3803993622350295832[272] = 0;
   out_3803993622350295832[273] = 0;
   out_3803993622350295832[274] = 0;
   out_3803993622350295832[275] = 0;
   out_3803993622350295832[276] = 0;
   out_3803993622350295832[277] = 0;
   out_3803993622350295832[278] = 0;
   out_3803993622350295832[279] = 0;
   out_3803993622350295832[280] = 0;
   out_3803993622350295832[281] = 0;
   out_3803993622350295832[282] = 0;
   out_3803993622350295832[283] = 0;
   out_3803993622350295832[284] = 0;
   out_3803993622350295832[285] = 1;
   out_3803993622350295832[286] = 0;
   out_3803993622350295832[287] = 0;
   out_3803993622350295832[288] = 0;
   out_3803993622350295832[289] = 0;
   out_3803993622350295832[290] = 0;
   out_3803993622350295832[291] = 0;
   out_3803993622350295832[292] = 0;
   out_3803993622350295832[293] = 0;
   out_3803993622350295832[294] = 0;
   out_3803993622350295832[295] = 0;
   out_3803993622350295832[296] = 0;
   out_3803993622350295832[297] = 0;
   out_3803993622350295832[298] = 0;
   out_3803993622350295832[299] = 0;
   out_3803993622350295832[300] = 0;
   out_3803993622350295832[301] = 0;
   out_3803993622350295832[302] = 0;
   out_3803993622350295832[303] = 0;
   out_3803993622350295832[304] = 1;
   out_3803993622350295832[305] = 0;
   out_3803993622350295832[306] = 0;
   out_3803993622350295832[307] = 0;
   out_3803993622350295832[308] = 0;
   out_3803993622350295832[309] = 0;
   out_3803993622350295832[310] = 0;
   out_3803993622350295832[311] = 0;
   out_3803993622350295832[312] = 0;
   out_3803993622350295832[313] = 0;
   out_3803993622350295832[314] = 0;
   out_3803993622350295832[315] = 0;
   out_3803993622350295832[316] = 0;
   out_3803993622350295832[317] = 0;
   out_3803993622350295832[318] = 0;
   out_3803993622350295832[319] = 0;
   out_3803993622350295832[320] = 0;
   out_3803993622350295832[321] = 0;
   out_3803993622350295832[322] = 0;
   out_3803993622350295832[323] = 1;
}
void h_4(double *state, double *unused, double *out_2415773389544840251) {
   out_2415773389544840251[0] = state[6] + state[9];
   out_2415773389544840251[1] = state[7] + state[10];
   out_2415773389544840251[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_4675372429039802218) {
   out_4675372429039802218[0] = 0;
   out_4675372429039802218[1] = 0;
   out_4675372429039802218[2] = 0;
   out_4675372429039802218[3] = 0;
   out_4675372429039802218[4] = 0;
   out_4675372429039802218[5] = 0;
   out_4675372429039802218[6] = 1;
   out_4675372429039802218[7] = 0;
   out_4675372429039802218[8] = 0;
   out_4675372429039802218[9] = 1;
   out_4675372429039802218[10] = 0;
   out_4675372429039802218[11] = 0;
   out_4675372429039802218[12] = 0;
   out_4675372429039802218[13] = 0;
   out_4675372429039802218[14] = 0;
   out_4675372429039802218[15] = 0;
   out_4675372429039802218[16] = 0;
   out_4675372429039802218[17] = 0;
   out_4675372429039802218[18] = 0;
   out_4675372429039802218[19] = 0;
   out_4675372429039802218[20] = 0;
   out_4675372429039802218[21] = 0;
   out_4675372429039802218[22] = 0;
   out_4675372429039802218[23] = 0;
   out_4675372429039802218[24] = 0;
   out_4675372429039802218[25] = 1;
   out_4675372429039802218[26] = 0;
   out_4675372429039802218[27] = 0;
   out_4675372429039802218[28] = 1;
   out_4675372429039802218[29] = 0;
   out_4675372429039802218[30] = 0;
   out_4675372429039802218[31] = 0;
   out_4675372429039802218[32] = 0;
   out_4675372429039802218[33] = 0;
   out_4675372429039802218[34] = 0;
   out_4675372429039802218[35] = 0;
   out_4675372429039802218[36] = 0;
   out_4675372429039802218[37] = 0;
   out_4675372429039802218[38] = 0;
   out_4675372429039802218[39] = 0;
   out_4675372429039802218[40] = 0;
   out_4675372429039802218[41] = 0;
   out_4675372429039802218[42] = 0;
   out_4675372429039802218[43] = 0;
   out_4675372429039802218[44] = 1;
   out_4675372429039802218[45] = 0;
   out_4675372429039802218[46] = 0;
   out_4675372429039802218[47] = 1;
   out_4675372429039802218[48] = 0;
   out_4675372429039802218[49] = 0;
   out_4675372429039802218[50] = 0;
   out_4675372429039802218[51] = 0;
   out_4675372429039802218[52] = 0;
   out_4675372429039802218[53] = 0;
}
void h_10(double *state, double *unused, double *out_2790393475270154159) {
   out_2790393475270154159[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_2790393475270154159[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_2790393475270154159[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_6532362201945156818) {
   out_6532362201945156818[0] = 0;
   out_6532362201945156818[1] = 9.8100000000000005*cos(state[1]);
   out_6532362201945156818[2] = 0;
   out_6532362201945156818[3] = 0;
   out_6532362201945156818[4] = -state[8];
   out_6532362201945156818[5] = state[7];
   out_6532362201945156818[6] = 0;
   out_6532362201945156818[7] = state[5];
   out_6532362201945156818[8] = -state[4];
   out_6532362201945156818[9] = 0;
   out_6532362201945156818[10] = 0;
   out_6532362201945156818[11] = 0;
   out_6532362201945156818[12] = 1;
   out_6532362201945156818[13] = 0;
   out_6532362201945156818[14] = 0;
   out_6532362201945156818[15] = 1;
   out_6532362201945156818[16] = 0;
   out_6532362201945156818[17] = 0;
   out_6532362201945156818[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_6532362201945156818[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_6532362201945156818[20] = 0;
   out_6532362201945156818[21] = state[8];
   out_6532362201945156818[22] = 0;
   out_6532362201945156818[23] = -state[6];
   out_6532362201945156818[24] = -state[5];
   out_6532362201945156818[25] = 0;
   out_6532362201945156818[26] = state[3];
   out_6532362201945156818[27] = 0;
   out_6532362201945156818[28] = 0;
   out_6532362201945156818[29] = 0;
   out_6532362201945156818[30] = 0;
   out_6532362201945156818[31] = 1;
   out_6532362201945156818[32] = 0;
   out_6532362201945156818[33] = 0;
   out_6532362201945156818[34] = 1;
   out_6532362201945156818[35] = 0;
   out_6532362201945156818[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_6532362201945156818[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_6532362201945156818[38] = 0;
   out_6532362201945156818[39] = -state[7];
   out_6532362201945156818[40] = state[6];
   out_6532362201945156818[41] = 0;
   out_6532362201945156818[42] = state[4];
   out_6532362201945156818[43] = -state[3];
   out_6532362201945156818[44] = 0;
   out_6532362201945156818[45] = 0;
   out_6532362201945156818[46] = 0;
   out_6532362201945156818[47] = 0;
   out_6532362201945156818[48] = 0;
   out_6532362201945156818[49] = 0;
   out_6532362201945156818[50] = 1;
   out_6532362201945156818[51] = 0;
   out_6532362201945156818[52] = 0;
   out_6532362201945156818[53] = 1;
}
void h_13(double *state, double *unused, double *out_7195971841206647407) {
   out_7195971841206647407[0] = state[3];
   out_7195971841206647407[1] = state[4];
   out_7195971841206647407[2] = state[5];
}
void H_13(double *state, double *unused, double *out_2935258779276898711) {
   out_2935258779276898711[0] = 0;
   out_2935258779276898711[1] = 0;
   out_2935258779276898711[2] = 0;
   out_2935258779276898711[3] = 1;
   out_2935258779276898711[4] = 0;
   out_2935258779276898711[5] = 0;
   out_2935258779276898711[6] = 0;
   out_2935258779276898711[7] = 0;
   out_2935258779276898711[8] = 0;
   out_2935258779276898711[9] = 0;
   out_2935258779276898711[10] = 0;
   out_2935258779276898711[11] = 0;
   out_2935258779276898711[12] = 0;
   out_2935258779276898711[13] = 0;
   out_2935258779276898711[14] = 0;
   out_2935258779276898711[15] = 0;
   out_2935258779276898711[16] = 0;
   out_2935258779276898711[17] = 0;
   out_2935258779276898711[18] = 0;
   out_2935258779276898711[19] = 0;
   out_2935258779276898711[20] = 0;
   out_2935258779276898711[21] = 0;
   out_2935258779276898711[22] = 1;
   out_2935258779276898711[23] = 0;
   out_2935258779276898711[24] = 0;
   out_2935258779276898711[25] = 0;
   out_2935258779276898711[26] = 0;
   out_2935258779276898711[27] = 0;
   out_2935258779276898711[28] = 0;
   out_2935258779276898711[29] = 0;
   out_2935258779276898711[30] = 0;
   out_2935258779276898711[31] = 0;
   out_2935258779276898711[32] = 0;
   out_2935258779276898711[33] = 0;
   out_2935258779276898711[34] = 0;
   out_2935258779276898711[35] = 0;
   out_2935258779276898711[36] = 0;
   out_2935258779276898711[37] = 0;
   out_2935258779276898711[38] = 0;
   out_2935258779276898711[39] = 0;
   out_2935258779276898711[40] = 0;
   out_2935258779276898711[41] = 1;
   out_2935258779276898711[42] = 0;
   out_2935258779276898711[43] = 0;
   out_2935258779276898711[44] = 0;
   out_2935258779276898711[45] = 0;
   out_2935258779276898711[46] = 0;
   out_2935258779276898711[47] = 0;
   out_2935258779276898711[48] = 0;
   out_2935258779276898711[49] = 0;
   out_2935258779276898711[50] = 0;
   out_2935258779276898711[51] = 0;
   out_2935258779276898711[52] = 0;
   out_2935258779276898711[53] = 0;
}
void h_14(double *state, double *unused, double *out_6081561149972683319) {
   out_6081561149972683319[0] = state[6];
   out_6081561149972683319[1] = state[7];
   out_6081561149972683319[2] = state[8];
}
void H_14(double *state, double *unused, double *out_712131572700317689) {
   out_712131572700317689[0] = 0;
   out_712131572700317689[1] = 0;
   out_712131572700317689[2] = 0;
   out_712131572700317689[3] = 0;
   out_712131572700317689[4] = 0;
   out_712131572700317689[5] = 0;
   out_712131572700317689[6] = 1;
   out_712131572700317689[7] = 0;
   out_712131572700317689[8] = 0;
   out_712131572700317689[9] = 0;
   out_712131572700317689[10] = 0;
   out_712131572700317689[11] = 0;
   out_712131572700317689[12] = 0;
   out_712131572700317689[13] = 0;
   out_712131572700317689[14] = 0;
   out_712131572700317689[15] = 0;
   out_712131572700317689[16] = 0;
   out_712131572700317689[17] = 0;
   out_712131572700317689[18] = 0;
   out_712131572700317689[19] = 0;
   out_712131572700317689[20] = 0;
   out_712131572700317689[21] = 0;
   out_712131572700317689[22] = 0;
   out_712131572700317689[23] = 0;
   out_712131572700317689[24] = 0;
   out_712131572700317689[25] = 1;
   out_712131572700317689[26] = 0;
   out_712131572700317689[27] = 0;
   out_712131572700317689[28] = 0;
   out_712131572700317689[29] = 0;
   out_712131572700317689[30] = 0;
   out_712131572700317689[31] = 0;
   out_712131572700317689[32] = 0;
   out_712131572700317689[33] = 0;
   out_712131572700317689[34] = 0;
   out_712131572700317689[35] = 0;
   out_712131572700317689[36] = 0;
   out_712131572700317689[37] = 0;
   out_712131572700317689[38] = 0;
   out_712131572700317689[39] = 0;
   out_712131572700317689[40] = 0;
   out_712131572700317689[41] = 0;
   out_712131572700317689[42] = 0;
   out_712131572700317689[43] = 0;
   out_712131572700317689[44] = 1;
   out_712131572700317689[45] = 0;
   out_712131572700317689[46] = 0;
   out_712131572700317689[47] = 0;
   out_712131572700317689[48] = 0;
   out_712131572700317689[49] = 0;
   out_712131572700317689[50] = 0;
   out_712131572700317689[51] = 0;
   out_712131572700317689[52] = 0;
   out_712131572700317689[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_5792379923597347601) {
  err_fun(nom_x, delta_x, out_5792379923597347601);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3127222841440964837) {
  inv_err_fun(nom_x, true_x, out_3127222841440964837);
}
void pose_H_mod_fun(double *state, double *out_1474962527938332194) {
  H_mod_fun(state, out_1474962527938332194);
}
void pose_f_fun(double *state, double dt, double *out_8107096558916273409) {
  f_fun(state,  dt, out_8107096558916273409);
}
void pose_F_fun(double *state, double dt, double *out_3803993622350295832) {
  F_fun(state,  dt, out_3803993622350295832);
}
void pose_h_4(double *state, double *unused, double *out_2415773389544840251) {
  h_4(state, unused, out_2415773389544840251);
}
void pose_H_4(double *state, double *unused, double *out_4675372429039802218) {
  H_4(state, unused, out_4675372429039802218);
}
void pose_h_10(double *state, double *unused, double *out_2790393475270154159) {
  h_10(state, unused, out_2790393475270154159);
}
void pose_H_10(double *state, double *unused, double *out_6532362201945156818) {
  H_10(state, unused, out_6532362201945156818);
}
void pose_h_13(double *state, double *unused, double *out_7195971841206647407) {
  h_13(state, unused, out_7195971841206647407);
}
void pose_H_13(double *state, double *unused, double *out_2935258779276898711) {
  H_13(state, unused, out_2935258779276898711);
}
void pose_h_14(double *state, double *unused, double *out_6081561149972683319) {
  h_14(state, unused, out_6081561149972683319);
}
void pose_H_14(double *state, double *unused, double *out_712131572700317689) {
  H_14(state, unused, out_712131572700317689);
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

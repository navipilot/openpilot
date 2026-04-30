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
void err_fun(double *nom_x, double *delta_x, double *out_4091115333149920439) {
   out_4091115333149920439[0] = delta_x[0] + nom_x[0];
   out_4091115333149920439[1] = delta_x[1] + nom_x[1];
   out_4091115333149920439[2] = delta_x[2] + nom_x[2];
   out_4091115333149920439[3] = delta_x[3] + nom_x[3];
   out_4091115333149920439[4] = delta_x[4] + nom_x[4];
   out_4091115333149920439[5] = delta_x[5] + nom_x[5];
   out_4091115333149920439[6] = delta_x[6] + nom_x[6];
   out_4091115333149920439[7] = delta_x[7] + nom_x[7];
   out_4091115333149920439[8] = delta_x[8] + nom_x[8];
   out_4091115333149920439[9] = delta_x[9] + nom_x[9];
   out_4091115333149920439[10] = delta_x[10] + nom_x[10];
   out_4091115333149920439[11] = delta_x[11] + nom_x[11];
   out_4091115333149920439[12] = delta_x[12] + nom_x[12];
   out_4091115333149920439[13] = delta_x[13] + nom_x[13];
   out_4091115333149920439[14] = delta_x[14] + nom_x[14];
   out_4091115333149920439[15] = delta_x[15] + nom_x[15];
   out_4091115333149920439[16] = delta_x[16] + nom_x[16];
   out_4091115333149920439[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6264186140300305590) {
   out_6264186140300305590[0] = -nom_x[0] + true_x[0];
   out_6264186140300305590[1] = -nom_x[1] + true_x[1];
   out_6264186140300305590[2] = -nom_x[2] + true_x[2];
   out_6264186140300305590[3] = -nom_x[3] + true_x[3];
   out_6264186140300305590[4] = -nom_x[4] + true_x[4];
   out_6264186140300305590[5] = -nom_x[5] + true_x[5];
   out_6264186140300305590[6] = -nom_x[6] + true_x[6];
   out_6264186140300305590[7] = -nom_x[7] + true_x[7];
   out_6264186140300305590[8] = -nom_x[8] + true_x[8];
   out_6264186140300305590[9] = -nom_x[9] + true_x[9];
   out_6264186140300305590[10] = -nom_x[10] + true_x[10];
   out_6264186140300305590[11] = -nom_x[11] + true_x[11];
   out_6264186140300305590[12] = -nom_x[12] + true_x[12];
   out_6264186140300305590[13] = -nom_x[13] + true_x[13];
   out_6264186140300305590[14] = -nom_x[14] + true_x[14];
   out_6264186140300305590[15] = -nom_x[15] + true_x[15];
   out_6264186140300305590[16] = -nom_x[16] + true_x[16];
   out_6264186140300305590[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_4881048693765674271) {
   out_4881048693765674271[0] = 1.0;
   out_4881048693765674271[1] = 0.0;
   out_4881048693765674271[2] = 0.0;
   out_4881048693765674271[3] = 0.0;
   out_4881048693765674271[4] = 0.0;
   out_4881048693765674271[5] = 0.0;
   out_4881048693765674271[6] = 0.0;
   out_4881048693765674271[7] = 0.0;
   out_4881048693765674271[8] = 0.0;
   out_4881048693765674271[9] = 0.0;
   out_4881048693765674271[10] = 0.0;
   out_4881048693765674271[11] = 0.0;
   out_4881048693765674271[12] = 0.0;
   out_4881048693765674271[13] = 0.0;
   out_4881048693765674271[14] = 0.0;
   out_4881048693765674271[15] = 0.0;
   out_4881048693765674271[16] = 0.0;
   out_4881048693765674271[17] = 0.0;
   out_4881048693765674271[18] = 0.0;
   out_4881048693765674271[19] = 1.0;
   out_4881048693765674271[20] = 0.0;
   out_4881048693765674271[21] = 0.0;
   out_4881048693765674271[22] = 0.0;
   out_4881048693765674271[23] = 0.0;
   out_4881048693765674271[24] = 0.0;
   out_4881048693765674271[25] = 0.0;
   out_4881048693765674271[26] = 0.0;
   out_4881048693765674271[27] = 0.0;
   out_4881048693765674271[28] = 0.0;
   out_4881048693765674271[29] = 0.0;
   out_4881048693765674271[30] = 0.0;
   out_4881048693765674271[31] = 0.0;
   out_4881048693765674271[32] = 0.0;
   out_4881048693765674271[33] = 0.0;
   out_4881048693765674271[34] = 0.0;
   out_4881048693765674271[35] = 0.0;
   out_4881048693765674271[36] = 0.0;
   out_4881048693765674271[37] = 0.0;
   out_4881048693765674271[38] = 1.0;
   out_4881048693765674271[39] = 0.0;
   out_4881048693765674271[40] = 0.0;
   out_4881048693765674271[41] = 0.0;
   out_4881048693765674271[42] = 0.0;
   out_4881048693765674271[43] = 0.0;
   out_4881048693765674271[44] = 0.0;
   out_4881048693765674271[45] = 0.0;
   out_4881048693765674271[46] = 0.0;
   out_4881048693765674271[47] = 0.0;
   out_4881048693765674271[48] = 0.0;
   out_4881048693765674271[49] = 0.0;
   out_4881048693765674271[50] = 0.0;
   out_4881048693765674271[51] = 0.0;
   out_4881048693765674271[52] = 0.0;
   out_4881048693765674271[53] = 0.0;
   out_4881048693765674271[54] = 0.0;
   out_4881048693765674271[55] = 0.0;
   out_4881048693765674271[56] = 0.0;
   out_4881048693765674271[57] = 1.0;
   out_4881048693765674271[58] = 0.0;
   out_4881048693765674271[59] = 0.0;
   out_4881048693765674271[60] = 0.0;
   out_4881048693765674271[61] = 0.0;
   out_4881048693765674271[62] = 0.0;
   out_4881048693765674271[63] = 0.0;
   out_4881048693765674271[64] = 0.0;
   out_4881048693765674271[65] = 0.0;
   out_4881048693765674271[66] = 0.0;
   out_4881048693765674271[67] = 0.0;
   out_4881048693765674271[68] = 0.0;
   out_4881048693765674271[69] = 0.0;
   out_4881048693765674271[70] = 0.0;
   out_4881048693765674271[71] = 0.0;
   out_4881048693765674271[72] = 0.0;
   out_4881048693765674271[73] = 0.0;
   out_4881048693765674271[74] = 0.0;
   out_4881048693765674271[75] = 0.0;
   out_4881048693765674271[76] = 1.0;
   out_4881048693765674271[77] = 0.0;
   out_4881048693765674271[78] = 0.0;
   out_4881048693765674271[79] = 0.0;
   out_4881048693765674271[80] = 0.0;
   out_4881048693765674271[81] = 0.0;
   out_4881048693765674271[82] = 0.0;
   out_4881048693765674271[83] = 0.0;
   out_4881048693765674271[84] = 0.0;
   out_4881048693765674271[85] = 0.0;
   out_4881048693765674271[86] = 0.0;
   out_4881048693765674271[87] = 0.0;
   out_4881048693765674271[88] = 0.0;
   out_4881048693765674271[89] = 0.0;
   out_4881048693765674271[90] = 0.0;
   out_4881048693765674271[91] = 0.0;
   out_4881048693765674271[92] = 0.0;
   out_4881048693765674271[93] = 0.0;
   out_4881048693765674271[94] = 0.0;
   out_4881048693765674271[95] = 1.0;
   out_4881048693765674271[96] = 0.0;
   out_4881048693765674271[97] = 0.0;
   out_4881048693765674271[98] = 0.0;
   out_4881048693765674271[99] = 0.0;
   out_4881048693765674271[100] = 0.0;
   out_4881048693765674271[101] = 0.0;
   out_4881048693765674271[102] = 0.0;
   out_4881048693765674271[103] = 0.0;
   out_4881048693765674271[104] = 0.0;
   out_4881048693765674271[105] = 0.0;
   out_4881048693765674271[106] = 0.0;
   out_4881048693765674271[107] = 0.0;
   out_4881048693765674271[108] = 0.0;
   out_4881048693765674271[109] = 0.0;
   out_4881048693765674271[110] = 0.0;
   out_4881048693765674271[111] = 0.0;
   out_4881048693765674271[112] = 0.0;
   out_4881048693765674271[113] = 0.0;
   out_4881048693765674271[114] = 1.0;
   out_4881048693765674271[115] = 0.0;
   out_4881048693765674271[116] = 0.0;
   out_4881048693765674271[117] = 0.0;
   out_4881048693765674271[118] = 0.0;
   out_4881048693765674271[119] = 0.0;
   out_4881048693765674271[120] = 0.0;
   out_4881048693765674271[121] = 0.0;
   out_4881048693765674271[122] = 0.0;
   out_4881048693765674271[123] = 0.0;
   out_4881048693765674271[124] = 0.0;
   out_4881048693765674271[125] = 0.0;
   out_4881048693765674271[126] = 0.0;
   out_4881048693765674271[127] = 0.0;
   out_4881048693765674271[128] = 0.0;
   out_4881048693765674271[129] = 0.0;
   out_4881048693765674271[130] = 0.0;
   out_4881048693765674271[131] = 0.0;
   out_4881048693765674271[132] = 0.0;
   out_4881048693765674271[133] = 1.0;
   out_4881048693765674271[134] = 0.0;
   out_4881048693765674271[135] = 0.0;
   out_4881048693765674271[136] = 0.0;
   out_4881048693765674271[137] = 0.0;
   out_4881048693765674271[138] = 0.0;
   out_4881048693765674271[139] = 0.0;
   out_4881048693765674271[140] = 0.0;
   out_4881048693765674271[141] = 0.0;
   out_4881048693765674271[142] = 0.0;
   out_4881048693765674271[143] = 0.0;
   out_4881048693765674271[144] = 0.0;
   out_4881048693765674271[145] = 0.0;
   out_4881048693765674271[146] = 0.0;
   out_4881048693765674271[147] = 0.0;
   out_4881048693765674271[148] = 0.0;
   out_4881048693765674271[149] = 0.0;
   out_4881048693765674271[150] = 0.0;
   out_4881048693765674271[151] = 0.0;
   out_4881048693765674271[152] = 1.0;
   out_4881048693765674271[153] = 0.0;
   out_4881048693765674271[154] = 0.0;
   out_4881048693765674271[155] = 0.0;
   out_4881048693765674271[156] = 0.0;
   out_4881048693765674271[157] = 0.0;
   out_4881048693765674271[158] = 0.0;
   out_4881048693765674271[159] = 0.0;
   out_4881048693765674271[160] = 0.0;
   out_4881048693765674271[161] = 0.0;
   out_4881048693765674271[162] = 0.0;
   out_4881048693765674271[163] = 0.0;
   out_4881048693765674271[164] = 0.0;
   out_4881048693765674271[165] = 0.0;
   out_4881048693765674271[166] = 0.0;
   out_4881048693765674271[167] = 0.0;
   out_4881048693765674271[168] = 0.0;
   out_4881048693765674271[169] = 0.0;
   out_4881048693765674271[170] = 0.0;
   out_4881048693765674271[171] = 1.0;
   out_4881048693765674271[172] = 0.0;
   out_4881048693765674271[173] = 0.0;
   out_4881048693765674271[174] = 0.0;
   out_4881048693765674271[175] = 0.0;
   out_4881048693765674271[176] = 0.0;
   out_4881048693765674271[177] = 0.0;
   out_4881048693765674271[178] = 0.0;
   out_4881048693765674271[179] = 0.0;
   out_4881048693765674271[180] = 0.0;
   out_4881048693765674271[181] = 0.0;
   out_4881048693765674271[182] = 0.0;
   out_4881048693765674271[183] = 0.0;
   out_4881048693765674271[184] = 0.0;
   out_4881048693765674271[185] = 0.0;
   out_4881048693765674271[186] = 0.0;
   out_4881048693765674271[187] = 0.0;
   out_4881048693765674271[188] = 0.0;
   out_4881048693765674271[189] = 0.0;
   out_4881048693765674271[190] = 1.0;
   out_4881048693765674271[191] = 0.0;
   out_4881048693765674271[192] = 0.0;
   out_4881048693765674271[193] = 0.0;
   out_4881048693765674271[194] = 0.0;
   out_4881048693765674271[195] = 0.0;
   out_4881048693765674271[196] = 0.0;
   out_4881048693765674271[197] = 0.0;
   out_4881048693765674271[198] = 0.0;
   out_4881048693765674271[199] = 0.0;
   out_4881048693765674271[200] = 0.0;
   out_4881048693765674271[201] = 0.0;
   out_4881048693765674271[202] = 0.0;
   out_4881048693765674271[203] = 0.0;
   out_4881048693765674271[204] = 0.0;
   out_4881048693765674271[205] = 0.0;
   out_4881048693765674271[206] = 0.0;
   out_4881048693765674271[207] = 0.0;
   out_4881048693765674271[208] = 0.0;
   out_4881048693765674271[209] = 1.0;
   out_4881048693765674271[210] = 0.0;
   out_4881048693765674271[211] = 0.0;
   out_4881048693765674271[212] = 0.0;
   out_4881048693765674271[213] = 0.0;
   out_4881048693765674271[214] = 0.0;
   out_4881048693765674271[215] = 0.0;
   out_4881048693765674271[216] = 0.0;
   out_4881048693765674271[217] = 0.0;
   out_4881048693765674271[218] = 0.0;
   out_4881048693765674271[219] = 0.0;
   out_4881048693765674271[220] = 0.0;
   out_4881048693765674271[221] = 0.0;
   out_4881048693765674271[222] = 0.0;
   out_4881048693765674271[223] = 0.0;
   out_4881048693765674271[224] = 0.0;
   out_4881048693765674271[225] = 0.0;
   out_4881048693765674271[226] = 0.0;
   out_4881048693765674271[227] = 0.0;
   out_4881048693765674271[228] = 1.0;
   out_4881048693765674271[229] = 0.0;
   out_4881048693765674271[230] = 0.0;
   out_4881048693765674271[231] = 0.0;
   out_4881048693765674271[232] = 0.0;
   out_4881048693765674271[233] = 0.0;
   out_4881048693765674271[234] = 0.0;
   out_4881048693765674271[235] = 0.0;
   out_4881048693765674271[236] = 0.0;
   out_4881048693765674271[237] = 0.0;
   out_4881048693765674271[238] = 0.0;
   out_4881048693765674271[239] = 0.0;
   out_4881048693765674271[240] = 0.0;
   out_4881048693765674271[241] = 0.0;
   out_4881048693765674271[242] = 0.0;
   out_4881048693765674271[243] = 0.0;
   out_4881048693765674271[244] = 0.0;
   out_4881048693765674271[245] = 0.0;
   out_4881048693765674271[246] = 0.0;
   out_4881048693765674271[247] = 1.0;
   out_4881048693765674271[248] = 0.0;
   out_4881048693765674271[249] = 0.0;
   out_4881048693765674271[250] = 0.0;
   out_4881048693765674271[251] = 0.0;
   out_4881048693765674271[252] = 0.0;
   out_4881048693765674271[253] = 0.0;
   out_4881048693765674271[254] = 0.0;
   out_4881048693765674271[255] = 0.0;
   out_4881048693765674271[256] = 0.0;
   out_4881048693765674271[257] = 0.0;
   out_4881048693765674271[258] = 0.0;
   out_4881048693765674271[259] = 0.0;
   out_4881048693765674271[260] = 0.0;
   out_4881048693765674271[261] = 0.0;
   out_4881048693765674271[262] = 0.0;
   out_4881048693765674271[263] = 0.0;
   out_4881048693765674271[264] = 0.0;
   out_4881048693765674271[265] = 0.0;
   out_4881048693765674271[266] = 1.0;
   out_4881048693765674271[267] = 0.0;
   out_4881048693765674271[268] = 0.0;
   out_4881048693765674271[269] = 0.0;
   out_4881048693765674271[270] = 0.0;
   out_4881048693765674271[271] = 0.0;
   out_4881048693765674271[272] = 0.0;
   out_4881048693765674271[273] = 0.0;
   out_4881048693765674271[274] = 0.0;
   out_4881048693765674271[275] = 0.0;
   out_4881048693765674271[276] = 0.0;
   out_4881048693765674271[277] = 0.0;
   out_4881048693765674271[278] = 0.0;
   out_4881048693765674271[279] = 0.0;
   out_4881048693765674271[280] = 0.0;
   out_4881048693765674271[281] = 0.0;
   out_4881048693765674271[282] = 0.0;
   out_4881048693765674271[283] = 0.0;
   out_4881048693765674271[284] = 0.0;
   out_4881048693765674271[285] = 1.0;
   out_4881048693765674271[286] = 0.0;
   out_4881048693765674271[287] = 0.0;
   out_4881048693765674271[288] = 0.0;
   out_4881048693765674271[289] = 0.0;
   out_4881048693765674271[290] = 0.0;
   out_4881048693765674271[291] = 0.0;
   out_4881048693765674271[292] = 0.0;
   out_4881048693765674271[293] = 0.0;
   out_4881048693765674271[294] = 0.0;
   out_4881048693765674271[295] = 0.0;
   out_4881048693765674271[296] = 0.0;
   out_4881048693765674271[297] = 0.0;
   out_4881048693765674271[298] = 0.0;
   out_4881048693765674271[299] = 0.0;
   out_4881048693765674271[300] = 0.0;
   out_4881048693765674271[301] = 0.0;
   out_4881048693765674271[302] = 0.0;
   out_4881048693765674271[303] = 0.0;
   out_4881048693765674271[304] = 1.0;
   out_4881048693765674271[305] = 0.0;
   out_4881048693765674271[306] = 0.0;
   out_4881048693765674271[307] = 0.0;
   out_4881048693765674271[308] = 0.0;
   out_4881048693765674271[309] = 0.0;
   out_4881048693765674271[310] = 0.0;
   out_4881048693765674271[311] = 0.0;
   out_4881048693765674271[312] = 0.0;
   out_4881048693765674271[313] = 0.0;
   out_4881048693765674271[314] = 0.0;
   out_4881048693765674271[315] = 0.0;
   out_4881048693765674271[316] = 0.0;
   out_4881048693765674271[317] = 0.0;
   out_4881048693765674271[318] = 0.0;
   out_4881048693765674271[319] = 0.0;
   out_4881048693765674271[320] = 0.0;
   out_4881048693765674271[321] = 0.0;
   out_4881048693765674271[322] = 0.0;
   out_4881048693765674271[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_2074460560152714132) {
   out_2074460560152714132[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_2074460560152714132[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_2074460560152714132[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_2074460560152714132[3] = dt*state[12] + state[3];
   out_2074460560152714132[4] = dt*state[13] + state[4];
   out_2074460560152714132[5] = dt*state[14] + state[5];
   out_2074460560152714132[6] = state[6];
   out_2074460560152714132[7] = state[7];
   out_2074460560152714132[8] = state[8];
   out_2074460560152714132[9] = state[9];
   out_2074460560152714132[10] = state[10];
   out_2074460560152714132[11] = state[11];
   out_2074460560152714132[12] = state[12];
   out_2074460560152714132[13] = state[13];
   out_2074460560152714132[14] = state[14];
   out_2074460560152714132[15] = state[15];
   out_2074460560152714132[16] = state[16];
   out_2074460560152714132[17] = state[17];
}
void F_fun(double *state, double dt, double *out_2950416268004545311) {
   out_2950416268004545311[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2950416268004545311[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2950416268004545311[2] = 0;
   out_2950416268004545311[3] = 0;
   out_2950416268004545311[4] = 0;
   out_2950416268004545311[5] = 0;
   out_2950416268004545311[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2950416268004545311[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2950416268004545311[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2950416268004545311[9] = 0;
   out_2950416268004545311[10] = 0;
   out_2950416268004545311[11] = 0;
   out_2950416268004545311[12] = 0;
   out_2950416268004545311[13] = 0;
   out_2950416268004545311[14] = 0;
   out_2950416268004545311[15] = 0;
   out_2950416268004545311[16] = 0;
   out_2950416268004545311[17] = 0;
   out_2950416268004545311[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2950416268004545311[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2950416268004545311[20] = 0;
   out_2950416268004545311[21] = 0;
   out_2950416268004545311[22] = 0;
   out_2950416268004545311[23] = 0;
   out_2950416268004545311[24] = 0;
   out_2950416268004545311[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2950416268004545311[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2950416268004545311[27] = 0;
   out_2950416268004545311[28] = 0;
   out_2950416268004545311[29] = 0;
   out_2950416268004545311[30] = 0;
   out_2950416268004545311[31] = 0;
   out_2950416268004545311[32] = 0;
   out_2950416268004545311[33] = 0;
   out_2950416268004545311[34] = 0;
   out_2950416268004545311[35] = 0;
   out_2950416268004545311[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2950416268004545311[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2950416268004545311[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2950416268004545311[39] = 0;
   out_2950416268004545311[40] = 0;
   out_2950416268004545311[41] = 0;
   out_2950416268004545311[42] = 0;
   out_2950416268004545311[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2950416268004545311[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2950416268004545311[45] = 0;
   out_2950416268004545311[46] = 0;
   out_2950416268004545311[47] = 0;
   out_2950416268004545311[48] = 0;
   out_2950416268004545311[49] = 0;
   out_2950416268004545311[50] = 0;
   out_2950416268004545311[51] = 0;
   out_2950416268004545311[52] = 0;
   out_2950416268004545311[53] = 0;
   out_2950416268004545311[54] = 0;
   out_2950416268004545311[55] = 0;
   out_2950416268004545311[56] = 0;
   out_2950416268004545311[57] = 1;
   out_2950416268004545311[58] = 0;
   out_2950416268004545311[59] = 0;
   out_2950416268004545311[60] = 0;
   out_2950416268004545311[61] = 0;
   out_2950416268004545311[62] = 0;
   out_2950416268004545311[63] = 0;
   out_2950416268004545311[64] = 0;
   out_2950416268004545311[65] = 0;
   out_2950416268004545311[66] = dt;
   out_2950416268004545311[67] = 0;
   out_2950416268004545311[68] = 0;
   out_2950416268004545311[69] = 0;
   out_2950416268004545311[70] = 0;
   out_2950416268004545311[71] = 0;
   out_2950416268004545311[72] = 0;
   out_2950416268004545311[73] = 0;
   out_2950416268004545311[74] = 0;
   out_2950416268004545311[75] = 0;
   out_2950416268004545311[76] = 1;
   out_2950416268004545311[77] = 0;
   out_2950416268004545311[78] = 0;
   out_2950416268004545311[79] = 0;
   out_2950416268004545311[80] = 0;
   out_2950416268004545311[81] = 0;
   out_2950416268004545311[82] = 0;
   out_2950416268004545311[83] = 0;
   out_2950416268004545311[84] = 0;
   out_2950416268004545311[85] = dt;
   out_2950416268004545311[86] = 0;
   out_2950416268004545311[87] = 0;
   out_2950416268004545311[88] = 0;
   out_2950416268004545311[89] = 0;
   out_2950416268004545311[90] = 0;
   out_2950416268004545311[91] = 0;
   out_2950416268004545311[92] = 0;
   out_2950416268004545311[93] = 0;
   out_2950416268004545311[94] = 0;
   out_2950416268004545311[95] = 1;
   out_2950416268004545311[96] = 0;
   out_2950416268004545311[97] = 0;
   out_2950416268004545311[98] = 0;
   out_2950416268004545311[99] = 0;
   out_2950416268004545311[100] = 0;
   out_2950416268004545311[101] = 0;
   out_2950416268004545311[102] = 0;
   out_2950416268004545311[103] = 0;
   out_2950416268004545311[104] = dt;
   out_2950416268004545311[105] = 0;
   out_2950416268004545311[106] = 0;
   out_2950416268004545311[107] = 0;
   out_2950416268004545311[108] = 0;
   out_2950416268004545311[109] = 0;
   out_2950416268004545311[110] = 0;
   out_2950416268004545311[111] = 0;
   out_2950416268004545311[112] = 0;
   out_2950416268004545311[113] = 0;
   out_2950416268004545311[114] = 1;
   out_2950416268004545311[115] = 0;
   out_2950416268004545311[116] = 0;
   out_2950416268004545311[117] = 0;
   out_2950416268004545311[118] = 0;
   out_2950416268004545311[119] = 0;
   out_2950416268004545311[120] = 0;
   out_2950416268004545311[121] = 0;
   out_2950416268004545311[122] = 0;
   out_2950416268004545311[123] = 0;
   out_2950416268004545311[124] = 0;
   out_2950416268004545311[125] = 0;
   out_2950416268004545311[126] = 0;
   out_2950416268004545311[127] = 0;
   out_2950416268004545311[128] = 0;
   out_2950416268004545311[129] = 0;
   out_2950416268004545311[130] = 0;
   out_2950416268004545311[131] = 0;
   out_2950416268004545311[132] = 0;
   out_2950416268004545311[133] = 1;
   out_2950416268004545311[134] = 0;
   out_2950416268004545311[135] = 0;
   out_2950416268004545311[136] = 0;
   out_2950416268004545311[137] = 0;
   out_2950416268004545311[138] = 0;
   out_2950416268004545311[139] = 0;
   out_2950416268004545311[140] = 0;
   out_2950416268004545311[141] = 0;
   out_2950416268004545311[142] = 0;
   out_2950416268004545311[143] = 0;
   out_2950416268004545311[144] = 0;
   out_2950416268004545311[145] = 0;
   out_2950416268004545311[146] = 0;
   out_2950416268004545311[147] = 0;
   out_2950416268004545311[148] = 0;
   out_2950416268004545311[149] = 0;
   out_2950416268004545311[150] = 0;
   out_2950416268004545311[151] = 0;
   out_2950416268004545311[152] = 1;
   out_2950416268004545311[153] = 0;
   out_2950416268004545311[154] = 0;
   out_2950416268004545311[155] = 0;
   out_2950416268004545311[156] = 0;
   out_2950416268004545311[157] = 0;
   out_2950416268004545311[158] = 0;
   out_2950416268004545311[159] = 0;
   out_2950416268004545311[160] = 0;
   out_2950416268004545311[161] = 0;
   out_2950416268004545311[162] = 0;
   out_2950416268004545311[163] = 0;
   out_2950416268004545311[164] = 0;
   out_2950416268004545311[165] = 0;
   out_2950416268004545311[166] = 0;
   out_2950416268004545311[167] = 0;
   out_2950416268004545311[168] = 0;
   out_2950416268004545311[169] = 0;
   out_2950416268004545311[170] = 0;
   out_2950416268004545311[171] = 1;
   out_2950416268004545311[172] = 0;
   out_2950416268004545311[173] = 0;
   out_2950416268004545311[174] = 0;
   out_2950416268004545311[175] = 0;
   out_2950416268004545311[176] = 0;
   out_2950416268004545311[177] = 0;
   out_2950416268004545311[178] = 0;
   out_2950416268004545311[179] = 0;
   out_2950416268004545311[180] = 0;
   out_2950416268004545311[181] = 0;
   out_2950416268004545311[182] = 0;
   out_2950416268004545311[183] = 0;
   out_2950416268004545311[184] = 0;
   out_2950416268004545311[185] = 0;
   out_2950416268004545311[186] = 0;
   out_2950416268004545311[187] = 0;
   out_2950416268004545311[188] = 0;
   out_2950416268004545311[189] = 0;
   out_2950416268004545311[190] = 1;
   out_2950416268004545311[191] = 0;
   out_2950416268004545311[192] = 0;
   out_2950416268004545311[193] = 0;
   out_2950416268004545311[194] = 0;
   out_2950416268004545311[195] = 0;
   out_2950416268004545311[196] = 0;
   out_2950416268004545311[197] = 0;
   out_2950416268004545311[198] = 0;
   out_2950416268004545311[199] = 0;
   out_2950416268004545311[200] = 0;
   out_2950416268004545311[201] = 0;
   out_2950416268004545311[202] = 0;
   out_2950416268004545311[203] = 0;
   out_2950416268004545311[204] = 0;
   out_2950416268004545311[205] = 0;
   out_2950416268004545311[206] = 0;
   out_2950416268004545311[207] = 0;
   out_2950416268004545311[208] = 0;
   out_2950416268004545311[209] = 1;
   out_2950416268004545311[210] = 0;
   out_2950416268004545311[211] = 0;
   out_2950416268004545311[212] = 0;
   out_2950416268004545311[213] = 0;
   out_2950416268004545311[214] = 0;
   out_2950416268004545311[215] = 0;
   out_2950416268004545311[216] = 0;
   out_2950416268004545311[217] = 0;
   out_2950416268004545311[218] = 0;
   out_2950416268004545311[219] = 0;
   out_2950416268004545311[220] = 0;
   out_2950416268004545311[221] = 0;
   out_2950416268004545311[222] = 0;
   out_2950416268004545311[223] = 0;
   out_2950416268004545311[224] = 0;
   out_2950416268004545311[225] = 0;
   out_2950416268004545311[226] = 0;
   out_2950416268004545311[227] = 0;
   out_2950416268004545311[228] = 1;
   out_2950416268004545311[229] = 0;
   out_2950416268004545311[230] = 0;
   out_2950416268004545311[231] = 0;
   out_2950416268004545311[232] = 0;
   out_2950416268004545311[233] = 0;
   out_2950416268004545311[234] = 0;
   out_2950416268004545311[235] = 0;
   out_2950416268004545311[236] = 0;
   out_2950416268004545311[237] = 0;
   out_2950416268004545311[238] = 0;
   out_2950416268004545311[239] = 0;
   out_2950416268004545311[240] = 0;
   out_2950416268004545311[241] = 0;
   out_2950416268004545311[242] = 0;
   out_2950416268004545311[243] = 0;
   out_2950416268004545311[244] = 0;
   out_2950416268004545311[245] = 0;
   out_2950416268004545311[246] = 0;
   out_2950416268004545311[247] = 1;
   out_2950416268004545311[248] = 0;
   out_2950416268004545311[249] = 0;
   out_2950416268004545311[250] = 0;
   out_2950416268004545311[251] = 0;
   out_2950416268004545311[252] = 0;
   out_2950416268004545311[253] = 0;
   out_2950416268004545311[254] = 0;
   out_2950416268004545311[255] = 0;
   out_2950416268004545311[256] = 0;
   out_2950416268004545311[257] = 0;
   out_2950416268004545311[258] = 0;
   out_2950416268004545311[259] = 0;
   out_2950416268004545311[260] = 0;
   out_2950416268004545311[261] = 0;
   out_2950416268004545311[262] = 0;
   out_2950416268004545311[263] = 0;
   out_2950416268004545311[264] = 0;
   out_2950416268004545311[265] = 0;
   out_2950416268004545311[266] = 1;
   out_2950416268004545311[267] = 0;
   out_2950416268004545311[268] = 0;
   out_2950416268004545311[269] = 0;
   out_2950416268004545311[270] = 0;
   out_2950416268004545311[271] = 0;
   out_2950416268004545311[272] = 0;
   out_2950416268004545311[273] = 0;
   out_2950416268004545311[274] = 0;
   out_2950416268004545311[275] = 0;
   out_2950416268004545311[276] = 0;
   out_2950416268004545311[277] = 0;
   out_2950416268004545311[278] = 0;
   out_2950416268004545311[279] = 0;
   out_2950416268004545311[280] = 0;
   out_2950416268004545311[281] = 0;
   out_2950416268004545311[282] = 0;
   out_2950416268004545311[283] = 0;
   out_2950416268004545311[284] = 0;
   out_2950416268004545311[285] = 1;
   out_2950416268004545311[286] = 0;
   out_2950416268004545311[287] = 0;
   out_2950416268004545311[288] = 0;
   out_2950416268004545311[289] = 0;
   out_2950416268004545311[290] = 0;
   out_2950416268004545311[291] = 0;
   out_2950416268004545311[292] = 0;
   out_2950416268004545311[293] = 0;
   out_2950416268004545311[294] = 0;
   out_2950416268004545311[295] = 0;
   out_2950416268004545311[296] = 0;
   out_2950416268004545311[297] = 0;
   out_2950416268004545311[298] = 0;
   out_2950416268004545311[299] = 0;
   out_2950416268004545311[300] = 0;
   out_2950416268004545311[301] = 0;
   out_2950416268004545311[302] = 0;
   out_2950416268004545311[303] = 0;
   out_2950416268004545311[304] = 1;
   out_2950416268004545311[305] = 0;
   out_2950416268004545311[306] = 0;
   out_2950416268004545311[307] = 0;
   out_2950416268004545311[308] = 0;
   out_2950416268004545311[309] = 0;
   out_2950416268004545311[310] = 0;
   out_2950416268004545311[311] = 0;
   out_2950416268004545311[312] = 0;
   out_2950416268004545311[313] = 0;
   out_2950416268004545311[314] = 0;
   out_2950416268004545311[315] = 0;
   out_2950416268004545311[316] = 0;
   out_2950416268004545311[317] = 0;
   out_2950416268004545311[318] = 0;
   out_2950416268004545311[319] = 0;
   out_2950416268004545311[320] = 0;
   out_2950416268004545311[321] = 0;
   out_2950416268004545311[322] = 0;
   out_2950416268004545311[323] = 1;
}
void h_4(double *state, double *unused, double *out_7825885472463858534) {
   out_7825885472463858534[0] = state[6] + state[9];
   out_7825885472463858534[1] = state[7] + state[10];
   out_7825885472463858534[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_3220493145206738279) {
   out_3220493145206738279[0] = 0;
   out_3220493145206738279[1] = 0;
   out_3220493145206738279[2] = 0;
   out_3220493145206738279[3] = 0;
   out_3220493145206738279[4] = 0;
   out_3220493145206738279[5] = 0;
   out_3220493145206738279[6] = 1;
   out_3220493145206738279[7] = 0;
   out_3220493145206738279[8] = 0;
   out_3220493145206738279[9] = 1;
   out_3220493145206738279[10] = 0;
   out_3220493145206738279[11] = 0;
   out_3220493145206738279[12] = 0;
   out_3220493145206738279[13] = 0;
   out_3220493145206738279[14] = 0;
   out_3220493145206738279[15] = 0;
   out_3220493145206738279[16] = 0;
   out_3220493145206738279[17] = 0;
   out_3220493145206738279[18] = 0;
   out_3220493145206738279[19] = 0;
   out_3220493145206738279[20] = 0;
   out_3220493145206738279[21] = 0;
   out_3220493145206738279[22] = 0;
   out_3220493145206738279[23] = 0;
   out_3220493145206738279[24] = 0;
   out_3220493145206738279[25] = 1;
   out_3220493145206738279[26] = 0;
   out_3220493145206738279[27] = 0;
   out_3220493145206738279[28] = 1;
   out_3220493145206738279[29] = 0;
   out_3220493145206738279[30] = 0;
   out_3220493145206738279[31] = 0;
   out_3220493145206738279[32] = 0;
   out_3220493145206738279[33] = 0;
   out_3220493145206738279[34] = 0;
   out_3220493145206738279[35] = 0;
   out_3220493145206738279[36] = 0;
   out_3220493145206738279[37] = 0;
   out_3220493145206738279[38] = 0;
   out_3220493145206738279[39] = 0;
   out_3220493145206738279[40] = 0;
   out_3220493145206738279[41] = 0;
   out_3220493145206738279[42] = 0;
   out_3220493145206738279[43] = 0;
   out_3220493145206738279[44] = 1;
   out_3220493145206738279[45] = 0;
   out_3220493145206738279[46] = 0;
   out_3220493145206738279[47] = 1;
   out_3220493145206738279[48] = 0;
   out_3220493145206738279[49] = 0;
   out_3220493145206738279[50] = 0;
   out_3220493145206738279[51] = 0;
   out_3220493145206738279[52] = 0;
   out_3220493145206738279[53] = 0;
}
void h_10(double *state, double *unused, double *out_6806037226153200611) {
   out_6806037226153200611[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_6806037226153200611[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_6806037226153200611[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_3332311375343894911) {
   out_3332311375343894911[0] = 0;
   out_3332311375343894911[1] = 9.8100000000000005*cos(state[1]);
   out_3332311375343894911[2] = 0;
   out_3332311375343894911[3] = 0;
   out_3332311375343894911[4] = -state[8];
   out_3332311375343894911[5] = state[7];
   out_3332311375343894911[6] = 0;
   out_3332311375343894911[7] = state[5];
   out_3332311375343894911[8] = -state[4];
   out_3332311375343894911[9] = 0;
   out_3332311375343894911[10] = 0;
   out_3332311375343894911[11] = 0;
   out_3332311375343894911[12] = 1;
   out_3332311375343894911[13] = 0;
   out_3332311375343894911[14] = 0;
   out_3332311375343894911[15] = 1;
   out_3332311375343894911[16] = 0;
   out_3332311375343894911[17] = 0;
   out_3332311375343894911[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_3332311375343894911[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_3332311375343894911[20] = 0;
   out_3332311375343894911[21] = state[8];
   out_3332311375343894911[22] = 0;
   out_3332311375343894911[23] = -state[6];
   out_3332311375343894911[24] = -state[5];
   out_3332311375343894911[25] = 0;
   out_3332311375343894911[26] = state[3];
   out_3332311375343894911[27] = 0;
   out_3332311375343894911[28] = 0;
   out_3332311375343894911[29] = 0;
   out_3332311375343894911[30] = 0;
   out_3332311375343894911[31] = 1;
   out_3332311375343894911[32] = 0;
   out_3332311375343894911[33] = 0;
   out_3332311375343894911[34] = 1;
   out_3332311375343894911[35] = 0;
   out_3332311375343894911[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_3332311375343894911[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_3332311375343894911[38] = 0;
   out_3332311375343894911[39] = -state[7];
   out_3332311375343894911[40] = state[6];
   out_3332311375343894911[41] = 0;
   out_3332311375343894911[42] = state[4];
   out_3332311375343894911[43] = -state[3];
   out_3332311375343894911[44] = 0;
   out_3332311375343894911[45] = 0;
   out_3332311375343894911[46] = 0;
   out_3332311375343894911[47] = 0;
   out_3332311375343894911[48] = 0;
   out_3332311375343894911[49] = 0;
   out_3332311375343894911[50] = 1;
   out_3332311375343894911[51] = 0;
   out_3332311375343894911[52] = 0;
   out_3332311375343894911[53] = 1;
}
void h_13(double *state, double *unused, double *out_6113289319154753757) {
   out_6113289319154753757[0] = state[3];
   out_6113289319154753757[1] = state[4];
   out_6113289319154753757[2] = state[5];
}
void H_13(double *state, double *unused, double *out_8219319874405478) {
   out_8219319874405478[0] = 0;
   out_8219319874405478[1] = 0;
   out_8219319874405478[2] = 0;
   out_8219319874405478[3] = 1;
   out_8219319874405478[4] = 0;
   out_8219319874405478[5] = 0;
   out_8219319874405478[6] = 0;
   out_8219319874405478[7] = 0;
   out_8219319874405478[8] = 0;
   out_8219319874405478[9] = 0;
   out_8219319874405478[10] = 0;
   out_8219319874405478[11] = 0;
   out_8219319874405478[12] = 0;
   out_8219319874405478[13] = 0;
   out_8219319874405478[14] = 0;
   out_8219319874405478[15] = 0;
   out_8219319874405478[16] = 0;
   out_8219319874405478[17] = 0;
   out_8219319874405478[18] = 0;
   out_8219319874405478[19] = 0;
   out_8219319874405478[20] = 0;
   out_8219319874405478[21] = 0;
   out_8219319874405478[22] = 1;
   out_8219319874405478[23] = 0;
   out_8219319874405478[24] = 0;
   out_8219319874405478[25] = 0;
   out_8219319874405478[26] = 0;
   out_8219319874405478[27] = 0;
   out_8219319874405478[28] = 0;
   out_8219319874405478[29] = 0;
   out_8219319874405478[30] = 0;
   out_8219319874405478[31] = 0;
   out_8219319874405478[32] = 0;
   out_8219319874405478[33] = 0;
   out_8219319874405478[34] = 0;
   out_8219319874405478[35] = 0;
   out_8219319874405478[36] = 0;
   out_8219319874405478[37] = 0;
   out_8219319874405478[38] = 0;
   out_8219319874405478[39] = 0;
   out_8219319874405478[40] = 0;
   out_8219319874405478[41] = 1;
   out_8219319874405478[42] = 0;
   out_8219319874405478[43] = 0;
   out_8219319874405478[44] = 0;
   out_8219319874405478[45] = 0;
   out_8219319874405478[46] = 0;
   out_8219319874405478[47] = 0;
   out_8219319874405478[48] = 0;
   out_8219319874405478[49] = 0;
   out_8219319874405478[50] = 0;
   out_8219319874405478[51] = 0;
   out_8219319874405478[52] = 0;
   out_8219319874405478[53] = 0;
}
void h_14(double *state, double *unused, double *out_6792742952256744963) {
   out_6792742952256744963[0] = state[6];
   out_6792742952256744963[1] = state[7];
   out_6792742952256744963[2] = state[8];
}
void H_14(double *state, double *unused, double *out_742747711132746250) {
   out_742747711132746250[0] = 0;
   out_742747711132746250[1] = 0;
   out_742747711132746250[2] = 0;
   out_742747711132746250[3] = 0;
   out_742747711132746250[4] = 0;
   out_742747711132746250[5] = 0;
   out_742747711132746250[6] = 1;
   out_742747711132746250[7] = 0;
   out_742747711132746250[8] = 0;
   out_742747711132746250[9] = 0;
   out_742747711132746250[10] = 0;
   out_742747711132746250[11] = 0;
   out_742747711132746250[12] = 0;
   out_742747711132746250[13] = 0;
   out_742747711132746250[14] = 0;
   out_742747711132746250[15] = 0;
   out_742747711132746250[16] = 0;
   out_742747711132746250[17] = 0;
   out_742747711132746250[18] = 0;
   out_742747711132746250[19] = 0;
   out_742747711132746250[20] = 0;
   out_742747711132746250[21] = 0;
   out_742747711132746250[22] = 0;
   out_742747711132746250[23] = 0;
   out_742747711132746250[24] = 0;
   out_742747711132746250[25] = 1;
   out_742747711132746250[26] = 0;
   out_742747711132746250[27] = 0;
   out_742747711132746250[28] = 0;
   out_742747711132746250[29] = 0;
   out_742747711132746250[30] = 0;
   out_742747711132746250[31] = 0;
   out_742747711132746250[32] = 0;
   out_742747711132746250[33] = 0;
   out_742747711132746250[34] = 0;
   out_742747711132746250[35] = 0;
   out_742747711132746250[36] = 0;
   out_742747711132746250[37] = 0;
   out_742747711132746250[38] = 0;
   out_742747711132746250[39] = 0;
   out_742747711132746250[40] = 0;
   out_742747711132746250[41] = 0;
   out_742747711132746250[42] = 0;
   out_742747711132746250[43] = 0;
   out_742747711132746250[44] = 1;
   out_742747711132746250[45] = 0;
   out_742747711132746250[46] = 0;
   out_742747711132746250[47] = 0;
   out_742747711132746250[48] = 0;
   out_742747711132746250[49] = 0;
   out_742747711132746250[50] = 0;
   out_742747711132746250[51] = 0;
   out_742747711132746250[52] = 0;
   out_742747711132746250[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_4091115333149920439) {
  err_fun(nom_x, delta_x, out_4091115333149920439);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6264186140300305590) {
  inv_err_fun(nom_x, true_x, out_6264186140300305590);
}
void pose_H_mod_fun(double *state, double *out_4881048693765674271) {
  H_mod_fun(state, out_4881048693765674271);
}
void pose_f_fun(double *state, double dt, double *out_2074460560152714132) {
  f_fun(state,  dt, out_2074460560152714132);
}
void pose_F_fun(double *state, double dt, double *out_2950416268004545311) {
  F_fun(state,  dt, out_2950416268004545311);
}
void pose_h_4(double *state, double *unused, double *out_7825885472463858534) {
  h_4(state, unused, out_7825885472463858534);
}
void pose_H_4(double *state, double *unused, double *out_3220493145206738279) {
  H_4(state, unused, out_3220493145206738279);
}
void pose_h_10(double *state, double *unused, double *out_6806037226153200611) {
  h_10(state, unused, out_6806037226153200611);
}
void pose_H_10(double *state, double *unused, double *out_3332311375343894911) {
  H_10(state, unused, out_3332311375343894911);
}
void pose_h_13(double *state, double *unused, double *out_6113289319154753757) {
  h_13(state, unused, out_6113289319154753757);
}
void pose_H_13(double *state, double *unused, double *out_8219319874405478) {
  H_13(state, unused, out_8219319874405478);
}
void pose_h_14(double *state, double *unused, double *out_6792742952256744963) {
  h_14(state, unused, out_6792742952256744963);
}
void pose_H_14(double *state, double *unused, double *out_742747711132746250) {
  H_14(state, unused, out_742747711132746250);
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

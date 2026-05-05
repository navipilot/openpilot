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
void err_fun(double *nom_x, double *delta_x, double *out_7389011504566069386) {
   out_7389011504566069386[0] = delta_x[0] + nom_x[0];
   out_7389011504566069386[1] = delta_x[1] + nom_x[1];
   out_7389011504566069386[2] = delta_x[2] + nom_x[2];
   out_7389011504566069386[3] = delta_x[3] + nom_x[3];
   out_7389011504566069386[4] = delta_x[4] + nom_x[4];
   out_7389011504566069386[5] = delta_x[5] + nom_x[5];
   out_7389011504566069386[6] = delta_x[6] + nom_x[6];
   out_7389011504566069386[7] = delta_x[7] + nom_x[7];
   out_7389011504566069386[8] = delta_x[8] + nom_x[8];
   out_7389011504566069386[9] = delta_x[9] + nom_x[9];
   out_7389011504566069386[10] = delta_x[10] + nom_x[10];
   out_7389011504566069386[11] = delta_x[11] + nom_x[11];
   out_7389011504566069386[12] = delta_x[12] + nom_x[12];
   out_7389011504566069386[13] = delta_x[13] + nom_x[13];
   out_7389011504566069386[14] = delta_x[14] + nom_x[14];
   out_7389011504566069386[15] = delta_x[15] + nom_x[15];
   out_7389011504566069386[16] = delta_x[16] + nom_x[16];
   out_7389011504566069386[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6795243368226050386) {
   out_6795243368226050386[0] = -nom_x[0] + true_x[0];
   out_6795243368226050386[1] = -nom_x[1] + true_x[1];
   out_6795243368226050386[2] = -nom_x[2] + true_x[2];
   out_6795243368226050386[3] = -nom_x[3] + true_x[3];
   out_6795243368226050386[4] = -nom_x[4] + true_x[4];
   out_6795243368226050386[5] = -nom_x[5] + true_x[5];
   out_6795243368226050386[6] = -nom_x[6] + true_x[6];
   out_6795243368226050386[7] = -nom_x[7] + true_x[7];
   out_6795243368226050386[8] = -nom_x[8] + true_x[8];
   out_6795243368226050386[9] = -nom_x[9] + true_x[9];
   out_6795243368226050386[10] = -nom_x[10] + true_x[10];
   out_6795243368226050386[11] = -nom_x[11] + true_x[11];
   out_6795243368226050386[12] = -nom_x[12] + true_x[12];
   out_6795243368226050386[13] = -nom_x[13] + true_x[13];
   out_6795243368226050386[14] = -nom_x[14] + true_x[14];
   out_6795243368226050386[15] = -nom_x[15] + true_x[15];
   out_6795243368226050386[16] = -nom_x[16] + true_x[16];
   out_6795243368226050386[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_6387810172757071512) {
   out_6387810172757071512[0] = 1.0;
   out_6387810172757071512[1] = 0.0;
   out_6387810172757071512[2] = 0.0;
   out_6387810172757071512[3] = 0.0;
   out_6387810172757071512[4] = 0.0;
   out_6387810172757071512[5] = 0.0;
   out_6387810172757071512[6] = 0.0;
   out_6387810172757071512[7] = 0.0;
   out_6387810172757071512[8] = 0.0;
   out_6387810172757071512[9] = 0.0;
   out_6387810172757071512[10] = 0.0;
   out_6387810172757071512[11] = 0.0;
   out_6387810172757071512[12] = 0.0;
   out_6387810172757071512[13] = 0.0;
   out_6387810172757071512[14] = 0.0;
   out_6387810172757071512[15] = 0.0;
   out_6387810172757071512[16] = 0.0;
   out_6387810172757071512[17] = 0.0;
   out_6387810172757071512[18] = 0.0;
   out_6387810172757071512[19] = 1.0;
   out_6387810172757071512[20] = 0.0;
   out_6387810172757071512[21] = 0.0;
   out_6387810172757071512[22] = 0.0;
   out_6387810172757071512[23] = 0.0;
   out_6387810172757071512[24] = 0.0;
   out_6387810172757071512[25] = 0.0;
   out_6387810172757071512[26] = 0.0;
   out_6387810172757071512[27] = 0.0;
   out_6387810172757071512[28] = 0.0;
   out_6387810172757071512[29] = 0.0;
   out_6387810172757071512[30] = 0.0;
   out_6387810172757071512[31] = 0.0;
   out_6387810172757071512[32] = 0.0;
   out_6387810172757071512[33] = 0.0;
   out_6387810172757071512[34] = 0.0;
   out_6387810172757071512[35] = 0.0;
   out_6387810172757071512[36] = 0.0;
   out_6387810172757071512[37] = 0.0;
   out_6387810172757071512[38] = 1.0;
   out_6387810172757071512[39] = 0.0;
   out_6387810172757071512[40] = 0.0;
   out_6387810172757071512[41] = 0.0;
   out_6387810172757071512[42] = 0.0;
   out_6387810172757071512[43] = 0.0;
   out_6387810172757071512[44] = 0.0;
   out_6387810172757071512[45] = 0.0;
   out_6387810172757071512[46] = 0.0;
   out_6387810172757071512[47] = 0.0;
   out_6387810172757071512[48] = 0.0;
   out_6387810172757071512[49] = 0.0;
   out_6387810172757071512[50] = 0.0;
   out_6387810172757071512[51] = 0.0;
   out_6387810172757071512[52] = 0.0;
   out_6387810172757071512[53] = 0.0;
   out_6387810172757071512[54] = 0.0;
   out_6387810172757071512[55] = 0.0;
   out_6387810172757071512[56] = 0.0;
   out_6387810172757071512[57] = 1.0;
   out_6387810172757071512[58] = 0.0;
   out_6387810172757071512[59] = 0.0;
   out_6387810172757071512[60] = 0.0;
   out_6387810172757071512[61] = 0.0;
   out_6387810172757071512[62] = 0.0;
   out_6387810172757071512[63] = 0.0;
   out_6387810172757071512[64] = 0.0;
   out_6387810172757071512[65] = 0.0;
   out_6387810172757071512[66] = 0.0;
   out_6387810172757071512[67] = 0.0;
   out_6387810172757071512[68] = 0.0;
   out_6387810172757071512[69] = 0.0;
   out_6387810172757071512[70] = 0.0;
   out_6387810172757071512[71] = 0.0;
   out_6387810172757071512[72] = 0.0;
   out_6387810172757071512[73] = 0.0;
   out_6387810172757071512[74] = 0.0;
   out_6387810172757071512[75] = 0.0;
   out_6387810172757071512[76] = 1.0;
   out_6387810172757071512[77] = 0.0;
   out_6387810172757071512[78] = 0.0;
   out_6387810172757071512[79] = 0.0;
   out_6387810172757071512[80] = 0.0;
   out_6387810172757071512[81] = 0.0;
   out_6387810172757071512[82] = 0.0;
   out_6387810172757071512[83] = 0.0;
   out_6387810172757071512[84] = 0.0;
   out_6387810172757071512[85] = 0.0;
   out_6387810172757071512[86] = 0.0;
   out_6387810172757071512[87] = 0.0;
   out_6387810172757071512[88] = 0.0;
   out_6387810172757071512[89] = 0.0;
   out_6387810172757071512[90] = 0.0;
   out_6387810172757071512[91] = 0.0;
   out_6387810172757071512[92] = 0.0;
   out_6387810172757071512[93] = 0.0;
   out_6387810172757071512[94] = 0.0;
   out_6387810172757071512[95] = 1.0;
   out_6387810172757071512[96] = 0.0;
   out_6387810172757071512[97] = 0.0;
   out_6387810172757071512[98] = 0.0;
   out_6387810172757071512[99] = 0.0;
   out_6387810172757071512[100] = 0.0;
   out_6387810172757071512[101] = 0.0;
   out_6387810172757071512[102] = 0.0;
   out_6387810172757071512[103] = 0.0;
   out_6387810172757071512[104] = 0.0;
   out_6387810172757071512[105] = 0.0;
   out_6387810172757071512[106] = 0.0;
   out_6387810172757071512[107] = 0.0;
   out_6387810172757071512[108] = 0.0;
   out_6387810172757071512[109] = 0.0;
   out_6387810172757071512[110] = 0.0;
   out_6387810172757071512[111] = 0.0;
   out_6387810172757071512[112] = 0.0;
   out_6387810172757071512[113] = 0.0;
   out_6387810172757071512[114] = 1.0;
   out_6387810172757071512[115] = 0.0;
   out_6387810172757071512[116] = 0.0;
   out_6387810172757071512[117] = 0.0;
   out_6387810172757071512[118] = 0.0;
   out_6387810172757071512[119] = 0.0;
   out_6387810172757071512[120] = 0.0;
   out_6387810172757071512[121] = 0.0;
   out_6387810172757071512[122] = 0.0;
   out_6387810172757071512[123] = 0.0;
   out_6387810172757071512[124] = 0.0;
   out_6387810172757071512[125] = 0.0;
   out_6387810172757071512[126] = 0.0;
   out_6387810172757071512[127] = 0.0;
   out_6387810172757071512[128] = 0.0;
   out_6387810172757071512[129] = 0.0;
   out_6387810172757071512[130] = 0.0;
   out_6387810172757071512[131] = 0.0;
   out_6387810172757071512[132] = 0.0;
   out_6387810172757071512[133] = 1.0;
   out_6387810172757071512[134] = 0.0;
   out_6387810172757071512[135] = 0.0;
   out_6387810172757071512[136] = 0.0;
   out_6387810172757071512[137] = 0.0;
   out_6387810172757071512[138] = 0.0;
   out_6387810172757071512[139] = 0.0;
   out_6387810172757071512[140] = 0.0;
   out_6387810172757071512[141] = 0.0;
   out_6387810172757071512[142] = 0.0;
   out_6387810172757071512[143] = 0.0;
   out_6387810172757071512[144] = 0.0;
   out_6387810172757071512[145] = 0.0;
   out_6387810172757071512[146] = 0.0;
   out_6387810172757071512[147] = 0.0;
   out_6387810172757071512[148] = 0.0;
   out_6387810172757071512[149] = 0.0;
   out_6387810172757071512[150] = 0.0;
   out_6387810172757071512[151] = 0.0;
   out_6387810172757071512[152] = 1.0;
   out_6387810172757071512[153] = 0.0;
   out_6387810172757071512[154] = 0.0;
   out_6387810172757071512[155] = 0.0;
   out_6387810172757071512[156] = 0.0;
   out_6387810172757071512[157] = 0.0;
   out_6387810172757071512[158] = 0.0;
   out_6387810172757071512[159] = 0.0;
   out_6387810172757071512[160] = 0.0;
   out_6387810172757071512[161] = 0.0;
   out_6387810172757071512[162] = 0.0;
   out_6387810172757071512[163] = 0.0;
   out_6387810172757071512[164] = 0.0;
   out_6387810172757071512[165] = 0.0;
   out_6387810172757071512[166] = 0.0;
   out_6387810172757071512[167] = 0.0;
   out_6387810172757071512[168] = 0.0;
   out_6387810172757071512[169] = 0.0;
   out_6387810172757071512[170] = 0.0;
   out_6387810172757071512[171] = 1.0;
   out_6387810172757071512[172] = 0.0;
   out_6387810172757071512[173] = 0.0;
   out_6387810172757071512[174] = 0.0;
   out_6387810172757071512[175] = 0.0;
   out_6387810172757071512[176] = 0.0;
   out_6387810172757071512[177] = 0.0;
   out_6387810172757071512[178] = 0.0;
   out_6387810172757071512[179] = 0.0;
   out_6387810172757071512[180] = 0.0;
   out_6387810172757071512[181] = 0.0;
   out_6387810172757071512[182] = 0.0;
   out_6387810172757071512[183] = 0.0;
   out_6387810172757071512[184] = 0.0;
   out_6387810172757071512[185] = 0.0;
   out_6387810172757071512[186] = 0.0;
   out_6387810172757071512[187] = 0.0;
   out_6387810172757071512[188] = 0.0;
   out_6387810172757071512[189] = 0.0;
   out_6387810172757071512[190] = 1.0;
   out_6387810172757071512[191] = 0.0;
   out_6387810172757071512[192] = 0.0;
   out_6387810172757071512[193] = 0.0;
   out_6387810172757071512[194] = 0.0;
   out_6387810172757071512[195] = 0.0;
   out_6387810172757071512[196] = 0.0;
   out_6387810172757071512[197] = 0.0;
   out_6387810172757071512[198] = 0.0;
   out_6387810172757071512[199] = 0.0;
   out_6387810172757071512[200] = 0.0;
   out_6387810172757071512[201] = 0.0;
   out_6387810172757071512[202] = 0.0;
   out_6387810172757071512[203] = 0.0;
   out_6387810172757071512[204] = 0.0;
   out_6387810172757071512[205] = 0.0;
   out_6387810172757071512[206] = 0.0;
   out_6387810172757071512[207] = 0.0;
   out_6387810172757071512[208] = 0.0;
   out_6387810172757071512[209] = 1.0;
   out_6387810172757071512[210] = 0.0;
   out_6387810172757071512[211] = 0.0;
   out_6387810172757071512[212] = 0.0;
   out_6387810172757071512[213] = 0.0;
   out_6387810172757071512[214] = 0.0;
   out_6387810172757071512[215] = 0.0;
   out_6387810172757071512[216] = 0.0;
   out_6387810172757071512[217] = 0.0;
   out_6387810172757071512[218] = 0.0;
   out_6387810172757071512[219] = 0.0;
   out_6387810172757071512[220] = 0.0;
   out_6387810172757071512[221] = 0.0;
   out_6387810172757071512[222] = 0.0;
   out_6387810172757071512[223] = 0.0;
   out_6387810172757071512[224] = 0.0;
   out_6387810172757071512[225] = 0.0;
   out_6387810172757071512[226] = 0.0;
   out_6387810172757071512[227] = 0.0;
   out_6387810172757071512[228] = 1.0;
   out_6387810172757071512[229] = 0.0;
   out_6387810172757071512[230] = 0.0;
   out_6387810172757071512[231] = 0.0;
   out_6387810172757071512[232] = 0.0;
   out_6387810172757071512[233] = 0.0;
   out_6387810172757071512[234] = 0.0;
   out_6387810172757071512[235] = 0.0;
   out_6387810172757071512[236] = 0.0;
   out_6387810172757071512[237] = 0.0;
   out_6387810172757071512[238] = 0.0;
   out_6387810172757071512[239] = 0.0;
   out_6387810172757071512[240] = 0.0;
   out_6387810172757071512[241] = 0.0;
   out_6387810172757071512[242] = 0.0;
   out_6387810172757071512[243] = 0.0;
   out_6387810172757071512[244] = 0.0;
   out_6387810172757071512[245] = 0.0;
   out_6387810172757071512[246] = 0.0;
   out_6387810172757071512[247] = 1.0;
   out_6387810172757071512[248] = 0.0;
   out_6387810172757071512[249] = 0.0;
   out_6387810172757071512[250] = 0.0;
   out_6387810172757071512[251] = 0.0;
   out_6387810172757071512[252] = 0.0;
   out_6387810172757071512[253] = 0.0;
   out_6387810172757071512[254] = 0.0;
   out_6387810172757071512[255] = 0.0;
   out_6387810172757071512[256] = 0.0;
   out_6387810172757071512[257] = 0.0;
   out_6387810172757071512[258] = 0.0;
   out_6387810172757071512[259] = 0.0;
   out_6387810172757071512[260] = 0.0;
   out_6387810172757071512[261] = 0.0;
   out_6387810172757071512[262] = 0.0;
   out_6387810172757071512[263] = 0.0;
   out_6387810172757071512[264] = 0.0;
   out_6387810172757071512[265] = 0.0;
   out_6387810172757071512[266] = 1.0;
   out_6387810172757071512[267] = 0.0;
   out_6387810172757071512[268] = 0.0;
   out_6387810172757071512[269] = 0.0;
   out_6387810172757071512[270] = 0.0;
   out_6387810172757071512[271] = 0.0;
   out_6387810172757071512[272] = 0.0;
   out_6387810172757071512[273] = 0.0;
   out_6387810172757071512[274] = 0.0;
   out_6387810172757071512[275] = 0.0;
   out_6387810172757071512[276] = 0.0;
   out_6387810172757071512[277] = 0.0;
   out_6387810172757071512[278] = 0.0;
   out_6387810172757071512[279] = 0.0;
   out_6387810172757071512[280] = 0.0;
   out_6387810172757071512[281] = 0.0;
   out_6387810172757071512[282] = 0.0;
   out_6387810172757071512[283] = 0.0;
   out_6387810172757071512[284] = 0.0;
   out_6387810172757071512[285] = 1.0;
   out_6387810172757071512[286] = 0.0;
   out_6387810172757071512[287] = 0.0;
   out_6387810172757071512[288] = 0.0;
   out_6387810172757071512[289] = 0.0;
   out_6387810172757071512[290] = 0.0;
   out_6387810172757071512[291] = 0.0;
   out_6387810172757071512[292] = 0.0;
   out_6387810172757071512[293] = 0.0;
   out_6387810172757071512[294] = 0.0;
   out_6387810172757071512[295] = 0.0;
   out_6387810172757071512[296] = 0.0;
   out_6387810172757071512[297] = 0.0;
   out_6387810172757071512[298] = 0.0;
   out_6387810172757071512[299] = 0.0;
   out_6387810172757071512[300] = 0.0;
   out_6387810172757071512[301] = 0.0;
   out_6387810172757071512[302] = 0.0;
   out_6387810172757071512[303] = 0.0;
   out_6387810172757071512[304] = 1.0;
   out_6387810172757071512[305] = 0.0;
   out_6387810172757071512[306] = 0.0;
   out_6387810172757071512[307] = 0.0;
   out_6387810172757071512[308] = 0.0;
   out_6387810172757071512[309] = 0.0;
   out_6387810172757071512[310] = 0.0;
   out_6387810172757071512[311] = 0.0;
   out_6387810172757071512[312] = 0.0;
   out_6387810172757071512[313] = 0.0;
   out_6387810172757071512[314] = 0.0;
   out_6387810172757071512[315] = 0.0;
   out_6387810172757071512[316] = 0.0;
   out_6387810172757071512[317] = 0.0;
   out_6387810172757071512[318] = 0.0;
   out_6387810172757071512[319] = 0.0;
   out_6387810172757071512[320] = 0.0;
   out_6387810172757071512[321] = 0.0;
   out_6387810172757071512[322] = 0.0;
   out_6387810172757071512[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_5190191951972352761) {
   out_5190191951972352761[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_5190191951972352761[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_5190191951972352761[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_5190191951972352761[3] = dt*state[12] + state[3];
   out_5190191951972352761[4] = dt*state[13] + state[4];
   out_5190191951972352761[5] = dt*state[14] + state[5];
   out_5190191951972352761[6] = state[6];
   out_5190191951972352761[7] = state[7];
   out_5190191951972352761[8] = state[8];
   out_5190191951972352761[9] = state[9];
   out_5190191951972352761[10] = state[10];
   out_5190191951972352761[11] = state[11];
   out_5190191951972352761[12] = state[12];
   out_5190191951972352761[13] = state[13];
   out_5190191951972352761[14] = state[14];
   out_5190191951972352761[15] = state[15];
   out_5190191951972352761[16] = state[16];
   out_5190191951972352761[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3425534515038953406) {
   out_3425534515038953406[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3425534515038953406[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3425534515038953406[2] = 0;
   out_3425534515038953406[3] = 0;
   out_3425534515038953406[4] = 0;
   out_3425534515038953406[5] = 0;
   out_3425534515038953406[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3425534515038953406[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3425534515038953406[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3425534515038953406[9] = 0;
   out_3425534515038953406[10] = 0;
   out_3425534515038953406[11] = 0;
   out_3425534515038953406[12] = 0;
   out_3425534515038953406[13] = 0;
   out_3425534515038953406[14] = 0;
   out_3425534515038953406[15] = 0;
   out_3425534515038953406[16] = 0;
   out_3425534515038953406[17] = 0;
   out_3425534515038953406[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3425534515038953406[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3425534515038953406[20] = 0;
   out_3425534515038953406[21] = 0;
   out_3425534515038953406[22] = 0;
   out_3425534515038953406[23] = 0;
   out_3425534515038953406[24] = 0;
   out_3425534515038953406[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3425534515038953406[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3425534515038953406[27] = 0;
   out_3425534515038953406[28] = 0;
   out_3425534515038953406[29] = 0;
   out_3425534515038953406[30] = 0;
   out_3425534515038953406[31] = 0;
   out_3425534515038953406[32] = 0;
   out_3425534515038953406[33] = 0;
   out_3425534515038953406[34] = 0;
   out_3425534515038953406[35] = 0;
   out_3425534515038953406[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3425534515038953406[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3425534515038953406[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3425534515038953406[39] = 0;
   out_3425534515038953406[40] = 0;
   out_3425534515038953406[41] = 0;
   out_3425534515038953406[42] = 0;
   out_3425534515038953406[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3425534515038953406[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3425534515038953406[45] = 0;
   out_3425534515038953406[46] = 0;
   out_3425534515038953406[47] = 0;
   out_3425534515038953406[48] = 0;
   out_3425534515038953406[49] = 0;
   out_3425534515038953406[50] = 0;
   out_3425534515038953406[51] = 0;
   out_3425534515038953406[52] = 0;
   out_3425534515038953406[53] = 0;
   out_3425534515038953406[54] = 0;
   out_3425534515038953406[55] = 0;
   out_3425534515038953406[56] = 0;
   out_3425534515038953406[57] = 1;
   out_3425534515038953406[58] = 0;
   out_3425534515038953406[59] = 0;
   out_3425534515038953406[60] = 0;
   out_3425534515038953406[61] = 0;
   out_3425534515038953406[62] = 0;
   out_3425534515038953406[63] = 0;
   out_3425534515038953406[64] = 0;
   out_3425534515038953406[65] = 0;
   out_3425534515038953406[66] = dt;
   out_3425534515038953406[67] = 0;
   out_3425534515038953406[68] = 0;
   out_3425534515038953406[69] = 0;
   out_3425534515038953406[70] = 0;
   out_3425534515038953406[71] = 0;
   out_3425534515038953406[72] = 0;
   out_3425534515038953406[73] = 0;
   out_3425534515038953406[74] = 0;
   out_3425534515038953406[75] = 0;
   out_3425534515038953406[76] = 1;
   out_3425534515038953406[77] = 0;
   out_3425534515038953406[78] = 0;
   out_3425534515038953406[79] = 0;
   out_3425534515038953406[80] = 0;
   out_3425534515038953406[81] = 0;
   out_3425534515038953406[82] = 0;
   out_3425534515038953406[83] = 0;
   out_3425534515038953406[84] = 0;
   out_3425534515038953406[85] = dt;
   out_3425534515038953406[86] = 0;
   out_3425534515038953406[87] = 0;
   out_3425534515038953406[88] = 0;
   out_3425534515038953406[89] = 0;
   out_3425534515038953406[90] = 0;
   out_3425534515038953406[91] = 0;
   out_3425534515038953406[92] = 0;
   out_3425534515038953406[93] = 0;
   out_3425534515038953406[94] = 0;
   out_3425534515038953406[95] = 1;
   out_3425534515038953406[96] = 0;
   out_3425534515038953406[97] = 0;
   out_3425534515038953406[98] = 0;
   out_3425534515038953406[99] = 0;
   out_3425534515038953406[100] = 0;
   out_3425534515038953406[101] = 0;
   out_3425534515038953406[102] = 0;
   out_3425534515038953406[103] = 0;
   out_3425534515038953406[104] = dt;
   out_3425534515038953406[105] = 0;
   out_3425534515038953406[106] = 0;
   out_3425534515038953406[107] = 0;
   out_3425534515038953406[108] = 0;
   out_3425534515038953406[109] = 0;
   out_3425534515038953406[110] = 0;
   out_3425534515038953406[111] = 0;
   out_3425534515038953406[112] = 0;
   out_3425534515038953406[113] = 0;
   out_3425534515038953406[114] = 1;
   out_3425534515038953406[115] = 0;
   out_3425534515038953406[116] = 0;
   out_3425534515038953406[117] = 0;
   out_3425534515038953406[118] = 0;
   out_3425534515038953406[119] = 0;
   out_3425534515038953406[120] = 0;
   out_3425534515038953406[121] = 0;
   out_3425534515038953406[122] = 0;
   out_3425534515038953406[123] = 0;
   out_3425534515038953406[124] = 0;
   out_3425534515038953406[125] = 0;
   out_3425534515038953406[126] = 0;
   out_3425534515038953406[127] = 0;
   out_3425534515038953406[128] = 0;
   out_3425534515038953406[129] = 0;
   out_3425534515038953406[130] = 0;
   out_3425534515038953406[131] = 0;
   out_3425534515038953406[132] = 0;
   out_3425534515038953406[133] = 1;
   out_3425534515038953406[134] = 0;
   out_3425534515038953406[135] = 0;
   out_3425534515038953406[136] = 0;
   out_3425534515038953406[137] = 0;
   out_3425534515038953406[138] = 0;
   out_3425534515038953406[139] = 0;
   out_3425534515038953406[140] = 0;
   out_3425534515038953406[141] = 0;
   out_3425534515038953406[142] = 0;
   out_3425534515038953406[143] = 0;
   out_3425534515038953406[144] = 0;
   out_3425534515038953406[145] = 0;
   out_3425534515038953406[146] = 0;
   out_3425534515038953406[147] = 0;
   out_3425534515038953406[148] = 0;
   out_3425534515038953406[149] = 0;
   out_3425534515038953406[150] = 0;
   out_3425534515038953406[151] = 0;
   out_3425534515038953406[152] = 1;
   out_3425534515038953406[153] = 0;
   out_3425534515038953406[154] = 0;
   out_3425534515038953406[155] = 0;
   out_3425534515038953406[156] = 0;
   out_3425534515038953406[157] = 0;
   out_3425534515038953406[158] = 0;
   out_3425534515038953406[159] = 0;
   out_3425534515038953406[160] = 0;
   out_3425534515038953406[161] = 0;
   out_3425534515038953406[162] = 0;
   out_3425534515038953406[163] = 0;
   out_3425534515038953406[164] = 0;
   out_3425534515038953406[165] = 0;
   out_3425534515038953406[166] = 0;
   out_3425534515038953406[167] = 0;
   out_3425534515038953406[168] = 0;
   out_3425534515038953406[169] = 0;
   out_3425534515038953406[170] = 0;
   out_3425534515038953406[171] = 1;
   out_3425534515038953406[172] = 0;
   out_3425534515038953406[173] = 0;
   out_3425534515038953406[174] = 0;
   out_3425534515038953406[175] = 0;
   out_3425534515038953406[176] = 0;
   out_3425534515038953406[177] = 0;
   out_3425534515038953406[178] = 0;
   out_3425534515038953406[179] = 0;
   out_3425534515038953406[180] = 0;
   out_3425534515038953406[181] = 0;
   out_3425534515038953406[182] = 0;
   out_3425534515038953406[183] = 0;
   out_3425534515038953406[184] = 0;
   out_3425534515038953406[185] = 0;
   out_3425534515038953406[186] = 0;
   out_3425534515038953406[187] = 0;
   out_3425534515038953406[188] = 0;
   out_3425534515038953406[189] = 0;
   out_3425534515038953406[190] = 1;
   out_3425534515038953406[191] = 0;
   out_3425534515038953406[192] = 0;
   out_3425534515038953406[193] = 0;
   out_3425534515038953406[194] = 0;
   out_3425534515038953406[195] = 0;
   out_3425534515038953406[196] = 0;
   out_3425534515038953406[197] = 0;
   out_3425534515038953406[198] = 0;
   out_3425534515038953406[199] = 0;
   out_3425534515038953406[200] = 0;
   out_3425534515038953406[201] = 0;
   out_3425534515038953406[202] = 0;
   out_3425534515038953406[203] = 0;
   out_3425534515038953406[204] = 0;
   out_3425534515038953406[205] = 0;
   out_3425534515038953406[206] = 0;
   out_3425534515038953406[207] = 0;
   out_3425534515038953406[208] = 0;
   out_3425534515038953406[209] = 1;
   out_3425534515038953406[210] = 0;
   out_3425534515038953406[211] = 0;
   out_3425534515038953406[212] = 0;
   out_3425534515038953406[213] = 0;
   out_3425534515038953406[214] = 0;
   out_3425534515038953406[215] = 0;
   out_3425534515038953406[216] = 0;
   out_3425534515038953406[217] = 0;
   out_3425534515038953406[218] = 0;
   out_3425534515038953406[219] = 0;
   out_3425534515038953406[220] = 0;
   out_3425534515038953406[221] = 0;
   out_3425534515038953406[222] = 0;
   out_3425534515038953406[223] = 0;
   out_3425534515038953406[224] = 0;
   out_3425534515038953406[225] = 0;
   out_3425534515038953406[226] = 0;
   out_3425534515038953406[227] = 0;
   out_3425534515038953406[228] = 1;
   out_3425534515038953406[229] = 0;
   out_3425534515038953406[230] = 0;
   out_3425534515038953406[231] = 0;
   out_3425534515038953406[232] = 0;
   out_3425534515038953406[233] = 0;
   out_3425534515038953406[234] = 0;
   out_3425534515038953406[235] = 0;
   out_3425534515038953406[236] = 0;
   out_3425534515038953406[237] = 0;
   out_3425534515038953406[238] = 0;
   out_3425534515038953406[239] = 0;
   out_3425534515038953406[240] = 0;
   out_3425534515038953406[241] = 0;
   out_3425534515038953406[242] = 0;
   out_3425534515038953406[243] = 0;
   out_3425534515038953406[244] = 0;
   out_3425534515038953406[245] = 0;
   out_3425534515038953406[246] = 0;
   out_3425534515038953406[247] = 1;
   out_3425534515038953406[248] = 0;
   out_3425534515038953406[249] = 0;
   out_3425534515038953406[250] = 0;
   out_3425534515038953406[251] = 0;
   out_3425534515038953406[252] = 0;
   out_3425534515038953406[253] = 0;
   out_3425534515038953406[254] = 0;
   out_3425534515038953406[255] = 0;
   out_3425534515038953406[256] = 0;
   out_3425534515038953406[257] = 0;
   out_3425534515038953406[258] = 0;
   out_3425534515038953406[259] = 0;
   out_3425534515038953406[260] = 0;
   out_3425534515038953406[261] = 0;
   out_3425534515038953406[262] = 0;
   out_3425534515038953406[263] = 0;
   out_3425534515038953406[264] = 0;
   out_3425534515038953406[265] = 0;
   out_3425534515038953406[266] = 1;
   out_3425534515038953406[267] = 0;
   out_3425534515038953406[268] = 0;
   out_3425534515038953406[269] = 0;
   out_3425534515038953406[270] = 0;
   out_3425534515038953406[271] = 0;
   out_3425534515038953406[272] = 0;
   out_3425534515038953406[273] = 0;
   out_3425534515038953406[274] = 0;
   out_3425534515038953406[275] = 0;
   out_3425534515038953406[276] = 0;
   out_3425534515038953406[277] = 0;
   out_3425534515038953406[278] = 0;
   out_3425534515038953406[279] = 0;
   out_3425534515038953406[280] = 0;
   out_3425534515038953406[281] = 0;
   out_3425534515038953406[282] = 0;
   out_3425534515038953406[283] = 0;
   out_3425534515038953406[284] = 0;
   out_3425534515038953406[285] = 1;
   out_3425534515038953406[286] = 0;
   out_3425534515038953406[287] = 0;
   out_3425534515038953406[288] = 0;
   out_3425534515038953406[289] = 0;
   out_3425534515038953406[290] = 0;
   out_3425534515038953406[291] = 0;
   out_3425534515038953406[292] = 0;
   out_3425534515038953406[293] = 0;
   out_3425534515038953406[294] = 0;
   out_3425534515038953406[295] = 0;
   out_3425534515038953406[296] = 0;
   out_3425534515038953406[297] = 0;
   out_3425534515038953406[298] = 0;
   out_3425534515038953406[299] = 0;
   out_3425534515038953406[300] = 0;
   out_3425534515038953406[301] = 0;
   out_3425534515038953406[302] = 0;
   out_3425534515038953406[303] = 0;
   out_3425534515038953406[304] = 1;
   out_3425534515038953406[305] = 0;
   out_3425534515038953406[306] = 0;
   out_3425534515038953406[307] = 0;
   out_3425534515038953406[308] = 0;
   out_3425534515038953406[309] = 0;
   out_3425534515038953406[310] = 0;
   out_3425534515038953406[311] = 0;
   out_3425534515038953406[312] = 0;
   out_3425534515038953406[313] = 0;
   out_3425534515038953406[314] = 0;
   out_3425534515038953406[315] = 0;
   out_3425534515038953406[316] = 0;
   out_3425534515038953406[317] = 0;
   out_3425534515038953406[318] = 0;
   out_3425534515038953406[319] = 0;
   out_3425534515038953406[320] = 0;
   out_3425534515038953406[321] = 0;
   out_3425534515038953406[322] = 0;
   out_3425534515038953406[323] = 1;
}
void h_4(double *state, double *unused, double *out_7576152197490450413) {
   out_7576152197490450413[0] = state[6] + state[9];
   out_7576152197490450413[1] = state[7] + state[10];
   out_7576152197490450413[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_3320737668869490966) {
   out_3320737668869490966[0] = 0;
   out_3320737668869490966[1] = 0;
   out_3320737668869490966[2] = 0;
   out_3320737668869490966[3] = 0;
   out_3320737668869490966[4] = 0;
   out_3320737668869490966[5] = 0;
   out_3320737668869490966[6] = 1;
   out_3320737668869490966[7] = 0;
   out_3320737668869490966[8] = 0;
   out_3320737668869490966[9] = 1;
   out_3320737668869490966[10] = 0;
   out_3320737668869490966[11] = 0;
   out_3320737668869490966[12] = 0;
   out_3320737668869490966[13] = 0;
   out_3320737668869490966[14] = 0;
   out_3320737668869490966[15] = 0;
   out_3320737668869490966[16] = 0;
   out_3320737668869490966[17] = 0;
   out_3320737668869490966[18] = 0;
   out_3320737668869490966[19] = 0;
   out_3320737668869490966[20] = 0;
   out_3320737668869490966[21] = 0;
   out_3320737668869490966[22] = 0;
   out_3320737668869490966[23] = 0;
   out_3320737668869490966[24] = 0;
   out_3320737668869490966[25] = 1;
   out_3320737668869490966[26] = 0;
   out_3320737668869490966[27] = 0;
   out_3320737668869490966[28] = 1;
   out_3320737668869490966[29] = 0;
   out_3320737668869490966[30] = 0;
   out_3320737668869490966[31] = 0;
   out_3320737668869490966[32] = 0;
   out_3320737668869490966[33] = 0;
   out_3320737668869490966[34] = 0;
   out_3320737668869490966[35] = 0;
   out_3320737668869490966[36] = 0;
   out_3320737668869490966[37] = 0;
   out_3320737668869490966[38] = 0;
   out_3320737668869490966[39] = 0;
   out_3320737668869490966[40] = 0;
   out_3320737668869490966[41] = 0;
   out_3320737668869490966[42] = 0;
   out_3320737668869490966[43] = 0;
   out_3320737668869490966[44] = 1;
   out_3320737668869490966[45] = 0;
   out_3320737668869490966[46] = 0;
   out_3320737668869490966[47] = 1;
   out_3320737668869490966[48] = 0;
   out_3320737668869490966[49] = 0;
   out_3320737668869490966[50] = 0;
   out_3320737668869490966[51] = 0;
   out_3320737668869490966[52] = 0;
   out_3320737668869490966[53] = 0;
}
void h_10(double *state, double *unused, double *out_3130452913144701320) {
   out_3130452913144701320[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_3130452913144701320[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_3130452913144701320[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_2096220204218508864) {
   out_2096220204218508864[0] = 0;
   out_2096220204218508864[1] = 9.8100000000000005*cos(state[1]);
   out_2096220204218508864[2] = 0;
   out_2096220204218508864[3] = 0;
   out_2096220204218508864[4] = -state[8];
   out_2096220204218508864[5] = state[7];
   out_2096220204218508864[6] = 0;
   out_2096220204218508864[7] = state[5];
   out_2096220204218508864[8] = -state[4];
   out_2096220204218508864[9] = 0;
   out_2096220204218508864[10] = 0;
   out_2096220204218508864[11] = 0;
   out_2096220204218508864[12] = 1;
   out_2096220204218508864[13] = 0;
   out_2096220204218508864[14] = 0;
   out_2096220204218508864[15] = 1;
   out_2096220204218508864[16] = 0;
   out_2096220204218508864[17] = 0;
   out_2096220204218508864[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_2096220204218508864[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_2096220204218508864[20] = 0;
   out_2096220204218508864[21] = state[8];
   out_2096220204218508864[22] = 0;
   out_2096220204218508864[23] = -state[6];
   out_2096220204218508864[24] = -state[5];
   out_2096220204218508864[25] = 0;
   out_2096220204218508864[26] = state[3];
   out_2096220204218508864[27] = 0;
   out_2096220204218508864[28] = 0;
   out_2096220204218508864[29] = 0;
   out_2096220204218508864[30] = 0;
   out_2096220204218508864[31] = 1;
   out_2096220204218508864[32] = 0;
   out_2096220204218508864[33] = 0;
   out_2096220204218508864[34] = 1;
   out_2096220204218508864[35] = 0;
   out_2096220204218508864[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_2096220204218508864[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_2096220204218508864[38] = 0;
   out_2096220204218508864[39] = -state[7];
   out_2096220204218508864[40] = state[6];
   out_2096220204218508864[41] = 0;
   out_2096220204218508864[42] = state[4];
   out_2096220204218508864[43] = -state[3];
   out_2096220204218508864[44] = 0;
   out_2096220204218508864[45] = 0;
   out_2096220204218508864[46] = 0;
   out_2096220204218508864[47] = 0;
   out_2096220204218508864[48] = 0;
   out_2096220204218508864[49] = 0;
   out_2096220204218508864[50] = 1;
   out_2096220204218508864[51] = 0;
   out_2096220204218508864[52] = 0;
   out_2096220204218508864[53] = 1;
}
void h_13(double *state, double *unused, double *out_8767951916852656852) {
   out_8767951916852656852[0] = state[3];
   out_8767951916852656852[1] = state[4];
   out_8767951916852656852[2] = state[5];
}
void H_13(double *state, double *unused, double *out_108463843537158165) {
   out_108463843537158165[0] = 0;
   out_108463843537158165[1] = 0;
   out_108463843537158165[2] = 0;
   out_108463843537158165[3] = 1;
   out_108463843537158165[4] = 0;
   out_108463843537158165[5] = 0;
   out_108463843537158165[6] = 0;
   out_108463843537158165[7] = 0;
   out_108463843537158165[8] = 0;
   out_108463843537158165[9] = 0;
   out_108463843537158165[10] = 0;
   out_108463843537158165[11] = 0;
   out_108463843537158165[12] = 0;
   out_108463843537158165[13] = 0;
   out_108463843537158165[14] = 0;
   out_108463843537158165[15] = 0;
   out_108463843537158165[16] = 0;
   out_108463843537158165[17] = 0;
   out_108463843537158165[18] = 0;
   out_108463843537158165[19] = 0;
   out_108463843537158165[20] = 0;
   out_108463843537158165[21] = 0;
   out_108463843537158165[22] = 1;
   out_108463843537158165[23] = 0;
   out_108463843537158165[24] = 0;
   out_108463843537158165[25] = 0;
   out_108463843537158165[26] = 0;
   out_108463843537158165[27] = 0;
   out_108463843537158165[28] = 0;
   out_108463843537158165[29] = 0;
   out_108463843537158165[30] = 0;
   out_108463843537158165[31] = 0;
   out_108463843537158165[32] = 0;
   out_108463843537158165[33] = 0;
   out_108463843537158165[34] = 0;
   out_108463843537158165[35] = 0;
   out_108463843537158165[36] = 0;
   out_108463843537158165[37] = 0;
   out_108463843537158165[38] = 0;
   out_108463843537158165[39] = 0;
   out_108463843537158165[40] = 0;
   out_108463843537158165[41] = 1;
   out_108463843537158165[42] = 0;
   out_108463843537158165[43] = 0;
   out_108463843537158165[44] = 0;
   out_108463843537158165[45] = 0;
   out_108463843537158165[46] = 0;
   out_108463843537158165[47] = 0;
   out_108463843537158165[48] = 0;
   out_108463843537158165[49] = 0;
   out_108463843537158165[50] = 0;
   out_108463843537158165[51] = 0;
   out_108463843537158165[52] = 0;
   out_108463843537158165[53] = 0;
}
void h_14(double *state, double *unused, double *out_7181242506613011838) {
   out_7181242506613011838[0] = state[6];
   out_7181242506613011838[1] = state[7];
   out_7181242506613011838[2] = state[8];
}
void H_14(double *state, double *unused, double *out_6403526101164863262) {
   out_6403526101164863262[0] = 0;
   out_6403526101164863262[1] = 0;
   out_6403526101164863262[2] = 0;
   out_6403526101164863262[3] = 0;
   out_6403526101164863262[4] = 0;
   out_6403526101164863262[5] = 0;
   out_6403526101164863262[6] = 1;
   out_6403526101164863262[7] = 0;
   out_6403526101164863262[8] = 0;
   out_6403526101164863262[9] = 0;
   out_6403526101164863262[10] = 0;
   out_6403526101164863262[11] = 0;
   out_6403526101164863262[12] = 0;
   out_6403526101164863262[13] = 0;
   out_6403526101164863262[14] = 0;
   out_6403526101164863262[15] = 0;
   out_6403526101164863262[16] = 0;
   out_6403526101164863262[17] = 0;
   out_6403526101164863262[18] = 0;
   out_6403526101164863262[19] = 0;
   out_6403526101164863262[20] = 0;
   out_6403526101164863262[21] = 0;
   out_6403526101164863262[22] = 0;
   out_6403526101164863262[23] = 0;
   out_6403526101164863262[24] = 0;
   out_6403526101164863262[25] = 1;
   out_6403526101164863262[26] = 0;
   out_6403526101164863262[27] = 0;
   out_6403526101164863262[28] = 0;
   out_6403526101164863262[29] = 0;
   out_6403526101164863262[30] = 0;
   out_6403526101164863262[31] = 0;
   out_6403526101164863262[32] = 0;
   out_6403526101164863262[33] = 0;
   out_6403526101164863262[34] = 0;
   out_6403526101164863262[35] = 0;
   out_6403526101164863262[36] = 0;
   out_6403526101164863262[37] = 0;
   out_6403526101164863262[38] = 0;
   out_6403526101164863262[39] = 0;
   out_6403526101164863262[40] = 0;
   out_6403526101164863262[41] = 0;
   out_6403526101164863262[42] = 0;
   out_6403526101164863262[43] = 0;
   out_6403526101164863262[44] = 1;
   out_6403526101164863262[45] = 0;
   out_6403526101164863262[46] = 0;
   out_6403526101164863262[47] = 0;
   out_6403526101164863262[48] = 0;
   out_6403526101164863262[49] = 0;
   out_6403526101164863262[50] = 0;
   out_6403526101164863262[51] = 0;
   out_6403526101164863262[52] = 0;
   out_6403526101164863262[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_7389011504566069386) {
  err_fun(nom_x, delta_x, out_7389011504566069386);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6795243368226050386) {
  inv_err_fun(nom_x, true_x, out_6795243368226050386);
}
void pose_H_mod_fun(double *state, double *out_6387810172757071512) {
  H_mod_fun(state, out_6387810172757071512);
}
void pose_f_fun(double *state, double dt, double *out_5190191951972352761) {
  f_fun(state,  dt, out_5190191951972352761);
}
void pose_F_fun(double *state, double dt, double *out_3425534515038953406) {
  F_fun(state,  dt, out_3425534515038953406);
}
void pose_h_4(double *state, double *unused, double *out_7576152197490450413) {
  h_4(state, unused, out_7576152197490450413);
}
void pose_H_4(double *state, double *unused, double *out_3320737668869490966) {
  H_4(state, unused, out_3320737668869490966);
}
void pose_h_10(double *state, double *unused, double *out_3130452913144701320) {
  h_10(state, unused, out_3130452913144701320);
}
void pose_H_10(double *state, double *unused, double *out_2096220204218508864) {
  H_10(state, unused, out_2096220204218508864);
}
void pose_h_13(double *state, double *unused, double *out_8767951916852656852) {
  h_13(state, unused, out_8767951916852656852);
}
void pose_H_13(double *state, double *unused, double *out_108463843537158165) {
  H_13(state, unused, out_108463843537158165);
}
void pose_h_14(double *state, double *unused, double *out_7181242506613011838) {
  h_14(state, unused, out_7181242506613011838);
}
void pose_H_14(double *state, double *unused, double *out_6403526101164863262) {
  H_14(state, unused, out_6403526101164863262);
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

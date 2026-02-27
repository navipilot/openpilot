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
void err_fun(double *nom_x, double *delta_x, double *out_8902926383898082118) {
   out_8902926383898082118[0] = delta_x[0] + nom_x[0];
   out_8902926383898082118[1] = delta_x[1] + nom_x[1];
   out_8902926383898082118[2] = delta_x[2] + nom_x[2];
   out_8902926383898082118[3] = delta_x[3] + nom_x[3];
   out_8902926383898082118[4] = delta_x[4] + nom_x[4];
   out_8902926383898082118[5] = delta_x[5] + nom_x[5];
   out_8902926383898082118[6] = delta_x[6] + nom_x[6];
   out_8902926383898082118[7] = delta_x[7] + nom_x[7];
   out_8902926383898082118[8] = delta_x[8] + nom_x[8];
   out_8902926383898082118[9] = delta_x[9] + nom_x[9];
   out_8902926383898082118[10] = delta_x[10] + nom_x[10];
   out_8902926383898082118[11] = delta_x[11] + nom_x[11];
   out_8902926383898082118[12] = delta_x[12] + nom_x[12];
   out_8902926383898082118[13] = delta_x[13] + nom_x[13];
   out_8902926383898082118[14] = delta_x[14] + nom_x[14];
   out_8902926383898082118[15] = delta_x[15] + nom_x[15];
   out_8902926383898082118[16] = delta_x[16] + nom_x[16];
   out_8902926383898082118[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_4257102035919230637) {
   out_4257102035919230637[0] = -nom_x[0] + true_x[0];
   out_4257102035919230637[1] = -nom_x[1] + true_x[1];
   out_4257102035919230637[2] = -nom_x[2] + true_x[2];
   out_4257102035919230637[3] = -nom_x[3] + true_x[3];
   out_4257102035919230637[4] = -nom_x[4] + true_x[4];
   out_4257102035919230637[5] = -nom_x[5] + true_x[5];
   out_4257102035919230637[6] = -nom_x[6] + true_x[6];
   out_4257102035919230637[7] = -nom_x[7] + true_x[7];
   out_4257102035919230637[8] = -nom_x[8] + true_x[8];
   out_4257102035919230637[9] = -nom_x[9] + true_x[9];
   out_4257102035919230637[10] = -nom_x[10] + true_x[10];
   out_4257102035919230637[11] = -nom_x[11] + true_x[11];
   out_4257102035919230637[12] = -nom_x[12] + true_x[12];
   out_4257102035919230637[13] = -nom_x[13] + true_x[13];
   out_4257102035919230637[14] = -nom_x[14] + true_x[14];
   out_4257102035919230637[15] = -nom_x[15] + true_x[15];
   out_4257102035919230637[16] = -nom_x[16] + true_x[16];
   out_4257102035919230637[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_4577645484999217247) {
   out_4577645484999217247[0] = 1.0;
   out_4577645484999217247[1] = 0.0;
   out_4577645484999217247[2] = 0.0;
   out_4577645484999217247[3] = 0.0;
   out_4577645484999217247[4] = 0.0;
   out_4577645484999217247[5] = 0.0;
   out_4577645484999217247[6] = 0.0;
   out_4577645484999217247[7] = 0.0;
   out_4577645484999217247[8] = 0.0;
   out_4577645484999217247[9] = 0.0;
   out_4577645484999217247[10] = 0.0;
   out_4577645484999217247[11] = 0.0;
   out_4577645484999217247[12] = 0.0;
   out_4577645484999217247[13] = 0.0;
   out_4577645484999217247[14] = 0.0;
   out_4577645484999217247[15] = 0.0;
   out_4577645484999217247[16] = 0.0;
   out_4577645484999217247[17] = 0.0;
   out_4577645484999217247[18] = 0.0;
   out_4577645484999217247[19] = 1.0;
   out_4577645484999217247[20] = 0.0;
   out_4577645484999217247[21] = 0.0;
   out_4577645484999217247[22] = 0.0;
   out_4577645484999217247[23] = 0.0;
   out_4577645484999217247[24] = 0.0;
   out_4577645484999217247[25] = 0.0;
   out_4577645484999217247[26] = 0.0;
   out_4577645484999217247[27] = 0.0;
   out_4577645484999217247[28] = 0.0;
   out_4577645484999217247[29] = 0.0;
   out_4577645484999217247[30] = 0.0;
   out_4577645484999217247[31] = 0.0;
   out_4577645484999217247[32] = 0.0;
   out_4577645484999217247[33] = 0.0;
   out_4577645484999217247[34] = 0.0;
   out_4577645484999217247[35] = 0.0;
   out_4577645484999217247[36] = 0.0;
   out_4577645484999217247[37] = 0.0;
   out_4577645484999217247[38] = 1.0;
   out_4577645484999217247[39] = 0.0;
   out_4577645484999217247[40] = 0.0;
   out_4577645484999217247[41] = 0.0;
   out_4577645484999217247[42] = 0.0;
   out_4577645484999217247[43] = 0.0;
   out_4577645484999217247[44] = 0.0;
   out_4577645484999217247[45] = 0.0;
   out_4577645484999217247[46] = 0.0;
   out_4577645484999217247[47] = 0.0;
   out_4577645484999217247[48] = 0.0;
   out_4577645484999217247[49] = 0.0;
   out_4577645484999217247[50] = 0.0;
   out_4577645484999217247[51] = 0.0;
   out_4577645484999217247[52] = 0.0;
   out_4577645484999217247[53] = 0.0;
   out_4577645484999217247[54] = 0.0;
   out_4577645484999217247[55] = 0.0;
   out_4577645484999217247[56] = 0.0;
   out_4577645484999217247[57] = 1.0;
   out_4577645484999217247[58] = 0.0;
   out_4577645484999217247[59] = 0.0;
   out_4577645484999217247[60] = 0.0;
   out_4577645484999217247[61] = 0.0;
   out_4577645484999217247[62] = 0.0;
   out_4577645484999217247[63] = 0.0;
   out_4577645484999217247[64] = 0.0;
   out_4577645484999217247[65] = 0.0;
   out_4577645484999217247[66] = 0.0;
   out_4577645484999217247[67] = 0.0;
   out_4577645484999217247[68] = 0.0;
   out_4577645484999217247[69] = 0.0;
   out_4577645484999217247[70] = 0.0;
   out_4577645484999217247[71] = 0.0;
   out_4577645484999217247[72] = 0.0;
   out_4577645484999217247[73] = 0.0;
   out_4577645484999217247[74] = 0.0;
   out_4577645484999217247[75] = 0.0;
   out_4577645484999217247[76] = 1.0;
   out_4577645484999217247[77] = 0.0;
   out_4577645484999217247[78] = 0.0;
   out_4577645484999217247[79] = 0.0;
   out_4577645484999217247[80] = 0.0;
   out_4577645484999217247[81] = 0.0;
   out_4577645484999217247[82] = 0.0;
   out_4577645484999217247[83] = 0.0;
   out_4577645484999217247[84] = 0.0;
   out_4577645484999217247[85] = 0.0;
   out_4577645484999217247[86] = 0.0;
   out_4577645484999217247[87] = 0.0;
   out_4577645484999217247[88] = 0.0;
   out_4577645484999217247[89] = 0.0;
   out_4577645484999217247[90] = 0.0;
   out_4577645484999217247[91] = 0.0;
   out_4577645484999217247[92] = 0.0;
   out_4577645484999217247[93] = 0.0;
   out_4577645484999217247[94] = 0.0;
   out_4577645484999217247[95] = 1.0;
   out_4577645484999217247[96] = 0.0;
   out_4577645484999217247[97] = 0.0;
   out_4577645484999217247[98] = 0.0;
   out_4577645484999217247[99] = 0.0;
   out_4577645484999217247[100] = 0.0;
   out_4577645484999217247[101] = 0.0;
   out_4577645484999217247[102] = 0.0;
   out_4577645484999217247[103] = 0.0;
   out_4577645484999217247[104] = 0.0;
   out_4577645484999217247[105] = 0.0;
   out_4577645484999217247[106] = 0.0;
   out_4577645484999217247[107] = 0.0;
   out_4577645484999217247[108] = 0.0;
   out_4577645484999217247[109] = 0.0;
   out_4577645484999217247[110] = 0.0;
   out_4577645484999217247[111] = 0.0;
   out_4577645484999217247[112] = 0.0;
   out_4577645484999217247[113] = 0.0;
   out_4577645484999217247[114] = 1.0;
   out_4577645484999217247[115] = 0.0;
   out_4577645484999217247[116] = 0.0;
   out_4577645484999217247[117] = 0.0;
   out_4577645484999217247[118] = 0.0;
   out_4577645484999217247[119] = 0.0;
   out_4577645484999217247[120] = 0.0;
   out_4577645484999217247[121] = 0.0;
   out_4577645484999217247[122] = 0.0;
   out_4577645484999217247[123] = 0.0;
   out_4577645484999217247[124] = 0.0;
   out_4577645484999217247[125] = 0.0;
   out_4577645484999217247[126] = 0.0;
   out_4577645484999217247[127] = 0.0;
   out_4577645484999217247[128] = 0.0;
   out_4577645484999217247[129] = 0.0;
   out_4577645484999217247[130] = 0.0;
   out_4577645484999217247[131] = 0.0;
   out_4577645484999217247[132] = 0.0;
   out_4577645484999217247[133] = 1.0;
   out_4577645484999217247[134] = 0.0;
   out_4577645484999217247[135] = 0.0;
   out_4577645484999217247[136] = 0.0;
   out_4577645484999217247[137] = 0.0;
   out_4577645484999217247[138] = 0.0;
   out_4577645484999217247[139] = 0.0;
   out_4577645484999217247[140] = 0.0;
   out_4577645484999217247[141] = 0.0;
   out_4577645484999217247[142] = 0.0;
   out_4577645484999217247[143] = 0.0;
   out_4577645484999217247[144] = 0.0;
   out_4577645484999217247[145] = 0.0;
   out_4577645484999217247[146] = 0.0;
   out_4577645484999217247[147] = 0.0;
   out_4577645484999217247[148] = 0.0;
   out_4577645484999217247[149] = 0.0;
   out_4577645484999217247[150] = 0.0;
   out_4577645484999217247[151] = 0.0;
   out_4577645484999217247[152] = 1.0;
   out_4577645484999217247[153] = 0.0;
   out_4577645484999217247[154] = 0.0;
   out_4577645484999217247[155] = 0.0;
   out_4577645484999217247[156] = 0.0;
   out_4577645484999217247[157] = 0.0;
   out_4577645484999217247[158] = 0.0;
   out_4577645484999217247[159] = 0.0;
   out_4577645484999217247[160] = 0.0;
   out_4577645484999217247[161] = 0.0;
   out_4577645484999217247[162] = 0.0;
   out_4577645484999217247[163] = 0.0;
   out_4577645484999217247[164] = 0.0;
   out_4577645484999217247[165] = 0.0;
   out_4577645484999217247[166] = 0.0;
   out_4577645484999217247[167] = 0.0;
   out_4577645484999217247[168] = 0.0;
   out_4577645484999217247[169] = 0.0;
   out_4577645484999217247[170] = 0.0;
   out_4577645484999217247[171] = 1.0;
   out_4577645484999217247[172] = 0.0;
   out_4577645484999217247[173] = 0.0;
   out_4577645484999217247[174] = 0.0;
   out_4577645484999217247[175] = 0.0;
   out_4577645484999217247[176] = 0.0;
   out_4577645484999217247[177] = 0.0;
   out_4577645484999217247[178] = 0.0;
   out_4577645484999217247[179] = 0.0;
   out_4577645484999217247[180] = 0.0;
   out_4577645484999217247[181] = 0.0;
   out_4577645484999217247[182] = 0.0;
   out_4577645484999217247[183] = 0.0;
   out_4577645484999217247[184] = 0.0;
   out_4577645484999217247[185] = 0.0;
   out_4577645484999217247[186] = 0.0;
   out_4577645484999217247[187] = 0.0;
   out_4577645484999217247[188] = 0.0;
   out_4577645484999217247[189] = 0.0;
   out_4577645484999217247[190] = 1.0;
   out_4577645484999217247[191] = 0.0;
   out_4577645484999217247[192] = 0.0;
   out_4577645484999217247[193] = 0.0;
   out_4577645484999217247[194] = 0.0;
   out_4577645484999217247[195] = 0.0;
   out_4577645484999217247[196] = 0.0;
   out_4577645484999217247[197] = 0.0;
   out_4577645484999217247[198] = 0.0;
   out_4577645484999217247[199] = 0.0;
   out_4577645484999217247[200] = 0.0;
   out_4577645484999217247[201] = 0.0;
   out_4577645484999217247[202] = 0.0;
   out_4577645484999217247[203] = 0.0;
   out_4577645484999217247[204] = 0.0;
   out_4577645484999217247[205] = 0.0;
   out_4577645484999217247[206] = 0.0;
   out_4577645484999217247[207] = 0.0;
   out_4577645484999217247[208] = 0.0;
   out_4577645484999217247[209] = 1.0;
   out_4577645484999217247[210] = 0.0;
   out_4577645484999217247[211] = 0.0;
   out_4577645484999217247[212] = 0.0;
   out_4577645484999217247[213] = 0.0;
   out_4577645484999217247[214] = 0.0;
   out_4577645484999217247[215] = 0.0;
   out_4577645484999217247[216] = 0.0;
   out_4577645484999217247[217] = 0.0;
   out_4577645484999217247[218] = 0.0;
   out_4577645484999217247[219] = 0.0;
   out_4577645484999217247[220] = 0.0;
   out_4577645484999217247[221] = 0.0;
   out_4577645484999217247[222] = 0.0;
   out_4577645484999217247[223] = 0.0;
   out_4577645484999217247[224] = 0.0;
   out_4577645484999217247[225] = 0.0;
   out_4577645484999217247[226] = 0.0;
   out_4577645484999217247[227] = 0.0;
   out_4577645484999217247[228] = 1.0;
   out_4577645484999217247[229] = 0.0;
   out_4577645484999217247[230] = 0.0;
   out_4577645484999217247[231] = 0.0;
   out_4577645484999217247[232] = 0.0;
   out_4577645484999217247[233] = 0.0;
   out_4577645484999217247[234] = 0.0;
   out_4577645484999217247[235] = 0.0;
   out_4577645484999217247[236] = 0.0;
   out_4577645484999217247[237] = 0.0;
   out_4577645484999217247[238] = 0.0;
   out_4577645484999217247[239] = 0.0;
   out_4577645484999217247[240] = 0.0;
   out_4577645484999217247[241] = 0.0;
   out_4577645484999217247[242] = 0.0;
   out_4577645484999217247[243] = 0.0;
   out_4577645484999217247[244] = 0.0;
   out_4577645484999217247[245] = 0.0;
   out_4577645484999217247[246] = 0.0;
   out_4577645484999217247[247] = 1.0;
   out_4577645484999217247[248] = 0.0;
   out_4577645484999217247[249] = 0.0;
   out_4577645484999217247[250] = 0.0;
   out_4577645484999217247[251] = 0.0;
   out_4577645484999217247[252] = 0.0;
   out_4577645484999217247[253] = 0.0;
   out_4577645484999217247[254] = 0.0;
   out_4577645484999217247[255] = 0.0;
   out_4577645484999217247[256] = 0.0;
   out_4577645484999217247[257] = 0.0;
   out_4577645484999217247[258] = 0.0;
   out_4577645484999217247[259] = 0.0;
   out_4577645484999217247[260] = 0.0;
   out_4577645484999217247[261] = 0.0;
   out_4577645484999217247[262] = 0.0;
   out_4577645484999217247[263] = 0.0;
   out_4577645484999217247[264] = 0.0;
   out_4577645484999217247[265] = 0.0;
   out_4577645484999217247[266] = 1.0;
   out_4577645484999217247[267] = 0.0;
   out_4577645484999217247[268] = 0.0;
   out_4577645484999217247[269] = 0.0;
   out_4577645484999217247[270] = 0.0;
   out_4577645484999217247[271] = 0.0;
   out_4577645484999217247[272] = 0.0;
   out_4577645484999217247[273] = 0.0;
   out_4577645484999217247[274] = 0.0;
   out_4577645484999217247[275] = 0.0;
   out_4577645484999217247[276] = 0.0;
   out_4577645484999217247[277] = 0.0;
   out_4577645484999217247[278] = 0.0;
   out_4577645484999217247[279] = 0.0;
   out_4577645484999217247[280] = 0.0;
   out_4577645484999217247[281] = 0.0;
   out_4577645484999217247[282] = 0.0;
   out_4577645484999217247[283] = 0.0;
   out_4577645484999217247[284] = 0.0;
   out_4577645484999217247[285] = 1.0;
   out_4577645484999217247[286] = 0.0;
   out_4577645484999217247[287] = 0.0;
   out_4577645484999217247[288] = 0.0;
   out_4577645484999217247[289] = 0.0;
   out_4577645484999217247[290] = 0.0;
   out_4577645484999217247[291] = 0.0;
   out_4577645484999217247[292] = 0.0;
   out_4577645484999217247[293] = 0.0;
   out_4577645484999217247[294] = 0.0;
   out_4577645484999217247[295] = 0.0;
   out_4577645484999217247[296] = 0.0;
   out_4577645484999217247[297] = 0.0;
   out_4577645484999217247[298] = 0.0;
   out_4577645484999217247[299] = 0.0;
   out_4577645484999217247[300] = 0.0;
   out_4577645484999217247[301] = 0.0;
   out_4577645484999217247[302] = 0.0;
   out_4577645484999217247[303] = 0.0;
   out_4577645484999217247[304] = 1.0;
   out_4577645484999217247[305] = 0.0;
   out_4577645484999217247[306] = 0.0;
   out_4577645484999217247[307] = 0.0;
   out_4577645484999217247[308] = 0.0;
   out_4577645484999217247[309] = 0.0;
   out_4577645484999217247[310] = 0.0;
   out_4577645484999217247[311] = 0.0;
   out_4577645484999217247[312] = 0.0;
   out_4577645484999217247[313] = 0.0;
   out_4577645484999217247[314] = 0.0;
   out_4577645484999217247[315] = 0.0;
   out_4577645484999217247[316] = 0.0;
   out_4577645484999217247[317] = 0.0;
   out_4577645484999217247[318] = 0.0;
   out_4577645484999217247[319] = 0.0;
   out_4577645484999217247[320] = 0.0;
   out_4577645484999217247[321] = 0.0;
   out_4577645484999217247[322] = 0.0;
   out_4577645484999217247[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_8384727316265601151) {
   out_8384727316265601151[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_8384727316265601151[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_8384727316265601151[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_8384727316265601151[3] = dt*state[12] + state[3];
   out_8384727316265601151[4] = dt*state[13] + state[4];
   out_8384727316265601151[5] = dt*state[14] + state[5];
   out_8384727316265601151[6] = state[6];
   out_8384727316265601151[7] = state[7];
   out_8384727316265601151[8] = state[8];
   out_8384727316265601151[9] = state[9];
   out_8384727316265601151[10] = state[10];
   out_8384727316265601151[11] = state[11];
   out_8384727316265601151[12] = state[12];
   out_8384727316265601151[13] = state[13];
   out_8384727316265601151[14] = state[14];
   out_8384727316265601151[15] = state[15];
   out_8384727316265601151[16] = state[16];
   out_8384727316265601151[17] = state[17];
}
void F_fun(double *state, double dt, double *out_26276543904169635) {
   out_26276543904169635[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_26276543904169635[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_26276543904169635[2] = 0;
   out_26276543904169635[3] = 0;
   out_26276543904169635[4] = 0;
   out_26276543904169635[5] = 0;
   out_26276543904169635[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_26276543904169635[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_26276543904169635[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_26276543904169635[9] = 0;
   out_26276543904169635[10] = 0;
   out_26276543904169635[11] = 0;
   out_26276543904169635[12] = 0;
   out_26276543904169635[13] = 0;
   out_26276543904169635[14] = 0;
   out_26276543904169635[15] = 0;
   out_26276543904169635[16] = 0;
   out_26276543904169635[17] = 0;
   out_26276543904169635[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_26276543904169635[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_26276543904169635[20] = 0;
   out_26276543904169635[21] = 0;
   out_26276543904169635[22] = 0;
   out_26276543904169635[23] = 0;
   out_26276543904169635[24] = 0;
   out_26276543904169635[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_26276543904169635[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_26276543904169635[27] = 0;
   out_26276543904169635[28] = 0;
   out_26276543904169635[29] = 0;
   out_26276543904169635[30] = 0;
   out_26276543904169635[31] = 0;
   out_26276543904169635[32] = 0;
   out_26276543904169635[33] = 0;
   out_26276543904169635[34] = 0;
   out_26276543904169635[35] = 0;
   out_26276543904169635[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_26276543904169635[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_26276543904169635[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_26276543904169635[39] = 0;
   out_26276543904169635[40] = 0;
   out_26276543904169635[41] = 0;
   out_26276543904169635[42] = 0;
   out_26276543904169635[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_26276543904169635[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_26276543904169635[45] = 0;
   out_26276543904169635[46] = 0;
   out_26276543904169635[47] = 0;
   out_26276543904169635[48] = 0;
   out_26276543904169635[49] = 0;
   out_26276543904169635[50] = 0;
   out_26276543904169635[51] = 0;
   out_26276543904169635[52] = 0;
   out_26276543904169635[53] = 0;
   out_26276543904169635[54] = 0;
   out_26276543904169635[55] = 0;
   out_26276543904169635[56] = 0;
   out_26276543904169635[57] = 1;
   out_26276543904169635[58] = 0;
   out_26276543904169635[59] = 0;
   out_26276543904169635[60] = 0;
   out_26276543904169635[61] = 0;
   out_26276543904169635[62] = 0;
   out_26276543904169635[63] = 0;
   out_26276543904169635[64] = 0;
   out_26276543904169635[65] = 0;
   out_26276543904169635[66] = dt;
   out_26276543904169635[67] = 0;
   out_26276543904169635[68] = 0;
   out_26276543904169635[69] = 0;
   out_26276543904169635[70] = 0;
   out_26276543904169635[71] = 0;
   out_26276543904169635[72] = 0;
   out_26276543904169635[73] = 0;
   out_26276543904169635[74] = 0;
   out_26276543904169635[75] = 0;
   out_26276543904169635[76] = 1;
   out_26276543904169635[77] = 0;
   out_26276543904169635[78] = 0;
   out_26276543904169635[79] = 0;
   out_26276543904169635[80] = 0;
   out_26276543904169635[81] = 0;
   out_26276543904169635[82] = 0;
   out_26276543904169635[83] = 0;
   out_26276543904169635[84] = 0;
   out_26276543904169635[85] = dt;
   out_26276543904169635[86] = 0;
   out_26276543904169635[87] = 0;
   out_26276543904169635[88] = 0;
   out_26276543904169635[89] = 0;
   out_26276543904169635[90] = 0;
   out_26276543904169635[91] = 0;
   out_26276543904169635[92] = 0;
   out_26276543904169635[93] = 0;
   out_26276543904169635[94] = 0;
   out_26276543904169635[95] = 1;
   out_26276543904169635[96] = 0;
   out_26276543904169635[97] = 0;
   out_26276543904169635[98] = 0;
   out_26276543904169635[99] = 0;
   out_26276543904169635[100] = 0;
   out_26276543904169635[101] = 0;
   out_26276543904169635[102] = 0;
   out_26276543904169635[103] = 0;
   out_26276543904169635[104] = dt;
   out_26276543904169635[105] = 0;
   out_26276543904169635[106] = 0;
   out_26276543904169635[107] = 0;
   out_26276543904169635[108] = 0;
   out_26276543904169635[109] = 0;
   out_26276543904169635[110] = 0;
   out_26276543904169635[111] = 0;
   out_26276543904169635[112] = 0;
   out_26276543904169635[113] = 0;
   out_26276543904169635[114] = 1;
   out_26276543904169635[115] = 0;
   out_26276543904169635[116] = 0;
   out_26276543904169635[117] = 0;
   out_26276543904169635[118] = 0;
   out_26276543904169635[119] = 0;
   out_26276543904169635[120] = 0;
   out_26276543904169635[121] = 0;
   out_26276543904169635[122] = 0;
   out_26276543904169635[123] = 0;
   out_26276543904169635[124] = 0;
   out_26276543904169635[125] = 0;
   out_26276543904169635[126] = 0;
   out_26276543904169635[127] = 0;
   out_26276543904169635[128] = 0;
   out_26276543904169635[129] = 0;
   out_26276543904169635[130] = 0;
   out_26276543904169635[131] = 0;
   out_26276543904169635[132] = 0;
   out_26276543904169635[133] = 1;
   out_26276543904169635[134] = 0;
   out_26276543904169635[135] = 0;
   out_26276543904169635[136] = 0;
   out_26276543904169635[137] = 0;
   out_26276543904169635[138] = 0;
   out_26276543904169635[139] = 0;
   out_26276543904169635[140] = 0;
   out_26276543904169635[141] = 0;
   out_26276543904169635[142] = 0;
   out_26276543904169635[143] = 0;
   out_26276543904169635[144] = 0;
   out_26276543904169635[145] = 0;
   out_26276543904169635[146] = 0;
   out_26276543904169635[147] = 0;
   out_26276543904169635[148] = 0;
   out_26276543904169635[149] = 0;
   out_26276543904169635[150] = 0;
   out_26276543904169635[151] = 0;
   out_26276543904169635[152] = 1;
   out_26276543904169635[153] = 0;
   out_26276543904169635[154] = 0;
   out_26276543904169635[155] = 0;
   out_26276543904169635[156] = 0;
   out_26276543904169635[157] = 0;
   out_26276543904169635[158] = 0;
   out_26276543904169635[159] = 0;
   out_26276543904169635[160] = 0;
   out_26276543904169635[161] = 0;
   out_26276543904169635[162] = 0;
   out_26276543904169635[163] = 0;
   out_26276543904169635[164] = 0;
   out_26276543904169635[165] = 0;
   out_26276543904169635[166] = 0;
   out_26276543904169635[167] = 0;
   out_26276543904169635[168] = 0;
   out_26276543904169635[169] = 0;
   out_26276543904169635[170] = 0;
   out_26276543904169635[171] = 1;
   out_26276543904169635[172] = 0;
   out_26276543904169635[173] = 0;
   out_26276543904169635[174] = 0;
   out_26276543904169635[175] = 0;
   out_26276543904169635[176] = 0;
   out_26276543904169635[177] = 0;
   out_26276543904169635[178] = 0;
   out_26276543904169635[179] = 0;
   out_26276543904169635[180] = 0;
   out_26276543904169635[181] = 0;
   out_26276543904169635[182] = 0;
   out_26276543904169635[183] = 0;
   out_26276543904169635[184] = 0;
   out_26276543904169635[185] = 0;
   out_26276543904169635[186] = 0;
   out_26276543904169635[187] = 0;
   out_26276543904169635[188] = 0;
   out_26276543904169635[189] = 0;
   out_26276543904169635[190] = 1;
   out_26276543904169635[191] = 0;
   out_26276543904169635[192] = 0;
   out_26276543904169635[193] = 0;
   out_26276543904169635[194] = 0;
   out_26276543904169635[195] = 0;
   out_26276543904169635[196] = 0;
   out_26276543904169635[197] = 0;
   out_26276543904169635[198] = 0;
   out_26276543904169635[199] = 0;
   out_26276543904169635[200] = 0;
   out_26276543904169635[201] = 0;
   out_26276543904169635[202] = 0;
   out_26276543904169635[203] = 0;
   out_26276543904169635[204] = 0;
   out_26276543904169635[205] = 0;
   out_26276543904169635[206] = 0;
   out_26276543904169635[207] = 0;
   out_26276543904169635[208] = 0;
   out_26276543904169635[209] = 1;
   out_26276543904169635[210] = 0;
   out_26276543904169635[211] = 0;
   out_26276543904169635[212] = 0;
   out_26276543904169635[213] = 0;
   out_26276543904169635[214] = 0;
   out_26276543904169635[215] = 0;
   out_26276543904169635[216] = 0;
   out_26276543904169635[217] = 0;
   out_26276543904169635[218] = 0;
   out_26276543904169635[219] = 0;
   out_26276543904169635[220] = 0;
   out_26276543904169635[221] = 0;
   out_26276543904169635[222] = 0;
   out_26276543904169635[223] = 0;
   out_26276543904169635[224] = 0;
   out_26276543904169635[225] = 0;
   out_26276543904169635[226] = 0;
   out_26276543904169635[227] = 0;
   out_26276543904169635[228] = 1;
   out_26276543904169635[229] = 0;
   out_26276543904169635[230] = 0;
   out_26276543904169635[231] = 0;
   out_26276543904169635[232] = 0;
   out_26276543904169635[233] = 0;
   out_26276543904169635[234] = 0;
   out_26276543904169635[235] = 0;
   out_26276543904169635[236] = 0;
   out_26276543904169635[237] = 0;
   out_26276543904169635[238] = 0;
   out_26276543904169635[239] = 0;
   out_26276543904169635[240] = 0;
   out_26276543904169635[241] = 0;
   out_26276543904169635[242] = 0;
   out_26276543904169635[243] = 0;
   out_26276543904169635[244] = 0;
   out_26276543904169635[245] = 0;
   out_26276543904169635[246] = 0;
   out_26276543904169635[247] = 1;
   out_26276543904169635[248] = 0;
   out_26276543904169635[249] = 0;
   out_26276543904169635[250] = 0;
   out_26276543904169635[251] = 0;
   out_26276543904169635[252] = 0;
   out_26276543904169635[253] = 0;
   out_26276543904169635[254] = 0;
   out_26276543904169635[255] = 0;
   out_26276543904169635[256] = 0;
   out_26276543904169635[257] = 0;
   out_26276543904169635[258] = 0;
   out_26276543904169635[259] = 0;
   out_26276543904169635[260] = 0;
   out_26276543904169635[261] = 0;
   out_26276543904169635[262] = 0;
   out_26276543904169635[263] = 0;
   out_26276543904169635[264] = 0;
   out_26276543904169635[265] = 0;
   out_26276543904169635[266] = 1;
   out_26276543904169635[267] = 0;
   out_26276543904169635[268] = 0;
   out_26276543904169635[269] = 0;
   out_26276543904169635[270] = 0;
   out_26276543904169635[271] = 0;
   out_26276543904169635[272] = 0;
   out_26276543904169635[273] = 0;
   out_26276543904169635[274] = 0;
   out_26276543904169635[275] = 0;
   out_26276543904169635[276] = 0;
   out_26276543904169635[277] = 0;
   out_26276543904169635[278] = 0;
   out_26276543904169635[279] = 0;
   out_26276543904169635[280] = 0;
   out_26276543904169635[281] = 0;
   out_26276543904169635[282] = 0;
   out_26276543904169635[283] = 0;
   out_26276543904169635[284] = 0;
   out_26276543904169635[285] = 1;
   out_26276543904169635[286] = 0;
   out_26276543904169635[287] = 0;
   out_26276543904169635[288] = 0;
   out_26276543904169635[289] = 0;
   out_26276543904169635[290] = 0;
   out_26276543904169635[291] = 0;
   out_26276543904169635[292] = 0;
   out_26276543904169635[293] = 0;
   out_26276543904169635[294] = 0;
   out_26276543904169635[295] = 0;
   out_26276543904169635[296] = 0;
   out_26276543904169635[297] = 0;
   out_26276543904169635[298] = 0;
   out_26276543904169635[299] = 0;
   out_26276543904169635[300] = 0;
   out_26276543904169635[301] = 0;
   out_26276543904169635[302] = 0;
   out_26276543904169635[303] = 0;
   out_26276543904169635[304] = 1;
   out_26276543904169635[305] = 0;
   out_26276543904169635[306] = 0;
   out_26276543904169635[307] = 0;
   out_26276543904169635[308] = 0;
   out_26276543904169635[309] = 0;
   out_26276543904169635[310] = 0;
   out_26276543904169635[311] = 0;
   out_26276543904169635[312] = 0;
   out_26276543904169635[313] = 0;
   out_26276543904169635[314] = 0;
   out_26276543904169635[315] = 0;
   out_26276543904169635[316] = 0;
   out_26276543904169635[317] = 0;
   out_26276543904169635[318] = 0;
   out_26276543904169635[319] = 0;
   out_26276543904169635[320] = 0;
   out_26276543904169635[321] = 0;
   out_26276543904169635[322] = 0;
   out_26276543904169635[323] = 1;
}
void h_4(double *state, double *unused, double *out_7158146674662545747) {
   out_7158146674662545747[0] = state[6] + state[9];
   out_7158146674662545747[1] = state[7] + state[10];
   out_7158146674662545747[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_8716580203671253634) {
   out_8716580203671253634[0] = 0;
   out_8716580203671253634[1] = 0;
   out_8716580203671253634[2] = 0;
   out_8716580203671253634[3] = 0;
   out_8716580203671253634[4] = 0;
   out_8716580203671253634[5] = 0;
   out_8716580203671253634[6] = 1;
   out_8716580203671253634[7] = 0;
   out_8716580203671253634[8] = 0;
   out_8716580203671253634[9] = 1;
   out_8716580203671253634[10] = 0;
   out_8716580203671253634[11] = 0;
   out_8716580203671253634[12] = 0;
   out_8716580203671253634[13] = 0;
   out_8716580203671253634[14] = 0;
   out_8716580203671253634[15] = 0;
   out_8716580203671253634[16] = 0;
   out_8716580203671253634[17] = 0;
   out_8716580203671253634[18] = 0;
   out_8716580203671253634[19] = 0;
   out_8716580203671253634[20] = 0;
   out_8716580203671253634[21] = 0;
   out_8716580203671253634[22] = 0;
   out_8716580203671253634[23] = 0;
   out_8716580203671253634[24] = 0;
   out_8716580203671253634[25] = 1;
   out_8716580203671253634[26] = 0;
   out_8716580203671253634[27] = 0;
   out_8716580203671253634[28] = 1;
   out_8716580203671253634[29] = 0;
   out_8716580203671253634[30] = 0;
   out_8716580203671253634[31] = 0;
   out_8716580203671253634[32] = 0;
   out_8716580203671253634[33] = 0;
   out_8716580203671253634[34] = 0;
   out_8716580203671253634[35] = 0;
   out_8716580203671253634[36] = 0;
   out_8716580203671253634[37] = 0;
   out_8716580203671253634[38] = 0;
   out_8716580203671253634[39] = 0;
   out_8716580203671253634[40] = 0;
   out_8716580203671253634[41] = 0;
   out_8716580203671253634[42] = 0;
   out_8716580203671253634[43] = 0;
   out_8716580203671253634[44] = 1;
   out_8716580203671253634[45] = 0;
   out_8716580203671253634[46] = 0;
   out_8716580203671253634[47] = 1;
   out_8716580203671253634[48] = 0;
   out_8716580203671253634[49] = 0;
   out_8716580203671253634[50] = 0;
   out_8716580203671253634[51] = 0;
   out_8716580203671253634[52] = 0;
   out_8716580203671253634[53] = 0;
}
void h_10(double *state, double *unused, double *out_5318715798257585615) {
   out_5318715798257585615[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_5318715798257585615[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_5318715798257585615[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_3564541191444468409) {
   out_3564541191444468409[0] = 0;
   out_3564541191444468409[1] = 9.8100000000000005*cos(state[1]);
   out_3564541191444468409[2] = 0;
   out_3564541191444468409[3] = 0;
   out_3564541191444468409[4] = -state[8];
   out_3564541191444468409[5] = state[7];
   out_3564541191444468409[6] = 0;
   out_3564541191444468409[7] = state[5];
   out_3564541191444468409[8] = -state[4];
   out_3564541191444468409[9] = 0;
   out_3564541191444468409[10] = 0;
   out_3564541191444468409[11] = 0;
   out_3564541191444468409[12] = 1;
   out_3564541191444468409[13] = 0;
   out_3564541191444468409[14] = 0;
   out_3564541191444468409[15] = 1;
   out_3564541191444468409[16] = 0;
   out_3564541191444468409[17] = 0;
   out_3564541191444468409[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_3564541191444468409[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_3564541191444468409[20] = 0;
   out_3564541191444468409[21] = state[8];
   out_3564541191444468409[22] = 0;
   out_3564541191444468409[23] = -state[6];
   out_3564541191444468409[24] = -state[5];
   out_3564541191444468409[25] = 0;
   out_3564541191444468409[26] = state[3];
   out_3564541191444468409[27] = 0;
   out_3564541191444468409[28] = 0;
   out_3564541191444468409[29] = 0;
   out_3564541191444468409[30] = 0;
   out_3564541191444468409[31] = 1;
   out_3564541191444468409[32] = 0;
   out_3564541191444468409[33] = 0;
   out_3564541191444468409[34] = 1;
   out_3564541191444468409[35] = 0;
   out_3564541191444468409[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_3564541191444468409[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_3564541191444468409[38] = 0;
   out_3564541191444468409[39] = -state[7];
   out_3564541191444468409[40] = state[6];
   out_3564541191444468409[41] = 0;
   out_3564541191444468409[42] = state[4];
   out_3564541191444468409[43] = -state[3];
   out_3564541191444468409[44] = 0;
   out_3564541191444468409[45] = 0;
   out_3564541191444468409[46] = 0;
   out_3564541191444468409[47] = 0;
   out_3564541191444468409[48] = 0;
   out_3564541191444468409[49] = 0;
   out_3564541191444468409[50] = 1;
   out_3564541191444468409[51] = 0;
   out_3564541191444468409[52] = 0;
   out_3564541191444468409[53] = 1;
}
void h_13(double *state, double *unused, double *out_8548651950161202149) {
   out_8548651950161202149[0] = state[3];
   out_8548651950161202149[1] = state[4];
   out_8548651950161202149[2] = state[5];
}
void H_13(double *state, double *unused, double *out_5504306378338920833) {
   out_5504306378338920833[0] = 0;
   out_5504306378338920833[1] = 0;
   out_5504306378338920833[2] = 0;
   out_5504306378338920833[3] = 1;
   out_5504306378338920833[4] = 0;
   out_5504306378338920833[5] = 0;
   out_5504306378338920833[6] = 0;
   out_5504306378338920833[7] = 0;
   out_5504306378338920833[8] = 0;
   out_5504306378338920833[9] = 0;
   out_5504306378338920833[10] = 0;
   out_5504306378338920833[11] = 0;
   out_5504306378338920833[12] = 0;
   out_5504306378338920833[13] = 0;
   out_5504306378338920833[14] = 0;
   out_5504306378338920833[15] = 0;
   out_5504306378338920833[16] = 0;
   out_5504306378338920833[17] = 0;
   out_5504306378338920833[18] = 0;
   out_5504306378338920833[19] = 0;
   out_5504306378338920833[20] = 0;
   out_5504306378338920833[21] = 0;
   out_5504306378338920833[22] = 1;
   out_5504306378338920833[23] = 0;
   out_5504306378338920833[24] = 0;
   out_5504306378338920833[25] = 0;
   out_5504306378338920833[26] = 0;
   out_5504306378338920833[27] = 0;
   out_5504306378338920833[28] = 0;
   out_5504306378338920833[29] = 0;
   out_5504306378338920833[30] = 0;
   out_5504306378338920833[31] = 0;
   out_5504306378338920833[32] = 0;
   out_5504306378338920833[33] = 0;
   out_5504306378338920833[34] = 0;
   out_5504306378338920833[35] = 0;
   out_5504306378338920833[36] = 0;
   out_5504306378338920833[37] = 0;
   out_5504306378338920833[38] = 0;
   out_5504306378338920833[39] = 0;
   out_5504306378338920833[40] = 0;
   out_5504306378338920833[41] = 1;
   out_5504306378338920833[42] = 0;
   out_5504306378338920833[43] = 0;
   out_5504306378338920833[44] = 0;
   out_5504306378338920833[45] = 0;
   out_5504306378338920833[46] = 0;
   out_5504306378338920833[47] = 0;
   out_5504306378338920833[48] = 0;
   out_5504306378338920833[49] = 0;
   out_5504306378338920833[50] = 0;
   out_5504306378338920833[51] = 0;
   out_5504306378338920833[52] = 0;
   out_5504306378338920833[53] = 0;
}
void h_14(double *state, double *unused, double *out_313380574240030784) {
   out_313380574240030784[0] = state[6];
   out_313380574240030784[1] = state[7];
   out_313380574240030784[2] = state[8];
}
void H_14(double *state, double *unused, double *out_4753339347331769105) {
   out_4753339347331769105[0] = 0;
   out_4753339347331769105[1] = 0;
   out_4753339347331769105[2] = 0;
   out_4753339347331769105[3] = 0;
   out_4753339347331769105[4] = 0;
   out_4753339347331769105[5] = 0;
   out_4753339347331769105[6] = 1;
   out_4753339347331769105[7] = 0;
   out_4753339347331769105[8] = 0;
   out_4753339347331769105[9] = 0;
   out_4753339347331769105[10] = 0;
   out_4753339347331769105[11] = 0;
   out_4753339347331769105[12] = 0;
   out_4753339347331769105[13] = 0;
   out_4753339347331769105[14] = 0;
   out_4753339347331769105[15] = 0;
   out_4753339347331769105[16] = 0;
   out_4753339347331769105[17] = 0;
   out_4753339347331769105[18] = 0;
   out_4753339347331769105[19] = 0;
   out_4753339347331769105[20] = 0;
   out_4753339347331769105[21] = 0;
   out_4753339347331769105[22] = 0;
   out_4753339347331769105[23] = 0;
   out_4753339347331769105[24] = 0;
   out_4753339347331769105[25] = 1;
   out_4753339347331769105[26] = 0;
   out_4753339347331769105[27] = 0;
   out_4753339347331769105[28] = 0;
   out_4753339347331769105[29] = 0;
   out_4753339347331769105[30] = 0;
   out_4753339347331769105[31] = 0;
   out_4753339347331769105[32] = 0;
   out_4753339347331769105[33] = 0;
   out_4753339347331769105[34] = 0;
   out_4753339347331769105[35] = 0;
   out_4753339347331769105[36] = 0;
   out_4753339347331769105[37] = 0;
   out_4753339347331769105[38] = 0;
   out_4753339347331769105[39] = 0;
   out_4753339347331769105[40] = 0;
   out_4753339347331769105[41] = 0;
   out_4753339347331769105[42] = 0;
   out_4753339347331769105[43] = 0;
   out_4753339347331769105[44] = 1;
   out_4753339347331769105[45] = 0;
   out_4753339347331769105[46] = 0;
   out_4753339347331769105[47] = 0;
   out_4753339347331769105[48] = 0;
   out_4753339347331769105[49] = 0;
   out_4753339347331769105[50] = 0;
   out_4753339347331769105[51] = 0;
   out_4753339347331769105[52] = 0;
   out_4753339347331769105[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_8902926383898082118) {
  err_fun(nom_x, delta_x, out_8902926383898082118);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_4257102035919230637) {
  inv_err_fun(nom_x, true_x, out_4257102035919230637);
}
void pose_H_mod_fun(double *state, double *out_4577645484999217247) {
  H_mod_fun(state, out_4577645484999217247);
}
void pose_f_fun(double *state, double dt, double *out_8384727316265601151) {
  f_fun(state,  dt, out_8384727316265601151);
}
void pose_F_fun(double *state, double dt, double *out_26276543904169635) {
  F_fun(state,  dt, out_26276543904169635);
}
void pose_h_4(double *state, double *unused, double *out_7158146674662545747) {
  h_4(state, unused, out_7158146674662545747);
}
void pose_H_4(double *state, double *unused, double *out_8716580203671253634) {
  H_4(state, unused, out_8716580203671253634);
}
void pose_h_10(double *state, double *unused, double *out_5318715798257585615) {
  h_10(state, unused, out_5318715798257585615);
}
void pose_H_10(double *state, double *unused, double *out_3564541191444468409) {
  H_10(state, unused, out_3564541191444468409);
}
void pose_h_13(double *state, double *unused, double *out_8548651950161202149) {
  h_13(state, unused, out_8548651950161202149);
}
void pose_H_13(double *state, double *unused, double *out_5504306378338920833) {
  H_13(state, unused, out_5504306378338920833);
}
void pose_h_14(double *state, double *unused, double *out_313380574240030784) {
  h_14(state, unused, out_313380574240030784);
}
void pose_H_14(double *state, double *unused, double *out_4753339347331769105) {
  H_14(state, unused, out_4753339347331769105);
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

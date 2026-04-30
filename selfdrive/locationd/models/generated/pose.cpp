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
void err_fun(double *nom_x, double *delta_x, double *out_1608596609641417580) {
   out_1608596609641417580[0] = delta_x[0] + nom_x[0];
   out_1608596609641417580[1] = delta_x[1] + nom_x[1];
   out_1608596609641417580[2] = delta_x[2] + nom_x[2];
   out_1608596609641417580[3] = delta_x[3] + nom_x[3];
   out_1608596609641417580[4] = delta_x[4] + nom_x[4];
   out_1608596609641417580[5] = delta_x[5] + nom_x[5];
   out_1608596609641417580[6] = delta_x[6] + nom_x[6];
   out_1608596609641417580[7] = delta_x[7] + nom_x[7];
   out_1608596609641417580[8] = delta_x[8] + nom_x[8];
   out_1608596609641417580[9] = delta_x[9] + nom_x[9];
   out_1608596609641417580[10] = delta_x[10] + nom_x[10];
   out_1608596609641417580[11] = delta_x[11] + nom_x[11];
   out_1608596609641417580[12] = delta_x[12] + nom_x[12];
   out_1608596609641417580[13] = delta_x[13] + nom_x[13];
   out_1608596609641417580[14] = delta_x[14] + nom_x[14];
   out_1608596609641417580[15] = delta_x[15] + nom_x[15];
   out_1608596609641417580[16] = delta_x[16] + nom_x[16];
   out_1608596609641417580[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1300195658449584630) {
   out_1300195658449584630[0] = -nom_x[0] + true_x[0];
   out_1300195658449584630[1] = -nom_x[1] + true_x[1];
   out_1300195658449584630[2] = -nom_x[2] + true_x[2];
   out_1300195658449584630[3] = -nom_x[3] + true_x[3];
   out_1300195658449584630[4] = -nom_x[4] + true_x[4];
   out_1300195658449584630[5] = -nom_x[5] + true_x[5];
   out_1300195658449584630[6] = -nom_x[6] + true_x[6];
   out_1300195658449584630[7] = -nom_x[7] + true_x[7];
   out_1300195658449584630[8] = -nom_x[8] + true_x[8];
   out_1300195658449584630[9] = -nom_x[9] + true_x[9];
   out_1300195658449584630[10] = -nom_x[10] + true_x[10];
   out_1300195658449584630[11] = -nom_x[11] + true_x[11];
   out_1300195658449584630[12] = -nom_x[12] + true_x[12];
   out_1300195658449584630[13] = -nom_x[13] + true_x[13];
   out_1300195658449584630[14] = -nom_x[14] + true_x[14];
   out_1300195658449584630[15] = -nom_x[15] + true_x[15];
   out_1300195658449584630[16] = -nom_x[16] + true_x[16];
   out_1300195658449584630[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_4243197136101237079) {
   out_4243197136101237079[0] = 1.0;
   out_4243197136101237079[1] = 0.0;
   out_4243197136101237079[2] = 0.0;
   out_4243197136101237079[3] = 0.0;
   out_4243197136101237079[4] = 0.0;
   out_4243197136101237079[5] = 0.0;
   out_4243197136101237079[6] = 0.0;
   out_4243197136101237079[7] = 0.0;
   out_4243197136101237079[8] = 0.0;
   out_4243197136101237079[9] = 0.0;
   out_4243197136101237079[10] = 0.0;
   out_4243197136101237079[11] = 0.0;
   out_4243197136101237079[12] = 0.0;
   out_4243197136101237079[13] = 0.0;
   out_4243197136101237079[14] = 0.0;
   out_4243197136101237079[15] = 0.0;
   out_4243197136101237079[16] = 0.0;
   out_4243197136101237079[17] = 0.0;
   out_4243197136101237079[18] = 0.0;
   out_4243197136101237079[19] = 1.0;
   out_4243197136101237079[20] = 0.0;
   out_4243197136101237079[21] = 0.0;
   out_4243197136101237079[22] = 0.0;
   out_4243197136101237079[23] = 0.0;
   out_4243197136101237079[24] = 0.0;
   out_4243197136101237079[25] = 0.0;
   out_4243197136101237079[26] = 0.0;
   out_4243197136101237079[27] = 0.0;
   out_4243197136101237079[28] = 0.0;
   out_4243197136101237079[29] = 0.0;
   out_4243197136101237079[30] = 0.0;
   out_4243197136101237079[31] = 0.0;
   out_4243197136101237079[32] = 0.0;
   out_4243197136101237079[33] = 0.0;
   out_4243197136101237079[34] = 0.0;
   out_4243197136101237079[35] = 0.0;
   out_4243197136101237079[36] = 0.0;
   out_4243197136101237079[37] = 0.0;
   out_4243197136101237079[38] = 1.0;
   out_4243197136101237079[39] = 0.0;
   out_4243197136101237079[40] = 0.0;
   out_4243197136101237079[41] = 0.0;
   out_4243197136101237079[42] = 0.0;
   out_4243197136101237079[43] = 0.0;
   out_4243197136101237079[44] = 0.0;
   out_4243197136101237079[45] = 0.0;
   out_4243197136101237079[46] = 0.0;
   out_4243197136101237079[47] = 0.0;
   out_4243197136101237079[48] = 0.0;
   out_4243197136101237079[49] = 0.0;
   out_4243197136101237079[50] = 0.0;
   out_4243197136101237079[51] = 0.0;
   out_4243197136101237079[52] = 0.0;
   out_4243197136101237079[53] = 0.0;
   out_4243197136101237079[54] = 0.0;
   out_4243197136101237079[55] = 0.0;
   out_4243197136101237079[56] = 0.0;
   out_4243197136101237079[57] = 1.0;
   out_4243197136101237079[58] = 0.0;
   out_4243197136101237079[59] = 0.0;
   out_4243197136101237079[60] = 0.0;
   out_4243197136101237079[61] = 0.0;
   out_4243197136101237079[62] = 0.0;
   out_4243197136101237079[63] = 0.0;
   out_4243197136101237079[64] = 0.0;
   out_4243197136101237079[65] = 0.0;
   out_4243197136101237079[66] = 0.0;
   out_4243197136101237079[67] = 0.0;
   out_4243197136101237079[68] = 0.0;
   out_4243197136101237079[69] = 0.0;
   out_4243197136101237079[70] = 0.0;
   out_4243197136101237079[71] = 0.0;
   out_4243197136101237079[72] = 0.0;
   out_4243197136101237079[73] = 0.0;
   out_4243197136101237079[74] = 0.0;
   out_4243197136101237079[75] = 0.0;
   out_4243197136101237079[76] = 1.0;
   out_4243197136101237079[77] = 0.0;
   out_4243197136101237079[78] = 0.0;
   out_4243197136101237079[79] = 0.0;
   out_4243197136101237079[80] = 0.0;
   out_4243197136101237079[81] = 0.0;
   out_4243197136101237079[82] = 0.0;
   out_4243197136101237079[83] = 0.0;
   out_4243197136101237079[84] = 0.0;
   out_4243197136101237079[85] = 0.0;
   out_4243197136101237079[86] = 0.0;
   out_4243197136101237079[87] = 0.0;
   out_4243197136101237079[88] = 0.0;
   out_4243197136101237079[89] = 0.0;
   out_4243197136101237079[90] = 0.0;
   out_4243197136101237079[91] = 0.0;
   out_4243197136101237079[92] = 0.0;
   out_4243197136101237079[93] = 0.0;
   out_4243197136101237079[94] = 0.0;
   out_4243197136101237079[95] = 1.0;
   out_4243197136101237079[96] = 0.0;
   out_4243197136101237079[97] = 0.0;
   out_4243197136101237079[98] = 0.0;
   out_4243197136101237079[99] = 0.0;
   out_4243197136101237079[100] = 0.0;
   out_4243197136101237079[101] = 0.0;
   out_4243197136101237079[102] = 0.0;
   out_4243197136101237079[103] = 0.0;
   out_4243197136101237079[104] = 0.0;
   out_4243197136101237079[105] = 0.0;
   out_4243197136101237079[106] = 0.0;
   out_4243197136101237079[107] = 0.0;
   out_4243197136101237079[108] = 0.0;
   out_4243197136101237079[109] = 0.0;
   out_4243197136101237079[110] = 0.0;
   out_4243197136101237079[111] = 0.0;
   out_4243197136101237079[112] = 0.0;
   out_4243197136101237079[113] = 0.0;
   out_4243197136101237079[114] = 1.0;
   out_4243197136101237079[115] = 0.0;
   out_4243197136101237079[116] = 0.0;
   out_4243197136101237079[117] = 0.0;
   out_4243197136101237079[118] = 0.0;
   out_4243197136101237079[119] = 0.0;
   out_4243197136101237079[120] = 0.0;
   out_4243197136101237079[121] = 0.0;
   out_4243197136101237079[122] = 0.0;
   out_4243197136101237079[123] = 0.0;
   out_4243197136101237079[124] = 0.0;
   out_4243197136101237079[125] = 0.0;
   out_4243197136101237079[126] = 0.0;
   out_4243197136101237079[127] = 0.0;
   out_4243197136101237079[128] = 0.0;
   out_4243197136101237079[129] = 0.0;
   out_4243197136101237079[130] = 0.0;
   out_4243197136101237079[131] = 0.0;
   out_4243197136101237079[132] = 0.0;
   out_4243197136101237079[133] = 1.0;
   out_4243197136101237079[134] = 0.0;
   out_4243197136101237079[135] = 0.0;
   out_4243197136101237079[136] = 0.0;
   out_4243197136101237079[137] = 0.0;
   out_4243197136101237079[138] = 0.0;
   out_4243197136101237079[139] = 0.0;
   out_4243197136101237079[140] = 0.0;
   out_4243197136101237079[141] = 0.0;
   out_4243197136101237079[142] = 0.0;
   out_4243197136101237079[143] = 0.0;
   out_4243197136101237079[144] = 0.0;
   out_4243197136101237079[145] = 0.0;
   out_4243197136101237079[146] = 0.0;
   out_4243197136101237079[147] = 0.0;
   out_4243197136101237079[148] = 0.0;
   out_4243197136101237079[149] = 0.0;
   out_4243197136101237079[150] = 0.0;
   out_4243197136101237079[151] = 0.0;
   out_4243197136101237079[152] = 1.0;
   out_4243197136101237079[153] = 0.0;
   out_4243197136101237079[154] = 0.0;
   out_4243197136101237079[155] = 0.0;
   out_4243197136101237079[156] = 0.0;
   out_4243197136101237079[157] = 0.0;
   out_4243197136101237079[158] = 0.0;
   out_4243197136101237079[159] = 0.0;
   out_4243197136101237079[160] = 0.0;
   out_4243197136101237079[161] = 0.0;
   out_4243197136101237079[162] = 0.0;
   out_4243197136101237079[163] = 0.0;
   out_4243197136101237079[164] = 0.0;
   out_4243197136101237079[165] = 0.0;
   out_4243197136101237079[166] = 0.0;
   out_4243197136101237079[167] = 0.0;
   out_4243197136101237079[168] = 0.0;
   out_4243197136101237079[169] = 0.0;
   out_4243197136101237079[170] = 0.0;
   out_4243197136101237079[171] = 1.0;
   out_4243197136101237079[172] = 0.0;
   out_4243197136101237079[173] = 0.0;
   out_4243197136101237079[174] = 0.0;
   out_4243197136101237079[175] = 0.0;
   out_4243197136101237079[176] = 0.0;
   out_4243197136101237079[177] = 0.0;
   out_4243197136101237079[178] = 0.0;
   out_4243197136101237079[179] = 0.0;
   out_4243197136101237079[180] = 0.0;
   out_4243197136101237079[181] = 0.0;
   out_4243197136101237079[182] = 0.0;
   out_4243197136101237079[183] = 0.0;
   out_4243197136101237079[184] = 0.0;
   out_4243197136101237079[185] = 0.0;
   out_4243197136101237079[186] = 0.0;
   out_4243197136101237079[187] = 0.0;
   out_4243197136101237079[188] = 0.0;
   out_4243197136101237079[189] = 0.0;
   out_4243197136101237079[190] = 1.0;
   out_4243197136101237079[191] = 0.0;
   out_4243197136101237079[192] = 0.0;
   out_4243197136101237079[193] = 0.0;
   out_4243197136101237079[194] = 0.0;
   out_4243197136101237079[195] = 0.0;
   out_4243197136101237079[196] = 0.0;
   out_4243197136101237079[197] = 0.0;
   out_4243197136101237079[198] = 0.0;
   out_4243197136101237079[199] = 0.0;
   out_4243197136101237079[200] = 0.0;
   out_4243197136101237079[201] = 0.0;
   out_4243197136101237079[202] = 0.0;
   out_4243197136101237079[203] = 0.0;
   out_4243197136101237079[204] = 0.0;
   out_4243197136101237079[205] = 0.0;
   out_4243197136101237079[206] = 0.0;
   out_4243197136101237079[207] = 0.0;
   out_4243197136101237079[208] = 0.0;
   out_4243197136101237079[209] = 1.0;
   out_4243197136101237079[210] = 0.0;
   out_4243197136101237079[211] = 0.0;
   out_4243197136101237079[212] = 0.0;
   out_4243197136101237079[213] = 0.0;
   out_4243197136101237079[214] = 0.0;
   out_4243197136101237079[215] = 0.0;
   out_4243197136101237079[216] = 0.0;
   out_4243197136101237079[217] = 0.0;
   out_4243197136101237079[218] = 0.0;
   out_4243197136101237079[219] = 0.0;
   out_4243197136101237079[220] = 0.0;
   out_4243197136101237079[221] = 0.0;
   out_4243197136101237079[222] = 0.0;
   out_4243197136101237079[223] = 0.0;
   out_4243197136101237079[224] = 0.0;
   out_4243197136101237079[225] = 0.0;
   out_4243197136101237079[226] = 0.0;
   out_4243197136101237079[227] = 0.0;
   out_4243197136101237079[228] = 1.0;
   out_4243197136101237079[229] = 0.0;
   out_4243197136101237079[230] = 0.0;
   out_4243197136101237079[231] = 0.0;
   out_4243197136101237079[232] = 0.0;
   out_4243197136101237079[233] = 0.0;
   out_4243197136101237079[234] = 0.0;
   out_4243197136101237079[235] = 0.0;
   out_4243197136101237079[236] = 0.0;
   out_4243197136101237079[237] = 0.0;
   out_4243197136101237079[238] = 0.0;
   out_4243197136101237079[239] = 0.0;
   out_4243197136101237079[240] = 0.0;
   out_4243197136101237079[241] = 0.0;
   out_4243197136101237079[242] = 0.0;
   out_4243197136101237079[243] = 0.0;
   out_4243197136101237079[244] = 0.0;
   out_4243197136101237079[245] = 0.0;
   out_4243197136101237079[246] = 0.0;
   out_4243197136101237079[247] = 1.0;
   out_4243197136101237079[248] = 0.0;
   out_4243197136101237079[249] = 0.0;
   out_4243197136101237079[250] = 0.0;
   out_4243197136101237079[251] = 0.0;
   out_4243197136101237079[252] = 0.0;
   out_4243197136101237079[253] = 0.0;
   out_4243197136101237079[254] = 0.0;
   out_4243197136101237079[255] = 0.0;
   out_4243197136101237079[256] = 0.0;
   out_4243197136101237079[257] = 0.0;
   out_4243197136101237079[258] = 0.0;
   out_4243197136101237079[259] = 0.0;
   out_4243197136101237079[260] = 0.0;
   out_4243197136101237079[261] = 0.0;
   out_4243197136101237079[262] = 0.0;
   out_4243197136101237079[263] = 0.0;
   out_4243197136101237079[264] = 0.0;
   out_4243197136101237079[265] = 0.0;
   out_4243197136101237079[266] = 1.0;
   out_4243197136101237079[267] = 0.0;
   out_4243197136101237079[268] = 0.0;
   out_4243197136101237079[269] = 0.0;
   out_4243197136101237079[270] = 0.0;
   out_4243197136101237079[271] = 0.0;
   out_4243197136101237079[272] = 0.0;
   out_4243197136101237079[273] = 0.0;
   out_4243197136101237079[274] = 0.0;
   out_4243197136101237079[275] = 0.0;
   out_4243197136101237079[276] = 0.0;
   out_4243197136101237079[277] = 0.0;
   out_4243197136101237079[278] = 0.0;
   out_4243197136101237079[279] = 0.0;
   out_4243197136101237079[280] = 0.0;
   out_4243197136101237079[281] = 0.0;
   out_4243197136101237079[282] = 0.0;
   out_4243197136101237079[283] = 0.0;
   out_4243197136101237079[284] = 0.0;
   out_4243197136101237079[285] = 1.0;
   out_4243197136101237079[286] = 0.0;
   out_4243197136101237079[287] = 0.0;
   out_4243197136101237079[288] = 0.0;
   out_4243197136101237079[289] = 0.0;
   out_4243197136101237079[290] = 0.0;
   out_4243197136101237079[291] = 0.0;
   out_4243197136101237079[292] = 0.0;
   out_4243197136101237079[293] = 0.0;
   out_4243197136101237079[294] = 0.0;
   out_4243197136101237079[295] = 0.0;
   out_4243197136101237079[296] = 0.0;
   out_4243197136101237079[297] = 0.0;
   out_4243197136101237079[298] = 0.0;
   out_4243197136101237079[299] = 0.0;
   out_4243197136101237079[300] = 0.0;
   out_4243197136101237079[301] = 0.0;
   out_4243197136101237079[302] = 0.0;
   out_4243197136101237079[303] = 0.0;
   out_4243197136101237079[304] = 1.0;
   out_4243197136101237079[305] = 0.0;
   out_4243197136101237079[306] = 0.0;
   out_4243197136101237079[307] = 0.0;
   out_4243197136101237079[308] = 0.0;
   out_4243197136101237079[309] = 0.0;
   out_4243197136101237079[310] = 0.0;
   out_4243197136101237079[311] = 0.0;
   out_4243197136101237079[312] = 0.0;
   out_4243197136101237079[313] = 0.0;
   out_4243197136101237079[314] = 0.0;
   out_4243197136101237079[315] = 0.0;
   out_4243197136101237079[316] = 0.0;
   out_4243197136101237079[317] = 0.0;
   out_4243197136101237079[318] = 0.0;
   out_4243197136101237079[319] = 0.0;
   out_4243197136101237079[320] = 0.0;
   out_4243197136101237079[321] = 0.0;
   out_4243197136101237079[322] = 0.0;
   out_4243197136101237079[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_5843270271854622054) {
   out_5843270271854622054[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_5843270271854622054[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_5843270271854622054[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_5843270271854622054[3] = dt*state[12] + state[3];
   out_5843270271854622054[4] = dt*state[13] + state[4];
   out_5843270271854622054[5] = dt*state[14] + state[5];
   out_5843270271854622054[6] = state[6];
   out_5843270271854622054[7] = state[7];
   out_5843270271854622054[8] = state[8];
   out_5843270271854622054[9] = state[9];
   out_5843270271854622054[10] = state[10];
   out_5843270271854622054[11] = state[11];
   out_5843270271854622054[12] = state[12];
   out_5843270271854622054[13] = state[13];
   out_5843270271854622054[14] = state[14];
   out_5843270271854622054[15] = state[15];
   out_5843270271854622054[16] = state[16];
   out_5843270271854622054[17] = state[17];
}
void F_fun(double *state, double dt, double *out_1867205219410655269) {
   out_1867205219410655269[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1867205219410655269[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1867205219410655269[2] = 0;
   out_1867205219410655269[3] = 0;
   out_1867205219410655269[4] = 0;
   out_1867205219410655269[5] = 0;
   out_1867205219410655269[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1867205219410655269[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1867205219410655269[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1867205219410655269[9] = 0;
   out_1867205219410655269[10] = 0;
   out_1867205219410655269[11] = 0;
   out_1867205219410655269[12] = 0;
   out_1867205219410655269[13] = 0;
   out_1867205219410655269[14] = 0;
   out_1867205219410655269[15] = 0;
   out_1867205219410655269[16] = 0;
   out_1867205219410655269[17] = 0;
   out_1867205219410655269[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1867205219410655269[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1867205219410655269[20] = 0;
   out_1867205219410655269[21] = 0;
   out_1867205219410655269[22] = 0;
   out_1867205219410655269[23] = 0;
   out_1867205219410655269[24] = 0;
   out_1867205219410655269[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1867205219410655269[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1867205219410655269[27] = 0;
   out_1867205219410655269[28] = 0;
   out_1867205219410655269[29] = 0;
   out_1867205219410655269[30] = 0;
   out_1867205219410655269[31] = 0;
   out_1867205219410655269[32] = 0;
   out_1867205219410655269[33] = 0;
   out_1867205219410655269[34] = 0;
   out_1867205219410655269[35] = 0;
   out_1867205219410655269[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1867205219410655269[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1867205219410655269[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1867205219410655269[39] = 0;
   out_1867205219410655269[40] = 0;
   out_1867205219410655269[41] = 0;
   out_1867205219410655269[42] = 0;
   out_1867205219410655269[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1867205219410655269[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1867205219410655269[45] = 0;
   out_1867205219410655269[46] = 0;
   out_1867205219410655269[47] = 0;
   out_1867205219410655269[48] = 0;
   out_1867205219410655269[49] = 0;
   out_1867205219410655269[50] = 0;
   out_1867205219410655269[51] = 0;
   out_1867205219410655269[52] = 0;
   out_1867205219410655269[53] = 0;
   out_1867205219410655269[54] = 0;
   out_1867205219410655269[55] = 0;
   out_1867205219410655269[56] = 0;
   out_1867205219410655269[57] = 1;
   out_1867205219410655269[58] = 0;
   out_1867205219410655269[59] = 0;
   out_1867205219410655269[60] = 0;
   out_1867205219410655269[61] = 0;
   out_1867205219410655269[62] = 0;
   out_1867205219410655269[63] = 0;
   out_1867205219410655269[64] = 0;
   out_1867205219410655269[65] = 0;
   out_1867205219410655269[66] = dt;
   out_1867205219410655269[67] = 0;
   out_1867205219410655269[68] = 0;
   out_1867205219410655269[69] = 0;
   out_1867205219410655269[70] = 0;
   out_1867205219410655269[71] = 0;
   out_1867205219410655269[72] = 0;
   out_1867205219410655269[73] = 0;
   out_1867205219410655269[74] = 0;
   out_1867205219410655269[75] = 0;
   out_1867205219410655269[76] = 1;
   out_1867205219410655269[77] = 0;
   out_1867205219410655269[78] = 0;
   out_1867205219410655269[79] = 0;
   out_1867205219410655269[80] = 0;
   out_1867205219410655269[81] = 0;
   out_1867205219410655269[82] = 0;
   out_1867205219410655269[83] = 0;
   out_1867205219410655269[84] = 0;
   out_1867205219410655269[85] = dt;
   out_1867205219410655269[86] = 0;
   out_1867205219410655269[87] = 0;
   out_1867205219410655269[88] = 0;
   out_1867205219410655269[89] = 0;
   out_1867205219410655269[90] = 0;
   out_1867205219410655269[91] = 0;
   out_1867205219410655269[92] = 0;
   out_1867205219410655269[93] = 0;
   out_1867205219410655269[94] = 0;
   out_1867205219410655269[95] = 1;
   out_1867205219410655269[96] = 0;
   out_1867205219410655269[97] = 0;
   out_1867205219410655269[98] = 0;
   out_1867205219410655269[99] = 0;
   out_1867205219410655269[100] = 0;
   out_1867205219410655269[101] = 0;
   out_1867205219410655269[102] = 0;
   out_1867205219410655269[103] = 0;
   out_1867205219410655269[104] = dt;
   out_1867205219410655269[105] = 0;
   out_1867205219410655269[106] = 0;
   out_1867205219410655269[107] = 0;
   out_1867205219410655269[108] = 0;
   out_1867205219410655269[109] = 0;
   out_1867205219410655269[110] = 0;
   out_1867205219410655269[111] = 0;
   out_1867205219410655269[112] = 0;
   out_1867205219410655269[113] = 0;
   out_1867205219410655269[114] = 1;
   out_1867205219410655269[115] = 0;
   out_1867205219410655269[116] = 0;
   out_1867205219410655269[117] = 0;
   out_1867205219410655269[118] = 0;
   out_1867205219410655269[119] = 0;
   out_1867205219410655269[120] = 0;
   out_1867205219410655269[121] = 0;
   out_1867205219410655269[122] = 0;
   out_1867205219410655269[123] = 0;
   out_1867205219410655269[124] = 0;
   out_1867205219410655269[125] = 0;
   out_1867205219410655269[126] = 0;
   out_1867205219410655269[127] = 0;
   out_1867205219410655269[128] = 0;
   out_1867205219410655269[129] = 0;
   out_1867205219410655269[130] = 0;
   out_1867205219410655269[131] = 0;
   out_1867205219410655269[132] = 0;
   out_1867205219410655269[133] = 1;
   out_1867205219410655269[134] = 0;
   out_1867205219410655269[135] = 0;
   out_1867205219410655269[136] = 0;
   out_1867205219410655269[137] = 0;
   out_1867205219410655269[138] = 0;
   out_1867205219410655269[139] = 0;
   out_1867205219410655269[140] = 0;
   out_1867205219410655269[141] = 0;
   out_1867205219410655269[142] = 0;
   out_1867205219410655269[143] = 0;
   out_1867205219410655269[144] = 0;
   out_1867205219410655269[145] = 0;
   out_1867205219410655269[146] = 0;
   out_1867205219410655269[147] = 0;
   out_1867205219410655269[148] = 0;
   out_1867205219410655269[149] = 0;
   out_1867205219410655269[150] = 0;
   out_1867205219410655269[151] = 0;
   out_1867205219410655269[152] = 1;
   out_1867205219410655269[153] = 0;
   out_1867205219410655269[154] = 0;
   out_1867205219410655269[155] = 0;
   out_1867205219410655269[156] = 0;
   out_1867205219410655269[157] = 0;
   out_1867205219410655269[158] = 0;
   out_1867205219410655269[159] = 0;
   out_1867205219410655269[160] = 0;
   out_1867205219410655269[161] = 0;
   out_1867205219410655269[162] = 0;
   out_1867205219410655269[163] = 0;
   out_1867205219410655269[164] = 0;
   out_1867205219410655269[165] = 0;
   out_1867205219410655269[166] = 0;
   out_1867205219410655269[167] = 0;
   out_1867205219410655269[168] = 0;
   out_1867205219410655269[169] = 0;
   out_1867205219410655269[170] = 0;
   out_1867205219410655269[171] = 1;
   out_1867205219410655269[172] = 0;
   out_1867205219410655269[173] = 0;
   out_1867205219410655269[174] = 0;
   out_1867205219410655269[175] = 0;
   out_1867205219410655269[176] = 0;
   out_1867205219410655269[177] = 0;
   out_1867205219410655269[178] = 0;
   out_1867205219410655269[179] = 0;
   out_1867205219410655269[180] = 0;
   out_1867205219410655269[181] = 0;
   out_1867205219410655269[182] = 0;
   out_1867205219410655269[183] = 0;
   out_1867205219410655269[184] = 0;
   out_1867205219410655269[185] = 0;
   out_1867205219410655269[186] = 0;
   out_1867205219410655269[187] = 0;
   out_1867205219410655269[188] = 0;
   out_1867205219410655269[189] = 0;
   out_1867205219410655269[190] = 1;
   out_1867205219410655269[191] = 0;
   out_1867205219410655269[192] = 0;
   out_1867205219410655269[193] = 0;
   out_1867205219410655269[194] = 0;
   out_1867205219410655269[195] = 0;
   out_1867205219410655269[196] = 0;
   out_1867205219410655269[197] = 0;
   out_1867205219410655269[198] = 0;
   out_1867205219410655269[199] = 0;
   out_1867205219410655269[200] = 0;
   out_1867205219410655269[201] = 0;
   out_1867205219410655269[202] = 0;
   out_1867205219410655269[203] = 0;
   out_1867205219410655269[204] = 0;
   out_1867205219410655269[205] = 0;
   out_1867205219410655269[206] = 0;
   out_1867205219410655269[207] = 0;
   out_1867205219410655269[208] = 0;
   out_1867205219410655269[209] = 1;
   out_1867205219410655269[210] = 0;
   out_1867205219410655269[211] = 0;
   out_1867205219410655269[212] = 0;
   out_1867205219410655269[213] = 0;
   out_1867205219410655269[214] = 0;
   out_1867205219410655269[215] = 0;
   out_1867205219410655269[216] = 0;
   out_1867205219410655269[217] = 0;
   out_1867205219410655269[218] = 0;
   out_1867205219410655269[219] = 0;
   out_1867205219410655269[220] = 0;
   out_1867205219410655269[221] = 0;
   out_1867205219410655269[222] = 0;
   out_1867205219410655269[223] = 0;
   out_1867205219410655269[224] = 0;
   out_1867205219410655269[225] = 0;
   out_1867205219410655269[226] = 0;
   out_1867205219410655269[227] = 0;
   out_1867205219410655269[228] = 1;
   out_1867205219410655269[229] = 0;
   out_1867205219410655269[230] = 0;
   out_1867205219410655269[231] = 0;
   out_1867205219410655269[232] = 0;
   out_1867205219410655269[233] = 0;
   out_1867205219410655269[234] = 0;
   out_1867205219410655269[235] = 0;
   out_1867205219410655269[236] = 0;
   out_1867205219410655269[237] = 0;
   out_1867205219410655269[238] = 0;
   out_1867205219410655269[239] = 0;
   out_1867205219410655269[240] = 0;
   out_1867205219410655269[241] = 0;
   out_1867205219410655269[242] = 0;
   out_1867205219410655269[243] = 0;
   out_1867205219410655269[244] = 0;
   out_1867205219410655269[245] = 0;
   out_1867205219410655269[246] = 0;
   out_1867205219410655269[247] = 1;
   out_1867205219410655269[248] = 0;
   out_1867205219410655269[249] = 0;
   out_1867205219410655269[250] = 0;
   out_1867205219410655269[251] = 0;
   out_1867205219410655269[252] = 0;
   out_1867205219410655269[253] = 0;
   out_1867205219410655269[254] = 0;
   out_1867205219410655269[255] = 0;
   out_1867205219410655269[256] = 0;
   out_1867205219410655269[257] = 0;
   out_1867205219410655269[258] = 0;
   out_1867205219410655269[259] = 0;
   out_1867205219410655269[260] = 0;
   out_1867205219410655269[261] = 0;
   out_1867205219410655269[262] = 0;
   out_1867205219410655269[263] = 0;
   out_1867205219410655269[264] = 0;
   out_1867205219410655269[265] = 0;
   out_1867205219410655269[266] = 1;
   out_1867205219410655269[267] = 0;
   out_1867205219410655269[268] = 0;
   out_1867205219410655269[269] = 0;
   out_1867205219410655269[270] = 0;
   out_1867205219410655269[271] = 0;
   out_1867205219410655269[272] = 0;
   out_1867205219410655269[273] = 0;
   out_1867205219410655269[274] = 0;
   out_1867205219410655269[275] = 0;
   out_1867205219410655269[276] = 0;
   out_1867205219410655269[277] = 0;
   out_1867205219410655269[278] = 0;
   out_1867205219410655269[279] = 0;
   out_1867205219410655269[280] = 0;
   out_1867205219410655269[281] = 0;
   out_1867205219410655269[282] = 0;
   out_1867205219410655269[283] = 0;
   out_1867205219410655269[284] = 0;
   out_1867205219410655269[285] = 1;
   out_1867205219410655269[286] = 0;
   out_1867205219410655269[287] = 0;
   out_1867205219410655269[288] = 0;
   out_1867205219410655269[289] = 0;
   out_1867205219410655269[290] = 0;
   out_1867205219410655269[291] = 0;
   out_1867205219410655269[292] = 0;
   out_1867205219410655269[293] = 0;
   out_1867205219410655269[294] = 0;
   out_1867205219410655269[295] = 0;
   out_1867205219410655269[296] = 0;
   out_1867205219410655269[297] = 0;
   out_1867205219410655269[298] = 0;
   out_1867205219410655269[299] = 0;
   out_1867205219410655269[300] = 0;
   out_1867205219410655269[301] = 0;
   out_1867205219410655269[302] = 0;
   out_1867205219410655269[303] = 0;
   out_1867205219410655269[304] = 1;
   out_1867205219410655269[305] = 0;
   out_1867205219410655269[306] = 0;
   out_1867205219410655269[307] = 0;
   out_1867205219410655269[308] = 0;
   out_1867205219410655269[309] = 0;
   out_1867205219410655269[310] = 0;
   out_1867205219410655269[311] = 0;
   out_1867205219410655269[312] = 0;
   out_1867205219410655269[313] = 0;
   out_1867205219410655269[314] = 0;
   out_1867205219410655269[315] = 0;
   out_1867205219410655269[316] = 0;
   out_1867205219410655269[317] = 0;
   out_1867205219410655269[318] = 0;
   out_1867205219410655269[319] = 0;
   out_1867205219410655269[320] = 0;
   out_1867205219410655269[321] = 0;
   out_1867205219410655269[322] = 0;
   out_1867205219410655269[323] = 1;
}
void h_4(double *state, double *unused, double *out_980464455538860381) {
   out_980464455538860381[0] = state[6] + state[9];
   out_980464455538860381[1] = state[7] + state[10];
   out_980464455538860381[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_471389926257329507) {
   out_471389926257329507[0] = 0;
   out_471389926257329507[1] = 0;
   out_471389926257329507[2] = 0;
   out_471389926257329507[3] = 0;
   out_471389926257329507[4] = 0;
   out_471389926257329507[5] = 0;
   out_471389926257329507[6] = 1;
   out_471389926257329507[7] = 0;
   out_471389926257329507[8] = 0;
   out_471389926257329507[9] = 1;
   out_471389926257329507[10] = 0;
   out_471389926257329507[11] = 0;
   out_471389926257329507[12] = 0;
   out_471389926257329507[13] = 0;
   out_471389926257329507[14] = 0;
   out_471389926257329507[15] = 0;
   out_471389926257329507[16] = 0;
   out_471389926257329507[17] = 0;
   out_471389926257329507[18] = 0;
   out_471389926257329507[19] = 0;
   out_471389926257329507[20] = 0;
   out_471389926257329507[21] = 0;
   out_471389926257329507[22] = 0;
   out_471389926257329507[23] = 0;
   out_471389926257329507[24] = 0;
   out_471389926257329507[25] = 1;
   out_471389926257329507[26] = 0;
   out_471389926257329507[27] = 0;
   out_471389926257329507[28] = 1;
   out_471389926257329507[29] = 0;
   out_471389926257329507[30] = 0;
   out_471389926257329507[31] = 0;
   out_471389926257329507[32] = 0;
   out_471389926257329507[33] = 0;
   out_471389926257329507[34] = 0;
   out_471389926257329507[35] = 0;
   out_471389926257329507[36] = 0;
   out_471389926257329507[37] = 0;
   out_471389926257329507[38] = 0;
   out_471389926257329507[39] = 0;
   out_471389926257329507[40] = 0;
   out_471389926257329507[41] = 0;
   out_471389926257329507[42] = 0;
   out_471389926257329507[43] = 0;
   out_471389926257329507[44] = 1;
   out_471389926257329507[45] = 0;
   out_471389926257329507[46] = 0;
   out_471389926257329507[47] = 1;
   out_471389926257329507[48] = 0;
   out_471389926257329507[49] = 0;
   out_471389926257329507[50] = 0;
   out_471389926257329507[51] = 0;
   out_471389926257329507[52] = 0;
   out_471389926257329507[53] = 0;
}
void h_10(double *state, double *unused, double *out_5036026826105273660) {
   out_5036026826105273660[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_5036026826105273660[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_5036026826105273660[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_6410218509014697900) {
   out_6410218509014697900[0] = 0;
   out_6410218509014697900[1] = 9.8100000000000005*cos(state[1]);
   out_6410218509014697900[2] = 0;
   out_6410218509014697900[3] = 0;
   out_6410218509014697900[4] = -state[8];
   out_6410218509014697900[5] = state[7];
   out_6410218509014697900[6] = 0;
   out_6410218509014697900[7] = state[5];
   out_6410218509014697900[8] = -state[4];
   out_6410218509014697900[9] = 0;
   out_6410218509014697900[10] = 0;
   out_6410218509014697900[11] = 0;
   out_6410218509014697900[12] = 1;
   out_6410218509014697900[13] = 0;
   out_6410218509014697900[14] = 0;
   out_6410218509014697900[15] = 1;
   out_6410218509014697900[16] = 0;
   out_6410218509014697900[17] = 0;
   out_6410218509014697900[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_6410218509014697900[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_6410218509014697900[20] = 0;
   out_6410218509014697900[21] = state[8];
   out_6410218509014697900[22] = 0;
   out_6410218509014697900[23] = -state[6];
   out_6410218509014697900[24] = -state[5];
   out_6410218509014697900[25] = 0;
   out_6410218509014697900[26] = state[3];
   out_6410218509014697900[27] = 0;
   out_6410218509014697900[28] = 0;
   out_6410218509014697900[29] = 0;
   out_6410218509014697900[30] = 0;
   out_6410218509014697900[31] = 1;
   out_6410218509014697900[32] = 0;
   out_6410218509014697900[33] = 0;
   out_6410218509014697900[34] = 1;
   out_6410218509014697900[35] = 0;
   out_6410218509014697900[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_6410218509014697900[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_6410218509014697900[38] = 0;
   out_6410218509014697900[39] = -state[7];
   out_6410218509014697900[40] = state[6];
   out_6410218509014697900[41] = 0;
   out_6410218509014697900[42] = state[4];
   out_6410218509014697900[43] = -state[3];
   out_6410218509014697900[44] = 0;
   out_6410218509014697900[45] = 0;
   out_6410218509014697900[46] = 0;
   out_6410218509014697900[47] = 0;
   out_6410218509014697900[48] = 0;
   out_6410218509014697900[49] = 0;
   out_6410218509014697900[50] = 1;
   out_6410218509014697900[51] = 0;
   out_6410218509014697900[52] = 0;
   out_6410218509014697900[53] = 1;
}
void h_13(double *state, double *unused, double *out_1633683833944592611) {
   out_1633683833944592611[0] = state[3];
   out_1633683833944592611[1] = state[4];
   out_1633683833944592611[2] = state[5];
}
void H_13(double *state, double *unused, double *out_7139241282059371422) {
   out_7139241282059371422[0] = 0;
   out_7139241282059371422[1] = 0;
   out_7139241282059371422[2] = 0;
   out_7139241282059371422[3] = 1;
   out_7139241282059371422[4] = 0;
   out_7139241282059371422[5] = 0;
   out_7139241282059371422[6] = 0;
   out_7139241282059371422[7] = 0;
   out_7139241282059371422[8] = 0;
   out_7139241282059371422[9] = 0;
   out_7139241282059371422[10] = 0;
   out_7139241282059371422[11] = 0;
   out_7139241282059371422[12] = 0;
   out_7139241282059371422[13] = 0;
   out_7139241282059371422[14] = 0;
   out_7139241282059371422[15] = 0;
   out_7139241282059371422[16] = 0;
   out_7139241282059371422[17] = 0;
   out_7139241282059371422[18] = 0;
   out_7139241282059371422[19] = 0;
   out_7139241282059371422[20] = 0;
   out_7139241282059371422[21] = 0;
   out_7139241282059371422[22] = 1;
   out_7139241282059371422[23] = 0;
   out_7139241282059371422[24] = 0;
   out_7139241282059371422[25] = 0;
   out_7139241282059371422[26] = 0;
   out_7139241282059371422[27] = 0;
   out_7139241282059371422[28] = 0;
   out_7139241282059371422[29] = 0;
   out_7139241282059371422[30] = 0;
   out_7139241282059371422[31] = 0;
   out_7139241282059371422[32] = 0;
   out_7139241282059371422[33] = 0;
   out_7139241282059371422[34] = 0;
   out_7139241282059371422[35] = 0;
   out_7139241282059371422[36] = 0;
   out_7139241282059371422[37] = 0;
   out_7139241282059371422[38] = 0;
   out_7139241282059371422[39] = 0;
   out_7139241282059371422[40] = 0;
   out_7139241282059371422[41] = 1;
   out_7139241282059371422[42] = 0;
   out_7139241282059371422[43] = 0;
   out_7139241282059371422[44] = 0;
   out_7139241282059371422[45] = 0;
   out_7139241282059371422[46] = 0;
   out_7139241282059371422[47] = 0;
   out_7139241282059371422[48] = 0;
   out_7139241282059371422[49] = 0;
   out_7139241282059371422[50] = 0;
   out_7139241282059371422[51] = 0;
   out_7139241282059371422[52] = 0;
   out_7139241282059371422[53] = 0;
}
void h_14(double *state, double *unused, double *out_8904836124963581929) {
   out_8904836124963581929[0] = state[6];
   out_8904836124963581929[1] = state[7];
   out_8904836124963581929[2] = state[8];
}
void H_14(double *state, double *unused, double *out_3491850930082155022) {
   out_3491850930082155022[0] = 0;
   out_3491850930082155022[1] = 0;
   out_3491850930082155022[2] = 0;
   out_3491850930082155022[3] = 0;
   out_3491850930082155022[4] = 0;
   out_3491850930082155022[5] = 0;
   out_3491850930082155022[6] = 1;
   out_3491850930082155022[7] = 0;
   out_3491850930082155022[8] = 0;
   out_3491850930082155022[9] = 0;
   out_3491850930082155022[10] = 0;
   out_3491850930082155022[11] = 0;
   out_3491850930082155022[12] = 0;
   out_3491850930082155022[13] = 0;
   out_3491850930082155022[14] = 0;
   out_3491850930082155022[15] = 0;
   out_3491850930082155022[16] = 0;
   out_3491850930082155022[17] = 0;
   out_3491850930082155022[18] = 0;
   out_3491850930082155022[19] = 0;
   out_3491850930082155022[20] = 0;
   out_3491850930082155022[21] = 0;
   out_3491850930082155022[22] = 0;
   out_3491850930082155022[23] = 0;
   out_3491850930082155022[24] = 0;
   out_3491850930082155022[25] = 1;
   out_3491850930082155022[26] = 0;
   out_3491850930082155022[27] = 0;
   out_3491850930082155022[28] = 0;
   out_3491850930082155022[29] = 0;
   out_3491850930082155022[30] = 0;
   out_3491850930082155022[31] = 0;
   out_3491850930082155022[32] = 0;
   out_3491850930082155022[33] = 0;
   out_3491850930082155022[34] = 0;
   out_3491850930082155022[35] = 0;
   out_3491850930082155022[36] = 0;
   out_3491850930082155022[37] = 0;
   out_3491850930082155022[38] = 0;
   out_3491850930082155022[39] = 0;
   out_3491850930082155022[40] = 0;
   out_3491850930082155022[41] = 0;
   out_3491850930082155022[42] = 0;
   out_3491850930082155022[43] = 0;
   out_3491850930082155022[44] = 1;
   out_3491850930082155022[45] = 0;
   out_3491850930082155022[46] = 0;
   out_3491850930082155022[47] = 0;
   out_3491850930082155022[48] = 0;
   out_3491850930082155022[49] = 0;
   out_3491850930082155022[50] = 0;
   out_3491850930082155022[51] = 0;
   out_3491850930082155022[52] = 0;
   out_3491850930082155022[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_1608596609641417580) {
  err_fun(nom_x, delta_x, out_1608596609641417580);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1300195658449584630) {
  inv_err_fun(nom_x, true_x, out_1300195658449584630);
}
void pose_H_mod_fun(double *state, double *out_4243197136101237079) {
  H_mod_fun(state, out_4243197136101237079);
}
void pose_f_fun(double *state, double dt, double *out_5843270271854622054) {
  f_fun(state,  dt, out_5843270271854622054);
}
void pose_F_fun(double *state, double dt, double *out_1867205219410655269) {
  F_fun(state,  dt, out_1867205219410655269);
}
void pose_h_4(double *state, double *unused, double *out_980464455538860381) {
  h_4(state, unused, out_980464455538860381);
}
void pose_H_4(double *state, double *unused, double *out_471389926257329507) {
  H_4(state, unused, out_471389926257329507);
}
void pose_h_10(double *state, double *unused, double *out_5036026826105273660) {
  h_10(state, unused, out_5036026826105273660);
}
void pose_H_10(double *state, double *unused, double *out_6410218509014697900) {
  H_10(state, unused, out_6410218509014697900);
}
void pose_h_13(double *state, double *unused, double *out_1633683833944592611) {
  h_13(state, unused, out_1633683833944592611);
}
void pose_H_13(double *state, double *unused, double *out_7139241282059371422) {
  H_13(state, unused, out_7139241282059371422);
}
void pose_h_14(double *state, double *unused, double *out_8904836124963581929) {
  h_14(state, unused, out_8904836124963581929);
}
void pose_H_14(double *state, double *unused, double *out_3491850930082155022) {
  H_14(state, unused, out_3491850930082155022);
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

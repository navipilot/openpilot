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
void err_fun(double *nom_x, double *delta_x, double *out_8612662017984698055) {
   out_8612662017984698055[0] = delta_x[0] + nom_x[0];
   out_8612662017984698055[1] = delta_x[1] + nom_x[1];
   out_8612662017984698055[2] = delta_x[2] + nom_x[2];
   out_8612662017984698055[3] = delta_x[3] + nom_x[3];
   out_8612662017984698055[4] = delta_x[4] + nom_x[4];
   out_8612662017984698055[5] = delta_x[5] + nom_x[5];
   out_8612662017984698055[6] = delta_x[6] + nom_x[6];
   out_8612662017984698055[7] = delta_x[7] + nom_x[7];
   out_8612662017984698055[8] = delta_x[8] + nom_x[8];
   out_8612662017984698055[9] = delta_x[9] + nom_x[9];
   out_8612662017984698055[10] = delta_x[10] + nom_x[10];
   out_8612662017984698055[11] = delta_x[11] + nom_x[11];
   out_8612662017984698055[12] = delta_x[12] + nom_x[12];
   out_8612662017984698055[13] = delta_x[13] + nom_x[13];
   out_8612662017984698055[14] = delta_x[14] + nom_x[14];
   out_8612662017984698055[15] = delta_x[15] + nom_x[15];
   out_8612662017984698055[16] = delta_x[16] + nom_x[16];
   out_8612662017984698055[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6583538545281744086) {
   out_6583538545281744086[0] = -nom_x[0] + true_x[0];
   out_6583538545281744086[1] = -nom_x[1] + true_x[1];
   out_6583538545281744086[2] = -nom_x[2] + true_x[2];
   out_6583538545281744086[3] = -nom_x[3] + true_x[3];
   out_6583538545281744086[4] = -nom_x[4] + true_x[4];
   out_6583538545281744086[5] = -nom_x[5] + true_x[5];
   out_6583538545281744086[6] = -nom_x[6] + true_x[6];
   out_6583538545281744086[7] = -nom_x[7] + true_x[7];
   out_6583538545281744086[8] = -nom_x[8] + true_x[8];
   out_6583538545281744086[9] = -nom_x[9] + true_x[9];
   out_6583538545281744086[10] = -nom_x[10] + true_x[10];
   out_6583538545281744086[11] = -nom_x[11] + true_x[11];
   out_6583538545281744086[12] = -nom_x[12] + true_x[12];
   out_6583538545281744086[13] = -nom_x[13] + true_x[13];
   out_6583538545281744086[14] = -nom_x[14] + true_x[14];
   out_6583538545281744086[15] = -nom_x[15] + true_x[15];
   out_6583538545281744086[16] = -nom_x[16] + true_x[16];
   out_6583538545281744086[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_7537065087244600898) {
   out_7537065087244600898[0] = 1.0;
   out_7537065087244600898[1] = 0.0;
   out_7537065087244600898[2] = 0.0;
   out_7537065087244600898[3] = 0.0;
   out_7537065087244600898[4] = 0.0;
   out_7537065087244600898[5] = 0.0;
   out_7537065087244600898[6] = 0.0;
   out_7537065087244600898[7] = 0.0;
   out_7537065087244600898[8] = 0.0;
   out_7537065087244600898[9] = 0.0;
   out_7537065087244600898[10] = 0.0;
   out_7537065087244600898[11] = 0.0;
   out_7537065087244600898[12] = 0.0;
   out_7537065087244600898[13] = 0.0;
   out_7537065087244600898[14] = 0.0;
   out_7537065087244600898[15] = 0.0;
   out_7537065087244600898[16] = 0.0;
   out_7537065087244600898[17] = 0.0;
   out_7537065087244600898[18] = 0.0;
   out_7537065087244600898[19] = 1.0;
   out_7537065087244600898[20] = 0.0;
   out_7537065087244600898[21] = 0.0;
   out_7537065087244600898[22] = 0.0;
   out_7537065087244600898[23] = 0.0;
   out_7537065087244600898[24] = 0.0;
   out_7537065087244600898[25] = 0.0;
   out_7537065087244600898[26] = 0.0;
   out_7537065087244600898[27] = 0.0;
   out_7537065087244600898[28] = 0.0;
   out_7537065087244600898[29] = 0.0;
   out_7537065087244600898[30] = 0.0;
   out_7537065087244600898[31] = 0.0;
   out_7537065087244600898[32] = 0.0;
   out_7537065087244600898[33] = 0.0;
   out_7537065087244600898[34] = 0.0;
   out_7537065087244600898[35] = 0.0;
   out_7537065087244600898[36] = 0.0;
   out_7537065087244600898[37] = 0.0;
   out_7537065087244600898[38] = 1.0;
   out_7537065087244600898[39] = 0.0;
   out_7537065087244600898[40] = 0.0;
   out_7537065087244600898[41] = 0.0;
   out_7537065087244600898[42] = 0.0;
   out_7537065087244600898[43] = 0.0;
   out_7537065087244600898[44] = 0.0;
   out_7537065087244600898[45] = 0.0;
   out_7537065087244600898[46] = 0.0;
   out_7537065087244600898[47] = 0.0;
   out_7537065087244600898[48] = 0.0;
   out_7537065087244600898[49] = 0.0;
   out_7537065087244600898[50] = 0.0;
   out_7537065087244600898[51] = 0.0;
   out_7537065087244600898[52] = 0.0;
   out_7537065087244600898[53] = 0.0;
   out_7537065087244600898[54] = 0.0;
   out_7537065087244600898[55] = 0.0;
   out_7537065087244600898[56] = 0.0;
   out_7537065087244600898[57] = 1.0;
   out_7537065087244600898[58] = 0.0;
   out_7537065087244600898[59] = 0.0;
   out_7537065087244600898[60] = 0.0;
   out_7537065087244600898[61] = 0.0;
   out_7537065087244600898[62] = 0.0;
   out_7537065087244600898[63] = 0.0;
   out_7537065087244600898[64] = 0.0;
   out_7537065087244600898[65] = 0.0;
   out_7537065087244600898[66] = 0.0;
   out_7537065087244600898[67] = 0.0;
   out_7537065087244600898[68] = 0.0;
   out_7537065087244600898[69] = 0.0;
   out_7537065087244600898[70] = 0.0;
   out_7537065087244600898[71] = 0.0;
   out_7537065087244600898[72] = 0.0;
   out_7537065087244600898[73] = 0.0;
   out_7537065087244600898[74] = 0.0;
   out_7537065087244600898[75] = 0.0;
   out_7537065087244600898[76] = 1.0;
   out_7537065087244600898[77] = 0.0;
   out_7537065087244600898[78] = 0.0;
   out_7537065087244600898[79] = 0.0;
   out_7537065087244600898[80] = 0.0;
   out_7537065087244600898[81] = 0.0;
   out_7537065087244600898[82] = 0.0;
   out_7537065087244600898[83] = 0.0;
   out_7537065087244600898[84] = 0.0;
   out_7537065087244600898[85] = 0.0;
   out_7537065087244600898[86] = 0.0;
   out_7537065087244600898[87] = 0.0;
   out_7537065087244600898[88] = 0.0;
   out_7537065087244600898[89] = 0.0;
   out_7537065087244600898[90] = 0.0;
   out_7537065087244600898[91] = 0.0;
   out_7537065087244600898[92] = 0.0;
   out_7537065087244600898[93] = 0.0;
   out_7537065087244600898[94] = 0.0;
   out_7537065087244600898[95] = 1.0;
   out_7537065087244600898[96] = 0.0;
   out_7537065087244600898[97] = 0.0;
   out_7537065087244600898[98] = 0.0;
   out_7537065087244600898[99] = 0.0;
   out_7537065087244600898[100] = 0.0;
   out_7537065087244600898[101] = 0.0;
   out_7537065087244600898[102] = 0.0;
   out_7537065087244600898[103] = 0.0;
   out_7537065087244600898[104] = 0.0;
   out_7537065087244600898[105] = 0.0;
   out_7537065087244600898[106] = 0.0;
   out_7537065087244600898[107] = 0.0;
   out_7537065087244600898[108] = 0.0;
   out_7537065087244600898[109] = 0.0;
   out_7537065087244600898[110] = 0.0;
   out_7537065087244600898[111] = 0.0;
   out_7537065087244600898[112] = 0.0;
   out_7537065087244600898[113] = 0.0;
   out_7537065087244600898[114] = 1.0;
   out_7537065087244600898[115] = 0.0;
   out_7537065087244600898[116] = 0.0;
   out_7537065087244600898[117] = 0.0;
   out_7537065087244600898[118] = 0.0;
   out_7537065087244600898[119] = 0.0;
   out_7537065087244600898[120] = 0.0;
   out_7537065087244600898[121] = 0.0;
   out_7537065087244600898[122] = 0.0;
   out_7537065087244600898[123] = 0.0;
   out_7537065087244600898[124] = 0.0;
   out_7537065087244600898[125] = 0.0;
   out_7537065087244600898[126] = 0.0;
   out_7537065087244600898[127] = 0.0;
   out_7537065087244600898[128] = 0.0;
   out_7537065087244600898[129] = 0.0;
   out_7537065087244600898[130] = 0.0;
   out_7537065087244600898[131] = 0.0;
   out_7537065087244600898[132] = 0.0;
   out_7537065087244600898[133] = 1.0;
   out_7537065087244600898[134] = 0.0;
   out_7537065087244600898[135] = 0.0;
   out_7537065087244600898[136] = 0.0;
   out_7537065087244600898[137] = 0.0;
   out_7537065087244600898[138] = 0.0;
   out_7537065087244600898[139] = 0.0;
   out_7537065087244600898[140] = 0.0;
   out_7537065087244600898[141] = 0.0;
   out_7537065087244600898[142] = 0.0;
   out_7537065087244600898[143] = 0.0;
   out_7537065087244600898[144] = 0.0;
   out_7537065087244600898[145] = 0.0;
   out_7537065087244600898[146] = 0.0;
   out_7537065087244600898[147] = 0.0;
   out_7537065087244600898[148] = 0.0;
   out_7537065087244600898[149] = 0.0;
   out_7537065087244600898[150] = 0.0;
   out_7537065087244600898[151] = 0.0;
   out_7537065087244600898[152] = 1.0;
   out_7537065087244600898[153] = 0.0;
   out_7537065087244600898[154] = 0.0;
   out_7537065087244600898[155] = 0.0;
   out_7537065087244600898[156] = 0.0;
   out_7537065087244600898[157] = 0.0;
   out_7537065087244600898[158] = 0.0;
   out_7537065087244600898[159] = 0.0;
   out_7537065087244600898[160] = 0.0;
   out_7537065087244600898[161] = 0.0;
   out_7537065087244600898[162] = 0.0;
   out_7537065087244600898[163] = 0.0;
   out_7537065087244600898[164] = 0.0;
   out_7537065087244600898[165] = 0.0;
   out_7537065087244600898[166] = 0.0;
   out_7537065087244600898[167] = 0.0;
   out_7537065087244600898[168] = 0.0;
   out_7537065087244600898[169] = 0.0;
   out_7537065087244600898[170] = 0.0;
   out_7537065087244600898[171] = 1.0;
   out_7537065087244600898[172] = 0.0;
   out_7537065087244600898[173] = 0.0;
   out_7537065087244600898[174] = 0.0;
   out_7537065087244600898[175] = 0.0;
   out_7537065087244600898[176] = 0.0;
   out_7537065087244600898[177] = 0.0;
   out_7537065087244600898[178] = 0.0;
   out_7537065087244600898[179] = 0.0;
   out_7537065087244600898[180] = 0.0;
   out_7537065087244600898[181] = 0.0;
   out_7537065087244600898[182] = 0.0;
   out_7537065087244600898[183] = 0.0;
   out_7537065087244600898[184] = 0.0;
   out_7537065087244600898[185] = 0.0;
   out_7537065087244600898[186] = 0.0;
   out_7537065087244600898[187] = 0.0;
   out_7537065087244600898[188] = 0.0;
   out_7537065087244600898[189] = 0.0;
   out_7537065087244600898[190] = 1.0;
   out_7537065087244600898[191] = 0.0;
   out_7537065087244600898[192] = 0.0;
   out_7537065087244600898[193] = 0.0;
   out_7537065087244600898[194] = 0.0;
   out_7537065087244600898[195] = 0.0;
   out_7537065087244600898[196] = 0.0;
   out_7537065087244600898[197] = 0.0;
   out_7537065087244600898[198] = 0.0;
   out_7537065087244600898[199] = 0.0;
   out_7537065087244600898[200] = 0.0;
   out_7537065087244600898[201] = 0.0;
   out_7537065087244600898[202] = 0.0;
   out_7537065087244600898[203] = 0.0;
   out_7537065087244600898[204] = 0.0;
   out_7537065087244600898[205] = 0.0;
   out_7537065087244600898[206] = 0.0;
   out_7537065087244600898[207] = 0.0;
   out_7537065087244600898[208] = 0.0;
   out_7537065087244600898[209] = 1.0;
   out_7537065087244600898[210] = 0.0;
   out_7537065087244600898[211] = 0.0;
   out_7537065087244600898[212] = 0.0;
   out_7537065087244600898[213] = 0.0;
   out_7537065087244600898[214] = 0.0;
   out_7537065087244600898[215] = 0.0;
   out_7537065087244600898[216] = 0.0;
   out_7537065087244600898[217] = 0.0;
   out_7537065087244600898[218] = 0.0;
   out_7537065087244600898[219] = 0.0;
   out_7537065087244600898[220] = 0.0;
   out_7537065087244600898[221] = 0.0;
   out_7537065087244600898[222] = 0.0;
   out_7537065087244600898[223] = 0.0;
   out_7537065087244600898[224] = 0.0;
   out_7537065087244600898[225] = 0.0;
   out_7537065087244600898[226] = 0.0;
   out_7537065087244600898[227] = 0.0;
   out_7537065087244600898[228] = 1.0;
   out_7537065087244600898[229] = 0.0;
   out_7537065087244600898[230] = 0.0;
   out_7537065087244600898[231] = 0.0;
   out_7537065087244600898[232] = 0.0;
   out_7537065087244600898[233] = 0.0;
   out_7537065087244600898[234] = 0.0;
   out_7537065087244600898[235] = 0.0;
   out_7537065087244600898[236] = 0.0;
   out_7537065087244600898[237] = 0.0;
   out_7537065087244600898[238] = 0.0;
   out_7537065087244600898[239] = 0.0;
   out_7537065087244600898[240] = 0.0;
   out_7537065087244600898[241] = 0.0;
   out_7537065087244600898[242] = 0.0;
   out_7537065087244600898[243] = 0.0;
   out_7537065087244600898[244] = 0.0;
   out_7537065087244600898[245] = 0.0;
   out_7537065087244600898[246] = 0.0;
   out_7537065087244600898[247] = 1.0;
   out_7537065087244600898[248] = 0.0;
   out_7537065087244600898[249] = 0.0;
   out_7537065087244600898[250] = 0.0;
   out_7537065087244600898[251] = 0.0;
   out_7537065087244600898[252] = 0.0;
   out_7537065087244600898[253] = 0.0;
   out_7537065087244600898[254] = 0.0;
   out_7537065087244600898[255] = 0.0;
   out_7537065087244600898[256] = 0.0;
   out_7537065087244600898[257] = 0.0;
   out_7537065087244600898[258] = 0.0;
   out_7537065087244600898[259] = 0.0;
   out_7537065087244600898[260] = 0.0;
   out_7537065087244600898[261] = 0.0;
   out_7537065087244600898[262] = 0.0;
   out_7537065087244600898[263] = 0.0;
   out_7537065087244600898[264] = 0.0;
   out_7537065087244600898[265] = 0.0;
   out_7537065087244600898[266] = 1.0;
   out_7537065087244600898[267] = 0.0;
   out_7537065087244600898[268] = 0.0;
   out_7537065087244600898[269] = 0.0;
   out_7537065087244600898[270] = 0.0;
   out_7537065087244600898[271] = 0.0;
   out_7537065087244600898[272] = 0.0;
   out_7537065087244600898[273] = 0.0;
   out_7537065087244600898[274] = 0.0;
   out_7537065087244600898[275] = 0.0;
   out_7537065087244600898[276] = 0.0;
   out_7537065087244600898[277] = 0.0;
   out_7537065087244600898[278] = 0.0;
   out_7537065087244600898[279] = 0.0;
   out_7537065087244600898[280] = 0.0;
   out_7537065087244600898[281] = 0.0;
   out_7537065087244600898[282] = 0.0;
   out_7537065087244600898[283] = 0.0;
   out_7537065087244600898[284] = 0.0;
   out_7537065087244600898[285] = 1.0;
   out_7537065087244600898[286] = 0.0;
   out_7537065087244600898[287] = 0.0;
   out_7537065087244600898[288] = 0.0;
   out_7537065087244600898[289] = 0.0;
   out_7537065087244600898[290] = 0.0;
   out_7537065087244600898[291] = 0.0;
   out_7537065087244600898[292] = 0.0;
   out_7537065087244600898[293] = 0.0;
   out_7537065087244600898[294] = 0.0;
   out_7537065087244600898[295] = 0.0;
   out_7537065087244600898[296] = 0.0;
   out_7537065087244600898[297] = 0.0;
   out_7537065087244600898[298] = 0.0;
   out_7537065087244600898[299] = 0.0;
   out_7537065087244600898[300] = 0.0;
   out_7537065087244600898[301] = 0.0;
   out_7537065087244600898[302] = 0.0;
   out_7537065087244600898[303] = 0.0;
   out_7537065087244600898[304] = 1.0;
   out_7537065087244600898[305] = 0.0;
   out_7537065087244600898[306] = 0.0;
   out_7537065087244600898[307] = 0.0;
   out_7537065087244600898[308] = 0.0;
   out_7537065087244600898[309] = 0.0;
   out_7537065087244600898[310] = 0.0;
   out_7537065087244600898[311] = 0.0;
   out_7537065087244600898[312] = 0.0;
   out_7537065087244600898[313] = 0.0;
   out_7537065087244600898[314] = 0.0;
   out_7537065087244600898[315] = 0.0;
   out_7537065087244600898[316] = 0.0;
   out_7537065087244600898[317] = 0.0;
   out_7537065087244600898[318] = 0.0;
   out_7537065087244600898[319] = 0.0;
   out_7537065087244600898[320] = 0.0;
   out_7537065087244600898[321] = 0.0;
   out_7537065087244600898[322] = 0.0;
   out_7537065087244600898[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_6289537546199484080) {
   out_6289537546199484080[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_6289537546199484080[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_6289537546199484080[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_6289537546199484080[3] = dt*state[12] + state[3];
   out_6289537546199484080[4] = dt*state[13] + state[4];
   out_6289537546199484080[5] = dt*state[14] + state[5];
   out_6289537546199484080[6] = state[6];
   out_6289537546199484080[7] = state[7];
   out_6289537546199484080[8] = state[8];
   out_6289537546199484080[9] = state[9];
   out_6289537546199484080[10] = state[10];
   out_6289537546199484080[11] = state[11];
   out_6289537546199484080[12] = state[12];
   out_6289537546199484080[13] = state[13];
   out_6289537546199484080[14] = state[14];
   out_6289537546199484080[15] = state[15];
   out_6289537546199484080[16] = state[16];
   out_6289537546199484080[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3263235949829739422) {
   out_3263235949829739422[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3263235949829739422[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3263235949829739422[2] = 0;
   out_3263235949829739422[3] = 0;
   out_3263235949829739422[4] = 0;
   out_3263235949829739422[5] = 0;
   out_3263235949829739422[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3263235949829739422[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3263235949829739422[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3263235949829739422[9] = 0;
   out_3263235949829739422[10] = 0;
   out_3263235949829739422[11] = 0;
   out_3263235949829739422[12] = 0;
   out_3263235949829739422[13] = 0;
   out_3263235949829739422[14] = 0;
   out_3263235949829739422[15] = 0;
   out_3263235949829739422[16] = 0;
   out_3263235949829739422[17] = 0;
   out_3263235949829739422[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3263235949829739422[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3263235949829739422[20] = 0;
   out_3263235949829739422[21] = 0;
   out_3263235949829739422[22] = 0;
   out_3263235949829739422[23] = 0;
   out_3263235949829739422[24] = 0;
   out_3263235949829739422[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3263235949829739422[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3263235949829739422[27] = 0;
   out_3263235949829739422[28] = 0;
   out_3263235949829739422[29] = 0;
   out_3263235949829739422[30] = 0;
   out_3263235949829739422[31] = 0;
   out_3263235949829739422[32] = 0;
   out_3263235949829739422[33] = 0;
   out_3263235949829739422[34] = 0;
   out_3263235949829739422[35] = 0;
   out_3263235949829739422[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3263235949829739422[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3263235949829739422[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3263235949829739422[39] = 0;
   out_3263235949829739422[40] = 0;
   out_3263235949829739422[41] = 0;
   out_3263235949829739422[42] = 0;
   out_3263235949829739422[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3263235949829739422[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3263235949829739422[45] = 0;
   out_3263235949829739422[46] = 0;
   out_3263235949829739422[47] = 0;
   out_3263235949829739422[48] = 0;
   out_3263235949829739422[49] = 0;
   out_3263235949829739422[50] = 0;
   out_3263235949829739422[51] = 0;
   out_3263235949829739422[52] = 0;
   out_3263235949829739422[53] = 0;
   out_3263235949829739422[54] = 0;
   out_3263235949829739422[55] = 0;
   out_3263235949829739422[56] = 0;
   out_3263235949829739422[57] = 1;
   out_3263235949829739422[58] = 0;
   out_3263235949829739422[59] = 0;
   out_3263235949829739422[60] = 0;
   out_3263235949829739422[61] = 0;
   out_3263235949829739422[62] = 0;
   out_3263235949829739422[63] = 0;
   out_3263235949829739422[64] = 0;
   out_3263235949829739422[65] = 0;
   out_3263235949829739422[66] = dt;
   out_3263235949829739422[67] = 0;
   out_3263235949829739422[68] = 0;
   out_3263235949829739422[69] = 0;
   out_3263235949829739422[70] = 0;
   out_3263235949829739422[71] = 0;
   out_3263235949829739422[72] = 0;
   out_3263235949829739422[73] = 0;
   out_3263235949829739422[74] = 0;
   out_3263235949829739422[75] = 0;
   out_3263235949829739422[76] = 1;
   out_3263235949829739422[77] = 0;
   out_3263235949829739422[78] = 0;
   out_3263235949829739422[79] = 0;
   out_3263235949829739422[80] = 0;
   out_3263235949829739422[81] = 0;
   out_3263235949829739422[82] = 0;
   out_3263235949829739422[83] = 0;
   out_3263235949829739422[84] = 0;
   out_3263235949829739422[85] = dt;
   out_3263235949829739422[86] = 0;
   out_3263235949829739422[87] = 0;
   out_3263235949829739422[88] = 0;
   out_3263235949829739422[89] = 0;
   out_3263235949829739422[90] = 0;
   out_3263235949829739422[91] = 0;
   out_3263235949829739422[92] = 0;
   out_3263235949829739422[93] = 0;
   out_3263235949829739422[94] = 0;
   out_3263235949829739422[95] = 1;
   out_3263235949829739422[96] = 0;
   out_3263235949829739422[97] = 0;
   out_3263235949829739422[98] = 0;
   out_3263235949829739422[99] = 0;
   out_3263235949829739422[100] = 0;
   out_3263235949829739422[101] = 0;
   out_3263235949829739422[102] = 0;
   out_3263235949829739422[103] = 0;
   out_3263235949829739422[104] = dt;
   out_3263235949829739422[105] = 0;
   out_3263235949829739422[106] = 0;
   out_3263235949829739422[107] = 0;
   out_3263235949829739422[108] = 0;
   out_3263235949829739422[109] = 0;
   out_3263235949829739422[110] = 0;
   out_3263235949829739422[111] = 0;
   out_3263235949829739422[112] = 0;
   out_3263235949829739422[113] = 0;
   out_3263235949829739422[114] = 1;
   out_3263235949829739422[115] = 0;
   out_3263235949829739422[116] = 0;
   out_3263235949829739422[117] = 0;
   out_3263235949829739422[118] = 0;
   out_3263235949829739422[119] = 0;
   out_3263235949829739422[120] = 0;
   out_3263235949829739422[121] = 0;
   out_3263235949829739422[122] = 0;
   out_3263235949829739422[123] = 0;
   out_3263235949829739422[124] = 0;
   out_3263235949829739422[125] = 0;
   out_3263235949829739422[126] = 0;
   out_3263235949829739422[127] = 0;
   out_3263235949829739422[128] = 0;
   out_3263235949829739422[129] = 0;
   out_3263235949829739422[130] = 0;
   out_3263235949829739422[131] = 0;
   out_3263235949829739422[132] = 0;
   out_3263235949829739422[133] = 1;
   out_3263235949829739422[134] = 0;
   out_3263235949829739422[135] = 0;
   out_3263235949829739422[136] = 0;
   out_3263235949829739422[137] = 0;
   out_3263235949829739422[138] = 0;
   out_3263235949829739422[139] = 0;
   out_3263235949829739422[140] = 0;
   out_3263235949829739422[141] = 0;
   out_3263235949829739422[142] = 0;
   out_3263235949829739422[143] = 0;
   out_3263235949829739422[144] = 0;
   out_3263235949829739422[145] = 0;
   out_3263235949829739422[146] = 0;
   out_3263235949829739422[147] = 0;
   out_3263235949829739422[148] = 0;
   out_3263235949829739422[149] = 0;
   out_3263235949829739422[150] = 0;
   out_3263235949829739422[151] = 0;
   out_3263235949829739422[152] = 1;
   out_3263235949829739422[153] = 0;
   out_3263235949829739422[154] = 0;
   out_3263235949829739422[155] = 0;
   out_3263235949829739422[156] = 0;
   out_3263235949829739422[157] = 0;
   out_3263235949829739422[158] = 0;
   out_3263235949829739422[159] = 0;
   out_3263235949829739422[160] = 0;
   out_3263235949829739422[161] = 0;
   out_3263235949829739422[162] = 0;
   out_3263235949829739422[163] = 0;
   out_3263235949829739422[164] = 0;
   out_3263235949829739422[165] = 0;
   out_3263235949829739422[166] = 0;
   out_3263235949829739422[167] = 0;
   out_3263235949829739422[168] = 0;
   out_3263235949829739422[169] = 0;
   out_3263235949829739422[170] = 0;
   out_3263235949829739422[171] = 1;
   out_3263235949829739422[172] = 0;
   out_3263235949829739422[173] = 0;
   out_3263235949829739422[174] = 0;
   out_3263235949829739422[175] = 0;
   out_3263235949829739422[176] = 0;
   out_3263235949829739422[177] = 0;
   out_3263235949829739422[178] = 0;
   out_3263235949829739422[179] = 0;
   out_3263235949829739422[180] = 0;
   out_3263235949829739422[181] = 0;
   out_3263235949829739422[182] = 0;
   out_3263235949829739422[183] = 0;
   out_3263235949829739422[184] = 0;
   out_3263235949829739422[185] = 0;
   out_3263235949829739422[186] = 0;
   out_3263235949829739422[187] = 0;
   out_3263235949829739422[188] = 0;
   out_3263235949829739422[189] = 0;
   out_3263235949829739422[190] = 1;
   out_3263235949829739422[191] = 0;
   out_3263235949829739422[192] = 0;
   out_3263235949829739422[193] = 0;
   out_3263235949829739422[194] = 0;
   out_3263235949829739422[195] = 0;
   out_3263235949829739422[196] = 0;
   out_3263235949829739422[197] = 0;
   out_3263235949829739422[198] = 0;
   out_3263235949829739422[199] = 0;
   out_3263235949829739422[200] = 0;
   out_3263235949829739422[201] = 0;
   out_3263235949829739422[202] = 0;
   out_3263235949829739422[203] = 0;
   out_3263235949829739422[204] = 0;
   out_3263235949829739422[205] = 0;
   out_3263235949829739422[206] = 0;
   out_3263235949829739422[207] = 0;
   out_3263235949829739422[208] = 0;
   out_3263235949829739422[209] = 1;
   out_3263235949829739422[210] = 0;
   out_3263235949829739422[211] = 0;
   out_3263235949829739422[212] = 0;
   out_3263235949829739422[213] = 0;
   out_3263235949829739422[214] = 0;
   out_3263235949829739422[215] = 0;
   out_3263235949829739422[216] = 0;
   out_3263235949829739422[217] = 0;
   out_3263235949829739422[218] = 0;
   out_3263235949829739422[219] = 0;
   out_3263235949829739422[220] = 0;
   out_3263235949829739422[221] = 0;
   out_3263235949829739422[222] = 0;
   out_3263235949829739422[223] = 0;
   out_3263235949829739422[224] = 0;
   out_3263235949829739422[225] = 0;
   out_3263235949829739422[226] = 0;
   out_3263235949829739422[227] = 0;
   out_3263235949829739422[228] = 1;
   out_3263235949829739422[229] = 0;
   out_3263235949829739422[230] = 0;
   out_3263235949829739422[231] = 0;
   out_3263235949829739422[232] = 0;
   out_3263235949829739422[233] = 0;
   out_3263235949829739422[234] = 0;
   out_3263235949829739422[235] = 0;
   out_3263235949829739422[236] = 0;
   out_3263235949829739422[237] = 0;
   out_3263235949829739422[238] = 0;
   out_3263235949829739422[239] = 0;
   out_3263235949829739422[240] = 0;
   out_3263235949829739422[241] = 0;
   out_3263235949829739422[242] = 0;
   out_3263235949829739422[243] = 0;
   out_3263235949829739422[244] = 0;
   out_3263235949829739422[245] = 0;
   out_3263235949829739422[246] = 0;
   out_3263235949829739422[247] = 1;
   out_3263235949829739422[248] = 0;
   out_3263235949829739422[249] = 0;
   out_3263235949829739422[250] = 0;
   out_3263235949829739422[251] = 0;
   out_3263235949829739422[252] = 0;
   out_3263235949829739422[253] = 0;
   out_3263235949829739422[254] = 0;
   out_3263235949829739422[255] = 0;
   out_3263235949829739422[256] = 0;
   out_3263235949829739422[257] = 0;
   out_3263235949829739422[258] = 0;
   out_3263235949829739422[259] = 0;
   out_3263235949829739422[260] = 0;
   out_3263235949829739422[261] = 0;
   out_3263235949829739422[262] = 0;
   out_3263235949829739422[263] = 0;
   out_3263235949829739422[264] = 0;
   out_3263235949829739422[265] = 0;
   out_3263235949829739422[266] = 1;
   out_3263235949829739422[267] = 0;
   out_3263235949829739422[268] = 0;
   out_3263235949829739422[269] = 0;
   out_3263235949829739422[270] = 0;
   out_3263235949829739422[271] = 0;
   out_3263235949829739422[272] = 0;
   out_3263235949829739422[273] = 0;
   out_3263235949829739422[274] = 0;
   out_3263235949829739422[275] = 0;
   out_3263235949829739422[276] = 0;
   out_3263235949829739422[277] = 0;
   out_3263235949829739422[278] = 0;
   out_3263235949829739422[279] = 0;
   out_3263235949829739422[280] = 0;
   out_3263235949829739422[281] = 0;
   out_3263235949829739422[282] = 0;
   out_3263235949829739422[283] = 0;
   out_3263235949829739422[284] = 0;
   out_3263235949829739422[285] = 1;
   out_3263235949829739422[286] = 0;
   out_3263235949829739422[287] = 0;
   out_3263235949829739422[288] = 0;
   out_3263235949829739422[289] = 0;
   out_3263235949829739422[290] = 0;
   out_3263235949829739422[291] = 0;
   out_3263235949829739422[292] = 0;
   out_3263235949829739422[293] = 0;
   out_3263235949829739422[294] = 0;
   out_3263235949829739422[295] = 0;
   out_3263235949829739422[296] = 0;
   out_3263235949829739422[297] = 0;
   out_3263235949829739422[298] = 0;
   out_3263235949829739422[299] = 0;
   out_3263235949829739422[300] = 0;
   out_3263235949829739422[301] = 0;
   out_3263235949829739422[302] = 0;
   out_3263235949829739422[303] = 0;
   out_3263235949829739422[304] = 1;
   out_3263235949829739422[305] = 0;
   out_3263235949829739422[306] = 0;
   out_3263235949829739422[307] = 0;
   out_3263235949829739422[308] = 0;
   out_3263235949829739422[309] = 0;
   out_3263235949829739422[310] = 0;
   out_3263235949829739422[311] = 0;
   out_3263235949829739422[312] = 0;
   out_3263235949829739422[313] = 0;
   out_3263235949829739422[314] = 0;
   out_3263235949829739422[315] = 0;
   out_3263235949829739422[316] = 0;
   out_3263235949829739422[317] = 0;
   out_3263235949829739422[318] = 0;
   out_3263235949829739422[319] = 0;
   out_3263235949829739422[320] = 0;
   out_3263235949829739422[321] = 0;
   out_3263235949829739422[322] = 0;
   out_3263235949829739422[323] = 1;
}
void h_4(double *state, double *unused, double *out_6871661940623032085) {
   out_6871661940623032085[0] = state[6] + state[9];
   out_6871661940623032085[1] = state[7] + state[10];
   out_6871661940623032085[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_5061597179712991997) {
   out_5061597179712991997[0] = 0;
   out_5061597179712991997[1] = 0;
   out_5061597179712991997[2] = 0;
   out_5061597179712991997[3] = 0;
   out_5061597179712991997[4] = 0;
   out_5061597179712991997[5] = 0;
   out_5061597179712991997[6] = 1;
   out_5061597179712991997[7] = 0;
   out_5061597179712991997[8] = 0;
   out_5061597179712991997[9] = 1;
   out_5061597179712991997[10] = 0;
   out_5061597179712991997[11] = 0;
   out_5061597179712991997[12] = 0;
   out_5061597179712991997[13] = 0;
   out_5061597179712991997[14] = 0;
   out_5061597179712991997[15] = 0;
   out_5061597179712991997[16] = 0;
   out_5061597179712991997[17] = 0;
   out_5061597179712991997[18] = 0;
   out_5061597179712991997[19] = 0;
   out_5061597179712991997[20] = 0;
   out_5061597179712991997[21] = 0;
   out_5061597179712991997[22] = 0;
   out_5061597179712991997[23] = 0;
   out_5061597179712991997[24] = 0;
   out_5061597179712991997[25] = 1;
   out_5061597179712991997[26] = 0;
   out_5061597179712991997[27] = 0;
   out_5061597179712991997[28] = 1;
   out_5061597179712991997[29] = 0;
   out_5061597179712991997[30] = 0;
   out_5061597179712991997[31] = 0;
   out_5061597179712991997[32] = 0;
   out_5061597179712991997[33] = 0;
   out_5061597179712991997[34] = 0;
   out_5061597179712991997[35] = 0;
   out_5061597179712991997[36] = 0;
   out_5061597179712991997[37] = 0;
   out_5061597179712991997[38] = 0;
   out_5061597179712991997[39] = 0;
   out_5061597179712991997[40] = 0;
   out_5061597179712991997[41] = 0;
   out_5061597179712991997[42] = 0;
   out_5061597179712991997[43] = 0;
   out_5061597179712991997[44] = 1;
   out_5061597179712991997[45] = 0;
   out_5061597179712991997[46] = 0;
   out_5061597179712991997[47] = 1;
   out_5061597179712991997[48] = 0;
   out_5061597179712991997[49] = 0;
   out_5061597179712991997[50] = 0;
   out_5061597179712991997[51] = 0;
   out_5061597179712991997[52] = 0;
   out_5061597179712991997[53] = 0;
}
void h_10(double *state, double *unused, double *out_974669649895256171) {
   out_974669649895256171[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_974669649895256171[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_974669649895256171[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_3862977066975629010) {
   out_3862977066975629010[0] = 0;
   out_3862977066975629010[1] = 9.8100000000000005*cos(state[1]);
   out_3862977066975629010[2] = 0;
   out_3862977066975629010[3] = 0;
   out_3862977066975629010[4] = -state[8];
   out_3862977066975629010[5] = state[7];
   out_3862977066975629010[6] = 0;
   out_3862977066975629010[7] = state[5];
   out_3862977066975629010[8] = -state[4];
   out_3862977066975629010[9] = 0;
   out_3862977066975629010[10] = 0;
   out_3862977066975629010[11] = 0;
   out_3862977066975629010[12] = 1;
   out_3862977066975629010[13] = 0;
   out_3862977066975629010[14] = 0;
   out_3862977066975629010[15] = 1;
   out_3862977066975629010[16] = 0;
   out_3862977066975629010[17] = 0;
   out_3862977066975629010[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_3862977066975629010[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_3862977066975629010[20] = 0;
   out_3862977066975629010[21] = state[8];
   out_3862977066975629010[22] = 0;
   out_3862977066975629010[23] = -state[6];
   out_3862977066975629010[24] = -state[5];
   out_3862977066975629010[25] = 0;
   out_3862977066975629010[26] = state[3];
   out_3862977066975629010[27] = 0;
   out_3862977066975629010[28] = 0;
   out_3862977066975629010[29] = 0;
   out_3862977066975629010[30] = 0;
   out_3862977066975629010[31] = 1;
   out_3862977066975629010[32] = 0;
   out_3862977066975629010[33] = 0;
   out_3862977066975629010[34] = 1;
   out_3862977066975629010[35] = 0;
   out_3862977066975629010[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_3862977066975629010[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_3862977066975629010[38] = 0;
   out_3862977066975629010[39] = -state[7];
   out_3862977066975629010[40] = state[6];
   out_3862977066975629010[41] = 0;
   out_3862977066975629010[42] = state[4];
   out_3862977066975629010[43] = -state[3];
   out_3862977066975629010[44] = 0;
   out_3862977066975629010[45] = 0;
   out_3862977066975629010[46] = 0;
   out_3862977066975629010[47] = 0;
   out_3862977066975629010[48] = 0;
   out_3862977066975629010[49] = 0;
   out_3862977066975629010[50] = 1;
   out_3862977066975629010[51] = 0;
   out_3862977066975629010[52] = 0;
   out_3862977066975629010[53] = 1;
}
void h_13(double *state, double *unused, double *out_2719357598944466940) {
   out_2719357598944466940[0] = state[3];
   out_2719357598944466940[1] = state[4];
   out_2719357598944466940[2] = state[5];
}
void H_13(double *state, double *unused, double *out_8273871005045324798) {
   out_8273871005045324798[0] = 0;
   out_8273871005045324798[1] = 0;
   out_8273871005045324798[2] = 0;
   out_8273871005045324798[3] = 1;
   out_8273871005045324798[4] = 0;
   out_8273871005045324798[5] = 0;
   out_8273871005045324798[6] = 0;
   out_8273871005045324798[7] = 0;
   out_8273871005045324798[8] = 0;
   out_8273871005045324798[9] = 0;
   out_8273871005045324798[10] = 0;
   out_8273871005045324798[11] = 0;
   out_8273871005045324798[12] = 0;
   out_8273871005045324798[13] = 0;
   out_8273871005045324798[14] = 0;
   out_8273871005045324798[15] = 0;
   out_8273871005045324798[16] = 0;
   out_8273871005045324798[17] = 0;
   out_8273871005045324798[18] = 0;
   out_8273871005045324798[19] = 0;
   out_8273871005045324798[20] = 0;
   out_8273871005045324798[21] = 0;
   out_8273871005045324798[22] = 1;
   out_8273871005045324798[23] = 0;
   out_8273871005045324798[24] = 0;
   out_8273871005045324798[25] = 0;
   out_8273871005045324798[26] = 0;
   out_8273871005045324798[27] = 0;
   out_8273871005045324798[28] = 0;
   out_8273871005045324798[29] = 0;
   out_8273871005045324798[30] = 0;
   out_8273871005045324798[31] = 0;
   out_8273871005045324798[32] = 0;
   out_8273871005045324798[33] = 0;
   out_8273871005045324798[34] = 0;
   out_8273871005045324798[35] = 0;
   out_8273871005045324798[36] = 0;
   out_8273871005045324798[37] = 0;
   out_8273871005045324798[38] = 0;
   out_8273871005045324798[39] = 0;
   out_8273871005045324798[40] = 0;
   out_8273871005045324798[41] = 1;
   out_8273871005045324798[42] = 0;
   out_8273871005045324798[43] = 0;
   out_8273871005045324798[44] = 0;
   out_8273871005045324798[45] = 0;
   out_8273871005045324798[46] = 0;
   out_8273871005045324798[47] = 0;
   out_8273871005045324798[48] = 0;
   out_8273871005045324798[49] = 0;
   out_8273871005045324798[50] = 0;
   out_8273871005045324798[51] = 0;
   out_8273871005045324798[52] = 0;
   out_8273871005045324798[53] = 0;
}
void h_14(double *state, double *unused, double *out_5270922229605318979) {
   out_5270922229605318979[0] = state[6];
   out_5270922229605318979[1] = state[7];
   out_5270922229605318979[2] = state[8];
}
void H_14(double *state, double *unused, double *out_9024838036052476526) {
   out_9024838036052476526[0] = 0;
   out_9024838036052476526[1] = 0;
   out_9024838036052476526[2] = 0;
   out_9024838036052476526[3] = 0;
   out_9024838036052476526[4] = 0;
   out_9024838036052476526[5] = 0;
   out_9024838036052476526[6] = 1;
   out_9024838036052476526[7] = 0;
   out_9024838036052476526[8] = 0;
   out_9024838036052476526[9] = 0;
   out_9024838036052476526[10] = 0;
   out_9024838036052476526[11] = 0;
   out_9024838036052476526[12] = 0;
   out_9024838036052476526[13] = 0;
   out_9024838036052476526[14] = 0;
   out_9024838036052476526[15] = 0;
   out_9024838036052476526[16] = 0;
   out_9024838036052476526[17] = 0;
   out_9024838036052476526[18] = 0;
   out_9024838036052476526[19] = 0;
   out_9024838036052476526[20] = 0;
   out_9024838036052476526[21] = 0;
   out_9024838036052476526[22] = 0;
   out_9024838036052476526[23] = 0;
   out_9024838036052476526[24] = 0;
   out_9024838036052476526[25] = 1;
   out_9024838036052476526[26] = 0;
   out_9024838036052476526[27] = 0;
   out_9024838036052476526[28] = 0;
   out_9024838036052476526[29] = 0;
   out_9024838036052476526[30] = 0;
   out_9024838036052476526[31] = 0;
   out_9024838036052476526[32] = 0;
   out_9024838036052476526[33] = 0;
   out_9024838036052476526[34] = 0;
   out_9024838036052476526[35] = 0;
   out_9024838036052476526[36] = 0;
   out_9024838036052476526[37] = 0;
   out_9024838036052476526[38] = 0;
   out_9024838036052476526[39] = 0;
   out_9024838036052476526[40] = 0;
   out_9024838036052476526[41] = 0;
   out_9024838036052476526[42] = 0;
   out_9024838036052476526[43] = 0;
   out_9024838036052476526[44] = 1;
   out_9024838036052476526[45] = 0;
   out_9024838036052476526[46] = 0;
   out_9024838036052476526[47] = 0;
   out_9024838036052476526[48] = 0;
   out_9024838036052476526[49] = 0;
   out_9024838036052476526[50] = 0;
   out_9024838036052476526[51] = 0;
   out_9024838036052476526[52] = 0;
   out_9024838036052476526[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_8612662017984698055) {
  err_fun(nom_x, delta_x, out_8612662017984698055);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6583538545281744086) {
  inv_err_fun(nom_x, true_x, out_6583538545281744086);
}
void pose_H_mod_fun(double *state, double *out_7537065087244600898) {
  H_mod_fun(state, out_7537065087244600898);
}
void pose_f_fun(double *state, double dt, double *out_6289537546199484080) {
  f_fun(state,  dt, out_6289537546199484080);
}
void pose_F_fun(double *state, double dt, double *out_3263235949829739422) {
  F_fun(state,  dt, out_3263235949829739422);
}
void pose_h_4(double *state, double *unused, double *out_6871661940623032085) {
  h_4(state, unused, out_6871661940623032085);
}
void pose_H_4(double *state, double *unused, double *out_5061597179712991997) {
  H_4(state, unused, out_5061597179712991997);
}
void pose_h_10(double *state, double *unused, double *out_974669649895256171) {
  h_10(state, unused, out_974669649895256171);
}
void pose_H_10(double *state, double *unused, double *out_3862977066975629010) {
  H_10(state, unused, out_3862977066975629010);
}
void pose_h_13(double *state, double *unused, double *out_2719357598944466940) {
  h_13(state, unused, out_2719357598944466940);
}
void pose_H_13(double *state, double *unused, double *out_8273871005045324798) {
  H_13(state, unused, out_8273871005045324798);
}
void pose_h_14(double *state, double *unused, double *out_5270922229605318979) {
  h_14(state, unused, out_5270922229605318979);
}
void pose_H_14(double *state, double *unused, double *out_9024838036052476526) {
  H_14(state, unused, out_9024838036052476526);
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

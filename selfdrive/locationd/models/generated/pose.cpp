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
void err_fun(double *nom_x, double *delta_x, double *out_4055321504301233767) {
   out_4055321504301233767[0] = delta_x[0] + nom_x[0];
   out_4055321504301233767[1] = delta_x[1] + nom_x[1];
   out_4055321504301233767[2] = delta_x[2] + nom_x[2];
   out_4055321504301233767[3] = delta_x[3] + nom_x[3];
   out_4055321504301233767[4] = delta_x[4] + nom_x[4];
   out_4055321504301233767[5] = delta_x[5] + nom_x[5];
   out_4055321504301233767[6] = delta_x[6] + nom_x[6];
   out_4055321504301233767[7] = delta_x[7] + nom_x[7];
   out_4055321504301233767[8] = delta_x[8] + nom_x[8];
   out_4055321504301233767[9] = delta_x[9] + nom_x[9];
   out_4055321504301233767[10] = delta_x[10] + nom_x[10];
   out_4055321504301233767[11] = delta_x[11] + nom_x[11];
   out_4055321504301233767[12] = delta_x[12] + nom_x[12];
   out_4055321504301233767[13] = delta_x[13] + nom_x[13];
   out_4055321504301233767[14] = delta_x[14] + nom_x[14];
   out_4055321504301233767[15] = delta_x[15] + nom_x[15];
   out_4055321504301233767[16] = delta_x[16] + nom_x[16];
   out_4055321504301233767[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_5242087564548968290) {
   out_5242087564548968290[0] = -nom_x[0] + true_x[0];
   out_5242087564548968290[1] = -nom_x[1] + true_x[1];
   out_5242087564548968290[2] = -nom_x[2] + true_x[2];
   out_5242087564548968290[3] = -nom_x[3] + true_x[3];
   out_5242087564548968290[4] = -nom_x[4] + true_x[4];
   out_5242087564548968290[5] = -nom_x[5] + true_x[5];
   out_5242087564548968290[6] = -nom_x[6] + true_x[6];
   out_5242087564548968290[7] = -nom_x[7] + true_x[7];
   out_5242087564548968290[8] = -nom_x[8] + true_x[8];
   out_5242087564548968290[9] = -nom_x[9] + true_x[9];
   out_5242087564548968290[10] = -nom_x[10] + true_x[10];
   out_5242087564548968290[11] = -nom_x[11] + true_x[11];
   out_5242087564548968290[12] = -nom_x[12] + true_x[12];
   out_5242087564548968290[13] = -nom_x[13] + true_x[13];
   out_5242087564548968290[14] = -nom_x[14] + true_x[14];
   out_5242087564548968290[15] = -nom_x[15] + true_x[15];
   out_5242087564548968290[16] = -nom_x[16] + true_x[16];
   out_5242087564548968290[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_6187186838300588967) {
   out_6187186838300588967[0] = 1.0;
   out_6187186838300588967[1] = 0.0;
   out_6187186838300588967[2] = 0.0;
   out_6187186838300588967[3] = 0.0;
   out_6187186838300588967[4] = 0.0;
   out_6187186838300588967[5] = 0.0;
   out_6187186838300588967[6] = 0.0;
   out_6187186838300588967[7] = 0.0;
   out_6187186838300588967[8] = 0.0;
   out_6187186838300588967[9] = 0.0;
   out_6187186838300588967[10] = 0.0;
   out_6187186838300588967[11] = 0.0;
   out_6187186838300588967[12] = 0.0;
   out_6187186838300588967[13] = 0.0;
   out_6187186838300588967[14] = 0.0;
   out_6187186838300588967[15] = 0.0;
   out_6187186838300588967[16] = 0.0;
   out_6187186838300588967[17] = 0.0;
   out_6187186838300588967[18] = 0.0;
   out_6187186838300588967[19] = 1.0;
   out_6187186838300588967[20] = 0.0;
   out_6187186838300588967[21] = 0.0;
   out_6187186838300588967[22] = 0.0;
   out_6187186838300588967[23] = 0.0;
   out_6187186838300588967[24] = 0.0;
   out_6187186838300588967[25] = 0.0;
   out_6187186838300588967[26] = 0.0;
   out_6187186838300588967[27] = 0.0;
   out_6187186838300588967[28] = 0.0;
   out_6187186838300588967[29] = 0.0;
   out_6187186838300588967[30] = 0.0;
   out_6187186838300588967[31] = 0.0;
   out_6187186838300588967[32] = 0.0;
   out_6187186838300588967[33] = 0.0;
   out_6187186838300588967[34] = 0.0;
   out_6187186838300588967[35] = 0.0;
   out_6187186838300588967[36] = 0.0;
   out_6187186838300588967[37] = 0.0;
   out_6187186838300588967[38] = 1.0;
   out_6187186838300588967[39] = 0.0;
   out_6187186838300588967[40] = 0.0;
   out_6187186838300588967[41] = 0.0;
   out_6187186838300588967[42] = 0.0;
   out_6187186838300588967[43] = 0.0;
   out_6187186838300588967[44] = 0.0;
   out_6187186838300588967[45] = 0.0;
   out_6187186838300588967[46] = 0.0;
   out_6187186838300588967[47] = 0.0;
   out_6187186838300588967[48] = 0.0;
   out_6187186838300588967[49] = 0.0;
   out_6187186838300588967[50] = 0.0;
   out_6187186838300588967[51] = 0.0;
   out_6187186838300588967[52] = 0.0;
   out_6187186838300588967[53] = 0.0;
   out_6187186838300588967[54] = 0.0;
   out_6187186838300588967[55] = 0.0;
   out_6187186838300588967[56] = 0.0;
   out_6187186838300588967[57] = 1.0;
   out_6187186838300588967[58] = 0.0;
   out_6187186838300588967[59] = 0.0;
   out_6187186838300588967[60] = 0.0;
   out_6187186838300588967[61] = 0.0;
   out_6187186838300588967[62] = 0.0;
   out_6187186838300588967[63] = 0.0;
   out_6187186838300588967[64] = 0.0;
   out_6187186838300588967[65] = 0.0;
   out_6187186838300588967[66] = 0.0;
   out_6187186838300588967[67] = 0.0;
   out_6187186838300588967[68] = 0.0;
   out_6187186838300588967[69] = 0.0;
   out_6187186838300588967[70] = 0.0;
   out_6187186838300588967[71] = 0.0;
   out_6187186838300588967[72] = 0.0;
   out_6187186838300588967[73] = 0.0;
   out_6187186838300588967[74] = 0.0;
   out_6187186838300588967[75] = 0.0;
   out_6187186838300588967[76] = 1.0;
   out_6187186838300588967[77] = 0.0;
   out_6187186838300588967[78] = 0.0;
   out_6187186838300588967[79] = 0.0;
   out_6187186838300588967[80] = 0.0;
   out_6187186838300588967[81] = 0.0;
   out_6187186838300588967[82] = 0.0;
   out_6187186838300588967[83] = 0.0;
   out_6187186838300588967[84] = 0.0;
   out_6187186838300588967[85] = 0.0;
   out_6187186838300588967[86] = 0.0;
   out_6187186838300588967[87] = 0.0;
   out_6187186838300588967[88] = 0.0;
   out_6187186838300588967[89] = 0.0;
   out_6187186838300588967[90] = 0.0;
   out_6187186838300588967[91] = 0.0;
   out_6187186838300588967[92] = 0.0;
   out_6187186838300588967[93] = 0.0;
   out_6187186838300588967[94] = 0.0;
   out_6187186838300588967[95] = 1.0;
   out_6187186838300588967[96] = 0.0;
   out_6187186838300588967[97] = 0.0;
   out_6187186838300588967[98] = 0.0;
   out_6187186838300588967[99] = 0.0;
   out_6187186838300588967[100] = 0.0;
   out_6187186838300588967[101] = 0.0;
   out_6187186838300588967[102] = 0.0;
   out_6187186838300588967[103] = 0.0;
   out_6187186838300588967[104] = 0.0;
   out_6187186838300588967[105] = 0.0;
   out_6187186838300588967[106] = 0.0;
   out_6187186838300588967[107] = 0.0;
   out_6187186838300588967[108] = 0.0;
   out_6187186838300588967[109] = 0.0;
   out_6187186838300588967[110] = 0.0;
   out_6187186838300588967[111] = 0.0;
   out_6187186838300588967[112] = 0.0;
   out_6187186838300588967[113] = 0.0;
   out_6187186838300588967[114] = 1.0;
   out_6187186838300588967[115] = 0.0;
   out_6187186838300588967[116] = 0.0;
   out_6187186838300588967[117] = 0.0;
   out_6187186838300588967[118] = 0.0;
   out_6187186838300588967[119] = 0.0;
   out_6187186838300588967[120] = 0.0;
   out_6187186838300588967[121] = 0.0;
   out_6187186838300588967[122] = 0.0;
   out_6187186838300588967[123] = 0.0;
   out_6187186838300588967[124] = 0.0;
   out_6187186838300588967[125] = 0.0;
   out_6187186838300588967[126] = 0.0;
   out_6187186838300588967[127] = 0.0;
   out_6187186838300588967[128] = 0.0;
   out_6187186838300588967[129] = 0.0;
   out_6187186838300588967[130] = 0.0;
   out_6187186838300588967[131] = 0.0;
   out_6187186838300588967[132] = 0.0;
   out_6187186838300588967[133] = 1.0;
   out_6187186838300588967[134] = 0.0;
   out_6187186838300588967[135] = 0.0;
   out_6187186838300588967[136] = 0.0;
   out_6187186838300588967[137] = 0.0;
   out_6187186838300588967[138] = 0.0;
   out_6187186838300588967[139] = 0.0;
   out_6187186838300588967[140] = 0.0;
   out_6187186838300588967[141] = 0.0;
   out_6187186838300588967[142] = 0.0;
   out_6187186838300588967[143] = 0.0;
   out_6187186838300588967[144] = 0.0;
   out_6187186838300588967[145] = 0.0;
   out_6187186838300588967[146] = 0.0;
   out_6187186838300588967[147] = 0.0;
   out_6187186838300588967[148] = 0.0;
   out_6187186838300588967[149] = 0.0;
   out_6187186838300588967[150] = 0.0;
   out_6187186838300588967[151] = 0.0;
   out_6187186838300588967[152] = 1.0;
   out_6187186838300588967[153] = 0.0;
   out_6187186838300588967[154] = 0.0;
   out_6187186838300588967[155] = 0.0;
   out_6187186838300588967[156] = 0.0;
   out_6187186838300588967[157] = 0.0;
   out_6187186838300588967[158] = 0.0;
   out_6187186838300588967[159] = 0.0;
   out_6187186838300588967[160] = 0.0;
   out_6187186838300588967[161] = 0.0;
   out_6187186838300588967[162] = 0.0;
   out_6187186838300588967[163] = 0.0;
   out_6187186838300588967[164] = 0.0;
   out_6187186838300588967[165] = 0.0;
   out_6187186838300588967[166] = 0.0;
   out_6187186838300588967[167] = 0.0;
   out_6187186838300588967[168] = 0.0;
   out_6187186838300588967[169] = 0.0;
   out_6187186838300588967[170] = 0.0;
   out_6187186838300588967[171] = 1.0;
   out_6187186838300588967[172] = 0.0;
   out_6187186838300588967[173] = 0.0;
   out_6187186838300588967[174] = 0.0;
   out_6187186838300588967[175] = 0.0;
   out_6187186838300588967[176] = 0.0;
   out_6187186838300588967[177] = 0.0;
   out_6187186838300588967[178] = 0.0;
   out_6187186838300588967[179] = 0.0;
   out_6187186838300588967[180] = 0.0;
   out_6187186838300588967[181] = 0.0;
   out_6187186838300588967[182] = 0.0;
   out_6187186838300588967[183] = 0.0;
   out_6187186838300588967[184] = 0.0;
   out_6187186838300588967[185] = 0.0;
   out_6187186838300588967[186] = 0.0;
   out_6187186838300588967[187] = 0.0;
   out_6187186838300588967[188] = 0.0;
   out_6187186838300588967[189] = 0.0;
   out_6187186838300588967[190] = 1.0;
   out_6187186838300588967[191] = 0.0;
   out_6187186838300588967[192] = 0.0;
   out_6187186838300588967[193] = 0.0;
   out_6187186838300588967[194] = 0.0;
   out_6187186838300588967[195] = 0.0;
   out_6187186838300588967[196] = 0.0;
   out_6187186838300588967[197] = 0.0;
   out_6187186838300588967[198] = 0.0;
   out_6187186838300588967[199] = 0.0;
   out_6187186838300588967[200] = 0.0;
   out_6187186838300588967[201] = 0.0;
   out_6187186838300588967[202] = 0.0;
   out_6187186838300588967[203] = 0.0;
   out_6187186838300588967[204] = 0.0;
   out_6187186838300588967[205] = 0.0;
   out_6187186838300588967[206] = 0.0;
   out_6187186838300588967[207] = 0.0;
   out_6187186838300588967[208] = 0.0;
   out_6187186838300588967[209] = 1.0;
   out_6187186838300588967[210] = 0.0;
   out_6187186838300588967[211] = 0.0;
   out_6187186838300588967[212] = 0.0;
   out_6187186838300588967[213] = 0.0;
   out_6187186838300588967[214] = 0.0;
   out_6187186838300588967[215] = 0.0;
   out_6187186838300588967[216] = 0.0;
   out_6187186838300588967[217] = 0.0;
   out_6187186838300588967[218] = 0.0;
   out_6187186838300588967[219] = 0.0;
   out_6187186838300588967[220] = 0.0;
   out_6187186838300588967[221] = 0.0;
   out_6187186838300588967[222] = 0.0;
   out_6187186838300588967[223] = 0.0;
   out_6187186838300588967[224] = 0.0;
   out_6187186838300588967[225] = 0.0;
   out_6187186838300588967[226] = 0.0;
   out_6187186838300588967[227] = 0.0;
   out_6187186838300588967[228] = 1.0;
   out_6187186838300588967[229] = 0.0;
   out_6187186838300588967[230] = 0.0;
   out_6187186838300588967[231] = 0.0;
   out_6187186838300588967[232] = 0.0;
   out_6187186838300588967[233] = 0.0;
   out_6187186838300588967[234] = 0.0;
   out_6187186838300588967[235] = 0.0;
   out_6187186838300588967[236] = 0.0;
   out_6187186838300588967[237] = 0.0;
   out_6187186838300588967[238] = 0.0;
   out_6187186838300588967[239] = 0.0;
   out_6187186838300588967[240] = 0.0;
   out_6187186838300588967[241] = 0.0;
   out_6187186838300588967[242] = 0.0;
   out_6187186838300588967[243] = 0.0;
   out_6187186838300588967[244] = 0.0;
   out_6187186838300588967[245] = 0.0;
   out_6187186838300588967[246] = 0.0;
   out_6187186838300588967[247] = 1.0;
   out_6187186838300588967[248] = 0.0;
   out_6187186838300588967[249] = 0.0;
   out_6187186838300588967[250] = 0.0;
   out_6187186838300588967[251] = 0.0;
   out_6187186838300588967[252] = 0.0;
   out_6187186838300588967[253] = 0.0;
   out_6187186838300588967[254] = 0.0;
   out_6187186838300588967[255] = 0.0;
   out_6187186838300588967[256] = 0.0;
   out_6187186838300588967[257] = 0.0;
   out_6187186838300588967[258] = 0.0;
   out_6187186838300588967[259] = 0.0;
   out_6187186838300588967[260] = 0.0;
   out_6187186838300588967[261] = 0.0;
   out_6187186838300588967[262] = 0.0;
   out_6187186838300588967[263] = 0.0;
   out_6187186838300588967[264] = 0.0;
   out_6187186838300588967[265] = 0.0;
   out_6187186838300588967[266] = 1.0;
   out_6187186838300588967[267] = 0.0;
   out_6187186838300588967[268] = 0.0;
   out_6187186838300588967[269] = 0.0;
   out_6187186838300588967[270] = 0.0;
   out_6187186838300588967[271] = 0.0;
   out_6187186838300588967[272] = 0.0;
   out_6187186838300588967[273] = 0.0;
   out_6187186838300588967[274] = 0.0;
   out_6187186838300588967[275] = 0.0;
   out_6187186838300588967[276] = 0.0;
   out_6187186838300588967[277] = 0.0;
   out_6187186838300588967[278] = 0.0;
   out_6187186838300588967[279] = 0.0;
   out_6187186838300588967[280] = 0.0;
   out_6187186838300588967[281] = 0.0;
   out_6187186838300588967[282] = 0.0;
   out_6187186838300588967[283] = 0.0;
   out_6187186838300588967[284] = 0.0;
   out_6187186838300588967[285] = 1.0;
   out_6187186838300588967[286] = 0.0;
   out_6187186838300588967[287] = 0.0;
   out_6187186838300588967[288] = 0.0;
   out_6187186838300588967[289] = 0.0;
   out_6187186838300588967[290] = 0.0;
   out_6187186838300588967[291] = 0.0;
   out_6187186838300588967[292] = 0.0;
   out_6187186838300588967[293] = 0.0;
   out_6187186838300588967[294] = 0.0;
   out_6187186838300588967[295] = 0.0;
   out_6187186838300588967[296] = 0.0;
   out_6187186838300588967[297] = 0.0;
   out_6187186838300588967[298] = 0.0;
   out_6187186838300588967[299] = 0.0;
   out_6187186838300588967[300] = 0.0;
   out_6187186838300588967[301] = 0.0;
   out_6187186838300588967[302] = 0.0;
   out_6187186838300588967[303] = 0.0;
   out_6187186838300588967[304] = 1.0;
   out_6187186838300588967[305] = 0.0;
   out_6187186838300588967[306] = 0.0;
   out_6187186838300588967[307] = 0.0;
   out_6187186838300588967[308] = 0.0;
   out_6187186838300588967[309] = 0.0;
   out_6187186838300588967[310] = 0.0;
   out_6187186838300588967[311] = 0.0;
   out_6187186838300588967[312] = 0.0;
   out_6187186838300588967[313] = 0.0;
   out_6187186838300588967[314] = 0.0;
   out_6187186838300588967[315] = 0.0;
   out_6187186838300588967[316] = 0.0;
   out_6187186838300588967[317] = 0.0;
   out_6187186838300588967[318] = 0.0;
   out_6187186838300588967[319] = 0.0;
   out_6187186838300588967[320] = 0.0;
   out_6187186838300588967[321] = 0.0;
   out_6187186838300588967[322] = 0.0;
   out_6187186838300588967[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_8843286105500853100) {
   out_8843286105500853100[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_8843286105500853100[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_8843286105500853100[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_8843286105500853100[3] = dt*state[12] + state[3];
   out_8843286105500853100[4] = dt*state[13] + state[4];
   out_8843286105500853100[5] = dt*state[14] + state[5];
   out_8843286105500853100[6] = state[6];
   out_8843286105500853100[7] = state[7];
   out_8843286105500853100[8] = state[8];
   out_8843286105500853100[9] = state[9];
   out_8843286105500853100[10] = state[10];
   out_8843286105500853100[11] = state[11];
   out_8843286105500853100[12] = state[12];
   out_8843286105500853100[13] = state[13];
   out_8843286105500853100[14] = state[14];
   out_8843286105500853100[15] = state[15];
   out_8843286105500853100[16] = state[16];
   out_8843286105500853100[17] = state[17];
}
void F_fun(double *state, double dt, double *out_4695420932330585758) {
   out_4695420932330585758[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4695420932330585758[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4695420932330585758[2] = 0;
   out_4695420932330585758[3] = 0;
   out_4695420932330585758[4] = 0;
   out_4695420932330585758[5] = 0;
   out_4695420932330585758[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4695420932330585758[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4695420932330585758[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4695420932330585758[9] = 0;
   out_4695420932330585758[10] = 0;
   out_4695420932330585758[11] = 0;
   out_4695420932330585758[12] = 0;
   out_4695420932330585758[13] = 0;
   out_4695420932330585758[14] = 0;
   out_4695420932330585758[15] = 0;
   out_4695420932330585758[16] = 0;
   out_4695420932330585758[17] = 0;
   out_4695420932330585758[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4695420932330585758[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4695420932330585758[20] = 0;
   out_4695420932330585758[21] = 0;
   out_4695420932330585758[22] = 0;
   out_4695420932330585758[23] = 0;
   out_4695420932330585758[24] = 0;
   out_4695420932330585758[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4695420932330585758[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4695420932330585758[27] = 0;
   out_4695420932330585758[28] = 0;
   out_4695420932330585758[29] = 0;
   out_4695420932330585758[30] = 0;
   out_4695420932330585758[31] = 0;
   out_4695420932330585758[32] = 0;
   out_4695420932330585758[33] = 0;
   out_4695420932330585758[34] = 0;
   out_4695420932330585758[35] = 0;
   out_4695420932330585758[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4695420932330585758[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4695420932330585758[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4695420932330585758[39] = 0;
   out_4695420932330585758[40] = 0;
   out_4695420932330585758[41] = 0;
   out_4695420932330585758[42] = 0;
   out_4695420932330585758[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4695420932330585758[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4695420932330585758[45] = 0;
   out_4695420932330585758[46] = 0;
   out_4695420932330585758[47] = 0;
   out_4695420932330585758[48] = 0;
   out_4695420932330585758[49] = 0;
   out_4695420932330585758[50] = 0;
   out_4695420932330585758[51] = 0;
   out_4695420932330585758[52] = 0;
   out_4695420932330585758[53] = 0;
   out_4695420932330585758[54] = 0;
   out_4695420932330585758[55] = 0;
   out_4695420932330585758[56] = 0;
   out_4695420932330585758[57] = 1;
   out_4695420932330585758[58] = 0;
   out_4695420932330585758[59] = 0;
   out_4695420932330585758[60] = 0;
   out_4695420932330585758[61] = 0;
   out_4695420932330585758[62] = 0;
   out_4695420932330585758[63] = 0;
   out_4695420932330585758[64] = 0;
   out_4695420932330585758[65] = 0;
   out_4695420932330585758[66] = dt;
   out_4695420932330585758[67] = 0;
   out_4695420932330585758[68] = 0;
   out_4695420932330585758[69] = 0;
   out_4695420932330585758[70] = 0;
   out_4695420932330585758[71] = 0;
   out_4695420932330585758[72] = 0;
   out_4695420932330585758[73] = 0;
   out_4695420932330585758[74] = 0;
   out_4695420932330585758[75] = 0;
   out_4695420932330585758[76] = 1;
   out_4695420932330585758[77] = 0;
   out_4695420932330585758[78] = 0;
   out_4695420932330585758[79] = 0;
   out_4695420932330585758[80] = 0;
   out_4695420932330585758[81] = 0;
   out_4695420932330585758[82] = 0;
   out_4695420932330585758[83] = 0;
   out_4695420932330585758[84] = 0;
   out_4695420932330585758[85] = dt;
   out_4695420932330585758[86] = 0;
   out_4695420932330585758[87] = 0;
   out_4695420932330585758[88] = 0;
   out_4695420932330585758[89] = 0;
   out_4695420932330585758[90] = 0;
   out_4695420932330585758[91] = 0;
   out_4695420932330585758[92] = 0;
   out_4695420932330585758[93] = 0;
   out_4695420932330585758[94] = 0;
   out_4695420932330585758[95] = 1;
   out_4695420932330585758[96] = 0;
   out_4695420932330585758[97] = 0;
   out_4695420932330585758[98] = 0;
   out_4695420932330585758[99] = 0;
   out_4695420932330585758[100] = 0;
   out_4695420932330585758[101] = 0;
   out_4695420932330585758[102] = 0;
   out_4695420932330585758[103] = 0;
   out_4695420932330585758[104] = dt;
   out_4695420932330585758[105] = 0;
   out_4695420932330585758[106] = 0;
   out_4695420932330585758[107] = 0;
   out_4695420932330585758[108] = 0;
   out_4695420932330585758[109] = 0;
   out_4695420932330585758[110] = 0;
   out_4695420932330585758[111] = 0;
   out_4695420932330585758[112] = 0;
   out_4695420932330585758[113] = 0;
   out_4695420932330585758[114] = 1;
   out_4695420932330585758[115] = 0;
   out_4695420932330585758[116] = 0;
   out_4695420932330585758[117] = 0;
   out_4695420932330585758[118] = 0;
   out_4695420932330585758[119] = 0;
   out_4695420932330585758[120] = 0;
   out_4695420932330585758[121] = 0;
   out_4695420932330585758[122] = 0;
   out_4695420932330585758[123] = 0;
   out_4695420932330585758[124] = 0;
   out_4695420932330585758[125] = 0;
   out_4695420932330585758[126] = 0;
   out_4695420932330585758[127] = 0;
   out_4695420932330585758[128] = 0;
   out_4695420932330585758[129] = 0;
   out_4695420932330585758[130] = 0;
   out_4695420932330585758[131] = 0;
   out_4695420932330585758[132] = 0;
   out_4695420932330585758[133] = 1;
   out_4695420932330585758[134] = 0;
   out_4695420932330585758[135] = 0;
   out_4695420932330585758[136] = 0;
   out_4695420932330585758[137] = 0;
   out_4695420932330585758[138] = 0;
   out_4695420932330585758[139] = 0;
   out_4695420932330585758[140] = 0;
   out_4695420932330585758[141] = 0;
   out_4695420932330585758[142] = 0;
   out_4695420932330585758[143] = 0;
   out_4695420932330585758[144] = 0;
   out_4695420932330585758[145] = 0;
   out_4695420932330585758[146] = 0;
   out_4695420932330585758[147] = 0;
   out_4695420932330585758[148] = 0;
   out_4695420932330585758[149] = 0;
   out_4695420932330585758[150] = 0;
   out_4695420932330585758[151] = 0;
   out_4695420932330585758[152] = 1;
   out_4695420932330585758[153] = 0;
   out_4695420932330585758[154] = 0;
   out_4695420932330585758[155] = 0;
   out_4695420932330585758[156] = 0;
   out_4695420932330585758[157] = 0;
   out_4695420932330585758[158] = 0;
   out_4695420932330585758[159] = 0;
   out_4695420932330585758[160] = 0;
   out_4695420932330585758[161] = 0;
   out_4695420932330585758[162] = 0;
   out_4695420932330585758[163] = 0;
   out_4695420932330585758[164] = 0;
   out_4695420932330585758[165] = 0;
   out_4695420932330585758[166] = 0;
   out_4695420932330585758[167] = 0;
   out_4695420932330585758[168] = 0;
   out_4695420932330585758[169] = 0;
   out_4695420932330585758[170] = 0;
   out_4695420932330585758[171] = 1;
   out_4695420932330585758[172] = 0;
   out_4695420932330585758[173] = 0;
   out_4695420932330585758[174] = 0;
   out_4695420932330585758[175] = 0;
   out_4695420932330585758[176] = 0;
   out_4695420932330585758[177] = 0;
   out_4695420932330585758[178] = 0;
   out_4695420932330585758[179] = 0;
   out_4695420932330585758[180] = 0;
   out_4695420932330585758[181] = 0;
   out_4695420932330585758[182] = 0;
   out_4695420932330585758[183] = 0;
   out_4695420932330585758[184] = 0;
   out_4695420932330585758[185] = 0;
   out_4695420932330585758[186] = 0;
   out_4695420932330585758[187] = 0;
   out_4695420932330585758[188] = 0;
   out_4695420932330585758[189] = 0;
   out_4695420932330585758[190] = 1;
   out_4695420932330585758[191] = 0;
   out_4695420932330585758[192] = 0;
   out_4695420932330585758[193] = 0;
   out_4695420932330585758[194] = 0;
   out_4695420932330585758[195] = 0;
   out_4695420932330585758[196] = 0;
   out_4695420932330585758[197] = 0;
   out_4695420932330585758[198] = 0;
   out_4695420932330585758[199] = 0;
   out_4695420932330585758[200] = 0;
   out_4695420932330585758[201] = 0;
   out_4695420932330585758[202] = 0;
   out_4695420932330585758[203] = 0;
   out_4695420932330585758[204] = 0;
   out_4695420932330585758[205] = 0;
   out_4695420932330585758[206] = 0;
   out_4695420932330585758[207] = 0;
   out_4695420932330585758[208] = 0;
   out_4695420932330585758[209] = 1;
   out_4695420932330585758[210] = 0;
   out_4695420932330585758[211] = 0;
   out_4695420932330585758[212] = 0;
   out_4695420932330585758[213] = 0;
   out_4695420932330585758[214] = 0;
   out_4695420932330585758[215] = 0;
   out_4695420932330585758[216] = 0;
   out_4695420932330585758[217] = 0;
   out_4695420932330585758[218] = 0;
   out_4695420932330585758[219] = 0;
   out_4695420932330585758[220] = 0;
   out_4695420932330585758[221] = 0;
   out_4695420932330585758[222] = 0;
   out_4695420932330585758[223] = 0;
   out_4695420932330585758[224] = 0;
   out_4695420932330585758[225] = 0;
   out_4695420932330585758[226] = 0;
   out_4695420932330585758[227] = 0;
   out_4695420932330585758[228] = 1;
   out_4695420932330585758[229] = 0;
   out_4695420932330585758[230] = 0;
   out_4695420932330585758[231] = 0;
   out_4695420932330585758[232] = 0;
   out_4695420932330585758[233] = 0;
   out_4695420932330585758[234] = 0;
   out_4695420932330585758[235] = 0;
   out_4695420932330585758[236] = 0;
   out_4695420932330585758[237] = 0;
   out_4695420932330585758[238] = 0;
   out_4695420932330585758[239] = 0;
   out_4695420932330585758[240] = 0;
   out_4695420932330585758[241] = 0;
   out_4695420932330585758[242] = 0;
   out_4695420932330585758[243] = 0;
   out_4695420932330585758[244] = 0;
   out_4695420932330585758[245] = 0;
   out_4695420932330585758[246] = 0;
   out_4695420932330585758[247] = 1;
   out_4695420932330585758[248] = 0;
   out_4695420932330585758[249] = 0;
   out_4695420932330585758[250] = 0;
   out_4695420932330585758[251] = 0;
   out_4695420932330585758[252] = 0;
   out_4695420932330585758[253] = 0;
   out_4695420932330585758[254] = 0;
   out_4695420932330585758[255] = 0;
   out_4695420932330585758[256] = 0;
   out_4695420932330585758[257] = 0;
   out_4695420932330585758[258] = 0;
   out_4695420932330585758[259] = 0;
   out_4695420932330585758[260] = 0;
   out_4695420932330585758[261] = 0;
   out_4695420932330585758[262] = 0;
   out_4695420932330585758[263] = 0;
   out_4695420932330585758[264] = 0;
   out_4695420932330585758[265] = 0;
   out_4695420932330585758[266] = 1;
   out_4695420932330585758[267] = 0;
   out_4695420932330585758[268] = 0;
   out_4695420932330585758[269] = 0;
   out_4695420932330585758[270] = 0;
   out_4695420932330585758[271] = 0;
   out_4695420932330585758[272] = 0;
   out_4695420932330585758[273] = 0;
   out_4695420932330585758[274] = 0;
   out_4695420932330585758[275] = 0;
   out_4695420932330585758[276] = 0;
   out_4695420932330585758[277] = 0;
   out_4695420932330585758[278] = 0;
   out_4695420932330585758[279] = 0;
   out_4695420932330585758[280] = 0;
   out_4695420932330585758[281] = 0;
   out_4695420932330585758[282] = 0;
   out_4695420932330585758[283] = 0;
   out_4695420932330585758[284] = 0;
   out_4695420932330585758[285] = 1;
   out_4695420932330585758[286] = 0;
   out_4695420932330585758[287] = 0;
   out_4695420932330585758[288] = 0;
   out_4695420932330585758[289] = 0;
   out_4695420932330585758[290] = 0;
   out_4695420932330585758[291] = 0;
   out_4695420932330585758[292] = 0;
   out_4695420932330585758[293] = 0;
   out_4695420932330585758[294] = 0;
   out_4695420932330585758[295] = 0;
   out_4695420932330585758[296] = 0;
   out_4695420932330585758[297] = 0;
   out_4695420932330585758[298] = 0;
   out_4695420932330585758[299] = 0;
   out_4695420932330585758[300] = 0;
   out_4695420932330585758[301] = 0;
   out_4695420932330585758[302] = 0;
   out_4695420932330585758[303] = 0;
   out_4695420932330585758[304] = 1;
   out_4695420932330585758[305] = 0;
   out_4695420932330585758[306] = 0;
   out_4695420932330585758[307] = 0;
   out_4695420932330585758[308] = 0;
   out_4695420932330585758[309] = 0;
   out_4695420932330585758[310] = 0;
   out_4695420932330585758[311] = 0;
   out_4695420932330585758[312] = 0;
   out_4695420932330585758[313] = 0;
   out_4695420932330585758[314] = 0;
   out_4695420932330585758[315] = 0;
   out_4695420932330585758[316] = 0;
   out_4695420932330585758[317] = 0;
   out_4695420932330585758[318] = 0;
   out_4695420932330585758[319] = 0;
   out_4695420932330585758[320] = 0;
   out_4695420932330585758[321] = 0;
   out_4695420932330585758[322] = 0;
   out_4695420932330585758[323] = 1;
}
void h_4(double *state, double *unused, double *out_2276007570038364899) {
   out_2276007570038364899[0] = state[6] + state[9];
   out_2276007570038364899[1] = state[7] + state[10];
   out_2276007570038364899[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_339105031548630246) {
   out_339105031548630246[0] = 0;
   out_339105031548630246[1] = 0;
   out_339105031548630246[2] = 0;
   out_339105031548630246[3] = 0;
   out_339105031548630246[4] = 0;
   out_339105031548630246[5] = 0;
   out_339105031548630246[6] = 1;
   out_339105031548630246[7] = 0;
   out_339105031548630246[8] = 0;
   out_339105031548630246[9] = 1;
   out_339105031548630246[10] = 0;
   out_339105031548630246[11] = 0;
   out_339105031548630246[12] = 0;
   out_339105031548630246[13] = 0;
   out_339105031548630246[14] = 0;
   out_339105031548630246[15] = 0;
   out_339105031548630246[16] = 0;
   out_339105031548630246[17] = 0;
   out_339105031548630246[18] = 0;
   out_339105031548630246[19] = 0;
   out_339105031548630246[20] = 0;
   out_339105031548630246[21] = 0;
   out_339105031548630246[22] = 0;
   out_339105031548630246[23] = 0;
   out_339105031548630246[24] = 0;
   out_339105031548630246[25] = 1;
   out_339105031548630246[26] = 0;
   out_339105031548630246[27] = 0;
   out_339105031548630246[28] = 1;
   out_339105031548630246[29] = 0;
   out_339105031548630246[30] = 0;
   out_339105031548630246[31] = 0;
   out_339105031548630246[32] = 0;
   out_339105031548630246[33] = 0;
   out_339105031548630246[34] = 0;
   out_339105031548630246[35] = 0;
   out_339105031548630246[36] = 0;
   out_339105031548630246[37] = 0;
   out_339105031548630246[38] = 0;
   out_339105031548630246[39] = 0;
   out_339105031548630246[40] = 0;
   out_339105031548630246[41] = 0;
   out_339105031548630246[42] = 0;
   out_339105031548630246[43] = 0;
   out_339105031548630246[44] = 1;
   out_339105031548630246[45] = 0;
   out_339105031548630246[46] = 0;
   out_339105031548630246[47] = 1;
   out_339105031548630246[48] = 0;
   out_339105031548630246[49] = 0;
   out_339105031548630246[50] = 0;
   out_339105031548630246[51] = 0;
   out_339105031548630246[52] = 0;
   out_339105031548630246[53] = 0;
}
void h_10(double *state, double *unused, double *out_530488663462245196) {
   out_530488663462245196[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_530488663462245196[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_530488663462245196[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_3914913893563359029) {
   out_3914913893563359029[0] = 0;
   out_3914913893563359029[1] = 9.8100000000000005*cos(state[1]);
   out_3914913893563359029[2] = 0;
   out_3914913893563359029[3] = 0;
   out_3914913893563359029[4] = -state[8];
   out_3914913893563359029[5] = state[7];
   out_3914913893563359029[6] = 0;
   out_3914913893563359029[7] = state[5];
   out_3914913893563359029[8] = -state[4];
   out_3914913893563359029[9] = 0;
   out_3914913893563359029[10] = 0;
   out_3914913893563359029[11] = 0;
   out_3914913893563359029[12] = 1;
   out_3914913893563359029[13] = 0;
   out_3914913893563359029[14] = 0;
   out_3914913893563359029[15] = 1;
   out_3914913893563359029[16] = 0;
   out_3914913893563359029[17] = 0;
   out_3914913893563359029[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_3914913893563359029[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_3914913893563359029[20] = 0;
   out_3914913893563359029[21] = state[8];
   out_3914913893563359029[22] = 0;
   out_3914913893563359029[23] = -state[6];
   out_3914913893563359029[24] = -state[5];
   out_3914913893563359029[25] = 0;
   out_3914913893563359029[26] = state[3];
   out_3914913893563359029[27] = 0;
   out_3914913893563359029[28] = 0;
   out_3914913893563359029[29] = 0;
   out_3914913893563359029[30] = 0;
   out_3914913893563359029[31] = 1;
   out_3914913893563359029[32] = 0;
   out_3914913893563359029[33] = 0;
   out_3914913893563359029[34] = 1;
   out_3914913893563359029[35] = 0;
   out_3914913893563359029[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_3914913893563359029[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_3914913893563359029[38] = 0;
   out_3914913893563359029[39] = -state[7];
   out_3914913893563359029[40] = state[6];
   out_3914913893563359029[41] = 0;
   out_3914913893563359029[42] = state[4];
   out_3914913893563359029[43] = -state[3];
   out_3914913893563359029[44] = 0;
   out_3914913893563359029[45] = 0;
   out_3914913893563359029[46] = 0;
   out_3914913893563359029[47] = 0;
   out_3914913893563359029[48] = 0;
   out_3914913893563359029[49] = 0;
   out_3914913893563359029[50] = 1;
   out_3914913893563359029[51] = 0;
   out_3914913893563359029[52] = 0;
   out_3914913893563359029[53] = 1;
}
void h_13(double *state, double *unused, double *out_7290329458186336837) {
   out_7290329458186336837[0] = state[3];
   out_7290329458186336837[1] = state[4];
   out_7290329458186336837[2] = state[5];
}
void H_13(double *state, double *unused, double *out_3551378856880963047) {
   out_3551378856880963047[0] = 0;
   out_3551378856880963047[1] = 0;
   out_3551378856880963047[2] = 0;
   out_3551378856880963047[3] = 1;
   out_3551378856880963047[4] = 0;
   out_3551378856880963047[5] = 0;
   out_3551378856880963047[6] = 0;
   out_3551378856880963047[7] = 0;
   out_3551378856880963047[8] = 0;
   out_3551378856880963047[9] = 0;
   out_3551378856880963047[10] = 0;
   out_3551378856880963047[11] = 0;
   out_3551378856880963047[12] = 0;
   out_3551378856880963047[13] = 0;
   out_3551378856880963047[14] = 0;
   out_3551378856880963047[15] = 0;
   out_3551378856880963047[16] = 0;
   out_3551378856880963047[17] = 0;
   out_3551378856880963047[18] = 0;
   out_3551378856880963047[19] = 0;
   out_3551378856880963047[20] = 0;
   out_3551378856880963047[21] = 0;
   out_3551378856880963047[22] = 1;
   out_3551378856880963047[23] = 0;
   out_3551378856880963047[24] = 0;
   out_3551378856880963047[25] = 0;
   out_3551378856880963047[26] = 0;
   out_3551378856880963047[27] = 0;
   out_3551378856880963047[28] = 0;
   out_3551378856880963047[29] = 0;
   out_3551378856880963047[30] = 0;
   out_3551378856880963047[31] = 0;
   out_3551378856880963047[32] = 0;
   out_3551378856880963047[33] = 0;
   out_3551378856880963047[34] = 0;
   out_3551378856880963047[35] = 0;
   out_3551378856880963047[36] = 0;
   out_3551378856880963047[37] = 0;
   out_3551378856880963047[38] = 0;
   out_3551378856880963047[39] = 0;
   out_3551378856880963047[40] = 0;
   out_3551378856880963047[41] = 1;
   out_3551378856880963047[42] = 0;
   out_3551378856880963047[43] = 0;
   out_3551378856880963047[44] = 0;
   out_3551378856880963047[45] = 0;
   out_3551378856880963047[46] = 0;
   out_3551378856880963047[47] = 0;
   out_3551378856880963047[48] = 0;
   out_3551378856880963047[49] = 0;
   out_3551378856880963047[50] = 0;
   out_3551378856880963047[51] = 0;
   out_3551378856880963047[52] = 0;
   out_3551378856880963047[53] = 0;
}
void h_14(double *state, double *unused, double *out_9018381316782859541) {
   out_9018381316782859541[0] = state[6];
   out_9018381316782859541[1] = state[7];
   out_9018381316782859541[2] = state[8];
}
void H_14(double *state, double *unused, double *out_4302345887888114775) {
   out_4302345887888114775[0] = 0;
   out_4302345887888114775[1] = 0;
   out_4302345887888114775[2] = 0;
   out_4302345887888114775[3] = 0;
   out_4302345887888114775[4] = 0;
   out_4302345887888114775[5] = 0;
   out_4302345887888114775[6] = 1;
   out_4302345887888114775[7] = 0;
   out_4302345887888114775[8] = 0;
   out_4302345887888114775[9] = 0;
   out_4302345887888114775[10] = 0;
   out_4302345887888114775[11] = 0;
   out_4302345887888114775[12] = 0;
   out_4302345887888114775[13] = 0;
   out_4302345887888114775[14] = 0;
   out_4302345887888114775[15] = 0;
   out_4302345887888114775[16] = 0;
   out_4302345887888114775[17] = 0;
   out_4302345887888114775[18] = 0;
   out_4302345887888114775[19] = 0;
   out_4302345887888114775[20] = 0;
   out_4302345887888114775[21] = 0;
   out_4302345887888114775[22] = 0;
   out_4302345887888114775[23] = 0;
   out_4302345887888114775[24] = 0;
   out_4302345887888114775[25] = 1;
   out_4302345887888114775[26] = 0;
   out_4302345887888114775[27] = 0;
   out_4302345887888114775[28] = 0;
   out_4302345887888114775[29] = 0;
   out_4302345887888114775[30] = 0;
   out_4302345887888114775[31] = 0;
   out_4302345887888114775[32] = 0;
   out_4302345887888114775[33] = 0;
   out_4302345887888114775[34] = 0;
   out_4302345887888114775[35] = 0;
   out_4302345887888114775[36] = 0;
   out_4302345887888114775[37] = 0;
   out_4302345887888114775[38] = 0;
   out_4302345887888114775[39] = 0;
   out_4302345887888114775[40] = 0;
   out_4302345887888114775[41] = 0;
   out_4302345887888114775[42] = 0;
   out_4302345887888114775[43] = 0;
   out_4302345887888114775[44] = 1;
   out_4302345887888114775[45] = 0;
   out_4302345887888114775[46] = 0;
   out_4302345887888114775[47] = 0;
   out_4302345887888114775[48] = 0;
   out_4302345887888114775[49] = 0;
   out_4302345887888114775[50] = 0;
   out_4302345887888114775[51] = 0;
   out_4302345887888114775[52] = 0;
   out_4302345887888114775[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_4055321504301233767) {
  err_fun(nom_x, delta_x, out_4055321504301233767);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5242087564548968290) {
  inv_err_fun(nom_x, true_x, out_5242087564548968290);
}
void pose_H_mod_fun(double *state, double *out_6187186838300588967) {
  H_mod_fun(state, out_6187186838300588967);
}
void pose_f_fun(double *state, double dt, double *out_8843286105500853100) {
  f_fun(state,  dt, out_8843286105500853100);
}
void pose_F_fun(double *state, double dt, double *out_4695420932330585758) {
  F_fun(state,  dt, out_4695420932330585758);
}
void pose_h_4(double *state, double *unused, double *out_2276007570038364899) {
  h_4(state, unused, out_2276007570038364899);
}
void pose_H_4(double *state, double *unused, double *out_339105031548630246) {
  H_4(state, unused, out_339105031548630246);
}
void pose_h_10(double *state, double *unused, double *out_530488663462245196) {
  h_10(state, unused, out_530488663462245196);
}
void pose_H_10(double *state, double *unused, double *out_3914913893563359029) {
  H_10(state, unused, out_3914913893563359029);
}
void pose_h_13(double *state, double *unused, double *out_7290329458186336837) {
  h_13(state, unused, out_7290329458186336837);
}
void pose_H_13(double *state, double *unused, double *out_3551378856880963047) {
  H_13(state, unused, out_3551378856880963047);
}
void pose_h_14(double *state, double *unused, double *out_9018381316782859541) {
  h_14(state, unused, out_9018381316782859541);
}
void pose_H_14(double *state, double *unused, double *out_4302345887888114775) {
  H_14(state, unused, out_4302345887888114775);
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

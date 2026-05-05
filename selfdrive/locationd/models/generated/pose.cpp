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
void err_fun(double *nom_x, double *delta_x, double *out_3212432162194349052) {
   out_3212432162194349052[0] = delta_x[0] + nom_x[0];
   out_3212432162194349052[1] = delta_x[1] + nom_x[1];
   out_3212432162194349052[2] = delta_x[2] + nom_x[2];
   out_3212432162194349052[3] = delta_x[3] + nom_x[3];
   out_3212432162194349052[4] = delta_x[4] + nom_x[4];
   out_3212432162194349052[5] = delta_x[5] + nom_x[5];
   out_3212432162194349052[6] = delta_x[6] + nom_x[6];
   out_3212432162194349052[7] = delta_x[7] + nom_x[7];
   out_3212432162194349052[8] = delta_x[8] + nom_x[8];
   out_3212432162194349052[9] = delta_x[9] + nom_x[9];
   out_3212432162194349052[10] = delta_x[10] + nom_x[10];
   out_3212432162194349052[11] = delta_x[11] + nom_x[11];
   out_3212432162194349052[12] = delta_x[12] + nom_x[12];
   out_3212432162194349052[13] = delta_x[13] + nom_x[13];
   out_3212432162194349052[14] = delta_x[14] + nom_x[14];
   out_3212432162194349052[15] = delta_x[15] + nom_x[15];
   out_3212432162194349052[16] = delta_x[16] + nom_x[16];
   out_3212432162194349052[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_8973271523308122061) {
   out_8973271523308122061[0] = -nom_x[0] + true_x[0];
   out_8973271523308122061[1] = -nom_x[1] + true_x[1];
   out_8973271523308122061[2] = -nom_x[2] + true_x[2];
   out_8973271523308122061[3] = -nom_x[3] + true_x[3];
   out_8973271523308122061[4] = -nom_x[4] + true_x[4];
   out_8973271523308122061[5] = -nom_x[5] + true_x[5];
   out_8973271523308122061[6] = -nom_x[6] + true_x[6];
   out_8973271523308122061[7] = -nom_x[7] + true_x[7];
   out_8973271523308122061[8] = -nom_x[8] + true_x[8];
   out_8973271523308122061[9] = -nom_x[9] + true_x[9];
   out_8973271523308122061[10] = -nom_x[10] + true_x[10];
   out_8973271523308122061[11] = -nom_x[11] + true_x[11];
   out_8973271523308122061[12] = -nom_x[12] + true_x[12];
   out_8973271523308122061[13] = -nom_x[13] + true_x[13];
   out_8973271523308122061[14] = -nom_x[14] + true_x[14];
   out_8973271523308122061[15] = -nom_x[15] + true_x[15];
   out_8973271523308122061[16] = -nom_x[16] + true_x[16];
   out_8973271523308122061[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_7731698092980355432) {
   out_7731698092980355432[0] = 1.0;
   out_7731698092980355432[1] = 0.0;
   out_7731698092980355432[2] = 0.0;
   out_7731698092980355432[3] = 0.0;
   out_7731698092980355432[4] = 0.0;
   out_7731698092980355432[5] = 0.0;
   out_7731698092980355432[6] = 0.0;
   out_7731698092980355432[7] = 0.0;
   out_7731698092980355432[8] = 0.0;
   out_7731698092980355432[9] = 0.0;
   out_7731698092980355432[10] = 0.0;
   out_7731698092980355432[11] = 0.0;
   out_7731698092980355432[12] = 0.0;
   out_7731698092980355432[13] = 0.0;
   out_7731698092980355432[14] = 0.0;
   out_7731698092980355432[15] = 0.0;
   out_7731698092980355432[16] = 0.0;
   out_7731698092980355432[17] = 0.0;
   out_7731698092980355432[18] = 0.0;
   out_7731698092980355432[19] = 1.0;
   out_7731698092980355432[20] = 0.0;
   out_7731698092980355432[21] = 0.0;
   out_7731698092980355432[22] = 0.0;
   out_7731698092980355432[23] = 0.0;
   out_7731698092980355432[24] = 0.0;
   out_7731698092980355432[25] = 0.0;
   out_7731698092980355432[26] = 0.0;
   out_7731698092980355432[27] = 0.0;
   out_7731698092980355432[28] = 0.0;
   out_7731698092980355432[29] = 0.0;
   out_7731698092980355432[30] = 0.0;
   out_7731698092980355432[31] = 0.0;
   out_7731698092980355432[32] = 0.0;
   out_7731698092980355432[33] = 0.0;
   out_7731698092980355432[34] = 0.0;
   out_7731698092980355432[35] = 0.0;
   out_7731698092980355432[36] = 0.0;
   out_7731698092980355432[37] = 0.0;
   out_7731698092980355432[38] = 1.0;
   out_7731698092980355432[39] = 0.0;
   out_7731698092980355432[40] = 0.0;
   out_7731698092980355432[41] = 0.0;
   out_7731698092980355432[42] = 0.0;
   out_7731698092980355432[43] = 0.0;
   out_7731698092980355432[44] = 0.0;
   out_7731698092980355432[45] = 0.0;
   out_7731698092980355432[46] = 0.0;
   out_7731698092980355432[47] = 0.0;
   out_7731698092980355432[48] = 0.0;
   out_7731698092980355432[49] = 0.0;
   out_7731698092980355432[50] = 0.0;
   out_7731698092980355432[51] = 0.0;
   out_7731698092980355432[52] = 0.0;
   out_7731698092980355432[53] = 0.0;
   out_7731698092980355432[54] = 0.0;
   out_7731698092980355432[55] = 0.0;
   out_7731698092980355432[56] = 0.0;
   out_7731698092980355432[57] = 1.0;
   out_7731698092980355432[58] = 0.0;
   out_7731698092980355432[59] = 0.0;
   out_7731698092980355432[60] = 0.0;
   out_7731698092980355432[61] = 0.0;
   out_7731698092980355432[62] = 0.0;
   out_7731698092980355432[63] = 0.0;
   out_7731698092980355432[64] = 0.0;
   out_7731698092980355432[65] = 0.0;
   out_7731698092980355432[66] = 0.0;
   out_7731698092980355432[67] = 0.0;
   out_7731698092980355432[68] = 0.0;
   out_7731698092980355432[69] = 0.0;
   out_7731698092980355432[70] = 0.0;
   out_7731698092980355432[71] = 0.0;
   out_7731698092980355432[72] = 0.0;
   out_7731698092980355432[73] = 0.0;
   out_7731698092980355432[74] = 0.0;
   out_7731698092980355432[75] = 0.0;
   out_7731698092980355432[76] = 1.0;
   out_7731698092980355432[77] = 0.0;
   out_7731698092980355432[78] = 0.0;
   out_7731698092980355432[79] = 0.0;
   out_7731698092980355432[80] = 0.0;
   out_7731698092980355432[81] = 0.0;
   out_7731698092980355432[82] = 0.0;
   out_7731698092980355432[83] = 0.0;
   out_7731698092980355432[84] = 0.0;
   out_7731698092980355432[85] = 0.0;
   out_7731698092980355432[86] = 0.0;
   out_7731698092980355432[87] = 0.0;
   out_7731698092980355432[88] = 0.0;
   out_7731698092980355432[89] = 0.0;
   out_7731698092980355432[90] = 0.0;
   out_7731698092980355432[91] = 0.0;
   out_7731698092980355432[92] = 0.0;
   out_7731698092980355432[93] = 0.0;
   out_7731698092980355432[94] = 0.0;
   out_7731698092980355432[95] = 1.0;
   out_7731698092980355432[96] = 0.0;
   out_7731698092980355432[97] = 0.0;
   out_7731698092980355432[98] = 0.0;
   out_7731698092980355432[99] = 0.0;
   out_7731698092980355432[100] = 0.0;
   out_7731698092980355432[101] = 0.0;
   out_7731698092980355432[102] = 0.0;
   out_7731698092980355432[103] = 0.0;
   out_7731698092980355432[104] = 0.0;
   out_7731698092980355432[105] = 0.0;
   out_7731698092980355432[106] = 0.0;
   out_7731698092980355432[107] = 0.0;
   out_7731698092980355432[108] = 0.0;
   out_7731698092980355432[109] = 0.0;
   out_7731698092980355432[110] = 0.0;
   out_7731698092980355432[111] = 0.0;
   out_7731698092980355432[112] = 0.0;
   out_7731698092980355432[113] = 0.0;
   out_7731698092980355432[114] = 1.0;
   out_7731698092980355432[115] = 0.0;
   out_7731698092980355432[116] = 0.0;
   out_7731698092980355432[117] = 0.0;
   out_7731698092980355432[118] = 0.0;
   out_7731698092980355432[119] = 0.0;
   out_7731698092980355432[120] = 0.0;
   out_7731698092980355432[121] = 0.0;
   out_7731698092980355432[122] = 0.0;
   out_7731698092980355432[123] = 0.0;
   out_7731698092980355432[124] = 0.0;
   out_7731698092980355432[125] = 0.0;
   out_7731698092980355432[126] = 0.0;
   out_7731698092980355432[127] = 0.0;
   out_7731698092980355432[128] = 0.0;
   out_7731698092980355432[129] = 0.0;
   out_7731698092980355432[130] = 0.0;
   out_7731698092980355432[131] = 0.0;
   out_7731698092980355432[132] = 0.0;
   out_7731698092980355432[133] = 1.0;
   out_7731698092980355432[134] = 0.0;
   out_7731698092980355432[135] = 0.0;
   out_7731698092980355432[136] = 0.0;
   out_7731698092980355432[137] = 0.0;
   out_7731698092980355432[138] = 0.0;
   out_7731698092980355432[139] = 0.0;
   out_7731698092980355432[140] = 0.0;
   out_7731698092980355432[141] = 0.0;
   out_7731698092980355432[142] = 0.0;
   out_7731698092980355432[143] = 0.0;
   out_7731698092980355432[144] = 0.0;
   out_7731698092980355432[145] = 0.0;
   out_7731698092980355432[146] = 0.0;
   out_7731698092980355432[147] = 0.0;
   out_7731698092980355432[148] = 0.0;
   out_7731698092980355432[149] = 0.0;
   out_7731698092980355432[150] = 0.0;
   out_7731698092980355432[151] = 0.0;
   out_7731698092980355432[152] = 1.0;
   out_7731698092980355432[153] = 0.0;
   out_7731698092980355432[154] = 0.0;
   out_7731698092980355432[155] = 0.0;
   out_7731698092980355432[156] = 0.0;
   out_7731698092980355432[157] = 0.0;
   out_7731698092980355432[158] = 0.0;
   out_7731698092980355432[159] = 0.0;
   out_7731698092980355432[160] = 0.0;
   out_7731698092980355432[161] = 0.0;
   out_7731698092980355432[162] = 0.0;
   out_7731698092980355432[163] = 0.0;
   out_7731698092980355432[164] = 0.0;
   out_7731698092980355432[165] = 0.0;
   out_7731698092980355432[166] = 0.0;
   out_7731698092980355432[167] = 0.0;
   out_7731698092980355432[168] = 0.0;
   out_7731698092980355432[169] = 0.0;
   out_7731698092980355432[170] = 0.0;
   out_7731698092980355432[171] = 1.0;
   out_7731698092980355432[172] = 0.0;
   out_7731698092980355432[173] = 0.0;
   out_7731698092980355432[174] = 0.0;
   out_7731698092980355432[175] = 0.0;
   out_7731698092980355432[176] = 0.0;
   out_7731698092980355432[177] = 0.0;
   out_7731698092980355432[178] = 0.0;
   out_7731698092980355432[179] = 0.0;
   out_7731698092980355432[180] = 0.0;
   out_7731698092980355432[181] = 0.0;
   out_7731698092980355432[182] = 0.0;
   out_7731698092980355432[183] = 0.0;
   out_7731698092980355432[184] = 0.0;
   out_7731698092980355432[185] = 0.0;
   out_7731698092980355432[186] = 0.0;
   out_7731698092980355432[187] = 0.0;
   out_7731698092980355432[188] = 0.0;
   out_7731698092980355432[189] = 0.0;
   out_7731698092980355432[190] = 1.0;
   out_7731698092980355432[191] = 0.0;
   out_7731698092980355432[192] = 0.0;
   out_7731698092980355432[193] = 0.0;
   out_7731698092980355432[194] = 0.0;
   out_7731698092980355432[195] = 0.0;
   out_7731698092980355432[196] = 0.0;
   out_7731698092980355432[197] = 0.0;
   out_7731698092980355432[198] = 0.0;
   out_7731698092980355432[199] = 0.0;
   out_7731698092980355432[200] = 0.0;
   out_7731698092980355432[201] = 0.0;
   out_7731698092980355432[202] = 0.0;
   out_7731698092980355432[203] = 0.0;
   out_7731698092980355432[204] = 0.0;
   out_7731698092980355432[205] = 0.0;
   out_7731698092980355432[206] = 0.0;
   out_7731698092980355432[207] = 0.0;
   out_7731698092980355432[208] = 0.0;
   out_7731698092980355432[209] = 1.0;
   out_7731698092980355432[210] = 0.0;
   out_7731698092980355432[211] = 0.0;
   out_7731698092980355432[212] = 0.0;
   out_7731698092980355432[213] = 0.0;
   out_7731698092980355432[214] = 0.0;
   out_7731698092980355432[215] = 0.0;
   out_7731698092980355432[216] = 0.0;
   out_7731698092980355432[217] = 0.0;
   out_7731698092980355432[218] = 0.0;
   out_7731698092980355432[219] = 0.0;
   out_7731698092980355432[220] = 0.0;
   out_7731698092980355432[221] = 0.0;
   out_7731698092980355432[222] = 0.0;
   out_7731698092980355432[223] = 0.0;
   out_7731698092980355432[224] = 0.0;
   out_7731698092980355432[225] = 0.0;
   out_7731698092980355432[226] = 0.0;
   out_7731698092980355432[227] = 0.0;
   out_7731698092980355432[228] = 1.0;
   out_7731698092980355432[229] = 0.0;
   out_7731698092980355432[230] = 0.0;
   out_7731698092980355432[231] = 0.0;
   out_7731698092980355432[232] = 0.0;
   out_7731698092980355432[233] = 0.0;
   out_7731698092980355432[234] = 0.0;
   out_7731698092980355432[235] = 0.0;
   out_7731698092980355432[236] = 0.0;
   out_7731698092980355432[237] = 0.0;
   out_7731698092980355432[238] = 0.0;
   out_7731698092980355432[239] = 0.0;
   out_7731698092980355432[240] = 0.0;
   out_7731698092980355432[241] = 0.0;
   out_7731698092980355432[242] = 0.0;
   out_7731698092980355432[243] = 0.0;
   out_7731698092980355432[244] = 0.0;
   out_7731698092980355432[245] = 0.0;
   out_7731698092980355432[246] = 0.0;
   out_7731698092980355432[247] = 1.0;
   out_7731698092980355432[248] = 0.0;
   out_7731698092980355432[249] = 0.0;
   out_7731698092980355432[250] = 0.0;
   out_7731698092980355432[251] = 0.0;
   out_7731698092980355432[252] = 0.0;
   out_7731698092980355432[253] = 0.0;
   out_7731698092980355432[254] = 0.0;
   out_7731698092980355432[255] = 0.0;
   out_7731698092980355432[256] = 0.0;
   out_7731698092980355432[257] = 0.0;
   out_7731698092980355432[258] = 0.0;
   out_7731698092980355432[259] = 0.0;
   out_7731698092980355432[260] = 0.0;
   out_7731698092980355432[261] = 0.0;
   out_7731698092980355432[262] = 0.0;
   out_7731698092980355432[263] = 0.0;
   out_7731698092980355432[264] = 0.0;
   out_7731698092980355432[265] = 0.0;
   out_7731698092980355432[266] = 1.0;
   out_7731698092980355432[267] = 0.0;
   out_7731698092980355432[268] = 0.0;
   out_7731698092980355432[269] = 0.0;
   out_7731698092980355432[270] = 0.0;
   out_7731698092980355432[271] = 0.0;
   out_7731698092980355432[272] = 0.0;
   out_7731698092980355432[273] = 0.0;
   out_7731698092980355432[274] = 0.0;
   out_7731698092980355432[275] = 0.0;
   out_7731698092980355432[276] = 0.0;
   out_7731698092980355432[277] = 0.0;
   out_7731698092980355432[278] = 0.0;
   out_7731698092980355432[279] = 0.0;
   out_7731698092980355432[280] = 0.0;
   out_7731698092980355432[281] = 0.0;
   out_7731698092980355432[282] = 0.0;
   out_7731698092980355432[283] = 0.0;
   out_7731698092980355432[284] = 0.0;
   out_7731698092980355432[285] = 1.0;
   out_7731698092980355432[286] = 0.0;
   out_7731698092980355432[287] = 0.0;
   out_7731698092980355432[288] = 0.0;
   out_7731698092980355432[289] = 0.0;
   out_7731698092980355432[290] = 0.0;
   out_7731698092980355432[291] = 0.0;
   out_7731698092980355432[292] = 0.0;
   out_7731698092980355432[293] = 0.0;
   out_7731698092980355432[294] = 0.0;
   out_7731698092980355432[295] = 0.0;
   out_7731698092980355432[296] = 0.0;
   out_7731698092980355432[297] = 0.0;
   out_7731698092980355432[298] = 0.0;
   out_7731698092980355432[299] = 0.0;
   out_7731698092980355432[300] = 0.0;
   out_7731698092980355432[301] = 0.0;
   out_7731698092980355432[302] = 0.0;
   out_7731698092980355432[303] = 0.0;
   out_7731698092980355432[304] = 1.0;
   out_7731698092980355432[305] = 0.0;
   out_7731698092980355432[306] = 0.0;
   out_7731698092980355432[307] = 0.0;
   out_7731698092980355432[308] = 0.0;
   out_7731698092980355432[309] = 0.0;
   out_7731698092980355432[310] = 0.0;
   out_7731698092980355432[311] = 0.0;
   out_7731698092980355432[312] = 0.0;
   out_7731698092980355432[313] = 0.0;
   out_7731698092980355432[314] = 0.0;
   out_7731698092980355432[315] = 0.0;
   out_7731698092980355432[316] = 0.0;
   out_7731698092980355432[317] = 0.0;
   out_7731698092980355432[318] = 0.0;
   out_7731698092980355432[319] = 0.0;
   out_7731698092980355432[320] = 0.0;
   out_7731698092980355432[321] = 0.0;
   out_7731698092980355432[322] = 0.0;
   out_7731698092980355432[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_8923016311880444304) {
   out_8923016311880444304[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_8923016311880444304[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_8923016311880444304[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_8923016311880444304[3] = dt*state[12] + state[3];
   out_8923016311880444304[4] = dt*state[13] + state[4];
   out_8923016311880444304[5] = dt*state[14] + state[5];
   out_8923016311880444304[6] = state[6];
   out_8923016311880444304[7] = state[7];
   out_8923016311880444304[8] = state[8];
   out_8923016311880444304[9] = state[9];
   out_8923016311880444304[10] = state[10];
   out_8923016311880444304[11] = state[11];
   out_8923016311880444304[12] = state[12];
   out_8923016311880444304[13] = state[13];
   out_8923016311880444304[14] = state[14];
   out_8923016311880444304[15] = state[15];
   out_8923016311880444304[16] = state[16];
   out_8923016311880444304[17] = state[17];
}
void F_fun(double *state, double dt, double *out_210116950988546947) {
   out_210116950988546947[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_210116950988546947[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_210116950988546947[2] = 0;
   out_210116950988546947[3] = 0;
   out_210116950988546947[4] = 0;
   out_210116950988546947[5] = 0;
   out_210116950988546947[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_210116950988546947[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_210116950988546947[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_210116950988546947[9] = 0;
   out_210116950988546947[10] = 0;
   out_210116950988546947[11] = 0;
   out_210116950988546947[12] = 0;
   out_210116950988546947[13] = 0;
   out_210116950988546947[14] = 0;
   out_210116950988546947[15] = 0;
   out_210116950988546947[16] = 0;
   out_210116950988546947[17] = 0;
   out_210116950988546947[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_210116950988546947[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_210116950988546947[20] = 0;
   out_210116950988546947[21] = 0;
   out_210116950988546947[22] = 0;
   out_210116950988546947[23] = 0;
   out_210116950988546947[24] = 0;
   out_210116950988546947[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_210116950988546947[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_210116950988546947[27] = 0;
   out_210116950988546947[28] = 0;
   out_210116950988546947[29] = 0;
   out_210116950988546947[30] = 0;
   out_210116950988546947[31] = 0;
   out_210116950988546947[32] = 0;
   out_210116950988546947[33] = 0;
   out_210116950988546947[34] = 0;
   out_210116950988546947[35] = 0;
   out_210116950988546947[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_210116950988546947[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_210116950988546947[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_210116950988546947[39] = 0;
   out_210116950988546947[40] = 0;
   out_210116950988546947[41] = 0;
   out_210116950988546947[42] = 0;
   out_210116950988546947[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_210116950988546947[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_210116950988546947[45] = 0;
   out_210116950988546947[46] = 0;
   out_210116950988546947[47] = 0;
   out_210116950988546947[48] = 0;
   out_210116950988546947[49] = 0;
   out_210116950988546947[50] = 0;
   out_210116950988546947[51] = 0;
   out_210116950988546947[52] = 0;
   out_210116950988546947[53] = 0;
   out_210116950988546947[54] = 0;
   out_210116950988546947[55] = 0;
   out_210116950988546947[56] = 0;
   out_210116950988546947[57] = 1;
   out_210116950988546947[58] = 0;
   out_210116950988546947[59] = 0;
   out_210116950988546947[60] = 0;
   out_210116950988546947[61] = 0;
   out_210116950988546947[62] = 0;
   out_210116950988546947[63] = 0;
   out_210116950988546947[64] = 0;
   out_210116950988546947[65] = 0;
   out_210116950988546947[66] = dt;
   out_210116950988546947[67] = 0;
   out_210116950988546947[68] = 0;
   out_210116950988546947[69] = 0;
   out_210116950988546947[70] = 0;
   out_210116950988546947[71] = 0;
   out_210116950988546947[72] = 0;
   out_210116950988546947[73] = 0;
   out_210116950988546947[74] = 0;
   out_210116950988546947[75] = 0;
   out_210116950988546947[76] = 1;
   out_210116950988546947[77] = 0;
   out_210116950988546947[78] = 0;
   out_210116950988546947[79] = 0;
   out_210116950988546947[80] = 0;
   out_210116950988546947[81] = 0;
   out_210116950988546947[82] = 0;
   out_210116950988546947[83] = 0;
   out_210116950988546947[84] = 0;
   out_210116950988546947[85] = dt;
   out_210116950988546947[86] = 0;
   out_210116950988546947[87] = 0;
   out_210116950988546947[88] = 0;
   out_210116950988546947[89] = 0;
   out_210116950988546947[90] = 0;
   out_210116950988546947[91] = 0;
   out_210116950988546947[92] = 0;
   out_210116950988546947[93] = 0;
   out_210116950988546947[94] = 0;
   out_210116950988546947[95] = 1;
   out_210116950988546947[96] = 0;
   out_210116950988546947[97] = 0;
   out_210116950988546947[98] = 0;
   out_210116950988546947[99] = 0;
   out_210116950988546947[100] = 0;
   out_210116950988546947[101] = 0;
   out_210116950988546947[102] = 0;
   out_210116950988546947[103] = 0;
   out_210116950988546947[104] = dt;
   out_210116950988546947[105] = 0;
   out_210116950988546947[106] = 0;
   out_210116950988546947[107] = 0;
   out_210116950988546947[108] = 0;
   out_210116950988546947[109] = 0;
   out_210116950988546947[110] = 0;
   out_210116950988546947[111] = 0;
   out_210116950988546947[112] = 0;
   out_210116950988546947[113] = 0;
   out_210116950988546947[114] = 1;
   out_210116950988546947[115] = 0;
   out_210116950988546947[116] = 0;
   out_210116950988546947[117] = 0;
   out_210116950988546947[118] = 0;
   out_210116950988546947[119] = 0;
   out_210116950988546947[120] = 0;
   out_210116950988546947[121] = 0;
   out_210116950988546947[122] = 0;
   out_210116950988546947[123] = 0;
   out_210116950988546947[124] = 0;
   out_210116950988546947[125] = 0;
   out_210116950988546947[126] = 0;
   out_210116950988546947[127] = 0;
   out_210116950988546947[128] = 0;
   out_210116950988546947[129] = 0;
   out_210116950988546947[130] = 0;
   out_210116950988546947[131] = 0;
   out_210116950988546947[132] = 0;
   out_210116950988546947[133] = 1;
   out_210116950988546947[134] = 0;
   out_210116950988546947[135] = 0;
   out_210116950988546947[136] = 0;
   out_210116950988546947[137] = 0;
   out_210116950988546947[138] = 0;
   out_210116950988546947[139] = 0;
   out_210116950988546947[140] = 0;
   out_210116950988546947[141] = 0;
   out_210116950988546947[142] = 0;
   out_210116950988546947[143] = 0;
   out_210116950988546947[144] = 0;
   out_210116950988546947[145] = 0;
   out_210116950988546947[146] = 0;
   out_210116950988546947[147] = 0;
   out_210116950988546947[148] = 0;
   out_210116950988546947[149] = 0;
   out_210116950988546947[150] = 0;
   out_210116950988546947[151] = 0;
   out_210116950988546947[152] = 1;
   out_210116950988546947[153] = 0;
   out_210116950988546947[154] = 0;
   out_210116950988546947[155] = 0;
   out_210116950988546947[156] = 0;
   out_210116950988546947[157] = 0;
   out_210116950988546947[158] = 0;
   out_210116950988546947[159] = 0;
   out_210116950988546947[160] = 0;
   out_210116950988546947[161] = 0;
   out_210116950988546947[162] = 0;
   out_210116950988546947[163] = 0;
   out_210116950988546947[164] = 0;
   out_210116950988546947[165] = 0;
   out_210116950988546947[166] = 0;
   out_210116950988546947[167] = 0;
   out_210116950988546947[168] = 0;
   out_210116950988546947[169] = 0;
   out_210116950988546947[170] = 0;
   out_210116950988546947[171] = 1;
   out_210116950988546947[172] = 0;
   out_210116950988546947[173] = 0;
   out_210116950988546947[174] = 0;
   out_210116950988546947[175] = 0;
   out_210116950988546947[176] = 0;
   out_210116950988546947[177] = 0;
   out_210116950988546947[178] = 0;
   out_210116950988546947[179] = 0;
   out_210116950988546947[180] = 0;
   out_210116950988546947[181] = 0;
   out_210116950988546947[182] = 0;
   out_210116950988546947[183] = 0;
   out_210116950988546947[184] = 0;
   out_210116950988546947[185] = 0;
   out_210116950988546947[186] = 0;
   out_210116950988546947[187] = 0;
   out_210116950988546947[188] = 0;
   out_210116950988546947[189] = 0;
   out_210116950988546947[190] = 1;
   out_210116950988546947[191] = 0;
   out_210116950988546947[192] = 0;
   out_210116950988546947[193] = 0;
   out_210116950988546947[194] = 0;
   out_210116950988546947[195] = 0;
   out_210116950988546947[196] = 0;
   out_210116950988546947[197] = 0;
   out_210116950988546947[198] = 0;
   out_210116950988546947[199] = 0;
   out_210116950988546947[200] = 0;
   out_210116950988546947[201] = 0;
   out_210116950988546947[202] = 0;
   out_210116950988546947[203] = 0;
   out_210116950988546947[204] = 0;
   out_210116950988546947[205] = 0;
   out_210116950988546947[206] = 0;
   out_210116950988546947[207] = 0;
   out_210116950988546947[208] = 0;
   out_210116950988546947[209] = 1;
   out_210116950988546947[210] = 0;
   out_210116950988546947[211] = 0;
   out_210116950988546947[212] = 0;
   out_210116950988546947[213] = 0;
   out_210116950988546947[214] = 0;
   out_210116950988546947[215] = 0;
   out_210116950988546947[216] = 0;
   out_210116950988546947[217] = 0;
   out_210116950988546947[218] = 0;
   out_210116950988546947[219] = 0;
   out_210116950988546947[220] = 0;
   out_210116950988546947[221] = 0;
   out_210116950988546947[222] = 0;
   out_210116950988546947[223] = 0;
   out_210116950988546947[224] = 0;
   out_210116950988546947[225] = 0;
   out_210116950988546947[226] = 0;
   out_210116950988546947[227] = 0;
   out_210116950988546947[228] = 1;
   out_210116950988546947[229] = 0;
   out_210116950988546947[230] = 0;
   out_210116950988546947[231] = 0;
   out_210116950988546947[232] = 0;
   out_210116950988546947[233] = 0;
   out_210116950988546947[234] = 0;
   out_210116950988546947[235] = 0;
   out_210116950988546947[236] = 0;
   out_210116950988546947[237] = 0;
   out_210116950988546947[238] = 0;
   out_210116950988546947[239] = 0;
   out_210116950988546947[240] = 0;
   out_210116950988546947[241] = 0;
   out_210116950988546947[242] = 0;
   out_210116950988546947[243] = 0;
   out_210116950988546947[244] = 0;
   out_210116950988546947[245] = 0;
   out_210116950988546947[246] = 0;
   out_210116950988546947[247] = 1;
   out_210116950988546947[248] = 0;
   out_210116950988546947[249] = 0;
   out_210116950988546947[250] = 0;
   out_210116950988546947[251] = 0;
   out_210116950988546947[252] = 0;
   out_210116950988546947[253] = 0;
   out_210116950988546947[254] = 0;
   out_210116950988546947[255] = 0;
   out_210116950988546947[256] = 0;
   out_210116950988546947[257] = 0;
   out_210116950988546947[258] = 0;
   out_210116950988546947[259] = 0;
   out_210116950988546947[260] = 0;
   out_210116950988546947[261] = 0;
   out_210116950988546947[262] = 0;
   out_210116950988546947[263] = 0;
   out_210116950988546947[264] = 0;
   out_210116950988546947[265] = 0;
   out_210116950988546947[266] = 1;
   out_210116950988546947[267] = 0;
   out_210116950988546947[268] = 0;
   out_210116950988546947[269] = 0;
   out_210116950988546947[270] = 0;
   out_210116950988546947[271] = 0;
   out_210116950988546947[272] = 0;
   out_210116950988546947[273] = 0;
   out_210116950988546947[274] = 0;
   out_210116950988546947[275] = 0;
   out_210116950988546947[276] = 0;
   out_210116950988546947[277] = 0;
   out_210116950988546947[278] = 0;
   out_210116950988546947[279] = 0;
   out_210116950988546947[280] = 0;
   out_210116950988546947[281] = 0;
   out_210116950988546947[282] = 0;
   out_210116950988546947[283] = 0;
   out_210116950988546947[284] = 0;
   out_210116950988546947[285] = 1;
   out_210116950988546947[286] = 0;
   out_210116950988546947[287] = 0;
   out_210116950988546947[288] = 0;
   out_210116950988546947[289] = 0;
   out_210116950988546947[290] = 0;
   out_210116950988546947[291] = 0;
   out_210116950988546947[292] = 0;
   out_210116950988546947[293] = 0;
   out_210116950988546947[294] = 0;
   out_210116950988546947[295] = 0;
   out_210116950988546947[296] = 0;
   out_210116950988546947[297] = 0;
   out_210116950988546947[298] = 0;
   out_210116950988546947[299] = 0;
   out_210116950988546947[300] = 0;
   out_210116950988546947[301] = 0;
   out_210116950988546947[302] = 0;
   out_210116950988546947[303] = 0;
   out_210116950988546947[304] = 1;
   out_210116950988546947[305] = 0;
   out_210116950988546947[306] = 0;
   out_210116950988546947[307] = 0;
   out_210116950988546947[308] = 0;
   out_210116950988546947[309] = 0;
   out_210116950988546947[310] = 0;
   out_210116950988546947[311] = 0;
   out_210116950988546947[312] = 0;
   out_210116950988546947[313] = 0;
   out_210116950988546947[314] = 0;
   out_210116950988546947[315] = 0;
   out_210116950988546947[316] = 0;
   out_210116950988546947[317] = 0;
   out_210116950988546947[318] = 0;
   out_210116950988546947[319] = 0;
   out_210116950988546947[320] = 0;
   out_210116950988546947[321] = 0;
   out_210116950988546947[322] = 0;
   out_210116950988546947[323] = 1;
}
void h_4(double *state, double *unused, double *out_7648624411257127562) {
   out_7648624411257127562[0] = state[6] + state[9];
   out_7648624411257127562[1] = state[7] + state[10];
   out_7648624411257127562[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_3959890883136447860) {
   out_3959890883136447860[0] = 0;
   out_3959890883136447860[1] = 0;
   out_3959890883136447860[2] = 0;
   out_3959890883136447860[3] = 0;
   out_3959890883136447860[4] = 0;
   out_3959890883136447860[5] = 0;
   out_3959890883136447860[6] = 1;
   out_3959890883136447860[7] = 0;
   out_3959890883136447860[8] = 0;
   out_3959890883136447860[9] = 1;
   out_3959890883136447860[10] = 0;
   out_3959890883136447860[11] = 0;
   out_3959890883136447860[12] = 0;
   out_3959890883136447860[13] = 0;
   out_3959890883136447860[14] = 0;
   out_3959890883136447860[15] = 0;
   out_3959890883136447860[16] = 0;
   out_3959890883136447860[17] = 0;
   out_3959890883136447860[18] = 0;
   out_3959890883136447860[19] = 0;
   out_3959890883136447860[20] = 0;
   out_3959890883136447860[21] = 0;
   out_3959890883136447860[22] = 0;
   out_3959890883136447860[23] = 0;
   out_3959890883136447860[24] = 0;
   out_3959890883136447860[25] = 1;
   out_3959890883136447860[26] = 0;
   out_3959890883136447860[27] = 0;
   out_3959890883136447860[28] = 1;
   out_3959890883136447860[29] = 0;
   out_3959890883136447860[30] = 0;
   out_3959890883136447860[31] = 0;
   out_3959890883136447860[32] = 0;
   out_3959890883136447860[33] = 0;
   out_3959890883136447860[34] = 0;
   out_3959890883136447860[35] = 0;
   out_3959890883136447860[36] = 0;
   out_3959890883136447860[37] = 0;
   out_3959890883136447860[38] = 0;
   out_3959890883136447860[39] = 0;
   out_3959890883136447860[40] = 0;
   out_3959890883136447860[41] = 0;
   out_3959890883136447860[42] = 0;
   out_3959890883136447860[43] = 0;
   out_3959890883136447860[44] = 1;
   out_3959890883136447860[45] = 0;
   out_3959890883136447860[46] = 0;
   out_3959890883136447860[47] = 1;
   out_3959890883136447860[48] = 0;
   out_3959890883136447860[49] = 0;
   out_3959890883136447860[50] = 0;
   out_3959890883136447860[51] = 0;
   out_3959890883136447860[52] = 0;
   out_3959890883136447860[53] = 0;
}
void h_10(double *state, double *unused, double *out_6099133755902511145) {
   out_6099133755902511145[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_6099133755902511145[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_6099133755902511145[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_2569809615037315587) {
   out_2569809615037315587[0] = 0;
   out_2569809615037315587[1] = 9.8100000000000005*cos(state[1]);
   out_2569809615037315587[2] = 0;
   out_2569809615037315587[3] = 0;
   out_2569809615037315587[4] = -state[8];
   out_2569809615037315587[5] = state[7];
   out_2569809615037315587[6] = 0;
   out_2569809615037315587[7] = state[5];
   out_2569809615037315587[8] = -state[4];
   out_2569809615037315587[9] = 0;
   out_2569809615037315587[10] = 0;
   out_2569809615037315587[11] = 0;
   out_2569809615037315587[12] = 1;
   out_2569809615037315587[13] = 0;
   out_2569809615037315587[14] = 0;
   out_2569809615037315587[15] = 1;
   out_2569809615037315587[16] = 0;
   out_2569809615037315587[17] = 0;
   out_2569809615037315587[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_2569809615037315587[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_2569809615037315587[20] = 0;
   out_2569809615037315587[21] = state[8];
   out_2569809615037315587[22] = 0;
   out_2569809615037315587[23] = -state[6];
   out_2569809615037315587[24] = -state[5];
   out_2569809615037315587[25] = 0;
   out_2569809615037315587[26] = state[3];
   out_2569809615037315587[27] = 0;
   out_2569809615037315587[28] = 0;
   out_2569809615037315587[29] = 0;
   out_2569809615037315587[30] = 0;
   out_2569809615037315587[31] = 1;
   out_2569809615037315587[32] = 0;
   out_2569809615037315587[33] = 0;
   out_2569809615037315587[34] = 1;
   out_2569809615037315587[35] = 0;
   out_2569809615037315587[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_2569809615037315587[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_2569809615037315587[38] = 0;
   out_2569809615037315587[39] = -state[7];
   out_2569809615037315587[40] = state[6];
   out_2569809615037315587[41] = 0;
   out_2569809615037315587[42] = state[4];
   out_2569809615037315587[43] = -state[3];
   out_2569809615037315587[44] = 0;
   out_2569809615037315587[45] = 0;
   out_2569809615037315587[46] = 0;
   out_2569809615037315587[47] = 0;
   out_2569809615037315587[48] = 0;
   out_2569809615037315587[49] = 0;
   out_2569809615037315587[50] = 1;
   out_2569809615037315587[51] = 0;
   out_2569809615037315587[52] = 0;
   out_2569809615037315587[53] = 1;
}
void h_13(double *state, double *unused, double *out_2653699383913433253) {
   out_2653699383913433253[0] = state[3];
   out_2653699383913433253[1] = state[4];
   out_2653699383913433253[2] = state[5];
}
void H_13(double *state, double *unused, double *out_747617057804115059) {
   out_747617057804115059[0] = 0;
   out_747617057804115059[1] = 0;
   out_747617057804115059[2] = 0;
   out_747617057804115059[3] = 1;
   out_747617057804115059[4] = 0;
   out_747617057804115059[5] = 0;
   out_747617057804115059[6] = 0;
   out_747617057804115059[7] = 0;
   out_747617057804115059[8] = 0;
   out_747617057804115059[9] = 0;
   out_747617057804115059[10] = 0;
   out_747617057804115059[11] = 0;
   out_747617057804115059[12] = 0;
   out_747617057804115059[13] = 0;
   out_747617057804115059[14] = 0;
   out_747617057804115059[15] = 0;
   out_747617057804115059[16] = 0;
   out_747617057804115059[17] = 0;
   out_747617057804115059[18] = 0;
   out_747617057804115059[19] = 0;
   out_747617057804115059[20] = 0;
   out_747617057804115059[21] = 0;
   out_747617057804115059[22] = 1;
   out_747617057804115059[23] = 0;
   out_747617057804115059[24] = 0;
   out_747617057804115059[25] = 0;
   out_747617057804115059[26] = 0;
   out_747617057804115059[27] = 0;
   out_747617057804115059[28] = 0;
   out_747617057804115059[29] = 0;
   out_747617057804115059[30] = 0;
   out_747617057804115059[31] = 0;
   out_747617057804115059[32] = 0;
   out_747617057804115059[33] = 0;
   out_747617057804115059[34] = 0;
   out_747617057804115059[35] = 0;
   out_747617057804115059[36] = 0;
   out_747617057804115059[37] = 0;
   out_747617057804115059[38] = 0;
   out_747617057804115059[39] = 0;
   out_747617057804115059[40] = 0;
   out_747617057804115059[41] = 1;
   out_747617057804115059[42] = 0;
   out_747617057804115059[43] = 0;
   out_747617057804115059[44] = 0;
   out_747617057804115059[45] = 0;
   out_747617057804115059[46] = 0;
   out_747617057804115059[47] = 0;
   out_747617057804115059[48] = 0;
   out_747617057804115059[49] = 0;
   out_747617057804115059[50] = 0;
   out_747617057804115059[51] = 0;
   out_747617057804115059[52] = 0;
   out_747617057804115059[53] = 0;
}
void h_14(double *state, double *unused, double *out_7344654912082976031) {
   out_7344654912082976031[0] = state[6];
   out_7344654912082976031[1] = state[7];
   out_7344654912082976031[2] = state[8];
}
void H_14(double *state, double *unused, double *out_3349973203036669) {
   out_3349973203036669[0] = 0;
   out_3349973203036669[1] = 0;
   out_3349973203036669[2] = 0;
   out_3349973203036669[3] = 0;
   out_3349973203036669[4] = 0;
   out_3349973203036669[5] = 0;
   out_3349973203036669[6] = 1;
   out_3349973203036669[7] = 0;
   out_3349973203036669[8] = 0;
   out_3349973203036669[9] = 0;
   out_3349973203036669[10] = 0;
   out_3349973203036669[11] = 0;
   out_3349973203036669[12] = 0;
   out_3349973203036669[13] = 0;
   out_3349973203036669[14] = 0;
   out_3349973203036669[15] = 0;
   out_3349973203036669[16] = 0;
   out_3349973203036669[17] = 0;
   out_3349973203036669[18] = 0;
   out_3349973203036669[19] = 0;
   out_3349973203036669[20] = 0;
   out_3349973203036669[21] = 0;
   out_3349973203036669[22] = 0;
   out_3349973203036669[23] = 0;
   out_3349973203036669[24] = 0;
   out_3349973203036669[25] = 1;
   out_3349973203036669[26] = 0;
   out_3349973203036669[27] = 0;
   out_3349973203036669[28] = 0;
   out_3349973203036669[29] = 0;
   out_3349973203036669[30] = 0;
   out_3349973203036669[31] = 0;
   out_3349973203036669[32] = 0;
   out_3349973203036669[33] = 0;
   out_3349973203036669[34] = 0;
   out_3349973203036669[35] = 0;
   out_3349973203036669[36] = 0;
   out_3349973203036669[37] = 0;
   out_3349973203036669[38] = 0;
   out_3349973203036669[39] = 0;
   out_3349973203036669[40] = 0;
   out_3349973203036669[41] = 0;
   out_3349973203036669[42] = 0;
   out_3349973203036669[43] = 0;
   out_3349973203036669[44] = 1;
   out_3349973203036669[45] = 0;
   out_3349973203036669[46] = 0;
   out_3349973203036669[47] = 0;
   out_3349973203036669[48] = 0;
   out_3349973203036669[49] = 0;
   out_3349973203036669[50] = 0;
   out_3349973203036669[51] = 0;
   out_3349973203036669[52] = 0;
   out_3349973203036669[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_3212432162194349052) {
  err_fun(nom_x, delta_x, out_3212432162194349052);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_8973271523308122061) {
  inv_err_fun(nom_x, true_x, out_8973271523308122061);
}
void pose_H_mod_fun(double *state, double *out_7731698092980355432) {
  H_mod_fun(state, out_7731698092980355432);
}
void pose_f_fun(double *state, double dt, double *out_8923016311880444304) {
  f_fun(state,  dt, out_8923016311880444304);
}
void pose_F_fun(double *state, double dt, double *out_210116950988546947) {
  F_fun(state,  dt, out_210116950988546947);
}
void pose_h_4(double *state, double *unused, double *out_7648624411257127562) {
  h_4(state, unused, out_7648624411257127562);
}
void pose_H_4(double *state, double *unused, double *out_3959890883136447860) {
  H_4(state, unused, out_3959890883136447860);
}
void pose_h_10(double *state, double *unused, double *out_6099133755902511145) {
  h_10(state, unused, out_6099133755902511145);
}
void pose_H_10(double *state, double *unused, double *out_2569809615037315587) {
  H_10(state, unused, out_2569809615037315587);
}
void pose_h_13(double *state, double *unused, double *out_2653699383913433253) {
  h_13(state, unused, out_2653699383913433253);
}
void pose_H_13(double *state, double *unused, double *out_747617057804115059) {
  H_13(state, unused, out_747617057804115059);
}
void pose_h_14(double *state, double *unused, double *out_7344654912082976031) {
  h_14(state, unused, out_7344654912082976031);
}
void pose_H_14(double *state, double *unused, double *out_3349973203036669) {
  H_14(state, unused, out_3349973203036669);
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

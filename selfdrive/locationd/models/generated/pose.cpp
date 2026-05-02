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
void err_fun(double *nom_x, double *delta_x, double *out_1677822181415374238) {
   out_1677822181415374238[0] = delta_x[0] + nom_x[0];
   out_1677822181415374238[1] = delta_x[1] + nom_x[1];
   out_1677822181415374238[2] = delta_x[2] + nom_x[2];
   out_1677822181415374238[3] = delta_x[3] + nom_x[3];
   out_1677822181415374238[4] = delta_x[4] + nom_x[4];
   out_1677822181415374238[5] = delta_x[5] + nom_x[5];
   out_1677822181415374238[6] = delta_x[6] + nom_x[6];
   out_1677822181415374238[7] = delta_x[7] + nom_x[7];
   out_1677822181415374238[8] = delta_x[8] + nom_x[8];
   out_1677822181415374238[9] = delta_x[9] + nom_x[9];
   out_1677822181415374238[10] = delta_x[10] + nom_x[10];
   out_1677822181415374238[11] = delta_x[11] + nom_x[11];
   out_1677822181415374238[12] = delta_x[12] + nom_x[12];
   out_1677822181415374238[13] = delta_x[13] + nom_x[13];
   out_1677822181415374238[14] = delta_x[14] + nom_x[14];
   out_1677822181415374238[15] = delta_x[15] + nom_x[15];
   out_1677822181415374238[16] = delta_x[16] + nom_x[16];
   out_1677822181415374238[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1645070502606612381) {
   out_1645070502606612381[0] = -nom_x[0] + true_x[0];
   out_1645070502606612381[1] = -nom_x[1] + true_x[1];
   out_1645070502606612381[2] = -nom_x[2] + true_x[2];
   out_1645070502606612381[3] = -nom_x[3] + true_x[3];
   out_1645070502606612381[4] = -nom_x[4] + true_x[4];
   out_1645070502606612381[5] = -nom_x[5] + true_x[5];
   out_1645070502606612381[6] = -nom_x[6] + true_x[6];
   out_1645070502606612381[7] = -nom_x[7] + true_x[7];
   out_1645070502606612381[8] = -nom_x[8] + true_x[8];
   out_1645070502606612381[9] = -nom_x[9] + true_x[9];
   out_1645070502606612381[10] = -nom_x[10] + true_x[10];
   out_1645070502606612381[11] = -nom_x[11] + true_x[11];
   out_1645070502606612381[12] = -nom_x[12] + true_x[12];
   out_1645070502606612381[13] = -nom_x[13] + true_x[13];
   out_1645070502606612381[14] = -nom_x[14] + true_x[14];
   out_1645070502606612381[15] = -nom_x[15] + true_x[15];
   out_1645070502606612381[16] = -nom_x[16] + true_x[16];
   out_1645070502606612381[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_6168804416891607091) {
   out_6168804416891607091[0] = 1.0;
   out_6168804416891607091[1] = 0.0;
   out_6168804416891607091[2] = 0.0;
   out_6168804416891607091[3] = 0.0;
   out_6168804416891607091[4] = 0.0;
   out_6168804416891607091[5] = 0.0;
   out_6168804416891607091[6] = 0.0;
   out_6168804416891607091[7] = 0.0;
   out_6168804416891607091[8] = 0.0;
   out_6168804416891607091[9] = 0.0;
   out_6168804416891607091[10] = 0.0;
   out_6168804416891607091[11] = 0.0;
   out_6168804416891607091[12] = 0.0;
   out_6168804416891607091[13] = 0.0;
   out_6168804416891607091[14] = 0.0;
   out_6168804416891607091[15] = 0.0;
   out_6168804416891607091[16] = 0.0;
   out_6168804416891607091[17] = 0.0;
   out_6168804416891607091[18] = 0.0;
   out_6168804416891607091[19] = 1.0;
   out_6168804416891607091[20] = 0.0;
   out_6168804416891607091[21] = 0.0;
   out_6168804416891607091[22] = 0.0;
   out_6168804416891607091[23] = 0.0;
   out_6168804416891607091[24] = 0.0;
   out_6168804416891607091[25] = 0.0;
   out_6168804416891607091[26] = 0.0;
   out_6168804416891607091[27] = 0.0;
   out_6168804416891607091[28] = 0.0;
   out_6168804416891607091[29] = 0.0;
   out_6168804416891607091[30] = 0.0;
   out_6168804416891607091[31] = 0.0;
   out_6168804416891607091[32] = 0.0;
   out_6168804416891607091[33] = 0.0;
   out_6168804416891607091[34] = 0.0;
   out_6168804416891607091[35] = 0.0;
   out_6168804416891607091[36] = 0.0;
   out_6168804416891607091[37] = 0.0;
   out_6168804416891607091[38] = 1.0;
   out_6168804416891607091[39] = 0.0;
   out_6168804416891607091[40] = 0.0;
   out_6168804416891607091[41] = 0.0;
   out_6168804416891607091[42] = 0.0;
   out_6168804416891607091[43] = 0.0;
   out_6168804416891607091[44] = 0.0;
   out_6168804416891607091[45] = 0.0;
   out_6168804416891607091[46] = 0.0;
   out_6168804416891607091[47] = 0.0;
   out_6168804416891607091[48] = 0.0;
   out_6168804416891607091[49] = 0.0;
   out_6168804416891607091[50] = 0.0;
   out_6168804416891607091[51] = 0.0;
   out_6168804416891607091[52] = 0.0;
   out_6168804416891607091[53] = 0.0;
   out_6168804416891607091[54] = 0.0;
   out_6168804416891607091[55] = 0.0;
   out_6168804416891607091[56] = 0.0;
   out_6168804416891607091[57] = 1.0;
   out_6168804416891607091[58] = 0.0;
   out_6168804416891607091[59] = 0.0;
   out_6168804416891607091[60] = 0.0;
   out_6168804416891607091[61] = 0.0;
   out_6168804416891607091[62] = 0.0;
   out_6168804416891607091[63] = 0.0;
   out_6168804416891607091[64] = 0.0;
   out_6168804416891607091[65] = 0.0;
   out_6168804416891607091[66] = 0.0;
   out_6168804416891607091[67] = 0.0;
   out_6168804416891607091[68] = 0.0;
   out_6168804416891607091[69] = 0.0;
   out_6168804416891607091[70] = 0.0;
   out_6168804416891607091[71] = 0.0;
   out_6168804416891607091[72] = 0.0;
   out_6168804416891607091[73] = 0.0;
   out_6168804416891607091[74] = 0.0;
   out_6168804416891607091[75] = 0.0;
   out_6168804416891607091[76] = 1.0;
   out_6168804416891607091[77] = 0.0;
   out_6168804416891607091[78] = 0.0;
   out_6168804416891607091[79] = 0.0;
   out_6168804416891607091[80] = 0.0;
   out_6168804416891607091[81] = 0.0;
   out_6168804416891607091[82] = 0.0;
   out_6168804416891607091[83] = 0.0;
   out_6168804416891607091[84] = 0.0;
   out_6168804416891607091[85] = 0.0;
   out_6168804416891607091[86] = 0.0;
   out_6168804416891607091[87] = 0.0;
   out_6168804416891607091[88] = 0.0;
   out_6168804416891607091[89] = 0.0;
   out_6168804416891607091[90] = 0.0;
   out_6168804416891607091[91] = 0.0;
   out_6168804416891607091[92] = 0.0;
   out_6168804416891607091[93] = 0.0;
   out_6168804416891607091[94] = 0.0;
   out_6168804416891607091[95] = 1.0;
   out_6168804416891607091[96] = 0.0;
   out_6168804416891607091[97] = 0.0;
   out_6168804416891607091[98] = 0.0;
   out_6168804416891607091[99] = 0.0;
   out_6168804416891607091[100] = 0.0;
   out_6168804416891607091[101] = 0.0;
   out_6168804416891607091[102] = 0.0;
   out_6168804416891607091[103] = 0.0;
   out_6168804416891607091[104] = 0.0;
   out_6168804416891607091[105] = 0.0;
   out_6168804416891607091[106] = 0.0;
   out_6168804416891607091[107] = 0.0;
   out_6168804416891607091[108] = 0.0;
   out_6168804416891607091[109] = 0.0;
   out_6168804416891607091[110] = 0.0;
   out_6168804416891607091[111] = 0.0;
   out_6168804416891607091[112] = 0.0;
   out_6168804416891607091[113] = 0.0;
   out_6168804416891607091[114] = 1.0;
   out_6168804416891607091[115] = 0.0;
   out_6168804416891607091[116] = 0.0;
   out_6168804416891607091[117] = 0.0;
   out_6168804416891607091[118] = 0.0;
   out_6168804416891607091[119] = 0.0;
   out_6168804416891607091[120] = 0.0;
   out_6168804416891607091[121] = 0.0;
   out_6168804416891607091[122] = 0.0;
   out_6168804416891607091[123] = 0.0;
   out_6168804416891607091[124] = 0.0;
   out_6168804416891607091[125] = 0.0;
   out_6168804416891607091[126] = 0.0;
   out_6168804416891607091[127] = 0.0;
   out_6168804416891607091[128] = 0.0;
   out_6168804416891607091[129] = 0.0;
   out_6168804416891607091[130] = 0.0;
   out_6168804416891607091[131] = 0.0;
   out_6168804416891607091[132] = 0.0;
   out_6168804416891607091[133] = 1.0;
   out_6168804416891607091[134] = 0.0;
   out_6168804416891607091[135] = 0.0;
   out_6168804416891607091[136] = 0.0;
   out_6168804416891607091[137] = 0.0;
   out_6168804416891607091[138] = 0.0;
   out_6168804416891607091[139] = 0.0;
   out_6168804416891607091[140] = 0.0;
   out_6168804416891607091[141] = 0.0;
   out_6168804416891607091[142] = 0.0;
   out_6168804416891607091[143] = 0.0;
   out_6168804416891607091[144] = 0.0;
   out_6168804416891607091[145] = 0.0;
   out_6168804416891607091[146] = 0.0;
   out_6168804416891607091[147] = 0.0;
   out_6168804416891607091[148] = 0.0;
   out_6168804416891607091[149] = 0.0;
   out_6168804416891607091[150] = 0.0;
   out_6168804416891607091[151] = 0.0;
   out_6168804416891607091[152] = 1.0;
   out_6168804416891607091[153] = 0.0;
   out_6168804416891607091[154] = 0.0;
   out_6168804416891607091[155] = 0.0;
   out_6168804416891607091[156] = 0.0;
   out_6168804416891607091[157] = 0.0;
   out_6168804416891607091[158] = 0.0;
   out_6168804416891607091[159] = 0.0;
   out_6168804416891607091[160] = 0.0;
   out_6168804416891607091[161] = 0.0;
   out_6168804416891607091[162] = 0.0;
   out_6168804416891607091[163] = 0.0;
   out_6168804416891607091[164] = 0.0;
   out_6168804416891607091[165] = 0.0;
   out_6168804416891607091[166] = 0.0;
   out_6168804416891607091[167] = 0.0;
   out_6168804416891607091[168] = 0.0;
   out_6168804416891607091[169] = 0.0;
   out_6168804416891607091[170] = 0.0;
   out_6168804416891607091[171] = 1.0;
   out_6168804416891607091[172] = 0.0;
   out_6168804416891607091[173] = 0.0;
   out_6168804416891607091[174] = 0.0;
   out_6168804416891607091[175] = 0.0;
   out_6168804416891607091[176] = 0.0;
   out_6168804416891607091[177] = 0.0;
   out_6168804416891607091[178] = 0.0;
   out_6168804416891607091[179] = 0.0;
   out_6168804416891607091[180] = 0.0;
   out_6168804416891607091[181] = 0.0;
   out_6168804416891607091[182] = 0.0;
   out_6168804416891607091[183] = 0.0;
   out_6168804416891607091[184] = 0.0;
   out_6168804416891607091[185] = 0.0;
   out_6168804416891607091[186] = 0.0;
   out_6168804416891607091[187] = 0.0;
   out_6168804416891607091[188] = 0.0;
   out_6168804416891607091[189] = 0.0;
   out_6168804416891607091[190] = 1.0;
   out_6168804416891607091[191] = 0.0;
   out_6168804416891607091[192] = 0.0;
   out_6168804416891607091[193] = 0.0;
   out_6168804416891607091[194] = 0.0;
   out_6168804416891607091[195] = 0.0;
   out_6168804416891607091[196] = 0.0;
   out_6168804416891607091[197] = 0.0;
   out_6168804416891607091[198] = 0.0;
   out_6168804416891607091[199] = 0.0;
   out_6168804416891607091[200] = 0.0;
   out_6168804416891607091[201] = 0.0;
   out_6168804416891607091[202] = 0.0;
   out_6168804416891607091[203] = 0.0;
   out_6168804416891607091[204] = 0.0;
   out_6168804416891607091[205] = 0.0;
   out_6168804416891607091[206] = 0.0;
   out_6168804416891607091[207] = 0.0;
   out_6168804416891607091[208] = 0.0;
   out_6168804416891607091[209] = 1.0;
   out_6168804416891607091[210] = 0.0;
   out_6168804416891607091[211] = 0.0;
   out_6168804416891607091[212] = 0.0;
   out_6168804416891607091[213] = 0.0;
   out_6168804416891607091[214] = 0.0;
   out_6168804416891607091[215] = 0.0;
   out_6168804416891607091[216] = 0.0;
   out_6168804416891607091[217] = 0.0;
   out_6168804416891607091[218] = 0.0;
   out_6168804416891607091[219] = 0.0;
   out_6168804416891607091[220] = 0.0;
   out_6168804416891607091[221] = 0.0;
   out_6168804416891607091[222] = 0.0;
   out_6168804416891607091[223] = 0.0;
   out_6168804416891607091[224] = 0.0;
   out_6168804416891607091[225] = 0.0;
   out_6168804416891607091[226] = 0.0;
   out_6168804416891607091[227] = 0.0;
   out_6168804416891607091[228] = 1.0;
   out_6168804416891607091[229] = 0.0;
   out_6168804416891607091[230] = 0.0;
   out_6168804416891607091[231] = 0.0;
   out_6168804416891607091[232] = 0.0;
   out_6168804416891607091[233] = 0.0;
   out_6168804416891607091[234] = 0.0;
   out_6168804416891607091[235] = 0.0;
   out_6168804416891607091[236] = 0.0;
   out_6168804416891607091[237] = 0.0;
   out_6168804416891607091[238] = 0.0;
   out_6168804416891607091[239] = 0.0;
   out_6168804416891607091[240] = 0.0;
   out_6168804416891607091[241] = 0.0;
   out_6168804416891607091[242] = 0.0;
   out_6168804416891607091[243] = 0.0;
   out_6168804416891607091[244] = 0.0;
   out_6168804416891607091[245] = 0.0;
   out_6168804416891607091[246] = 0.0;
   out_6168804416891607091[247] = 1.0;
   out_6168804416891607091[248] = 0.0;
   out_6168804416891607091[249] = 0.0;
   out_6168804416891607091[250] = 0.0;
   out_6168804416891607091[251] = 0.0;
   out_6168804416891607091[252] = 0.0;
   out_6168804416891607091[253] = 0.0;
   out_6168804416891607091[254] = 0.0;
   out_6168804416891607091[255] = 0.0;
   out_6168804416891607091[256] = 0.0;
   out_6168804416891607091[257] = 0.0;
   out_6168804416891607091[258] = 0.0;
   out_6168804416891607091[259] = 0.0;
   out_6168804416891607091[260] = 0.0;
   out_6168804416891607091[261] = 0.0;
   out_6168804416891607091[262] = 0.0;
   out_6168804416891607091[263] = 0.0;
   out_6168804416891607091[264] = 0.0;
   out_6168804416891607091[265] = 0.0;
   out_6168804416891607091[266] = 1.0;
   out_6168804416891607091[267] = 0.0;
   out_6168804416891607091[268] = 0.0;
   out_6168804416891607091[269] = 0.0;
   out_6168804416891607091[270] = 0.0;
   out_6168804416891607091[271] = 0.0;
   out_6168804416891607091[272] = 0.0;
   out_6168804416891607091[273] = 0.0;
   out_6168804416891607091[274] = 0.0;
   out_6168804416891607091[275] = 0.0;
   out_6168804416891607091[276] = 0.0;
   out_6168804416891607091[277] = 0.0;
   out_6168804416891607091[278] = 0.0;
   out_6168804416891607091[279] = 0.0;
   out_6168804416891607091[280] = 0.0;
   out_6168804416891607091[281] = 0.0;
   out_6168804416891607091[282] = 0.0;
   out_6168804416891607091[283] = 0.0;
   out_6168804416891607091[284] = 0.0;
   out_6168804416891607091[285] = 1.0;
   out_6168804416891607091[286] = 0.0;
   out_6168804416891607091[287] = 0.0;
   out_6168804416891607091[288] = 0.0;
   out_6168804416891607091[289] = 0.0;
   out_6168804416891607091[290] = 0.0;
   out_6168804416891607091[291] = 0.0;
   out_6168804416891607091[292] = 0.0;
   out_6168804416891607091[293] = 0.0;
   out_6168804416891607091[294] = 0.0;
   out_6168804416891607091[295] = 0.0;
   out_6168804416891607091[296] = 0.0;
   out_6168804416891607091[297] = 0.0;
   out_6168804416891607091[298] = 0.0;
   out_6168804416891607091[299] = 0.0;
   out_6168804416891607091[300] = 0.0;
   out_6168804416891607091[301] = 0.0;
   out_6168804416891607091[302] = 0.0;
   out_6168804416891607091[303] = 0.0;
   out_6168804416891607091[304] = 1.0;
   out_6168804416891607091[305] = 0.0;
   out_6168804416891607091[306] = 0.0;
   out_6168804416891607091[307] = 0.0;
   out_6168804416891607091[308] = 0.0;
   out_6168804416891607091[309] = 0.0;
   out_6168804416891607091[310] = 0.0;
   out_6168804416891607091[311] = 0.0;
   out_6168804416891607091[312] = 0.0;
   out_6168804416891607091[313] = 0.0;
   out_6168804416891607091[314] = 0.0;
   out_6168804416891607091[315] = 0.0;
   out_6168804416891607091[316] = 0.0;
   out_6168804416891607091[317] = 0.0;
   out_6168804416891607091[318] = 0.0;
   out_6168804416891607091[319] = 0.0;
   out_6168804416891607091[320] = 0.0;
   out_6168804416891607091[321] = 0.0;
   out_6168804416891607091[322] = 0.0;
   out_6168804416891607091[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_4562479811319953042) {
   out_4562479811319953042[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_4562479811319953042[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_4562479811319953042[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_4562479811319953042[3] = dt*state[12] + state[3];
   out_4562479811319953042[4] = dt*state[13] + state[4];
   out_4562479811319953042[5] = dt*state[14] + state[5];
   out_4562479811319953042[6] = state[6];
   out_4562479811319953042[7] = state[7];
   out_4562479811319953042[8] = state[8];
   out_4562479811319953042[9] = state[9];
   out_4562479811319953042[10] = state[10];
   out_4562479811319953042[11] = state[11];
   out_4562479811319953042[12] = state[12];
   out_4562479811319953042[13] = state[13];
   out_4562479811319953042[14] = state[14];
   out_4562479811319953042[15] = state[15];
   out_4562479811319953042[16] = state[16];
   out_4562479811319953042[17] = state[17];
}
void F_fun(double *state, double dt, double *out_1994404627029761325) {
   out_1994404627029761325[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1994404627029761325[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1994404627029761325[2] = 0;
   out_1994404627029761325[3] = 0;
   out_1994404627029761325[4] = 0;
   out_1994404627029761325[5] = 0;
   out_1994404627029761325[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1994404627029761325[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1994404627029761325[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_1994404627029761325[9] = 0;
   out_1994404627029761325[10] = 0;
   out_1994404627029761325[11] = 0;
   out_1994404627029761325[12] = 0;
   out_1994404627029761325[13] = 0;
   out_1994404627029761325[14] = 0;
   out_1994404627029761325[15] = 0;
   out_1994404627029761325[16] = 0;
   out_1994404627029761325[17] = 0;
   out_1994404627029761325[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1994404627029761325[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1994404627029761325[20] = 0;
   out_1994404627029761325[21] = 0;
   out_1994404627029761325[22] = 0;
   out_1994404627029761325[23] = 0;
   out_1994404627029761325[24] = 0;
   out_1994404627029761325[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1994404627029761325[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_1994404627029761325[27] = 0;
   out_1994404627029761325[28] = 0;
   out_1994404627029761325[29] = 0;
   out_1994404627029761325[30] = 0;
   out_1994404627029761325[31] = 0;
   out_1994404627029761325[32] = 0;
   out_1994404627029761325[33] = 0;
   out_1994404627029761325[34] = 0;
   out_1994404627029761325[35] = 0;
   out_1994404627029761325[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1994404627029761325[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1994404627029761325[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1994404627029761325[39] = 0;
   out_1994404627029761325[40] = 0;
   out_1994404627029761325[41] = 0;
   out_1994404627029761325[42] = 0;
   out_1994404627029761325[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1994404627029761325[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_1994404627029761325[45] = 0;
   out_1994404627029761325[46] = 0;
   out_1994404627029761325[47] = 0;
   out_1994404627029761325[48] = 0;
   out_1994404627029761325[49] = 0;
   out_1994404627029761325[50] = 0;
   out_1994404627029761325[51] = 0;
   out_1994404627029761325[52] = 0;
   out_1994404627029761325[53] = 0;
   out_1994404627029761325[54] = 0;
   out_1994404627029761325[55] = 0;
   out_1994404627029761325[56] = 0;
   out_1994404627029761325[57] = 1;
   out_1994404627029761325[58] = 0;
   out_1994404627029761325[59] = 0;
   out_1994404627029761325[60] = 0;
   out_1994404627029761325[61] = 0;
   out_1994404627029761325[62] = 0;
   out_1994404627029761325[63] = 0;
   out_1994404627029761325[64] = 0;
   out_1994404627029761325[65] = 0;
   out_1994404627029761325[66] = dt;
   out_1994404627029761325[67] = 0;
   out_1994404627029761325[68] = 0;
   out_1994404627029761325[69] = 0;
   out_1994404627029761325[70] = 0;
   out_1994404627029761325[71] = 0;
   out_1994404627029761325[72] = 0;
   out_1994404627029761325[73] = 0;
   out_1994404627029761325[74] = 0;
   out_1994404627029761325[75] = 0;
   out_1994404627029761325[76] = 1;
   out_1994404627029761325[77] = 0;
   out_1994404627029761325[78] = 0;
   out_1994404627029761325[79] = 0;
   out_1994404627029761325[80] = 0;
   out_1994404627029761325[81] = 0;
   out_1994404627029761325[82] = 0;
   out_1994404627029761325[83] = 0;
   out_1994404627029761325[84] = 0;
   out_1994404627029761325[85] = dt;
   out_1994404627029761325[86] = 0;
   out_1994404627029761325[87] = 0;
   out_1994404627029761325[88] = 0;
   out_1994404627029761325[89] = 0;
   out_1994404627029761325[90] = 0;
   out_1994404627029761325[91] = 0;
   out_1994404627029761325[92] = 0;
   out_1994404627029761325[93] = 0;
   out_1994404627029761325[94] = 0;
   out_1994404627029761325[95] = 1;
   out_1994404627029761325[96] = 0;
   out_1994404627029761325[97] = 0;
   out_1994404627029761325[98] = 0;
   out_1994404627029761325[99] = 0;
   out_1994404627029761325[100] = 0;
   out_1994404627029761325[101] = 0;
   out_1994404627029761325[102] = 0;
   out_1994404627029761325[103] = 0;
   out_1994404627029761325[104] = dt;
   out_1994404627029761325[105] = 0;
   out_1994404627029761325[106] = 0;
   out_1994404627029761325[107] = 0;
   out_1994404627029761325[108] = 0;
   out_1994404627029761325[109] = 0;
   out_1994404627029761325[110] = 0;
   out_1994404627029761325[111] = 0;
   out_1994404627029761325[112] = 0;
   out_1994404627029761325[113] = 0;
   out_1994404627029761325[114] = 1;
   out_1994404627029761325[115] = 0;
   out_1994404627029761325[116] = 0;
   out_1994404627029761325[117] = 0;
   out_1994404627029761325[118] = 0;
   out_1994404627029761325[119] = 0;
   out_1994404627029761325[120] = 0;
   out_1994404627029761325[121] = 0;
   out_1994404627029761325[122] = 0;
   out_1994404627029761325[123] = 0;
   out_1994404627029761325[124] = 0;
   out_1994404627029761325[125] = 0;
   out_1994404627029761325[126] = 0;
   out_1994404627029761325[127] = 0;
   out_1994404627029761325[128] = 0;
   out_1994404627029761325[129] = 0;
   out_1994404627029761325[130] = 0;
   out_1994404627029761325[131] = 0;
   out_1994404627029761325[132] = 0;
   out_1994404627029761325[133] = 1;
   out_1994404627029761325[134] = 0;
   out_1994404627029761325[135] = 0;
   out_1994404627029761325[136] = 0;
   out_1994404627029761325[137] = 0;
   out_1994404627029761325[138] = 0;
   out_1994404627029761325[139] = 0;
   out_1994404627029761325[140] = 0;
   out_1994404627029761325[141] = 0;
   out_1994404627029761325[142] = 0;
   out_1994404627029761325[143] = 0;
   out_1994404627029761325[144] = 0;
   out_1994404627029761325[145] = 0;
   out_1994404627029761325[146] = 0;
   out_1994404627029761325[147] = 0;
   out_1994404627029761325[148] = 0;
   out_1994404627029761325[149] = 0;
   out_1994404627029761325[150] = 0;
   out_1994404627029761325[151] = 0;
   out_1994404627029761325[152] = 1;
   out_1994404627029761325[153] = 0;
   out_1994404627029761325[154] = 0;
   out_1994404627029761325[155] = 0;
   out_1994404627029761325[156] = 0;
   out_1994404627029761325[157] = 0;
   out_1994404627029761325[158] = 0;
   out_1994404627029761325[159] = 0;
   out_1994404627029761325[160] = 0;
   out_1994404627029761325[161] = 0;
   out_1994404627029761325[162] = 0;
   out_1994404627029761325[163] = 0;
   out_1994404627029761325[164] = 0;
   out_1994404627029761325[165] = 0;
   out_1994404627029761325[166] = 0;
   out_1994404627029761325[167] = 0;
   out_1994404627029761325[168] = 0;
   out_1994404627029761325[169] = 0;
   out_1994404627029761325[170] = 0;
   out_1994404627029761325[171] = 1;
   out_1994404627029761325[172] = 0;
   out_1994404627029761325[173] = 0;
   out_1994404627029761325[174] = 0;
   out_1994404627029761325[175] = 0;
   out_1994404627029761325[176] = 0;
   out_1994404627029761325[177] = 0;
   out_1994404627029761325[178] = 0;
   out_1994404627029761325[179] = 0;
   out_1994404627029761325[180] = 0;
   out_1994404627029761325[181] = 0;
   out_1994404627029761325[182] = 0;
   out_1994404627029761325[183] = 0;
   out_1994404627029761325[184] = 0;
   out_1994404627029761325[185] = 0;
   out_1994404627029761325[186] = 0;
   out_1994404627029761325[187] = 0;
   out_1994404627029761325[188] = 0;
   out_1994404627029761325[189] = 0;
   out_1994404627029761325[190] = 1;
   out_1994404627029761325[191] = 0;
   out_1994404627029761325[192] = 0;
   out_1994404627029761325[193] = 0;
   out_1994404627029761325[194] = 0;
   out_1994404627029761325[195] = 0;
   out_1994404627029761325[196] = 0;
   out_1994404627029761325[197] = 0;
   out_1994404627029761325[198] = 0;
   out_1994404627029761325[199] = 0;
   out_1994404627029761325[200] = 0;
   out_1994404627029761325[201] = 0;
   out_1994404627029761325[202] = 0;
   out_1994404627029761325[203] = 0;
   out_1994404627029761325[204] = 0;
   out_1994404627029761325[205] = 0;
   out_1994404627029761325[206] = 0;
   out_1994404627029761325[207] = 0;
   out_1994404627029761325[208] = 0;
   out_1994404627029761325[209] = 1;
   out_1994404627029761325[210] = 0;
   out_1994404627029761325[211] = 0;
   out_1994404627029761325[212] = 0;
   out_1994404627029761325[213] = 0;
   out_1994404627029761325[214] = 0;
   out_1994404627029761325[215] = 0;
   out_1994404627029761325[216] = 0;
   out_1994404627029761325[217] = 0;
   out_1994404627029761325[218] = 0;
   out_1994404627029761325[219] = 0;
   out_1994404627029761325[220] = 0;
   out_1994404627029761325[221] = 0;
   out_1994404627029761325[222] = 0;
   out_1994404627029761325[223] = 0;
   out_1994404627029761325[224] = 0;
   out_1994404627029761325[225] = 0;
   out_1994404627029761325[226] = 0;
   out_1994404627029761325[227] = 0;
   out_1994404627029761325[228] = 1;
   out_1994404627029761325[229] = 0;
   out_1994404627029761325[230] = 0;
   out_1994404627029761325[231] = 0;
   out_1994404627029761325[232] = 0;
   out_1994404627029761325[233] = 0;
   out_1994404627029761325[234] = 0;
   out_1994404627029761325[235] = 0;
   out_1994404627029761325[236] = 0;
   out_1994404627029761325[237] = 0;
   out_1994404627029761325[238] = 0;
   out_1994404627029761325[239] = 0;
   out_1994404627029761325[240] = 0;
   out_1994404627029761325[241] = 0;
   out_1994404627029761325[242] = 0;
   out_1994404627029761325[243] = 0;
   out_1994404627029761325[244] = 0;
   out_1994404627029761325[245] = 0;
   out_1994404627029761325[246] = 0;
   out_1994404627029761325[247] = 1;
   out_1994404627029761325[248] = 0;
   out_1994404627029761325[249] = 0;
   out_1994404627029761325[250] = 0;
   out_1994404627029761325[251] = 0;
   out_1994404627029761325[252] = 0;
   out_1994404627029761325[253] = 0;
   out_1994404627029761325[254] = 0;
   out_1994404627029761325[255] = 0;
   out_1994404627029761325[256] = 0;
   out_1994404627029761325[257] = 0;
   out_1994404627029761325[258] = 0;
   out_1994404627029761325[259] = 0;
   out_1994404627029761325[260] = 0;
   out_1994404627029761325[261] = 0;
   out_1994404627029761325[262] = 0;
   out_1994404627029761325[263] = 0;
   out_1994404627029761325[264] = 0;
   out_1994404627029761325[265] = 0;
   out_1994404627029761325[266] = 1;
   out_1994404627029761325[267] = 0;
   out_1994404627029761325[268] = 0;
   out_1994404627029761325[269] = 0;
   out_1994404627029761325[270] = 0;
   out_1994404627029761325[271] = 0;
   out_1994404627029761325[272] = 0;
   out_1994404627029761325[273] = 0;
   out_1994404627029761325[274] = 0;
   out_1994404627029761325[275] = 0;
   out_1994404627029761325[276] = 0;
   out_1994404627029761325[277] = 0;
   out_1994404627029761325[278] = 0;
   out_1994404627029761325[279] = 0;
   out_1994404627029761325[280] = 0;
   out_1994404627029761325[281] = 0;
   out_1994404627029761325[282] = 0;
   out_1994404627029761325[283] = 0;
   out_1994404627029761325[284] = 0;
   out_1994404627029761325[285] = 1;
   out_1994404627029761325[286] = 0;
   out_1994404627029761325[287] = 0;
   out_1994404627029761325[288] = 0;
   out_1994404627029761325[289] = 0;
   out_1994404627029761325[290] = 0;
   out_1994404627029761325[291] = 0;
   out_1994404627029761325[292] = 0;
   out_1994404627029761325[293] = 0;
   out_1994404627029761325[294] = 0;
   out_1994404627029761325[295] = 0;
   out_1994404627029761325[296] = 0;
   out_1994404627029761325[297] = 0;
   out_1994404627029761325[298] = 0;
   out_1994404627029761325[299] = 0;
   out_1994404627029761325[300] = 0;
   out_1994404627029761325[301] = 0;
   out_1994404627029761325[302] = 0;
   out_1994404627029761325[303] = 0;
   out_1994404627029761325[304] = 1;
   out_1994404627029761325[305] = 0;
   out_1994404627029761325[306] = 0;
   out_1994404627029761325[307] = 0;
   out_1994404627029761325[308] = 0;
   out_1994404627029761325[309] = 0;
   out_1994404627029761325[310] = 0;
   out_1994404627029761325[311] = 0;
   out_1994404627029761325[312] = 0;
   out_1994404627029761325[313] = 0;
   out_1994404627029761325[314] = 0;
   out_1994404627029761325[315] = 0;
   out_1994404627029761325[316] = 0;
   out_1994404627029761325[317] = 0;
   out_1994404627029761325[318] = 0;
   out_1994404627029761325[319] = 0;
   out_1994404627029761325[320] = 0;
   out_1994404627029761325[321] = 0;
   out_1994404627029761325[322] = 0;
   out_1994404627029761325[323] = 1;
}
void h_4(double *state, double *unused, double *out_8379183754462585357) {
   out_8379183754462585357[0] = state[6] + state[9];
   out_8379183754462585357[1] = state[7] + state[10];
   out_8379183754462585357[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_3264955236904260033) {
   out_3264955236904260033[0] = 0;
   out_3264955236904260033[1] = 0;
   out_3264955236904260033[2] = 0;
   out_3264955236904260033[3] = 0;
   out_3264955236904260033[4] = 0;
   out_3264955236904260033[5] = 0;
   out_3264955236904260033[6] = 1;
   out_3264955236904260033[7] = 0;
   out_3264955236904260033[8] = 0;
   out_3264955236904260033[9] = 1;
   out_3264955236904260033[10] = 0;
   out_3264955236904260033[11] = 0;
   out_3264955236904260033[12] = 0;
   out_3264955236904260033[13] = 0;
   out_3264955236904260033[14] = 0;
   out_3264955236904260033[15] = 0;
   out_3264955236904260033[16] = 0;
   out_3264955236904260033[17] = 0;
   out_3264955236904260033[18] = 0;
   out_3264955236904260033[19] = 0;
   out_3264955236904260033[20] = 0;
   out_3264955236904260033[21] = 0;
   out_3264955236904260033[22] = 0;
   out_3264955236904260033[23] = 0;
   out_3264955236904260033[24] = 0;
   out_3264955236904260033[25] = 1;
   out_3264955236904260033[26] = 0;
   out_3264955236904260033[27] = 0;
   out_3264955236904260033[28] = 1;
   out_3264955236904260033[29] = 0;
   out_3264955236904260033[30] = 0;
   out_3264955236904260033[31] = 0;
   out_3264955236904260033[32] = 0;
   out_3264955236904260033[33] = 0;
   out_3264955236904260033[34] = 0;
   out_3264955236904260033[35] = 0;
   out_3264955236904260033[36] = 0;
   out_3264955236904260033[37] = 0;
   out_3264955236904260033[38] = 0;
   out_3264955236904260033[39] = 0;
   out_3264955236904260033[40] = 0;
   out_3264955236904260033[41] = 0;
   out_3264955236904260033[42] = 0;
   out_3264955236904260033[43] = 0;
   out_3264955236904260033[44] = 1;
   out_3264955236904260033[45] = 0;
   out_3264955236904260033[46] = 0;
   out_3264955236904260033[47] = 1;
   out_3264955236904260033[48] = 0;
   out_3264955236904260033[49] = 0;
   out_3264955236904260033[50] = 0;
   out_3264955236904260033[51] = 0;
   out_3264955236904260033[52] = 0;
   out_3264955236904260033[53] = 0;
}
void h_10(double *state, double *unused, double *out_694976072985625952) {
   out_694976072985625952[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_694976072985625952[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_694976072985625952[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_5824925052510502657) {
   out_5824925052510502657[0] = 0;
   out_5824925052510502657[1] = 9.8100000000000005*cos(state[1]);
   out_5824925052510502657[2] = 0;
   out_5824925052510502657[3] = 0;
   out_5824925052510502657[4] = -state[8];
   out_5824925052510502657[5] = state[7];
   out_5824925052510502657[6] = 0;
   out_5824925052510502657[7] = state[5];
   out_5824925052510502657[8] = -state[4];
   out_5824925052510502657[9] = 0;
   out_5824925052510502657[10] = 0;
   out_5824925052510502657[11] = 0;
   out_5824925052510502657[12] = 1;
   out_5824925052510502657[13] = 0;
   out_5824925052510502657[14] = 0;
   out_5824925052510502657[15] = 1;
   out_5824925052510502657[16] = 0;
   out_5824925052510502657[17] = 0;
   out_5824925052510502657[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_5824925052510502657[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_5824925052510502657[20] = 0;
   out_5824925052510502657[21] = state[8];
   out_5824925052510502657[22] = 0;
   out_5824925052510502657[23] = -state[6];
   out_5824925052510502657[24] = -state[5];
   out_5824925052510502657[25] = 0;
   out_5824925052510502657[26] = state[3];
   out_5824925052510502657[27] = 0;
   out_5824925052510502657[28] = 0;
   out_5824925052510502657[29] = 0;
   out_5824925052510502657[30] = 0;
   out_5824925052510502657[31] = 1;
   out_5824925052510502657[32] = 0;
   out_5824925052510502657[33] = 0;
   out_5824925052510502657[34] = 1;
   out_5824925052510502657[35] = 0;
   out_5824925052510502657[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_5824925052510502657[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_5824925052510502657[38] = 0;
   out_5824925052510502657[39] = -state[7];
   out_5824925052510502657[40] = state[6];
   out_5824925052510502657[41] = 0;
   out_5824925052510502657[42] = state[4];
   out_5824925052510502657[43] = -state[3];
   out_5824925052510502657[44] = 0;
   out_5824925052510502657[45] = 0;
   out_5824925052510502657[46] = 0;
   out_5824925052510502657[47] = 0;
   out_5824925052510502657[48] = 0;
   out_5824925052510502657[49] = 0;
   out_5824925052510502657[50] = 1;
   out_5824925052510502657[51] = 0;
   out_5824925052510502657[52] = 0;
   out_5824925052510502657[53] = 1;
}
void h_13(double *state, double *unused, double *out_1759161301712070188) {
   out_1759161301712070188[0] = state[3];
   out_1759161301712070188[1] = state[4];
   out_1759161301712070188[2] = state[5];
}
void H_13(double *state, double *unused, double *out_52681411571927232) {
   out_52681411571927232[0] = 0;
   out_52681411571927232[1] = 0;
   out_52681411571927232[2] = 0;
   out_52681411571927232[3] = 1;
   out_52681411571927232[4] = 0;
   out_52681411571927232[5] = 0;
   out_52681411571927232[6] = 0;
   out_52681411571927232[7] = 0;
   out_52681411571927232[8] = 0;
   out_52681411571927232[9] = 0;
   out_52681411571927232[10] = 0;
   out_52681411571927232[11] = 0;
   out_52681411571927232[12] = 0;
   out_52681411571927232[13] = 0;
   out_52681411571927232[14] = 0;
   out_52681411571927232[15] = 0;
   out_52681411571927232[16] = 0;
   out_52681411571927232[17] = 0;
   out_52681411571927232[18] = 0;
   out_52681411571927232[19] = 0;
   out_52681411571927232[20] = 0;
   out_52681411571927232[21] = 0;
   out_52681411571927232[22] = 1;
   out_52681411571927232[23] = 0;
   out_52681411571927232[24] = 0;
   out_52681411571927232[25] = 0;
   out_52681411571927232[26] = 0;
   out_52681411571927232[27] = 0;
   out_52681411571927232[28] = 0;
   out_52681411571927232[29] = 0;
   out_52681411571927232[30] = 0;
   out_52681411571927232[31] = 0;
   out_52681411571927232[32] = 0;
   out_52681411571927232[33] = 0;
   out_52681411571927232[34] = 0;
   out_52681411571927232[35] = 0;
   out_52681411571927232[36] = 0;
   out_52681411571927232[37] = 0;
   out_52681411571927232[38] = 0;
   out_52681411571927232[39] = 0;
   out_52681411571927232[40] = 0;
   out_52681411571927232[41] = 1;
   out_52681411571927232[42] = 0;
   out_52681411571927232[43] = 0;
   out_52681411571927232[44] = 0;
   out_52681411571927232[45] = 0;
   out_52681411571927232[46] = 0;
   out_52681411571927232[47] = 0;
   out_52681411571927232[48] = 0;
   out_52681411571927232[49] = 0;
   out_52681411571927232[50] = 0;
   out_52681411571927232[51] = 0;
   out_52681411571927232[52] = 0;
   out_52681411571927232[53] = 0;
}
void h_14(double *state, double *unused, double *out_9193330309912018009) {
   out_9193330309912018009[0] = state[6];
   out_9193330309912018009[1] = state[7];
   out_9193330309912018009[2] = state[8];
}
void H_14(double *state, double *unused, double *out_698285619435224496) {
   out_698285619435224496[0] = 0;
   out_698285619435224496[1] = 0;
   out_698285619435224496[2] = 0;
   out_698285619435224496[3] = 0;
   out_698285619435224496[4] = 0;
   out_698285619435224496[5] = 0;
   out_698285619435224496[6] = 1;
   out_698285619435224496[7] = 0;
   out_698285619435224496[8] = 0;
   out_698285619435224496[9] = 0;
   out_698285619435224496[10] = 0;
   out_698285619435224496[11] = 0;
   out_698285619435224496[12] = 0;
   out_698285619435224496[13] = 0;
   out_698285619435224496[14] = 0;
   out_698285619435224496[15] = 0;
   out_698285619435224496[16] = 0;
   out_698285619435224496[17] = 0;
   out_698285619435224496[18] = 0;
   out_698285619435224496[19] = 0;
   out_698285619435224496[20] = 0;
   out_698285619435224496[21] = 0;
   out_698285619435224496[22] = 0;
   out_698285619435224496[23] = 0;
   out_698285619435224496[24] = 0;
   out_698285619435224496[25] = 1;
   out_698285619435224496[26] = 0;
   out_698285619435224496[27] = 0;
   out_698285619435224496[28] = 0;
   out_698285619435224496[29] = 0;
   out_698285619435224496[30] = 0;
   out_698285619435224496[31] = 0;
   out_698285619435224496[32] = 0;
   out_698285619435224496[33] = 0;
   out_698285619435224496[34] = 0;
   out_698285619435224496[35] = 0;
   out_698285619435224496[36] = 0;
   out_698285619435224496[37] = 0;
   out_698285619435224496[38] = 0;
   out_698285619435224496[39] = 0;
   out_698285619435224496[40] = 0;
   out_698285619435224496[41] = 0;
   out_698285619435224496[42] = 0;
   out_698285619435224496[43] = 0;
   out_698285619435224496[44] = 1;
   out_698285619435224496[45] = 0;
   out_698285619435224496[46] = 0;
   out_698285619435224496[47] = 0;
   out_698285619435224496[48] = 0;
   out_698285619435224496[49] = 0;
   out_698285619435224496[50] = 0;
   out_698285619435224496[51] = 0;
   out_698285619435224496[52] = 0;
   out_698285619435224496[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_1677822181415374238) {
  err_fun(nom_x, delta_x, out_1677822181415374238);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1645070502606612381) {
  inv_err_fun(nom_x, true_x, out_1645070502606612381);
}
void pose_H_mod_fun(double *state, double *out_6168804416891607091) {
  H_mod_fun(state, out_6168804416891607091);
}
void pose_f_fun(double *state, double dt, double *out_4562479811319953042) {
  f_fun(state,  dt, out_4562479811319953042);
}
void pose_F_fun(double *state, double dt, double *out_1994404627029761325) {
  F_fun(state,  dt, out_1994404627029761325);
}
void pose_h_4(double *state, double *unused, double *out_8379183754462585357) {
  h_4(state, unused, out_8379183754462585357);
}
void pose_H_4(double *state, double *unused, double *out_3264955236904260033) {
  H_4(state, unused, out_3264955236904260033);
}
void pose_h_10(double *state, double *unused, double *out_694976072985625952) {
  h_10(state, unused, out_694976072985625952);
}
void pose_H_10(double *state, double *unused, double *out_5824925052510502657) {
  H_10(state, unused, out_5824925052510502657);
}
void pose_h_13(double *state, double *unused, double *out_1759161301712070188) {
  h_13(state, unused, out_1759161301712070188);
}
void pose_H_13(double *state, double *unused, double *out_52681411571927232) {
  H_13(state, unused, out_52681411571927232);
}
void pose_h_14(double *state, double *unused, double *out_9193330309912018009) {
  h_14(state, unused, out_9193330309912018009);
}
void pose_H_14(double *state, double *unused, double *out_698285619435224496) {
  H_14(state, unused, out_698285619435224496);
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

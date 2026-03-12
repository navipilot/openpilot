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
void err_fun(double *nom_x, double *delta_x, double *out_2843212773844661672) {
   out_2843212773844661672[0] = delta_x[0] + nom_x[0];
   out_2843212773844661672[1] = delta_x[1] + nom_x[1];
   out_2843212773844661672[2] = delta_x[2] + nom_x[2];
   out_2843212773844661672[3] = delta_x[3] + nom_x[3];
   out_2843212773844661672[4] = delta_x[4] + nom_x[4];
   out_2843212773844661672[5] = delta_x[5] + nom_x[5];
   out_2843212773844661672[6] = delta_x[6] + nom_x[6];
   out_2843212773844661672[7] = delta_x[7] + nom_x[7];
   out_2843212773844661672[8] = delta_x[8] + nom_x[8];
   out_2843212773844661672[9] = delta_x[9] + nom_x[9];
   out_2843212773844661672[10] = delta_x[10] + nom_x[10];
   out_2843212773844661672[11] = delta_x[11] + nom_x[11];
   out_2843212773844661672[12] = delta_x[12] + nom_x[12];
   out_2843212773844661672[13] = delta_x[13] + nom_x[13];
   out_2843212773844661672[14] = delta_x[14] + nom_x[14];
   out_2843212773844661672[15] = delta_x[15] + nom_x[15];
   out_2843212773844661672[16] = delta_x[16] + nom_x[16];
   out_2843212773844661672[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2943105406875713938) {
   out_2943105406875713938[0] = -nom_x[0] + true_x[0];
   out_2943105406875713938[1] = -nom_x[1] + true_x[1];
   out_2943105406875713938[2] = -nom_x[2] + true_x[2];
   out_2943105406875713938[3] = -nom_x[3] + true_x[3];
   out_2943105406875713938[4] = -nom_x[4] + true_x[4];
   out_2943105406875713938[5] = -nom_x[5] + true_x[5];
   out_2943105406875713938[6] = -nom_x[6] + true_x[6];
   out_2943105406875713938[7] = -nom_x[7] + true_x[7];
   out_2943105406875713938[8] = -nom_x[8] + true_x[8];
   out_2943105406875713938[9] = -nom_x[9] + true_x[9];
   out_2943105406875713938[10] = -nom_x[10] + true_x[10];
   out_2943105406875713938[11] = -nom_x[11] + true_x[11];
   out_2943105406875713938[12] = -nom_x[12] + true_x[12];
   out_2943105406875713938[13] = -nom_x[13] + true_x[13];
   out_2943105406875713938[14] = -nom_x[14] + true_x[14];
   out_2943105406875713938[15] = -nom_x[15] + true_x[15];
   out_2943105406875713938[16] = -nom_x[16] + true_x[16];
   out_2943105406875713938[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_2147512711416992695) {
   out_2147512711416992695[0] = 1.0;
   out_2147512711416992695[1] = 0.0;
   out_2147512711416992695[2] = 0.0;
   out_2147512711416992695[3] = 0.0;
   out_2147512711416992695[4] = 0.0;
   out_2147512711416992695[5] = 0.0;
   out_2147512711416992695[6] = 0.0;
   out_2147512711416992695[7] = 0.0;
   out_2147512711416992695[8] = 0.0;
   out_2147512711416992695[9] = 0.0;
   out_2147512711416992695[10] = 0.0;
   out_2147512711416992695[11] = 0.0;
   out_2147512711416992695[12] = 0.0;
   out_2147512711416992695[13] = 0.0;
   out_2147512711416992695[14] = 0.0;
   out_2147512711416992695[15] = 0.0;
   out_2147512711416992695[16] = 0.0;
   out_2147512711416992695[17] = 0.0;
   out_2147512711416992695[18] = 0.0;
   out_2147512711416992695[19] = 1.0;
   out_2147512711416992695[20] = 0.0;
   out_2147512711416992695[21] = 0.0;
   out_2147512711416992695[22] = 0.0;
   out_2147512711416992695[23] = 0.0;
   out_2147512711416992695[24] = 0.0;
   out_2147512711416992695[25] = 0.0;
   out_2147512711416992695[26] = 0.0;
   out_2147512711416992695[27] = 0.0;
   out_2147512711416992695[28] = 0.0;
   out_2147512711416992695[29] = 0.0;
   out_2147512711416992695[30] = 0.0;
   out_2147512711416992695[31] = 0.0;
   out_2147512711416992695[32] = 0.0;
   out_2147512711416992695[33] = 0.0;
   out_2147512711416992695[34] = 0.0;
   out_2147512711416992695[35] = 0.0;
   out_2147512711416992695[36] = 0.0;
   out_2147512711416992695[37] = 0.0;
   out_2147512711416992695[38] = 1.0;
   out_2147512711416992695[39] = 0.0;
   out_2147512711416992695[40] = 0.0;
   out_2147512711416992695[41] = 0.0;
   out_2147512711416992695[42] = 0.0;
   out_2147512711416992695[43] = 0.0;
   out_2147512711416992695[44] = 0.0;
   out_2147512711416992695[45] = 0.0;
   out_2147512711416992695[46] = 0.0;
   out_2147512711416992695[47] = 0.0;
   out_2147512711416992695[48] = 0.0;
   out_2147512711416992695[49] = 0.0;
   out_2147512711416992695[50] = 0.0;
   out_2147512711416992695[51] = 0.0;
   out_2147512711416992695[52] = 0.0;
   out_2147512711416992695[53] = 0.0;
   out_2147512711416992695[54] = 0.0;
   out_2147512711416992695[55] = 0.0;
   out_2147512711416992695[56] = 0.0;
   out_2147512711416992695[57] = 1.0;
   out_2147512711416992695[58] = 0.0;
   out_2147512711416992695[59] = 0.0;
   out_2147512711416992695[60] = 0.0;
   out_2147512711416992695[61] = 0.0;
   out_2147512711416992695[62] = 0.0;
   out_2147512711416992695[63] = 0.0;
   out_2147512711416992695[64] = 0.0;
   out_2147512711416992695[65] = 0.0;
   out_2147512711416992695[66] = 0.0;
   out_2147512711416992695[67] = 0.0;
   out_2147512711416992695[68] = 0.0;
   out_2147512711416992695[69] = 0.0;
   out_2147512711416992695[70] = 0.0;
   out_2147512711416992695[71] = 0.0;
   out_2147512711416992695[72] = 0.0;
   out_2147512711416992695[73] = 0.0;
   out_2147512711416992695[74] = 0.0;
   out_2147512711416992695[75] = 0.0;
   out_2147512711416992695[76] = 1.0;
   out_2147512711416992695[77] = 0.0;
   out_2147512711416992695[78] = 0.0;
   out_2147512711416992695[79] = 0.0;
   out_2147512711416992695[80] = 0.0;
   out_2147512711416992695[81] = 0.0;
   out_2147512711416992695[82] = 0.0;
   out_2147512711416992695[83] = 0.0;
   out_2147512711416992695[84] = 0.0;
   out_2147512711416992695[85] = 0.0;
   out_2147512711416992695[86] = 0.0;
   out_2147512711416992695[87] = 0.0;
   out_2147512711416992695[88] = 0.0;
   out_2147512711416992695[89] = 0.0;
   out_2147512711416992695[90] = 0.0;
   out_2147512711416992695[91] = 0.0;
   out_2147512711416992695[92] = 0.0;
   out_2147512711416992695[93] = 0.0;
   out_2147512711416992695[94] = 0.0;
   out_2147512711416992695[95] = 1.0;
   out_2147512711416992695[96] = 0.0;
   out_2147512711416992695[97] = 0.0;
   out_2147512711416992695[98] = 0.0;
   out_2147512711416992695[99] = 0.0;
   out_2147512711416992695[100] = 0.0;
   out_2147512711416992695[101] = 0.0;
   out_2147512711416992695[102] = 0.0;
   out_2147512711416992695[103] = 0.0;
   out_2147512711416992695[104] = 0.0;
   out_2147512711416992695[105] = 0.0;
   out_2147512711416992695[106] = 0.0;
   out_2147512711416992695[107] = 0.0;
   out_2147512711416992695[108] = 0.0;
   out_2147512711416992695[109] = 0.0;
   out_2147512711416992695[110] = 0.0;
   out_2147512711416992695[111] = 0.0;
   out_2147512711416992695[112] = 0.0;
   out_2147512711416992695[113] = 0.0;
   out_2147512711416992695[114] = 1.0;
   out_2147512711416992695[115] = 0.0;
   out_2147512711416992695[116] = 0.0;
   out_2147512711416992695[117] = 0.0;
   out_2147512711416992695[118] = 0.0;
   out_2147512711416992695[119] = 0.0;
   out_2147512711416992695[120] = 0.0;
   out_2147512711416992695[121] = 0.0;
   out_2147512711416992695[122] = 0.0;
   out_2147512711416992695[123] = 0.0;
   out_2147512711416992695[124] = 0.0;
   out_2147512711416992695[125] = 0.0;
   out_2147512711416992695[126] = 0.0;
   out_2147512711416992695[127] = 0.0;
   out_2147512711416992695[128] = 0.0;
   out_2147512711416992695[129] = 0.0;
   out_2147512711416992695[130] = 0.0;
   out_2147512711416992695[131] = 0.0;
   out_2147512711416992695[132] = 0.0;
   out_2147512711416992695[133] = 1.0;
   out_2147512711416992695[134] = 0.0;
   out_2147512711416992695[135] = 0.0;
   out_2147512711416992695[136] = 0.0;
   out_2147512711416992695[137] = 0.0;
   out_2147512711416992695[138] = 0.0;
   out_2147512711416992695[139] = 0.0;
   out_2147512711416992695[140] = 0.0;
   out_2147512711416992695[141] = 0.0;
   out_2147512711416992695[142] = 0.0;
   out_2147512711416992695[143] = 0.0;
   out_2147512711416992695[144] = 0.0;
   out_2147512711416992695[145] = 0.0;
   out_2147512711416992695[146] = 0.0;
   out_2147512711416992695[147] = 0.0;
   out_2147512711416992695[148] = 0.0;
   out_2147512711416992695[149] = 0.0;
   out_2147512711416992695[150] = 0.0;
   out_2147512711416992695[151] = 0.0;
   out_2147512711416992695[152] = 1.0;
   out_2147512711416992695[153] = 0.0;
   out_2147512711416992695[154] = 0.0;
   out_2147512711416992695[155] = 0.0;
   out_2147512711416992695[156] = 0.0;
   out_2147512711416992695[157] = 0.0;
   out_2147512711416992695[158] = 0.0;
   out_2147512711416992695[159] = 0.0;
   out_2147512711416992695[160] = 0.0;
   out_2147512711416992695[161] = 0.0;
   out_2147512711416992695[162] = 0.0;
   out_2147512711416992695[163] = 0.0;
   out_2147512711416992695[164] = 0.0;
   out_2147512711416992695[165] = 0.0;
   out_2147512711416992695[166] = 0.0;
   out_2147512711416992695[167] = 0.0;
   out_2147512711416992695[168] = 0.0;
   out_2147512711416992695[169] = 0.0;
   out_2147512711416992695[170] = 0.0;
   out_2147512711416992695[171] = 1.0;
   out_2147512711416992695[172] = 0.0;
   out_2147512711416992695[173] = 0.0;
   out_2147512711416992695[174] = 0.0;
   out_2147512711416992695[175] = 0.0;
   out_2147512711416992695[176] = 0.0;
   out_2147512711416992695[177] = 0.0;
   out_2147512711416992695[178] = 0.0;
   out_2147512711416992695[179] = 0.0;
   out_2147512711416992695[180] = 0.0;
   out_2147512711416992695[181] = 0.0;
   out_2147512711416992695[182] = 0.0;
   out_2147512711416992695[183] = 0.0;
   out_2147512711416992695[184] = 0.0;
   out_2147512711416992695[185] = 0.0;
   out_2147512711416992695[186] = 0.0;
   out_2147512711416992695[187] = 0.0;
   out_2147512711416992695[188] = 0.0;
   out_2147512711416992695[189] = 0.0;
   out_2147512711416992695[190] = 1.0;
   out_2147512711416992695[191] = 0.0;
   out_2147512711416992695[192] = 0.0;
   out_2147512711416992695[193] = 0.0;
   out_2147512711416992695[194] = 0.0;
   out_2147512711416992695[195] = 0.0;
   out_2147512711416992695[196] = 0.0;
   out_2147512711416992695[197] = 0.0;
   out_2147512711416992695[198] = 0.0;
   out_2147512711416992695[199] = 0.0;
   out_2147512711416992695[200] = 0.0;
   out_2147512711416992695[201] = 0.0;
   out_2147512711416992695[202] = 0.0;
   out_2147512711416992695[203] = 0.0;
   out_2147512711416992695[204] = 0.0;
   out_2147512711416992695[205] = 0.0;
   out_2147512711416992695[206] = 0.0;
   out_2147512711416992695[207] = 0.0;
   out_2147512711416992695[208] = 0.0;
   out_2147512711416992695[209] = 1.0;
   out_2147512711416992695[210] = 0.0;
   out_2147512711416992695[211] = 0.0;
   out_2147512711416992695[212] = 0.0;
   out_2147512711416992695[213] = 0.0;
   out_2147512711416992695[214] = 0.0;
   out_2147512711416992695[215] = 0.0;
   out_2147512711416992695[216] = 0.0;
   out_2147512711416992695[217] = 0.0;
   out_2147512711416992695[218] = 0.0;
   out_2147512711416992695[219] = 0.0;
   out_2147512711416992695[220] = 0.0;
   out_2147512711416992695[221] = 0.0;
   out_2147512711416992695[222] = 0.0;
   out_2147512711416992695[223] = 0.0;
   out_2147512711416992695[224] = 0.0;
   out_2147512711416992695[225] = 0.0;
   out_2147512711416992695[226] = 0.0;
   out_2147512711416992695[227] = 0.0;
   out_2147512711416992695[228] = 1.0;
   out_2147512711416992695[229] = 0.0;
   out_2147512711416992695[230] = 0.0;
   out_2147512711416992695[231] = 0.0;
   out_2147512711416992695[232] = 0.0;
   out_2147512711416992695[233] = 0.0;
   out_2147512711416992695[234] = 0.0;
   out_2147512711416992695[235] = 0.0;
   out_2147512711416992695[236] = 0.0;
   out_2147512711416992695[237] = 0.0;
   out_2147512711416992695[238] = 0.0;
   out_2147512711416992695[239] = 0.0;
   out_2147512711416992695[240] = 0.0;
   out_2147512711416992695[241] = 0.0;
   out_2147512711416992695[242] = 0.0;
   out_2147512711416992695[243] = 0.0;
   out_2147512711416992695[244] = 0.0;
   out_2147512711416992695[245] = 0.0;
   out_2147512711416992695[246] = 0.0;
   out_2147512711416992695[247] = 1.0;
   out_2147512711416992695[248] = 0.0;
   out_2147512711416992695[249] = 0.0;
   out_2147512711416992695[250] = 0.0;
   out_2147512711416992695[251] = 0.0;
   out_2147512711416992695[252] = 0.0;
   out_2147512711416992695[253] = 0.0;
   out_2147512711416992695[254] = 0.0;
   out_2147512711416992695[255] = 0.0;
   out_2147512711416992695[256] = 0.0;
   out_2147512711416992695[257] = 0.0;
   out_2147512711416992695[258] = 0.0;
   out_2147512711416992695[259] = 0.0;
   out_2147512711416992695[260] = 0.0;
   out_2147512711416992695[261] = 0.0;
   out_2147512711416992695[262] = 0.0;
   out_2147512711416992695[263] = 0.0;
   out_2147512711416992695[264] = 0.0;
   out_2147512711416992695[265] = 0.0;
   out_2147512711416992695[266] = 1.0;
   out_2147512711416992695[267] = 0.0;
   out_2147512711416992695[268] = 0.0;
   out_2147512711416992695[269] = 0.0;
   out_2147512711416992695[270] = 0.0;
   out_2147512711416992695[271] = 0.0;
   out_2147512711416992695[272] = 0.0;
   out_2147512711416992695[273] = 0.0;
   out_2147512711416992695[274] = 0.0;
   out_2147512711416992695[275] = 0.0;
   out_2147512711416992695[276] = 0.0;
   out_2147512711416992695[277] = 0.0;
   out_2147512711416992695[278] = 0.0;
   out_2147512711416992695[279] = 0.0;
   out_2147512711416992695[280] = 0.0;
   out_2147512711416992695[281] = 0.0;
   out_2147512711416992695[282] = 0.0;
   out_2147512711416992695[283] = 0.0;
   out_2147512711416992695[284] = 0.0;
   out_2147512711416992695[285] = 1.0;
   out_2147512711416992695[286] = 0.0;
   out_2147512711416992695[287] = 0.0;
   out_2147512711416992695[288] = 0.0;
   out_2147512711416992695[289] = 0.0;
   out_2147512711416992695[290] = 0.0;
   out_2147512711416992695[291] = 0.0;
   out_2147512711416992695[292] = 0.0;
   out_2147512711416992695[293] = 0.0;
   out_2147512711416992695[294] = 0.0;
   out_2147512711416992695[295] = 0.0;
   out_2147512711416992695[296] = 0.0;
   out_2147512711416992695[297] = 0.0;
   out_2147512711416992695[298] = 0.0;
   out_2147512711416992695[299] = 0.0;
   out_2147512711416992695[300] = 0.0;
   out_2147512711416992695[301] = 0.0;
   out_2147512711416992695[302] = 0.0;
   out_2147512711416992695[303] = 0.0;
   out_2147512711416992695[304] = 1.0;
   out_2147512711416992695[305] = 0.0;
   out_2147512711416992695[306] = 0.0;
   out_2147512711416992695[307] = 0.0;
   out_2147512711416992695[308] = 0.0;
   out_2147512711416992695[309] = 0.0;
   out_2147512711416992695[310] = 0.0;
   out_2147512711416992695[311] = 0.0;
   out_2147512711416992695[312] = 0.0;
   out_2147512711416992695[313] = 0.0;
   out_2147512711416992695[314] = 0.0;
   out_2147512711416992695[315] = 0.0;
   out_2147512711416992695[316] = 0.0;
   out_2147512711416992695[317] = 0.0;
   out_2147512711416992695[318] = 0.0;
   out_2147512711416992695[319] = 0.0;
   out_2147512711416992695[320] = 0.0;
   out_2147512711416992695[321] = 0.0;
   out_2147512711416992695[322] = 0.0;
   out_2147512711416992695[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_9076294150866414245) {
   out_9076294150866414245[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_9076294150866414245[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_9076294150866414245[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_9076294150866414245[3] = dt*state[12] + state[3];
   out_9076294150866414245[4] = dt*state[13] + state[4];
   out_9076294150866414245[5] = dt*state[14] + state[5];
   out_9076294150866414245[6] = state[6];
   out_9076294150866414245[7] = state[7];
   out_9076294150866414245[8] = state[8];
   out_9076294150866414245[9] = state[9];
   out_9076294150866414245[10] = state[10];
   out_9076294150866414245[11] = state[11];
   out_9076294150866414245[12] = state[12];
   out_9076294150866414245[13] = state[13];
   out_9076294150866414245[14] = state[14];
   out_9076294150866414245[15] = state[15];
   out_9076294150866414245[16] = state[16];
   out_9076294150866414245[17] = state[17];
}
void F_fun(double *state, double dt, double *out_4995496673877862289) {
   out_4995496673877862289[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4995496673877862289[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4995496673877862289[2] = 0;
   out_4995496673877862289[3] = 0;
   out_4995496673877862289[4] = 0;
   out_4995496673877862289[5] = 0;
   out_4995496673877862289[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4995496673877862289[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4995496673877862289[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4995496673877862289[9] = 0;
   out_4995496673877862289[10] = 0;
   out_4995496673877862289[11] = 0;
   out_4995496673877862289[12] = 0;
   out_4995496673877862289[13] = 0;
   out_4995496673877862289[14] = 0;
   out_4995496673877862289[15] = 0;
   out_4995496673877862289[16] = 0;
   out_4995496673877862289[17] = 0;
   out_4995496673877862289[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4995496673877862289[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4995496673877862289[20] = 0;
   out_4995496673877862289[21] = 0;
   out_4995496673877862289[22] = 0;
   out_4995496673877862289[23] = 0;
   out_4995496673877862289[24] = 0;
   out_4995496673877862289[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4995496673877862289[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4995496673877862289[27] = 0;
   out_4995496673877862289[28] = 0;
   out_4995496673877862289[29] = 0;
   out_4995496673877862289[30] = 0;
   out_4995496673877862289[31] = 0;
   out_4995496673877862289[32] = 0;
   out_4995496673877862289[33] = 0;
   out_4995496673877862289[34] = 0;
   out_4995496673877862289[35] = 0;
   out_4995496673877862289[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4995496673877862289[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4995496673877862289[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4995496673877862289[39] = 0;
   out_4995496673877862289[40] = 0;
   out_4995496673877862289[41] = 0;
   out_4995496673877862289[42] = 0;
   out_4995496673877862289[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4995496673877862289[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4995496673877862289[45] = 0;
   out_4995496673877862289[46] = 0;
   out_4995496673877862289[47] = 0;
   out_4995496673877862289[48] = 0;
   out_4995496673877862289[49] = 0;
   out_4995496673877862289[50] = 0;
   out_4995496673877862289[51] = 0;
   out_4995496673877862289[52] = 0;
   out_4995496673877862289[53] = 0;
   out_4995496673877862289[54] = 0;
   out_4995496673877862289[55] = 0;
   out_4995496673877862289[56] = 0;
   out_4995496673877862289[57] = 1;
   out_4995496673877862289[58] = 0;
   out_4995496673877862289[59] = 0;
   out_4995496673877862289[60] = 0;
   out_4995496673877862289[61] = 0;
   out_4995496673877862289[62] = 0;
   out_4995496673877862289[63] = 0;
   out_4995496673877862289[64] = 0;
   out_4995496673877862289[65] = 0;
   out_4995496673877862289[66] = dt;
   out_4995496673877862289[67] = 0;
   out_4995496673877862289[68] = 0;
   out_4995496673877862289[69] = 0;
   out_4995496673877862289[70] = 0;
   out_4995496673877862289[71] = 0;
   out_4995496673877862289[72] = 0;
   out_4995496673877862289[73] = 0;
   out_4995496673877862289[74] = 0;
   out_4995496673877862289[75] = 0;
   out_4995496673877862289[76] = 1;
   out_4995496673877862289[77] = 0;
   out_4995496673877862289[78] = 0;
   out_4995496673877862289[79] = 0;
   out_4995496673877862289[80] = 0;
   out_4995496673877862289[81] = 0;
   out_4995496673877862289[82] = 0;
   out_4995496673877862289[83] = 0;
   out_4995496673877862289[84] = 0;
   out_4995496673877862289[85] = dt;
   out_4995496673877862289[86] = 0;
   out_4995496673877862289[87] = 0;
   out_4995496673877862289[88] = 0;
   out_4995496673877862289[89] = 0;
   out_4995496673877862289[90] = 0;
   out_4995496673877862289[91] = 0;
   out_4995496673877862289[92] = 0;
   out_4995496673877862289[93] = 0;
   out_4995496673877862289[94] = 0;
   out_4995496673877862289[95] = 1;
   out_4995496673877862289[96] = 0;
   out_4995496673877862289[97] = 0;
   out_4995496673877862289[98] = 0;
   out_4995496673877862289[99] = 0;
   out_4995496673877862289[100] = 0;
   out_4995496673877862289[101] = 0;
   out_4995496673877862289[102] = 0;
   out_4995496673877862289[103] = 0;
   out_4995496673877862289[104] = dt;
   out_4995496673877862289[105] = 0;
   out_4995496673877862289[106] = 0;
   out_4995496673877862289[107] = 0;
   out_4995496673877862289[108] = 0;
   out_4995496673877862289[109] = 0;
   out_4995496673877862289[110] = 0;
   out_4995496673877862289[111] = 0;
   out_4995496673877862289[112] = 0;
   out_4995496673877862289[113] = 0;
   out_4995496673877862289[114] = 1;
   out_4995496673877862289[115] = 0;
   out_4995496673877862289[116] = 0;
   out_4995496673877862289[117] = 0;
   out_4995496673877862289[118] = 0;
   out_4995496673877862289[119] = 0;
   out_4995496673877862289[120] = 0;
   out_4995496673877862289[121] = 0;
   out_4995496673877862289[122] = 0;
   out_4995496673877862289[123] = 0;
   out_4995496673877862289[124] = 0;
   out_4995496673877862289[125] = 0;
   out_4995496673877862289[126] = 0;
   out_4995496673877862289[127] = 0;
   out_4995496673877862289[128] = 0;
   out_4995496673877862289[129] = 0;
   out_4995496673877862289[130] = 0;
   out_4995496673877862289[131] = 0;
   out_4995496673877862289[132] = 0;
   out_4995496673877862289[133] = 1;
   out_4995496673877862289[134] = 0;
   out_4995496673877862289[135] = 0;
   out_4995496673877862289[136] = 0;
   out_4995496673877862289[137] = 0;
   out_4995496673877862289[138] = 0;
   out_4995496673877862289[139] = 0;
   out_4995496673877862289[140] = 0;
   out_4995496673877862289[141] = 0;
   out_4995496673877862289[142] = 0;
   out_4995496673877862289[143] = 0;
   out_4995496673877862289[144] = 0;
   out_4995496673877862289[145] = 0;
   out_4995496673877862289[146] = 0;
   out_4995496673877862289[147] = 0;
   out_4995496673877862289[148] = 0;
   out_4995496673877862289[149] = 0;
   out_4995496673877862289[150] = 0;
   out_4995496673877862289[151] = 0;
   out_4995496673877862289[152] = 1;
   out_4995496673877862289[153] = 0;
   out_4995496673877862289[154] = 0;
   out_4995496673877862289[155] = 0;
   out_4995496673877862289[156] = 0;
   out_4995496673877862289[157] = 0;
   out_4995496673877862289[158] = 0;
   out_4995496673877862289[159] = 0;
   out_4995496673877862289[160] = 0;
   out_4995496673877862289[161] = 0;
   out_4995496673877862289[162] = 0;
   out_4995496673877862289[163] = 0;
   out_4995496673877862289[164] = 0;
   out_4995496673877862289[165] = 0;
   out_4995496673877862289[166] = 0;
   out_4995496673877862289[167] = 0;
   out_4995496673877862289[168] = 0;
   out_4995496673877862289[169] = 0;
   out_4995496673877862289[170] = 0;
   out_4995496673877862289[171] = 1;
   out_4995496673877862289[172] = 0;
   out_4995496673877862289[173] = 0;
   out_4995496673877862289[174] = 0;
   out_4995496673877862289[175] = 0;
   out_4995496673877862289[176] = 0;
   out_4995496673877862289[177] = 0;
   out_4995496673877862289[178] = 0;
   out_4995496673877862289[179] = 0;
   out_4995496673877862289[180] = 0;
   out_4995496673877862289[181] = 0;
   out_4995496673877862289[182] = 0;
   out_4995496673877862289[183] = 0;
   out_4995496673877862289[184] = 0;
   out_4995496673877862289[185] = 0;
   out_4995496673877862289[186] = 0;
   out_4995496673877862289[187] = 0;
   out_4995496673877862289[188] = 0;
   out_4995496673877862289[189] = 0;
   out_4995496673877862289[190] = 1;
   out_4995496673877862289[191] = 0;
   out_4995496673877862289[192] = 0;
   out_4995496673877862289[193] = 0;
   out_4995496673877862289[194] = 0;
   out_4995496673877862289[195] = 0;
   out_4995496673877862289[196] = 0;
   out_4995496673877862289[197] = 0;
   out_4995496673877862289[198] = 0;
   out_4995496673877862289[199] = 0;
   out_4995496673877862289[200] = 0;
   out_4995496673877862289[201] = 0;
   out_4995496673877862289[202] = 0;
   out_4995496673877862289[203] = 0;
   out_4995496673877862289[204] = 0;
   out_4995496673877862289[205] = 0;
   out_4995496673877862289[206] = 0;
   out_4995496673877862289[207] = 0;
   out_4995496673877862289[208] = 0;
   out_4995496673877862289[209] = 1;
   out_4995496673877862289[210] = 0;
   out_4995496673877862289[211] = 0;
   out_4995496673877862289[212] = 0;
   out_4995496673877862289[213] = 0;
   out_4995496673877862289[214] = 0;
   out_4995496673877862289[215] = 0;
   out_4995496673877862289[216] = 0;
   out_4995496673877862289[217] = 0;
   out_4995496673877862289[218] = 0;
   out_4995496673877862289[219] = 0;
   out_4995496673877862289[220] = 0;
   out_4995496673877862289[221] = 0;
   out_4995496673877862289[222] = 0;
   out_4995496673877862289[223] = 0;
   out_4995496673877862289[224] = 0;
   out_4995496673877862289[225] = 0;
   out_4995496673877862289[226] = 0;
   out_4995496673877862289[227] = 0;
   out_4995496673877862289[228] = 1;
   out_4995496673877862289[229] = 0;
   out_4995496673877862289[230] = 0;
   out_4995496673877862289[231] = 0;
   out_4995496673877862289[232] = 0;
   out_4995496673877862289[233] = 0;
   out_4995496673877862289[234] = 0;
   out_4995496673877862289[235] = 0;
   out_4995496673877862289[236] = 0;
   out_4995496673877862289[237] = 0;
   out_4995496673877862289[238] = 0;
   out_4995496673877862289[239] = 0;
   out_4995496673877862289[240] = 0;
   out_4995496673877862289[241] = 0;
   out_4995496673877862289[242] = 0;
   out_4995496673877862289[243] = 0;
   out_4995496673877862289[244] = 0;
   out_4995496673877862289[245] = 0;
   out_4995496673877862289[246] = 0;
   out_4995496673877862289[247] = 1;
   out_4995496673877862289[248] = 0;
   out_4995496673877862289[249] = 0;
   out_4995496673877862289[250] = 0;
   out_4995496673877862289[251] = 0;
   out_4995496673877862289[252] = 0;
   out_4995496673877862289[253] = 0;
   out_4995496673877862289[254] = 0;
   out_4995496673877862289[255] = 0;
   out_4995496673877862289[256] = 0;
   out_4995496673877862289[257] = 0;
   out_4995496673877862289[258] = 0;
   out_4995496673877862289[259] = 0;
   out_4995496673877862289[260] = 0;
   out_4995496673877862289[261] = 0;
   out_4995496673877862289[262] = 0;
   out_4995496673877862289[263] = 0;
   out_4995496673877862289[264] = 0;
   out_4995496673877862289[265] = 0;
   out_4995496673877862289[266] = 1;
   out_4995496673877862289[267] = 0;
   out_4995496673877862289[268] = 0;
   out_4995496673877862289[269] = 0;
   out_4995496673877862289[270] = 0;
   out_4995496673877862289[271] = 0;
   out_4995496673877862289[272] = 0;
   out_4995496673877862289[273] = 0;
   out_4995496673877862289[274] = 0;
   out_4995496673877862289[275] = 0;
   out_4995496673877862289[276] = 0;
   out_4995496673877862289[277] = 0;
   out_4995496673877862289[278] = 0;
   out_4995496673877862289[279] = 0;
   out_4995496673877862289[280] = 0;
   out_4995496673877862289[281] = 0;
   out_4995496673877862289[282] = 0;
   out_4995496673877862289[283] = 0;
   out_4995496673877862289[284] = 0;
   out_4995496673877862289[285] = 1;
   out_4995496673877862289[286] = 0;
   out_4995496673877862289[287] = 0;
   out_4995496673877862289[288] = 0;
   out_4995496673877862289[289] = 0;
   out_4995496673877862289[290] = 0;
   out_4995496673877862289[291] = 0;
   out_4995496673877862289[292] = 0;
   out_4995496673877862289[293] = 0;
   out_4995496673877862289[294] = 0;
   out_4995496673877862289[295] = 0;
   out_4995496673877862289[296] = 0;
   out_4995496673877862289[297] = 0;
   out_4995496673877862289[298] = 0;
   out_4995496673877862289[299] = 0;
   out_4995496673877862289[300] = 0;
   out_4995496673877862289[301] = 0;
   out_4995496673877862289[302] = 0;
   out_4995496673877862289[303] = 0;
   out_4995496673877862289[304] = 1;
   out_4995496673877862289[305] = 0;
   out_4995496673877862289[306] = 0;
   out_4995496673877862289[307] = 0;
   out_4995496673877862289[308] = 0;
   out_4995496673877862289[309] = 0;
   out_4995496673877862289[310] = 0;
   out_4995496673877862289[311] = 0;
   out_4995496673877862289[312] = 0;
   out_4995496673877862289[313] = 0;
   out_4995496673877862289[314] = 0;
   out_4995496673877862289[315] = 0;
   out_4995496673877862289[316] = 0;
   out_4995496673877862289[317] = 0;
   out_4995496673877862289[318] = 0;
   out_4995496673877862289[319] = 0;
   out_4995496673877862289[320] = 0;
   out_4995496673877862289[321] = 0;
   out_4995496673877862289[322] = 0;
   out_4995496673877862289[323] = 1;
}
void h_4(double *state, double *unused, double *out_7356118181286553510) {
   out_7356118181286553510[0] = state[6] + state[9];
   out_7356118181286553510[1] = state[7] + state[10];
   out_7356118181286553510[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_3967211437323289556) {
   out_3967211437323289556[0] = 0;
   out_3967211437323289556[1] = 0;
   out_3967211437323289556[2] = 0;
   out_3967211437323289556[3] = 0;
   out_3967211437323289556[4] = 0;
   out_3967211437323289556[5] = 0;
   out_3967211437323289556[6] = 1;
   out_3967211437323289556[7] = 0;
   out_3967211437323289556[8] = 0;
   out_3967211437323289556[9] = 1;
   out_3967211437323289556[10] = 0;
   out_3967211437323289556[11] = 0;
   out_3967211437323289556[12] = 0;
   out_3967211437323289556[13] = 0;
   out_3967211437323289556[14] = 0;
   out_3967211437323289556[15] = 0;
   out_3967211437323289556[16] = 0;
   out_3967211437323289556[17] = 0;
   out_3967211437323289556[18] = 0;
   out_3967211437323289556[19] = 0;
   out_3967211437323289556[20] = 0;
   out_3967211437323289556[21] = 0;
   out_3967211437323289556[22] = 0;
   out_3967211437323289556[23] = 0;
   out_3967211437323289556[24] = 0;
   out_3967211437323289556[25] = 1;
   out_3967211437323289556[26] = 0;
   out_3967211437323289556[27] = 0;
   out_3967211437323289556[28] = 1;
   out_3967211437323289556[29] = 0;
   out_3967211437323289556[30] = 0;
   out_3967211437323289556[31] = 0;
   out_3967211437323289556[32] = 0;
   out_3967211437323289556[33] = 0;
   out_3967211437323289556[34] = 0;
   out_3967211437323289556[35] = 0;
   out_3967211437323289556[36] = 0;
   out_3967211437323289556[37] = 0;
   out_3967211437323289556[38] = 0;
   out_3967211437323289556[39] = 0;
   out_3967211437323289556[40] = 0;
   out_3967211437323289556[41] = 0;
   out_3967211437323289556[42] = 0;
   out_3967211437323289556[43] = 0;
   out_3967211437323289556[44] = 1;
   out_3967211437323289556[45] = 0;
   out_3967211437323289556[46] = 0;
   out_3967211437323289556[47] = 1;
   out_3967211437323289556[48] = 0;
   out_3967211437323289556[49] = 0;
   out_3967211437323289556[50] = 0;
   out_3967211437323289556[51] = 0;
   out_3967211437323289556[52] = 0;
   out_3967211437323289556[53] = 0;
}
void h_10(double *state, double *unused, double *out_320685202434969677) {
   out_320685202434969677[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_320685202434969677[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_320685202434969677[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_156638784834038850) {
   out_156638784834038850[0] = 0;
   out_156638784834038850[1] = 9.8100000000000005*cos(state[1]);
   out_156638784834038850[2] = 0;
   out_156638784834038850[3] = 0;
   out_156638784834038850[4] = -state[8];
   out_156638784834038850[5] = state[7];
   out_156638784834038850[6] = 0;
   out_156638784834038850[7] = state[5];
   out_156638784834038850[8] = -state[4];
   out_156638784834038850[9] = 0;
   out_156638784834038850[10] = 0;
   out_156638784834038850[11] = 0;
   out_156638784834038850[12] = 1;
   out_156638784834038850[13] = 0;
   out_156638784834038850[14] = 0;
   out_156638784834038850[15] = 1;
   out_156638784834038850[16] = 0;
   out_156638784834038850[17] = 0;
   out_156638784834038850[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_156638784834038850[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_156638784834038850[20] = 0;
   out_156638784834038850[21] = state[8];
   out_156638784834038850[22] = 0;
   out_156638784834038850[23] = -state[6];
   out_156638784834038850[24] = -state[5];
   out_156638784834038850[25] = 0;
   out_156638784834038850[26] = state[3];
   out_156638784834038850[27] = 0;
   out_156638784834038850[28] = 0;
   out_156638784834038850[29] = 0;
   out_156638784834038850[30] = 0;
   out_156638784834038850[31] = 1;
   out_156638784834038850[32] = 0;
   out_156638784834038850[33] = 0;
   out_156638784834038850[34] = 1;
   out_156638784834038850[35] = 0;
   out_156638784834038850[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_156638784834038850[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_156638784834038850[38] = 0;
   out_156638784834038850[39] = -state[7];
   out_156638784834038850[40] = state[6];
   out_156638784834038850[41] = 0;
   out_156638784834038850[42] = state[4];
   out_156638784834038850[43] = -state[3];
   out_156638784834038850[44] = 0;
   out_156638784834038850[45] = 0;
   out_156638784834038850[46] = 0;
   out_156638784834038850[47] = 0;
   out_156638784834038850[48] = 0;
   out_156638784834038850[49] = 0;
   out_156638784834038850[50] = 1;
   out_156638784834038850[51] = 0;
   out_156638784834038850[52] = 0;
   out_156638784834038850[53] = 1;
}
void h_13(double *state, double *unused, double *out_4281107234856404284) {
   out_4281107234856404284[0] = state[3];
   out_4281107234856404284[1] = state[4];
   out_4281107234856404284[2] = state[5];
}
void H_13(double *state, double *unused, double *out_7800966900625813580) {
   out_7800966900625813580[0] = 0;
   out_7800966900625813580[1] = 0;
   out_7800966900625813580[2] = 0;
   out_7800966900625813580[3] = 1;
   out_7800966900625813580[4] = 0;
   out_7800966900625813580[5] = 0;
   out_7800966900625813580[6] = 0;
   out_7800966900625813580[7] = 0;
   out_7800966900625813580[8] = 0;
   out_7800966900625813580[9] = 0;
   out_7800966900625813580[10] = 0;
   out_7800966900625813580[11] = 0;
   out_7800966900625813580[12] = 0;
   out_7800966900625813580[13] = 0;
   out_7800966900625813580[14] = 0;
   out_7800966900625813580[15] = 0;
   out_7800966900625813580[16] = 0;
   out_7800966900625813580[17] = 0;
   out_7800966900625813580[18] = 0;
   out_7800966900625813580[19] = 0;
   out_7800966900625813580[20] = 0;
   out_7800966900625813580[21] = 0;
   out_7800966900625813580[22] = 1;
   out_7800966900625813580[23] = 0;
   out_7800966900625813580[24] = 0;
   out_7800966900625813580[25] = 0;
   out_7800966900625813580[26] = 0;
   out_7800966900625813580[27] = 0;
   out_7800966900625813580[28] = 0;
   out_7800966900625813580[29] = 0;
   out_7800966900625813580[30] = 0;
   out_7800966900625813580[31] = 0;
   out_7800966900625813580[32] = 0;
   out_7800966900625813580[33] = 0;
   out_7800966900625813580[34] = 0;
   out_7800966900625813580[35] = 0;
   out_7800966900625813580[36] = 0;
   out_7800966900625813580[37] = 0;
   out_7800966900625813580[38] = 0;
   out_7800966900625813580[39] = 0;
   out_7800966900625813580[40] = 0;
   out_7800966900625813580[41] = 1;
   out_7800966900625813580[42] = 0;
   out_7800966900625813580[43] = 0;
   out_7800966900625813580[44] = 0;
   out_7800966900625813580[45] = 0;
   out_7800966900625813580[46] = 0;
   out_7800966900625813580[47] = 0;
   out_7800966900625813580[48] = 0;
   out_7800966900625813580[49] = 0;
   out_7800966900625813580[50] = 0;
   out_7800966900625813580[51] = 0;
   out_7800966900625813580[52] = 0;
   out_7800966900625813580[53] = 0;
}
void h_14(double *state, double *unused, double *out_3917712072833752253) {
   out_3917712072833752253[0] = state[6];
   out_3917712072833752253[1] = state[7];
   out_3917712072833752253[2] = state[8];
}
void H_14(double *state, double *unused, double *out_7049999869618661852) {
   out_7049999869618661852[0] = 0;
   out_7049999869618661852[1] = 0;
   out_7049999869618661852[2] = 0;
   out_7049999869618661852[3] = 0;
   out_7049999869618661852[4] = 0;
   out_7049999869618661852[5] = 0;
   out_7049999869618661852[6] = 1;
   out_7049999869618661852[7] = 0;
   out_7049999869618661852[8] = 0;
   out_7049999869618661852[9] = 0;
   out_7049999869618661852[10] = 0;
   out_7049999869618661852[11] = 0;
   out_7049999869618661852[12] = 0;
   out_7049999869618661852[13] = 0;
   out_7049999869618661852[14] = 0;
   out_7049999869618661852[15] = 0;
   out_7049999869618661852[16] = 0;
   out_7049999869618661852[17] = 0;
   out_7049999869618661852[18] = 0;
   out_7049999869618661852[19] = 0;
   out_7049999869618661852[20] = 0;
   out_7049999869618661852[21] = 0;
   out_7049999869618661852[22] = 0;
   out_7049999869618661852[23] = 0;
   out_7049999869618661852[24] = 0;
   out_7049999869618661852[25] = 1;
   out_7049999869618661852[26] = 0;
   out_7049999869618661852[27] = 0;
   out_7049999869618661852[28] = 0;
   out_7049999869618661852[29] = 0;
   out_7049999869618661852[30] = 0;
   out_7049999869618661852[31] = 0;
   out_7049999869618661852[32] = 0;
   out_7049999869618661852[33] = 0;
   out_7049999869618661852[34] = 0;
   out_7049999869618661852[35] = 0;
   out_7049999869618661852[36] = 0;
   out_7049999869618661852[37] = 0;
   out_7049999869618661852[38] = 0;
   out_7049999869618661852[39] = 0;
   out_7049999869618661852[40] = 0;
   out_7049999869618661852[41] = 0;
   out_7049999869618661852[42] = 0;
   out_7049999869618661852[43] = 0;
   out_7049999869618661852[44] = 1;
   out_7049999869618661852[45] = 0;
   out_7049999869618661852[46] = 0;
   out_7049999869618661852[47] = 0;
   out_7049999869618661852[48] = 0;
   out_7049999869618661852[49] = 0;
   out_7049999869618661852[50] = 0;
   out_7049999869618661852[51] = 0;
   out_7049999869618661852[52] = 0;
   out_7049999869618661852[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_2843212773844661672) {
  err_fun(nom_x, delta_x, out_2843212773844661672);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2943105406875713938) {
  inv_err_fun(nom_x, true_x, out_2943105406875713938);
}
void pose_H_mod_fun(double *state, double *out_2147512711416992695) {
  H_mod_fun(state, out_2147512711416992695);
}
void pose_f_fun(double *state, double dt, double *out_9076294150866414245) {
  f_fun(state,  dt, out_9076294150866414245);
}
void pose_F_fun(double *state, double dt, double *out_4995496673877862289) {
  F_fun(state,  dt, out_4995496673877862289);
}
void pose_h_4(double *state, double *unused, double *out_7356118181286553510) {
  h_4(state, unused, out_7356118181286553510);
}
void pose_H_4(double *state, double *unused, double *out_3967211437323289556) {
  H_4(state, unused, out_3967211437323289556);
}
void pose_h_10(double *state, double *unused, double *out_320685202434969677) {
  h_10(state, unused, out_320685202434969677);
}
void pose_H_10(double *state, double *unused, double *out_156638784834038850) {
  H_10(state, unused, out_156638784834038850);
}
void pose_h_13(double *state, double *unused, double *out_4281107234856404284) {
  h_13(state, unused, out_4281107234856404284);
}
void pose_H_13(double *state, double *unused, double *out_7800966900625813580) {
  H_13(state, unused, out_7800966900625813580);
}
void pose_h_14(double *state, double *unused, double *out_3917712072833752253) {
  h_14(state, unused, out_3917712072833752253);
}
void pose_H_14(double *state, double *unused, double *out_7049999869618661852) {
  H_14(state, unused, out_7049999869618661852);
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

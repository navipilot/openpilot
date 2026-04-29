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
void err_fun(double *nom_x, double *delta_x, double *out_1903969700519405728) {
   out_1903969700519405728[0] = delta_x[0] + nom_x[0];
   out_1903969700519405728[1] = delta_x[1] + nom_x[1];
   out_1903969700519405728[2] = delta_x[2] + nom_x[2];
   out_1903969700519405728[3] = delta_x[3] + nom_x[3];
   out_1903969700519405728[4] = delta_x[4] + nom_x[4];
   out_1903969700519405728[5] = delta_x[5] + nom_x[5];
   out_1903969700519405728[6] = delta_x[6] + nom_x[6];
   out_1903969700519405728[7] = delta_x[7] + nom_x[7];
   out_1903969700519405728[8] = delta_x[8] + nom_x[8];
   out_1903969700519405728[9] = delta_x[9] + nom_x[9];
   out_1903969700519405728[10] = delta_x[10] + nom_x[10];
   out_1903969700519405728[11] = delta_x[11] + nom_x[11];
   out_1903969700519405728[12] = delta_x[12] + nom_x[12];
   out_1903969700519405728[13] = delta_x[13] + nom_x[13];
   out_1903969700519405728[14] = delta_x[14] + nom_x[14];
   out_1903969700519405728[15] = delta_x[15] + nom_x[15];
   out_1903969700519405728[16] = delta_x[16] + nom_x[16];
   out_1903969700519405728[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3362098330798648840) {
   out_3362098330798648840[0] = -nom_x[0] + true_x[0];
   out_3362098330798648840[1] = -nom_x[1] + true_x[1];
   out_3362098330798648840[2] = -nom_x[2] + true_x[2];
   out_3362098330798648840[3] = -nom_x[3] + true_x[3];
   out_3362098330798648840[4] = -nom_x[4] + true_x[4];
   out_3362098330798648840[5] = -nom_x[5] + true_x[5];
   out_3362098330798648840[6] = -nom_x[6] + true_x[6];
   out_3362098330798648840[7] = -nom_x[7] + true_x[7];
   out_3362098330798648840[8] = -nom_x[8] + true_x[8];
   out_3362098330798648840[9] = -nom_x[9] + true_x[9];
   out_3362098330798648840[10] = -nom_x[10] + true_x[10];
   out_3362098330798648840[11] = -nom_x[11] + true_x[11];
   out_3362098330798648840[12] = -nom_x[12] + true_x[12];
   out_3362098330798648840[13] = -nom_x[13] + true_x[13];
   out_3362098330798648840[14] = -nom_x[14] + true_x[14];
   out_3362098330798648840[15] = -nom_x[15] + true_x[15];
   out_3362098330798648840[16] = -nom_x[16] + true_x[16];
   out_3362098330798648840[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_3822470159260117769) {
   out_3822470159260117769[0] = 1.0;
   out_3822470159260117769[1] = 0.0;
   out_3822470159260117769[2] = 0.0;
   out_3822470159260117769[3] = 0.0;
   out_3822470159260117769[4] = 0.0;
   out_3822470159260117769[5] = 0.0;
   out_3822470159260117769[6] = 0.0;
   out_3822470159260117769[7] = 0.0;
   out_3822470159260117769[8] = 0.0;
   out_3822470159260117769[9] = 0.0;
   out_3822470159260117769[10] = 0.0;
   out_3822470159260117769[11] = 0.0;
   out_3822470159260117769[12] = 0.0;
   out_3822470159260117769[13] = 0.0;
   out_3822470159260117769[14] = 0.0;
   out_3822470159260117769[15] = 0.0;
   out_3822470159260117769[16] = 0.0;
   out_3822470159260117769[17] = 0.0;
   out_3822470159260117769[18] = 0.0;
   out_3822470159260117769[19] = 1.0;
   out_3822470159260117769[20] = 0.0;
   out_3822470159260117769[21] = 0.0;
   out_3822470159260117769[22] = 0.0;
   out_3822470159260117769[23] = 0.0;
   out_3822470159260117769[24] = 0.0;
   out_3822470159260117769[25] = 0.0;
   out_3822470159260117769[26] = 0.0;
   out_3822470159260117769[27] = 0.0;
   out_3822470159260117769[28] = 0.0;
   out_3822470159260117769[29] = 0.0;
   out_3822470159260117769[30] = 0.0;
   out_3822470159260117769[31] = 0.0;
   out_3822470159260117769[32] = 0.0;
   out_3822470159260117769[33] = 0.0;
   out_3822470159260117769[34] = 0.0;
   out_3822470159260117769[35] = 0.0;
   out_3822470159260117769[36] = 0.0;
   out_3822470159260117769[37] = 0.0;
   out_3822470159260117769[38] = 1.0;
   out_3822470159260117769[39] = 0.0;
   out_3822470159260117769[40] = 0.0;
   out_3822470159260117769[41] = 0.0;
   out_3822470159260117769[42] = 0.0;
   out_3822470159260117769[43] = 0.0;
   out_3822470159260117769[44] = 0.0;
   out_3822470159260117769[45] = 0.0;
   out_3822470159260117769[46] = 0.0;
   out_3822470159260117769[47] = 0.0;
   out_3822470159260117769[48] = 0.0;
   out_3822470159260117769[49] = 0.0;
   out_3822470159260117769[50] = 0.0;
   out_3822470159260117769[51] = 0.0;
   out_3822470159260117769[52] = 0.0;
   out_3822470159260117769[53] = 0.0;
   out_3822470159260117769[54] = 0.0;
   out_3822470159260117769[55] = 0.0;
   out_3822470159260117769[56] = 0.0;
   out_3822470159260117769[57] = 1.0;
   out_3822470159260117769[58] = 0.0;
   out_3822470159260117769[59] = 0.0;
   out_3822470159260117769[60] = 0.0;
   out_3822470159260117769[61] = 0.0;
   out_3822470159260117769[62] = 0.0;
   out_3822470159260117769[63] = 0.0;
   out_3822470159260117769[64] = 0.0;
   out_3822470159260117769[65] = 0.0;
   out_3822470159260117769[66] = 0.0;
   out_3822470159260117769[67] = 0.0;
   out_3822470159260117769[68] = 0.0;
   out_3822470159260117769[69] = 0.0;
   out_3822470159260117769[70] = 0.0;
   out_3822470159260117769[71] = 0.0;
   out_3822470159260117769[72] = 0.0;
   out_3822470159260117769[73] = 0.0;
   out_3822470159260117769[74] = 0.0;
   out_3822470159260117769[75] = 0.0;
   out_3822470159260117769[76] = 1.0;
   out_3822470159260117769[77] = 0.0;
   out_3822470159260117769[78] = 0.0;
   out_3822470159260117769[79] = 0.0;
   out_3822470159260117769[80] = 0.0;
   out_3822470159260117769[81] = 0.0;
   out_3822470159260117769[82] = 0.0;
   out_3822470159260117769[83] = 0.0;
   out_3822470159260117769[84] = 0.0;
   out_3822470159260117769[85] = 0.0;
   out_3822470159260117769[86] = 0.0;
   out_3822470159260117769[87] = 0.0;
   out_3822470159260117769[88] = 0.0;
   out_3822470159260117769[89] = 0.0;
   out_3822470159260117769[90] = 0.0;
   out_3822470159260117769[91] = 0.0;
   out_3822470159260117769[92] = 0.0;
   out_3822470159260117769[93] = 0.0;
   out_3822470159260117769[94] = 0.0;
   out_3822470159260117769[95] = 1.0;
   out_3822470159260117769[96] = 0.0;
   out_3822470159260117769[97] = 0.0;
   out_3822470159260117769[98] = 0.0;
   out_3822470159260117769[99] = 0.0;
   out_3822470159260117769[100] = 0.0;
   out_3822470159260117769[101] = 0.0;
   out_3822470159260117769[102] = 0.0;
   out_3822470159260117769[103] = 0.0;
   out_3822470159260117769[104] = 0.0;
   out_3822470159260117769[105] = 0.0;
   out_3822470159260117769[106] = 0.0;
   out_3822470159260117769[107] = 0.0;
   out_3822470159260117769[108] = 0.0;
   out_3822470159260117769[109] = 0.0;
   out_3822470159260117769[110] = 0.0;
   out_3822470159260117769[111] = 0.0;
   out_3822470159260117769[112] = 0.0;
   out_3822470159260117769[113] = 0.0;
   out_3822470159260117769[114] = 1.0;
   out_3822470159260117769[115] = 0.0;
   out_3822470159260117769[116] = 0.0;
   out_3822470159260117769[117] = 0.0;
   out_3822470159260117769[118] = 0.0;
   out_3822470159260117769[119] = 0.0;
   out_3822470159260117769[120] = 0.0;
   out_3822470159260117769[121] = 0.0;
   out_3822470159260117769[122] = 0.0;
   out_3822470159260117769[123] = 0.0;
   out_3822470159260117769[124] = 0.0;
   out_3822470159260117769[125] = 0.0;
   out_3822470159260117769[126] = 0.0;
   out_3822470159260117769[127] = 0.0;
   out_3822470159260117769[128] = 0.0;
   out_3822470159260117769[129] = 0.0;
   out_3822470159260117769[130] = 0.0;
   out_3822470159260117769[131] = 0.0;
   out_3822470159260117769[132] = 0.0;
   out_3822470159260117769[133] = 1.0;
   out_3822470159260117769[134] = 0.0;
   out_3822470159260117769[135] = 0.0;
   out_3822470159260117769[136] = 0.0;
   out_3822470159260117769[137] = 0.0;
   out_3822470159260117769[138] = 0.0;
   out_3822470159260117769[139] = 0.0;
   out_3822470159260117769[140] = 0.0;
   out_3822470159260117769[141] = 0.0;
   out_3822470159260117769[142] = 0.0;
   out_3822470159260117769[143] = 0.0;
   out_3822470159260117769[144] = 0.0;
   out_3822470159260117769[145] = 0.0;
   out_3822470159260117769[146] = 0.0;
   out_3822470159260117769[147] = 0.0;
   out_3822470159260117769[148] = 0.0;
   out_3822470159260117769[149] = 0.0;
   out_3822470159260117769[150] = 0.0;
   out_3822470159260117769[151] = 0.0;
   out_3822470159260117769[152] = 1.0;
   out_3822470159260117769[153] = 0.0;
   out_3822470159260117769[154] = 0.0;
   out_3822470159260117769[155] = 0.0;
   out_3822470159260117769[156] = 0.0;
   out_3822470159260117769[157] = 0.0;
   out_3822470159260117769[158] = 0.0;
   out_3822470159260117769[159] = 0.0;
   out_3822470159260117769[160] = 0.0;
   out_3822470159260117769[161] = 0.0;
   out_3822470159260117769[162] = 0.0;
   out_3822470159260117769[163] = 0.0;
   out_3822470159260117769[164] = 0.0;
   out_3822470159260117769[165] = 0.0;
   out_3822470159260117769[166] = 0.0;
   out_3822470159260117769[167] = 0.0;
   out_3822470159260117769[168] = 0.0;
   out_3822470159260117769[169] = 0.0;
   out_3822470159260117769[170] = 0.0;
   out_3822470159260117769[171] = 1.0;
   out_3822470159260117769[172] = 0.0;
   out_3822470159260117769[173] = 0.0;
   out_3822470159260117769[174] = 0.0;
   out_3822470159260117769[175] = 0.0;
   out_3822470159260117769[176] = 0.0;
   out_3822470159260117769[177] = 0.0;
   out_3822470159260117769[178] = 0.0;
   out_3822470159260117769[179] = 0.0;
   out_3822470159260117769[180] = 0.0;
   out_3822470159260117769[181] = 0.0;
   out_3822470159260117769[182] = 0.0;
   out_3822470159260117769[183] = 0.0;
   out_3822470159260117769[184] = 0.0;
   out_3822470159260117769[185] = 0.0;
   out_3822470159260117769[186] = 0.0;
   out_3822470159260117769[187] = 0.0;
   out_3822470159260117769[188] = 0.0;
   out_3822470159260117769[189] = 0.0;
   out_3822470159260117769[190] = 1.0;
   out_3822470159260117769[191] = 0.0;
   out_3822470159260117769[192] = 0.0;
   out_3822470159260117769[193] = 0.0;
   out_3822470159260117769[194] = 0.0;
   out_3822470159260117769[195] = 0.0;
   out_3822470159260117769[196] = 0.0;
   out_3822470159260117769[197] = 0.0;
   out_3822470159260117769[198] = 0.0;
   out_3822470159260117769[199] = 0.0;
   out_3822470159260117769[200] = 0.0;
   out_3822470159260117769[201] = 0.0;
   out_3822470159260117769[202] = 0.0;
   out_3822470159260117769[203] = 0.0;
   out_3822470159260117769[204] = 0.0;
   out_3822470159260117769[205] = 0.0;
   out_3822470159260117769[206] = 0.0;
   out_3822470159260117769[207] = 0.0;
   out_3822470159260117769[208] = 0.0;
   out_3822470159260117769[209] = 1.0;
   out_3822470159260117769[210] = 0.0;
   out_3822470159260117769[211] = 0.0;
   out_3822470159260117769[212] = 0.0;
   out_3822470159260117769[213] = 0.0;
   out_3822470159260117769[214] = 0.0;
   out_3822470159260117769[215] = 0.0;
   out_3822470159260117769[216] = 0.0;
   out_3822470159260117769[217] = 0.0;
   out_3822470159260117769[218] = 0.0;
   out_3822470159260117769[219] = 0.0;
   out_3822470159260117769[220] = 0.0;
   out_3822470159260117769[221] = 0.0;
   out_3822470159260117769[222] = 0.0;
   out_3822470159260117769[223] = 0.0;
   out_3822470159260117769[224] = 0.0;
   out_3822470159260117769[225] = 0.0;
   out_3822470159260117769[226] = 0.0;
   out_3822470159260117769[227] = 0.0;
   out_3822470159260117769[228] = 1.0;
   out_3822470159260117769[229] = 0.0;
   out_3822470159260117769[230] = 0.0;
   out_3822470159260117769[231] = 0.0;
   out_3822470159260117769[232] = 0.0;
   out_3822470159260117769[233] = 0.0;
   out_3822470159260117769[234] = 0.0;
   out_3822470159260117769[235] = 0.0;
   out_3822470159260117769[236] = 0.0;
   out_3822470159260117769[237] = 0.0;
   out_3822470159260117769[238] = 0.0;
   out_3822470159260117769[239] = 0.0;
   out_3822470159260117769[240] = 0.0;
   out_3822470159260117769[241] = 0.0;
   out_3822470159260117769[242] = 0.0;
   out_3822470159260117769[243] = 0.0;
   out_3822470159260117769[244] = 0.0;
   out_3822470159260117769[245] = 0.0;
   out_3822470159260117769[246] = 0.0;
   out_3822470159260117769[247] = 1.0;
   out_3822470159260117769[248] = 0.0;
   out_3822470159260117769[249] = 0.0;
   out_3822470159260117769[250] = 0.0;
   out_3822470159260117769[251] = 0.0;
   out_3822470159260117769[252] = 0.0;
   out_3822470159260117769[253] = 0.0;
   out_3822470159260117769[254] = 0.0;
   out_3822470159260117769[255] = 0.0;
   out_3822470159260117769[256] = 0.0;
   out_3822470159260117769[257] = 0.0;
   out_3822470159260117769[258] = 0.0;
   out_3822470159260117769[259] = 0.0;
   out_3822470159260117769[260] = 0.0;
   out_3822470159260117769[261] = 0.0;
   out_3822470159260117769[262] = 0.0;
   out_3822470159260117769[263] = 0.0;
   out_3822470159260117769[264] = 0.0;
   out_3822470159260117769[265] = 0.0;
   out_3822470159260117769[266] = 1.0;
   out_3822470159260117769[267] = 0.0;
   out_3822470159260117769[268] = 0.0;
   out_3822470159260117769[269] = 0.0;
   out_3822470159260117769[270] = 0.0;
   out_3822470159260117769[271] = 0.0;
   out_3822470159260117769[272] = 0.0;
   out_3822470159260117769[273] = 0.0;
   out_3822470159260117769[274] = 0.0;
   out_3822470159260117769[275] = 0.0;
   out_3822470159260117769[276] = 0.0;
   out_3822470159260117769[277] = 0.0;
   out_3822470159260117769[278] = 0.0;
   out_3822470159260117769[279] = 0.0;
   out_3822470159260117769[280] = 0.0;
   out_3822470159260117769[281] = 0.0;
   out_3822470159260117769[282] = 0.0;
   out_3822470159260117769[283] = 0.0;
   out_3822470159260117769[284] = 0.0;
   out_3822470159260117769[285] = 1.0;
   out_3822470159260117769[286] = 0.0;
   out_3822470159260117769[287] = 0.0;
   out_3822470159260117769[288] = 0.0;
   out_3822470159260117769[289] = 0.0;
   out_3822470159260117769[290] = 0.0;
   out_3822470159260117769[291] = 0.0;
   out_3822470159260117769[292] = 0.0;
   out_3822470159260117769[293] = 0.0;
   out_3822470159260117769[294] = 0.0;
   out_3822470159260117769[295] = 0.0;
   out_3822470159260117769[296] = 0.0;
   out_3822470159260117769[297] = 0.0;
   out_3822470159260117769[298] = 0.0;
   out_3822470159260117769[299] = 0.0;
   out_3822470159260117769[300] = 0.0;
   out_3822470159260117769[301] = 0.0;
   out_3822470159260117769[302] = 0.0;
   out_3822470159260117769[303] = 0.0;
   out_3822470159260117769[304] = 1.0;
   out_3822470159260117769[305] = 0.0;
   out_3822470159260117769[306] = 0.0;
   out_3822470159260117769[307] = 0.0;
   out_3822470159260117769[308] = 0.0;
   out_3822470159260117769[309] = 0.0;
   out_3822470159260117769[310] = 0.0;
   out_3822470159260117769[311] = 0.0;
   out_3822470159260117769[312] = 0.0;
   out_3822470159260117769[313] = 0.0;
   out_3822470159260117769[314] = 0.0;
   out_3822470159260117769[315] = 0.0;
   out_3822470159260117769[316] = 0.0;
   out_3822470159260117769[317] = 0.0;
   out_3822470159260117769[318] = 0.0;
   out_3822470159260117769[319] = 0.0;
   out_3822470159260117769[320] = 0.0;
   out_3822470159260117769[321] = 0.0;
   out_3822470159260117769[322] = 0.0;
   out_3822470159260117769[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_4539628665192259779) {
   out_4539628665192259779[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_4539628665192259779[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_4539628665192259779[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_4539628665192259779[3] = dt*state[12] + state[3];
   out_4539628665192259779[4] = dt*state[13] + state[4];
   out_4539628665192259779[5] = dt*state[14] + state[5];
   out_4539628665192259779[6] = state[6];
   out_4539628665192259779[7] = state[7];
   out_4539628665192259779[8] = state[8];
   out_4539628665192259779[9] = state[9];
   out_4539628665192259779[10] = state[10];
   out_4539628665192259779[11] = state[11];
   out_4539628665192259779[12] = state[12];
   out_4539628665192259779[13] = state[13];
   out_4539628665192259779[14] = state[14];
   out_4539628665192259779[15] = state[15];
   out_4539628665192259779[16] = state[16];
   out_4539628665192259779[17] = state[17];
}
void F_fun(double *state, double dt, double *out_4279109943705499799) {
   out_4279109943705499799[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4279109943705499799[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4279109943705499799[2] = 0;
   out_4279109943705499799[3] = 0;
   out_4279109943705499799[4] = 0;
   out_4279109943705499799[5] = 0;
   out_4279109943705499799[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4279109943705499799[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4279109943705499799[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_4279109943705499799[9] = 0;
   out_4279109943705499799[10] = 0;
   out_4279109943705499799[11] = 0;
   out_4279109943705499799[12] = 0;
   out_4279109943705499799[13] = 0;
   out_4279109943705499799[14] = 0;
   out_4279109943705499799[15] = 0;
   out_4279109943705499799[16] = 0;
   out_4279109943705499799[17] = 0;
   out_4279109943705499799[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4279109943705499799[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4279109943705499799[20] = 0;
   out_4279109943705499799[21] = 0;
   out_4279109943705499799[22] = 0;
   out_4279109943705499799[23] = 0;
   out_4279109943705499799[24] = 0;
   out_4279109943705499799[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4279109943705499799[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_4279109943705499799[27] = 0;
   out_4279109943705499799[28] = 0;
   out_4279109943705499799[29] = 0;
   out_4279109943705499799[30] = 0;
   out_4279109943705499799[31] = 0;
   out_4279109943705499799[32] = 0;
   out_4279109943705499799[33] = 0;
   out_4279109943705499799[34] = 0;
   out_4279109943705499799[35] = 0;
   out_4279109943705499799[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4279109943705499799[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4279109943705499799[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4279109943705499799[39] = 0;
   out_4279109943705499799[40] = 0;
   out_4279109943705499799[41] = 0;
   out_4279109943705499799[42] = 0;
   out_4279109943705499799[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4279109943705499799[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_4279109943705499799[45] = 0;
   out_4279109943705499799[46] = 0;
   out_4279109943705499799[47] = 0;
   out_4279109943705499799[48] = 0;
   out_4279109943705499799[49] = 0;
   out_4279109943705499799[50] = 0;
   out_4279109943705499799[51] = 0;
   out_4279109943705499799[52] = 0;
   out_4279109943705499799[53] = 0;
   out_4279109943705499799[54] = 0;
   out_4279109943705499799[55] = 0;
   out_4279109943705499799[56] = 0;
   out_4279109943705499799[57] = 1;
   out_4279109943705499799[58] = 0;
   out_4279109943705499799[59] = 0;
   out_4279109943705499799[60] = 0;
   out_4279109943705499799[61] = 0;
   out_4279109943705499799[62] = 0;
   out_4279109943705499799[63] = 0;
   out_4279109943705499799[64] = 0;
   out_4279109943705499799[65] = 0;
   out_4279109943705499799[66] = dt;
   out_4279109943705499799[67] = 0;
   out_4279109943705499799[68] = 0;
   out_4279109943705499799[69] = 0;
   out_4279109943705499799[70] = 0;
   out_4279109943705499799[71] = 0;
   out_4279109943705499799[72] = 0;
   out_4279109943705499799[73] = 0;
   out_4279109943705499799[74] = 0;
   out_4279109943705499799[75] = 0;
   out_4279109943705499799[76] = 1;
   out_4279109943705499799[77] = 0;
   out_4279109943705499799[78] = 0;
   out_4279109943705499799[79] = 0;
   out_4279109943705499799[80] = 0;
   out_4279109943705499799[81] = 0;
   out_4279109943705499799[82] = 0;
   out_4279109943705499799[83] = 0;
   out_4279109943705499799[84] = 0;
   out_4279109943705499799[85] = dt;
   out_4279109943705499799[86] = 0;
   out_4279109943705499799[87] = 0;
   out_4279109943705499799[88] = 0;
   out_4279109943705499799[89] = 0;
   out_4279109943705499799[90] = 0;
   out_4279109943705499799[91] = 0;
   out_4279109943705499799[92] = 0;
   out_4279109943705499799[93] = 0;
   out_4279109943705499799[94] = 0;
   out_4279109943705499799[95] = 1;
   out_4279109943705499799[96] = 0;
   out_4279109943705499799[97] = 0;
   out_4279109943705499799[98] = 0;
   out_4279109943705499799[99] = 0;
   out_4279109943705499799[100] = 0;
   out_4279109943705499799[101] = 0;
   out_4279109943705499799[102] = 0;
   out_4279109943705499799[103] = 0;
   out_4279109943705499799[104] = dt;
   out_4279109943705499799[105] = 0;
   out_4279109943705499799[106] = 0;
   out_4279109943705499799[107] = 0;
   out_4279109943705499799[108] = 0;
   out_4279109943705499799[109] = 0;
   out_4279109943705499799[110] = 0;
   out_4279109943705499799[111] = 0;
   out_4279109943705499799[112] = 0;
   out_4279109943705499799[113] = 0;
   out_4279109943705499799[114] = 1;
   out_4279109943705499799[115] = 0;
   out_4279109943705499799[116] = 0;
   out_4279109943705499799[117] = 0;
   out_4279109943705499799[118] = 0;
   out_4279109943705499799[119] = 0;
   out_4279109943705499799[120] = 0;
   out_4279109943705499799[121] = 0;
   out_4279109943705499799[122] = 0;
   out_4279109943705499799[123] = 0;
   out_4279109943705499799[124] = 0;
   out_4279109943705499799[125] = 0;
   out_4279109943705499799[126] = 0;
   out_4279109943705499799[127] = 0;
   out_4279109943705499799[128] = 0;
   out_4279109943705499799[129] = 0;
   out_4279109943705499799[130] = 0;
   out_4279109943705499799[131] = 0;
   out_4279109943705499799[132] = 0;
   out_4279109943705499799[133] = 1;
   out_4279109943705499799[134] = 0;
   out_4279109943705499799[135] = 0;
   out_4279109943705499799[136] = 0;
   out_4279109943705499799[137] = 0;
   out_4279109943705499799[138] = 0;
   out_4279109943705499799[139] = 0;
   out_4279109943705499799[140] = 0;
   out_4279109943705499799[141] = 0;
   out_4279109943705499799[142] = 0;
   out_4279109943705499799[143] = 0;
   out_4279109943705499799[144] = 0;
   out_4279109943705499799[145] = 0;
   out_4279109943705499799[146] = 0;
   out_4279109943705499799[147] = 0;
   out_4279109943705499799[148] = 0;
   out_4279109943705499799[149] = 0;
   out_4279109943705499799[150] = 0;
   out_4279109943705499799[151] = 0;
   out_4279109943705499799[152] = 1;
   out_4279109943705499799[153] = 0;
   out_4279109943705499799[154] = 0;
   out_4279109943705499799[155] = 0;
   out_4279109943705499799[156] = 0;
   out_4279109943705499799[157] = 0;
   out_4279109943705499799[158] = 0;
   out_4279109943705499799[159] = 0;
   out_4279109943705499799[160] = 0;
   out_4279109943705499799[161] = 0;
   out_4279109943705499799[162] = 0;
   out_4279109943705499799[163] = 0;
   out_4279109943705499799[164] = 0;
   out_4279109943705499799[165] = 0;
   out_4279109943705499799[166] = 0;
   out_4279109943705499799[167] = 0;
   out_4279109943705499799[168] = 0;
   out_4279109943705499799[169] = 0;
   out_4279109943705499799[170] = 0;
   out_4279109943705499799[171] = 1;
   out_4279109943705499799[172] = 0;
   out_4279109943705499799[173] = 0;
   out_4279109943705499799[174] = 0;
   out_4279109943705499799[175] = 0;
   out_4279109943705499799[176] = 0;
   out_4279109943705499799[177] = 0;
   out_4279109943705499799[178] = 0;
   out_4279109943705499799[179] = 0;
   out_4279109943705499799[180] = 0;
   out_4279109943705499799[181] = 0;
   out_4279109943705499799[182] = 0;
   out_4279109943705499799[183] = 0;
   out_4279109943705499799[184] = 0;
   out_4279109943705499799[185] = 0;
   out_4279109943705499799[186] = 0;
   out_4279109943705499799[187] = 0;
   out_4279109943705499799[188] = 0;
   out_4279109943705499799[189] = 0;
   out_4279109943705499799[190] = 1;
   out_4279109943705499799[191] = 0;
   out_4279109943705499799[192] = 0;
   out_4279109943705499799[193] = 0;
   out_4279109943705499799[194] = 0;
   out_4279109943705499799[195] = 0;
   out_4279109943705499799[196] = 0;
   out_4279109943705499799[197] = 0;
   out_4279109943705499799[198] = 0;
   out_4279109943705499799[199] = 0;
   out_4279109943705499799[200] = 0;
   out_4279109943705499799[201] = 0;
   out_4279109943705499799[202] = 0;
   out_4279109943705499799[203] = 0;
   out_4279109943705499799[204] = 0;
   out_4279109943705499799[205] = 0;
   out_4279109943705499799[206] = 0;
   out_4279109943705499799[207] = 0;
   out_4279109943705499799[208] = 0;
   out_4279109943705499799[209] = 1;
   out_4279109943705499799[210] = 0;
   out_4279109943705499799[211] = 0;
   out_4279109943705499799[212] = 0;
   out_4279109943705499799[213] = 0;
   out_4279109943705499799[214] = 0;
   out_4279109943705499799[215] = 0;
   out_4279109943705499799[216] = 0;
   out_4279109943705499799[217] = 0;
   out_4279109943705499799[218] = 0;
   out_4279109943705499799[219] = 0;
   out_4279109943705499799[220] = 0;
   out_4279109943705499799[221] = 0;
   out_4279109943705499799[222] = 0;
   out_4279109943705499799[223] = 0;
   out_4279109943705499799[224] = 0;
   out_4279109943705499799[225] = 0;
   out_4279109943705499799[226] = 0;
   out_4279109943705499799[227] = 0;
   out_4279109943705499799[228] = 1;
   out_4279109943705499799[229] = 0;
   out_4279109943705499799[230] = 0;
   out_4279109943705499799[231] = 0;
   out_4279109943705499799[232] = 0;
   out_4279109943705499799[233] = 0;
   out_4279109943705499799[234] = 0;
   out_4279109943705499799[235] = 0;
   out_4279109943705499799[236] = 0;
   out_4279109943705499799[237] = 0;
   out_4279109943705499799[238] = 0;
   out_4279109943705499799[239] = 0;
   out_4279109943705499799[240] = 0;
   out_4279109943705499799[241] = 0;
   out_4279109943705499799[242] = 0;
   out_4279109943705499799[243] = 0;
   out_4279109943705499799[244] = 0;
   out_4279109943705499799[245] = 0;
   out_4279109943705499799[246] = 0;
   out_4279109943705499799[247] = 1;
   out_4279109943705499799[248] = 0;
   out_4279109943705499799[249] = 0;
   out_4279109943705499799[250] = 0;
   out_4279109943705499799[251] = 0;
   out_4279109943705499799[252] = 0;
   out_4279109943705499799[253] = 0;
   out_4279109943705499799[254] = 0;
   out_4279109943705499799[255] = 0;
   out_4279109943705499799[256] = 0;
   out_4279109943705499799[257] = 0;
   out_4279109943705499799[258] = 0;
   out_4279109943705499799[259] = 0;
   out_4279109943705499799[260] = 0;
   out_4279109943705499799[261] = 0;
   out_4279109943705499799[262] = 0;
   out_4279109943705499799[263] = 0;
   out_4279109943705499799[264] = 0;
   out_4279109943705499799[265] = 0;
   out_4279109943705499799[266] = 1;
   out_4279109943705499799[267] = 0;
   out_4279109943705499799[268] = 0;
   out_4279109943705499799[269] = 0;
   out_4279109943705499799[270] = 0;
   out_4279109943705499799[271] = 0;
   out_4279109943705499799[272] = 0;
   out_4279109943705499799[273] = 0;
   out_4279109943705499799[274] = 0;
   out_4279109943705499799[275] = 0;
   out_4279109943705499799[276] = 0;
   out_4279109943705499799[277] = 0;
   out_4279109943705499799[278] = 0;
   out_4279109943705499799[279] = 0;
   out_4279109943705499799[280] = 0;
   out_4279109943705499799[281] = 0;
   out_4279109943705499799[282] = 0;
   out_4279109943705499799[283] = 0;
   out_4279109943705499799[284] = 0;
   out_4279109943705499799[285] = 1;
   out_4279109943705499799[286] = 0;
   out_4279109943705499799[287] = 0;
   out_4279109943705499799[288] = 0;
   out_4279109943705499799[289] = 0;
   out_4279109943705499799[290] = 0;
   out_4279109943705499799[291] = 0;
   out_4279109943705499799[292] = 0;
   out_4279109943705499799[293] = 0;
   out_4279109943705499799[294] = 0;
   out_4279109943705499799[295] = 0;
   out_4279109943705499799[296] = 0;
   out_4279109943705499799[297] = 0;
   out_4279109943705499799[298] = 0;
   out_4279109943705499799[299] = 0;
   out_4279109943705499799[300] = 0;
   out_4279109943705499799[301] = 0;
   out_4279109943705499799[302] = 0;
   out_4279109943705499799[303] = 0;
   out_4279109943705499799[304] = 1;
   out_4279109943705499799[305] = 0;
   out_4279109943705499799[306] = 0;
   out_4279109943705499799[307] = 0;
   out_4279109943705499799[308] = 0;
   out_4279109943705499799[309] = 0;
   out_4279109943705499799[310] = 0;
   out_4279109943705499799[311] = 0;
   out_4279109943705499799[312] = 0;
   out_4279109943705499799[313] = 0;
   out_4279109943705499799[314] = 0;
   out_4279109943705499799[315] = 0;
   out_4279109943705499799[316] = 0;
   out_4279109943705499799[317] = 0;
   out_4279109943705499799[318] = 0;
   out_4279109943705499799[319] = 0;
   out_4279109943705499799[320] = 0;
   out_4279109943705499799[321] = 0;
   out_4279109943705499799[322] = 0;
   out_4279109943705499799[323] = 1;
}
void h_4(double *state, double *unused, double *out_4468827944582346069) {
   out_4468827944582346069[0] = state[6] + state[9];
   out_4468827944582346069[1] = state[7] + state[10];
   out_4468827944582346069[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_4576631161314830376) {
   out_4576631161314830376[0] = 0;
   out_4576631161314830376[1] = 0;
   out_4576631161314830376[2] = 0;
   out_4576631161314830376[3] = 0;
   out_4576631161314830376[4] = 0;
   out_4576631161314830376[5] = 0;
   out_4576631161314830376[6] = 1;
   out_4576631161314830376[7] = 0;
   out_4576631161314830376[8] = 0;
   out_4576631161314830376[9] = 1;
   out_4576631161314830376[10] = 0;
   out_4576631161314830376[11] = 0;
   out_4576631161314830376[12] = 0;
   out_4576631161314830376[13] = 0;
   out_4576631161314830376[14] = 0;
   out_4576631161314830376[15] = 0;
   out_4576631161314830376[16] = 0;
   out_4576631161314830376[17] = 0;
   out_4576631161314830376[18] = 0;
   out_4576631161314830376[19] = 0;
   out_4576631161314830376[20] = 0;
   out_4576631161314830376[21] = 0;
   out_4576631161314830376[22] = 0;
   out_4576631161314830376[23] = 0;
   out_4576631161314830376[24] = 0;
   out_4576631161314830376[25] = 1;
   out_4576631161314830376[26] = 0;
   out_4576631161314830376[27] = 0;
   out_4576631161314830376[28] = 1;
   out_4576631161314830376[29] = 0;
   out_4576631161314830376[30] = 0;
   out_4576631161314830376[31] = 0;
   out_4576631161314830376[32] = 0;
   out_4576631161314830376[33] = 0;
   out_4576631161314830376[34] = 0;
   out_4576631161314830376[35] = 0;
   out_4576631161314830376[36] = 0;
   out_4576631161314830376[37] = 0;
   out_4576631161314830376[38] = 0;
   out_4576631161314830376[39] = 0;
   out_4576631161314830376[40] = 0;
   out_4576631161314830376[41] = 0;
   out_4576631161314830376[42] = 0;
   out_4576631161314830376[43] = 0;
   out_4576631161314830376[44] = 1;
   out_4576631161314830376[45] = 0;
   out_4576631161314830376[46] = 0;
   out_4576631161314830376[47] = 1;
   out_4576631161314830376[48] = 0;
   out_4576631161314830376[49] = 0;
   out_4576631161314830376[50] = 0;
   out_4576631161314830376[51] = 0;
   out_4576631161314830376[52] = 0;
   out_4576631161314830376[53] = 0;
}
void h_10(double *state, double *unused, double *out_7687610123346311094) {
   out_7687610123346311094[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_7687610123346311094[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_7687610123346311094[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_1094040887207712728) {
   out_1094040887207712728[0] = 0;
   out_1094040887207712728[1] = 9.8100000000000005*cos(state[1]);
   out_1094040887207712728[2] = 0;
   out_1094040887207712728[3] = 0;
   out_1094040887207712728[4] = -state[8];
   out_1094040887207712728[5] = state[7];
   out_1094040887207712728[6] = 0;
   out_1094040887207712728[7] = state[5];
   out_1094040887207712728[8] = -state[4];
   out_1094040887207712728[9] = 0;
   out_1094040887207712728[10] = 0;
   out_1094040887207712728[11] = 0;
   out_1094040887207712728[12] = 1;
   out_1094040887207712728[13] = 0;
   out_1094040887207712728[14] = 0;
   out_1094040887207712728[15] = 1;
   out_1094040887207712728[16] = 0;
   out_1094040887207712728[17] = 0;
   out_1094040887207712728[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_1094040887207712728[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_1094040887207712728[20] = 0;
   out_1094040887207712728[21] = state[8];
   out_1094040887207712728[22] = 0;
   out_1094040887207712728[23] = -state[6];
   out_1094040887207712728[24] = -state[5];
   out_1094040887207712728[25] = 0;
   out_1094040887207712728[26] = state[3];
   out_1094040887207712728[27] = 0;
   out_1094040887207712728[28] = 0;
   out_1094040887207712728[29] = 0;
   out_1094040887207712728[30] = 0;
   out_1094040887207712728[31] = 1;
   out_1094040887207712728[32] = 0;
   out_1094040887207712728[33] = 0;
   out_1094040887207712728[34] = 1;
   out_1094040887207712728[35] = 0;
   out_1094040887207712728[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_1094040887207712728[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_1094040887207712728[38] = 0;
   out_1094040887207712728[39] = -state[7];
   out_1094040887207712728[40] = state[6];
   out_1094040887207712728[41] = 0;
   out_1094040887207712728[42] = state[4];
   out_1094040887207712728[43] = -state[3];
   out_1094040887207712728[44] = 0;
   out_1094040887207712728[45] = 0;
   out_1094040887207712728[46] = 0;
   out_1094040887207712728[47] = 0;
   out_1094040887207712728[48] = 0;
   out_1094040887207712728[49] = 0;
   out_1094040887207712728[50] = 1;
   out_1094040887207712728[51] = 0;
   out_1094040887207712728[52] = 0;
   out_1094040887207712728[53] = 1;
}
void h_13(double *state, double *unused, double *out_649951466656749498) {
   out_649951466656749498[0] = state[3];
   out_649951466656749498[1] = state[4];
   out_649951466656749498[2] = state[5];
}
void H_13(double *state, double *unused, double *out_6259481704078020311) {
   out_6259481704078020311[0] = 0;
   out_6259481704078020311[1] = 0;
   out_6259481704078020311[2] = 0;
   out_6259481704078020311[3] = 1;
   out_6259481704078020311[4] = 0;
   out_6259481704078020311[5] = 0;
   out_6259481704078020311[6] = 0;
   out_6259481704078020311[7] = 0;
   out_6259481704078020311[8] = 0;
   out_6259481704078020311[9] = 0;
   out_6259481704078020311[10] = 0;
   out_6259481704078020311[11] = 0;
   out_6259481704078020311[12] = 0;
   out_6259481704078020311[13] = 0;
   out_6259481704078020311[14] = 0;
   out_6259481704078020311[15] = 0;
   out_6259481704078020311[16] = 0;
   out_6259481704078020311[17] = 0;
   out_6259481704078020311[18] = 0;
   out_6259481704078020311[19] = 0;
   out_6259481704078020311[20] = 0;
   out_6259481704078020311[21] = 0;
   out_6259481704078020311[22] = 1;
   out_6259481704078020311[23] = 0;
   out_6259481704078020311[24] = 0;
   out_6259481704078020311[25] = 0;
   out_6259481704078020311[26] = 0;
   out_6259481704078020311[27] = 0;
   out_6259481704078020311[28] = 0;
   out_6259481704078020311[29] = 0;
   out_6259481704078020311[30] = 0;
   out_6259481704078020311[31] = 0;
   out_6259481704078020311[32] = 0;
   out_6259481704078020311[33] = 0;
   out_6259481704078020311[34] = 0;
   out_6259481704078020311[35] = 0;
   out_6259481704078020311[36] = 0;
   out_6259481704078020311[37] = 0;
   out_6259481704078020311[38] = 0;
   out_6259481704078020311[39] = 0;
   out_6259481704078020311[40] = 0;
   out_6259481704078020311[41] = 1;
   out_6259481704078020311[42] = 0;
   out_6259481704078020311[43] = 0;
   out_6259481704078020311[44] = 0;
   out_6259481704078020311[45] = 0;
   out_6259481704078020311[46] = 0;
   out_6259481704078020311[47] = 0;
   out_6259481704078020311[48] = 0;
   out_6259481704078020311[49] = 0;
   out_6259481704078020311[50] = 0;
   out_6259481704078020311[51] = 0;
   out_6259481704078020311[52] = 0;
   out_6259481704078020311[53] = 0;
}
void h_14(double *state, double *unused, double *out_5063684233320489319) {
   out_5063684233320489319[0] = state[6];
   out_5063684233320489319[1] = state[7];
   out_5063684233320489319[2] = state[8];
}
void H_14(double *state, double *unused, double *out_8539872017654314905) {
   out_8539872017654314905[0] = 0;
   out_8539872017654314905[1] = 0;
   out_8539872017654314905[2] = 0;
   out_8539872017654314905[3] = 0;
   out_8539872017654314905[4] = 0;
   out_8539872017654314905[5] = 0;
   out_8539872017654314905[6] = 1;
   out_8539872017654314905[7] = 0;
   out_8539872017654314905[8] = 0;
   out_8539872017654314905[9] = 0;
   out_8539872017654314905[10] = 0;
   out_8539872017654314905[11] = 0;
   out_8539872017654314905[12] = 0;
   out_8539872017654314905[13] = 0;
   out_8539872017654314905[14] = 0;
   out_8539872017654314905[15] = 0;
   out_8539872017654314905[16] = 0;
   out_8539872017654314905[17] = 0;
   out_8539872017654314905[18] = 0;
   out_8539872017654314905[19] = 0;
   out_8539872017654314905[20] = 0;
   out_8539872017654314905[21] = 0;
   out_8539872017654314905[22] = 0;
   out_8539872017654314905[23] = 0;
   out_8539872017654314905[24] = 0;
   out_8539872017654314905[25] = 1;
   out_8539872017654314905[26] = 0;
   out_8539872017654314905[27] = 0;
   out_8539872017654314905[28] = 0;
   out_8539872017654314905[29] = 0;
   out_8539872017654314905[30] = 0;
   out_8539872017654314905[31] = 0;
   out_8539872017654314905[32] = 0;
   out_8539872017654314905[33] = 0;
   out_8539872017654314905[34] = 0;
   out_8539872017654314905[35] = 0;
   out_8539872017654314905[36] = 0;
   out_8539872017654314905[37] = 0;
   out_8539872017654314905[38] = 0;
   out_8539872017654314905[39] = 0;
   out_8539872017654314905[40] = 0;
   out_8539872017654314905[41] = 0;
   out_8539872017654314905[42] = 0;
   out_8539872017654314905[43] = 0;
   out_8539872017654314905[44] = 1;
   out_8539872017654314905[45] = 0;
   out_8539872017654314905[46] = 0;
   out_8539872017654314905[47] = 0;
   out_8539872017654314905[48] = 0;
   out_8539872017654314905[49] = 0;
   out_8539872017654314905[50] = 0;
   out_8539872017654314905[51] = 0;
   out_8539872017654314905[52] = 0;
   out_8539872017654314905[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_1903969700519405728) {
  err_fun(nom_x, delta_x, out_1903969700519405728);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3362098330798648840) {
  inv_err_fun(nom_x, true_x, out_3362098330798648840);
}
void pose_H_mod_fun(double *state, double *out_3822470159260117769) {
  H_mod_fun(state, out_3822470159260117769);
}
void pose_f_fun(double *state, double dt, double *out_4539628665192259779) {
  f_fun(state,  dt, out_4539628665192259779);
}
void pose_F_fun(double *state, double dt, double *out_4279109943705499799) {
  F_fun(state,  dt, out_4279109943705499799);
}
void pose_h_4(double *state, double *unused, double *out_4468827944582346069) {
  h_4(state, unused, out_4468827944582346069);
}
void pose_H_4(double *state, double *unused, double *out_4576631161314830376) {
  H_4(state, unused, out_4576631161314830376);
}
void pose_h_10(double *state, double *unused, double *out_7687610123346311094) {
  h_10(state, unused, out_7687610123346311094);
}
void pose_H_10(double *state, double *unused, double *out_1094040887207712728) {
  H_10(state, unused, out_1094040887207712728);
}
void pose_h_13(double *state, double *unused, double *out_649951466656749498) {
  h_13(state, unused, out_649951466656749498);
}
void pose_H_13(double *state, double *unused, double *out_6259481704078020311) {
  H_13(state, unused, out_6259481704078020311);
}
void pose_h_14(double *state, double *unused, double *out_5063684233320489319) {
  h_14(state, unused, out_5063684233320489319);
}
void pose_H_14(double *state, double *unused, double *out_8539872017654314905) {
  H_14(state, unused, out_8539872017654314905);
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

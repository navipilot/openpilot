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
void err_fun(double *nom_x, double *delta_x, double *out_5984861409348236479) {
   out_5984861409348236479[0] = delta_x[0] + nom_x[0];
   out_5984861409348236479[1] = delta_x[1] + nom_x[1];
   out_5984861409348236479[2] = delta_x[2] + nom_x[2];
   out_5984861409348236479[3] = delta_x[3] + nom_x[3];
   out_5984861409348236479[4] = delta_x[4] + nom_x[4];
   out_5984861409348236479[5] = delta_x[5] + nom_x[5];
   out_5984861409348236479[6] = delta_x[6] + nom_x[6];
   out_5984861409348236479[7] = delta_x[7] + nom_x[7];
   out_5984861409348236479[8] = delta_x[8] + nom_x[8];
   out_5984861409348236479[9] = delta_x[9] + nom_x[9];
   out_5984861409348236479[10] = delta_x[10] + nom_x[10];
   out_5984861409348236479[11] = delta_x[11] + nom_x[11];
   out_5984861409348236479[12] = delta_x[12] + nom_x[12];
   out_5984861409348236479[13] = delta_x[13] + nom_x[13];
   out_5984861409348236479[14] = delta_x[14] + nom_x[14];
   out_5984861409348236479[15] = delta_x[15] + nom_x[15];
   out_5984861409348236479[16] = delta_x[16] + nom_x[16];
   out_5984861409348236479[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1707565234864444406) {
   out_1707565234864444406[0] = -nom_x[0] + true_x[0];
   out_1707565234864444406[1] = -nom_x[1] + true_x[1];
   out_1707565234864444406[2] = -nom_x[2] + true_x[2];
   out_1707565234864444406[3] = -nom_x[3] + true_x[3];
   out_1707565234864444406[4] = -nom_x[4] + true_x[4];
   out_1707565234864444406[5] = -nom_x[5] + true_x[5];
   out_1707565234864444406[6] = -nom_x[6] + true_x[6];
   out_1707565234864444406[7] = -nom_x[7] + true_x[7];
   out_1707565234864444406[8] = -nom_x[8] + true_x[8];
   out_1707565234864444406[9] = -nom_x[9] + true_x[9];
   out_1707565234864444406[10] = -nom_x[10] + true_x[10];
   out_1707565234864444406[11] = -nom_x[11] + true_x[11];
   out_1707565234864444406[12] = -nom_x[12] + true_x[12];
   out_1707565234864444406[13] = -nom_x[13] + true_x[13];
   out_1707565234864444406[14] = -nom_x[14] + true_x[14];
   out_1707565234864444406[15] = -nom_x[15] + true_x[15];
   out_1707565234864444406[16] = -nom_x[16] + true_x[16];
   out_1707565234864444406[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_938651669609787352) {
   out_938651669609787352[0] = 1.0;
   out_938651669609787352[1] = 0.0;
   out_938651669609787352[2] = 0.0;
   out_938651669609787352[3] = 0.0;
   out_938651669609787352[4] = 0.0;
   out_938651669609787352[5] = 0.0;
   out_938651669609787352[6] = 0.0;
   out_938651669609787352[7] = 0.0;
   out_938651669609787352[8] = 0.0;
   out_938651669609787352[9] = 0.0;
   out_938651669609787352[10] = 0.0;
   out_938651669609787352[11] = 0.0;
   out_938651669609787352[12] = 0.0;
   out_938651669609787352[13] = 0.0;
   out_938651669609787352[14] = 0.0;
   out_938651669609787352[15] = 0.0;
   out_938651669609787352[16] = 0.0;
   out_938651669609787352[17] = 0.0;
   out_938651669609787352[18] = 0.0;
   out_938651669609787352[19] = 1.0;
   out_938651669609787352[20] = 0.0;
   out_938651669609787352[21] = 0.0;
   out_938651669609787352[22] = 0.0;
   out_938651669609787352[23] = 0.0;
   out_938651669609787352[24] = 0.0;
   out_938651669609787352[25] = 0.0;
   out_938651669609787352[26] = 0.0;
   out_938651669609787352[27] = 0.0;
   out_938651669609787352[28] = 0.0;
   out_938651669609787352[29] = 0.0;
   out_938651669609787352[30] = 0.0;
   out_938651669609787352[31] = 0.0;
   out_938651669609787352[32] = 0.0;
   out_938651669609787352[33] = 0.0;
   out_938651669609787352[34] = 0.0;
   out_938651669609787352[35] = 0.0;
   out_938651669609787352[36] = 0.0;
   out_938651669609787352[37] = 0.0;
   out_938651669609787352[38] = 1.0;
   out_938651669609787352[39] = 0.0;
   out_938651669609787352[40] = 0.0;
   out_938651669609787352[41] = 0.0;
   out_938651669609787352[42] = 0.0;
   out_938651669609787352[43] = 0.0;
   out_938651669609787352[44] = 0.0;
   out_938651669609787352[45] = 0.0;
   out_938651669609787352[46] = 0.0;
   out_938651669609787352[47] = 0.0;
   out_938651669609787352[48] = 0.0;
   out_938651669609787352[49] = 0.0;
   out_938651669609787352[50] = 0.0;
   out_938651669609787352[51] = 0.0;
   out_938651669609787352[52] = 0.0;
   out_938651669609787352[53] = 0.0;
   out_938651669609787352[54] = 0.0;
   out_938651669609787352[55] = 0.0;
   out_938651669609787352[56] = 0.0;
   out_938651669609787352[57] = 1.0;
   out_938651669609787352[58] = 0.0;
   out_938651669609787352[59] = 0.0;
   out_938651669609787352[60] = 0.0;
   out_938651669609787352[61] = 0.0;
   out_938651669609787352[62] = 0.0;
   out_938651669609787352[63] = 0.0;
   out_938651669609787352[64] = 0.0;
   out_938651669609787352[65] = 0.0;
   out_938651669609787352[66] = 0.0;
   out_938651669609787352[67] = 0.0;
   out_938651669609787352[68] = 0.0;
   out_938651669609787352[69] = 0.0;
   out_938651669609787352[70] = 0.0;
   out_938651669609787352[71] = 0.0;
   out_938651669609787352[72] = 0.0;
   out_938651669609787352[73] = 0.0;
   out_938651669609787352[74] = 0.0;
   out_938651669609787352[75] = 0.0;
   out_938651669609787352[76] = 1.0;
   out_938651669609787352[77] = 0.0;
   out_938651669609787352[78] = 0.0;
   out_938651669609787352[79] = 0.0;
   out_938651669609787352[80] = 0.0;
   out_938651669609787352[81] = 0.0;
   out_938651669609787352[82] = 0.0;
   out_938651669609787352[83] = 0.0;
   out_938651669609787352[84] = 0.0;
   out_938651669609787352[85] = 0.0;
   out_938651669609787352[86] = 0.0;
   out_938651669609787352[87] = 0.0;
   out_938651669609787352[88] = 0.0;
   out_938651669609787352[89] = 0.0;
   out_938651669609787352[90] = 0.0;
   out_938651669609787352[91] = 0.0;
   out_938651669609787352[92] = 0.0;
   out_938651669609787352[93] = 0.0;
   out_938651669609787352[94] = 0.0;
   out_938651669609787352[95] = 1.0;
   out_938651669609787352[96] = 0.0;
   out_938651669609787352[97] = 0.0;
   out_938651669609787352[98] = 0.0;
   out_938651669609787352[99] = 0.0;
   out_938651669609787352[100] = 0.0;
   out_938651669609787352[101] = 0.0;
   out_938651669609787352[102] = 0.0;
   out_938651669609787352[103] = 0.0;
   out_938651669609787352[104] = 0.0;
   out_938651669609787352[105] = 0.0;
   out_938651669609787352[106] = 0.0;
   out_938651669609787352[107] = 0.0;
   out_938651669609787352[108] = 0.0;
   out_938651669609787352[109] = 0.0;
   out_938651669609787352[110] = 0.0;
   out_938651669609787352[111] = 0.0;
   out_938651669609787352[112] = 0.0;
   out_938651669609787352[113] = 0.0;
   out_938651669609787352[114] = 1.0;
   out_938651669609787352[115] = 0.0;
   out_938651669609787352[116] = 0.0;
   out_938651669609787352[117] = 0.0;
   out_938651669609787352[118] = 0.0;
   out_938651669609787352[119] = 0.0;
   out_938651669609787352[120] = 0.0;
   out_938651669609787352[121] = 0.0;
   out_938651669609787352[122] = 0.0;
   out_938651669609787352[123] = 0.0;
   out_938651669609787352[124] = 0.0;
   out_938651669609787352[125] = 0.0;
   out_938651669609787352[126] = 0.0;
   out_938651669609787352[127] = 0.0;
   out_938651669609787352[128] = 0.0;
   out_938651669609787352[129] = 0.0;
   out_938651669609787352[130] = 0.0;
   out_938651669609787352[131] = 0.0;
   out_938651669609787352[132] = 0.0;
   out_938651669609787352[133] = 1.0;
   out_938651669609787352[134] = 0.0;
   out_938651669609787352[135] = 0.0;
   out_938651669609787352[136] = 0.0;
   out_938651669609787352[137] = 0.0;
   out_938651669609787352[138] = 0.0;
   out_938651669609787352[139] = 0.0;
   out_938651669609787352[140] = 0.0;
   out_938651669609787352[141] = 0.0;
   out_938651669609787352[142] = 0.0;
   out_938651669609787352[143] = 0.0;
   out_938651669609787352[144] = 0.0;
   out_938651669609787352[145] = 0.0;
   out_938651669609787352[146] = 0.0;
   out_938651669609787352[147] = 0.0;
   out_938651669609787352[148] = 0.0;
   out_938651669609787352[149] = 0.0;
   out_938651669609787352[150] = 0.0;
   out_938651669609787352[151] = 0.0;
   out_938651669609787352[152] = 1.0;
   out_938651669609787352[153] = 0.0;
   out_938651669609787352[154] = 0.0;
   out_938651669609787352[155] = 0.0;
   out_938651669609787352[156] = 0.0;
   out_938651669609787352[157] = 0.0;
   out_938651669609787352[158] = 0.0;
   out_938651669609787352[159] = 0.0;
   out_938651669609787352[160] = 0.0;
   out_938651669609787352[161] = 0.0;
   out_938651669609787352[162] = 0.0;
   out_938651669609787352[163] = 0.0;
   out_938651669609787352[164] = 0.0;
   out_938651669609787352[165] = 0.0;
   out_938651669609787352[166] = 0.0;
   out_938651669609787352[167] = 0.0;
   out_938651669609787352[168] = 0.0;
   out_938651669609787352[169] = 0.0;
   out_938651669609787352[170] = 0.0;
   out_938651669609787352[171] = 1.0;
   out_938651669609787352[172] = 0.0;
   out_938651669609787352[173] = 0.0;
   out_938651669609787352[174] = 0.0;
   out_938651669609787352[175] = 0.0;
   out_938651669609787352[176] = 0.0;
   out_938651669609787352[177] = 0.0;
   out_938651669609787352[178] = 0.0;
   out_938651669609787352[179] = 0.0;
   out_938651669609787352[180] = 0.0;
   out_938651669609787352[181] = 0.0;
   out_938651669609787352[182] = 0.0;
   out_938651669609787352[183] = 0.0;
   out_938651669609787352[184] = 0.0;
   out_938651669609787352[185] = 0.0;
   out_938651669609787352[186] = 0.0;
   out_938651669609787352[187] = 0.0;
   out_938651669609787352[188] = 0.0;
   out_938651669609787352[189] = 0.0;
   out_938651669609787352[190] = 1.0;
   out_938651669609787352[191] = 0.0;
   out_938651669609787352[192] = 0.0;
   out_938651669609787352[193] = 0.0;
   out_938651669609787352[194] = 0.0;
   out_938651669609787352[195] = 0.0;
   out_938651669609787352[196] = 0.0;
   out_938651669609787352[197] = 0.0;
   out_938651669609787352[198] = 0.0;
   out_938651669609787352[199] = 0.0;
   out_938651669609787352[200] = 0.0;
   out_938651669609787352[201] = 0.0;
   out_938651669609787352[202] = 0.0;
   out_938651669609787352[203] = 0.0;
   out_938651669609787352[204] = 0.0;
   out_938651669609787352[205] = 0.0;
   out_938651669609787352[206] = 0.0;
   out_938651669609787352[207] = 0.0;
   out_938651669609787352[208] = 0.0;
   out_938651669609787352[209] = 1.0;
   out_938651669609787352[210] = 0.0;
   out_938651669609787352[211] = 0.0;
   out_938651669609787352[212] = 0.0;
   out_938651669609787352[213] = 0.0;
   out_938651669609787352[214] = 0.0;
   out_938651669609787352[215] = 0.0;
   out_938651669609787352[216] = 0.0;
   out_938651669609787352[217] = 0.0;
   out_938651669609787352[218] = 0.0;
   out_938651669609787352[219] = 0.0;
   out_938651669609787352[220] = 0.0;
   out_938651669609787352[221] = 0.0;
   out_938651669609787352[222] = 0.0;
   out_938651669609787352[223] = 0.0;
   out_938651669609787352[224] = 0.0;
   out_938651669609787352[225] = 0.0;
   out_938651669609787352[226] = 0.0;
   out_938651669609787352[227] = 0.0;
   out_938651669609787352[228] = 1.0;
   out_938651669609787352[229] = 0.0;
   out_938651669609787352[230] = 0.0;
   out_938651669609787352[231] = 0.0;
   out_938651669609787352[232] = 0.0;
   out_938651669609787352[233] = 0.0;
   out_938651669609787352[234] = 0.0;
   out_938651669609787352[235] = 0.0;
   out_938651669609787352[236] = 0.0;
   out_938651669609787352[237] = 0.0;
   out_938651669609787352[238] = 0.0;
   out_938651669609787352[239] = 0.0;
   out_938651669609787352[240] = 0.0;
   out_938651669609787352[241] = 0.0;
   out_938651669609787352[242] = 0.0;
   out_938651669609787352[243] = 0.0;
   out_938651669609787352[244] = 0.0;
   out_938651669609787352[245] = 0.0;
   out_938651669609787352[246] = 0.0;
   out_938651669609787352[247] = 1.0;
   out_938651669609787352[248] = 0.0;
   out_938651669609787352[249] = 0.0;
   out_938651669609787352[250] = 0.0;
   out_938651669609787352[251] = 0.0;
   out_938651669609787352[252] = 0.0;
   out_938651669609787352[253] = 0.0;
   out_938651669609787352[254] = 0.0;
   out_938651669609787352[255] = 0.0;
   out_938651669609787352[256] = 0.0;
   out_938651669609787352[257] = 0.0;
   out_938651669609787352[258] = 0.0;
   out_938651669609787352[259] = 0.0;
   out_938651669609787352[260] = 0.0;
   out_938651669609787352[261] = 0.0;
   out_938651669609787352[262] = 0.0;
   out_938651669609787352[263] = 0.0;
   out_938651669609787352[264] = 0.0;
   out_938651669609787352[265] = 0.0;
   out_938651669609787352[266] = 1.0;
   out_938651669609787352[267] = 0.0;
   out_938651669609787352[268] = 0.0;
   out_938651669609787352[269] = 0.0;
   out_938651669609787352[270] = 0.0;
   out_938651669609787352[271] = 0.0;
   out_938651669609787352[272] = 0.0;
   out_938651669609787352[273] = 0.0;
   out_938651669609787352[274] = 0.0;
   out_938651669609787352[275] = 0.0;
   out_938651669609787352[276] = 0.0;
   out_938651669609787352[277] = 0.0;
   out_938651669609787352[278] = 0.0;
   out_938651669609787352[279] = 0.0;
   out_938651669609787352[280] = 0.0;
   out_938651669609787352[281] = 0.0;
   out_938651669609787352[282] = 0.0;
   out_938651669609787352[283] = 0.0;
   out_938651669609787352[284] = 0.0;
   out_938651669609787352[285] = 1.0;
   out_938651669609787352[286] = 0.0;
   out_938651669609787352[287] = 0.0;
   out_938651669609787352[288] = 0.0;
   out_938651669609787352[289] = 0.0;
   out_938651669609787352[290] = 0.0;
   out_938651669609787352[291] = 0.0;
   out_938651669609787352[292] = 0.0;
   out_938651669609787352[293] = 0.0;
   out_938651669609787352[294] = 0.0;
   out_938651669609787352[295] = 0.0;
   out_938651669609787352[296] = 0.0;
   out_938651669609787352[297] = 0.0;
   out_938651669609787352[298] = 0.0;
   out_938651669609787352[299] = 0.0;
   out_938651669609787352[300] = 0.0;
   out_938651669609787352[301] = 0.0;
   out_938651669609787352[302] = 0.0;
   out_938651669609787352[303] = 0.0;
   out_938651669609787352[304] = 1.0;
   out_938651669609787352[305] = 0.0;
   out_938651669609787352[306] = 0.0;
   out_938651669609787352[307] = 0.0;
   out_938651669609787352[308] = 0.0;
   out_938651669609787352[309] = 0.0;
   out_938651669609787352[310] = 0.0;
   out_938651669609787352[311] = 0.0;
   out_938651669609787352[312] = 0.0;
   out_938651669609787352[313] = 0.0;
   out_938651669609787352[314] = 0.0;
   out_938651669609787352[315] = 0.0;
   out_938651669609787352[316] = 0.0;
   out_938651669609787352[317] = 0.0;
   out_938651669609787352[318] = 0.0;
   out_938651669609787352[319] = 0.0;
   out_938651669609787352[320] = 0.0;
   out_938651669609787352[321] = 0.0;
   out_938651669609787352[322] = 0.0;
   out_938651669609787352[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_336566407888546136) {
   out_336566407888546136[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_336566407888546136[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_336566407888546136[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_336566407888546136[3] = dt*state[12] + state[3];
   out_336566407888546136[4] = dt*state[13] + state[4];
   out_336566407888546136[5] = dt*state[14] + state[5];
   out_336566407888546136[6] = state[6];
   out_336566407888546136[7] = state[7];
   out_336566407888546136[8] = state[8];
   out_336566407888546136[9] = state[9];
   out_336566407888546136[10] = state[10];
   out_336566407888546136[11] = state[11];
   out_336566407888546136[12] = state[12];
   out_336566407888546136[13] = state[13];
   out_336566407888546136[14] = state[14];
   out_336566407888546136[15] = state[15];
   out_336566407888546136[16] = state[16];
   out_336566407888546136[17] = state[17];
}
void F_fun(double *state, double dt, double *out_5996324224975557063) {
   out_5996324224975557063[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_5996324224975557063[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_5996324224975557063[2] = 0;
   out_5996324224975557063[3] = 0;
   out_5996324224975557063[4] = 0;
   out_5996324224975557063[5] = 0;
   out_5996324224975557063[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_5996324224975557063[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_5996324224975557063[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_5996324224975557063[9] = 0;
   out_5996324224975557063[10] = 0;
   out_5996324224975557063[11] = 0;
   out_5996324224975557063[12] = 0;
   out_5996324224975557063[13] = 0;
   out_5996324224975557063[14] = 0;
   out_5996324224975557063[15] = 0;
   out_5996324224975557063[16] = 0;
   out_5996324224975557063[17] = 0;
   out_5996324224975557063[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_5996324224975557063[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_5996324224975557063[20] = 0;
   out_5996324224975557063[21] = 0;
   out_5996324224975557063[22] = 0;
   out_5996324224975557063[23] = 0;
   out_5996324224975557063[24] = 0;
   out_5996324224975557063[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_5996324224975557063[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_5996324224975557063[27] = 0;
   out_5996324224975557063[28] = 0;
   out_5996324224975557063[29] = 0;
   out_5996324224975557063[30] = 0;
   out_5996324224975557063[31] = 0;
   out_5996324224975557063[32] = 0;
   out_5996324224975557063[33] = 0;
   out_5996324224975557063[34] = 0;
   out_5996324224975557063[35] = 0;
   out_5996324224975557063[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_5996324224975557063[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_5996324224975557063[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_5996324224975557063[39] = 0;
   out_5996324224975557063[40] = 0;
   out_5996324224975557063[41] = 0;
   out_5996324224975557063[42] = 0;
   out_5996324224975557063[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_5996324224975557063[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_5996324224975557063[45] = 0;
   out_5996324224975557063[46] = 0;
   out_5996324224975557063[47] = 0;
   out_5996324224975557063[48] = 0;
   out_5996324224975557063[49] = 0;
   out_5996324224975557063[50] = 0;
   out_5996324224975557063[51] = 0;
   out_5996324224975557063[52] = 0;
   out_5996324224975557063[53] = 0;
   out_5996324224975557063[54] = 0;
   out_5996324224975557063[55] = 0;
   out_5996324224975557063[56] = 0;
   out_5996324224975557063[57] = 1;
   out_5996324224975557063[58] = 0;
   out_5996324224975557063[59] = 0;
   out_5996324224975557063[60] = 0;
   out_5996324224975557063[61] = 0;
   out_5996324224975557063[62] = 0;
   out_5996324224975557063[63] = 0;
   out_5996324224975557063[64] = 0;
   out_5996324224975557063[65] = 0;
   out_5996324224975557063[66] = dt;
   out_5996324224975557063[67] = 0;
   out_5996324224975557063[68] = 0;
   out_5996324224975557063[69] = 0;
   out_5996324224975557063[70] = 0;
   out_5996324224975557063[71] = 0;
   out_5996324224975557063[72] = 0;
   out_5996324224975557063[73] = 0;
   out_5996324224975557063[74] = 0;
   out_5996324224975557063[75] = 0;
   out_5996324224975557063[76] = 1;
   out_5996324224975557063[77] = 0;
   out_5996324224975557063[78] = 0;
   out_5996324224975557063[79] = 0;
   out_5996324224975557063[80] = 0;
   out_5996324224975557063[81] = 0;
   out_5996324224975557063[82] = 0;
   out_5996324224975557063[83] = 0;
   out_5996324224975557063[84] = 0;
   out_5996324224975557063[85] = dt;
   out_5996324224975557063[86] = 0;
   out_5996324224975557063[87] = 0;
   out_5996324224975557063[88] = 0;
   out_5996324224975557063[89] = 0;
   out_5996324224975557063[90] = 0;
   out_5996324224975557063[91] = 0;
   out_5996324224975557063[92] = 0;
   out_5996324224975557063[93] = 0;
   out_5996324224975557063[94] = 0;
   out_5996324224975557063[95] = 1;
   out_5996324224975557063[96] = 0;
   out_5996324224975557063[97] = 0;
   out_5996324224975557063[98] = 0;
   out_5996324224975557063[99] = 0;
   out_5996324224975557063[100] = 0;
   out_5996324224975557063[101] = 0;
   out_5996324224975557063[102] = 0;
   out_5996324224975557063[103] = 0;
   out_5996324224975557063[104] = dt;
   out_5996324224975557063[105] = 0;
   out_5996324224975557063[106] = 0;
   out_5996324224975557063[107] = 0;
   out_5996324224975557063[108] = 0;
   out_5996324224975557063[109] = 0;
   out_5996324224975557063[110] = 0;
   out_5996324224975557063[111] = 0;
   out_5996324224975557063[112] = 0;
   out_5996324224975557063[113] = 0;
   out_5996324224975557063[114] = 1;
   out_5996324224975557063[115] = 0;
   out_5996324224975557063[116] = 0;
   out_5996324224975557063[117] = 0;
   out_5996324224975557063[118] = 0;
   out_5996324224975557063[119] = 0;
   out_5996324224975557063[120] = 0;
   out_5996324224975557063[121] = 0;
   out_5996324224975557063[122] = 0;
   out_5996324224975557063[123] = 0;
   out_5996324224975557063[124] = 0;
   out_5996324224975557063[125] = 0;
   out_5996324224975557063[126] = 0;
   out_5996324224975557063[127] = 0;
   out_5996324224975557063[128] = 0;
   out_5996324224975557063[129] = 0;
   out_5996324224975557063[130] = 0;
   out_5996324224975557063[131] = 0;
   out_5996324224975557063[132] = 0;
   out_5996324224975557063[133] = 1;
   out_5996324224975557063[134] = 0;
   out_5996324224975557063[135] = 0;
   out_5996324224975557063[136] = 0;
   out_5996324224975557063[137] = 0;
   out_5996324224975557063[138] = 0;
   out_5996324224975557063[139] = 0;
   out_5996324224975557063[140] = 0;
   out_5996324224975557063[141] = 0;
   out_5996324224975557063[142] = 0;
   out_5996324224975557063[143] = 0;
   out_5996324224975557063[144] = 0;
   out_5996324224975557063[145] = 0;
   out_5996324224975557063[146] = 0;
   out_5996324224975557063[147] = 0;
   out_5996324224975557063[148] = 0;
   out_5996324224975557063[149] = 0;
   out_5996324224975557063[150] = 0;
   out_5996324224975557063[151] = 0;
   out_5996324224975557063[152] = 1;
   out_5996324224975557063[153] = 0;
   out_5996324224975557063[154] = 0;
   out_5996324224975557063[155] = 0;
   out_5996324224975557063[156] = 0;
   out_5996324224975557063[157] = 0;
   out_5996324224975557063[158] = 0;
   out_5996324224975557063[159] = 0;
   out_5996324224975557063[160] = 0;
   out_5996324224975557063[161] = 0;
   out_5996324224975557063[162] = 0;
   out_5996324224975557063[163] = 0;
   out_5996324224975557063[164] = 0;
   out_5996324224975557063[165] = 0;
   out_5996324224975557063[166] = 0;
   out_5996324224975557063[167] = 0;
   out_5996324224975557063[168] = 0;
   out_5996324224975557063[169] = 0;
   out_5996324224975557063[170] = 0;
   out_5996324224975557063[171] = 1;
   out_5996324224975557063[172] = 0;
   out_5996324224975557063[173] = 0;
   out_5996324224975557063[174] = 0;
   out_5996324224975557063[175] = 0;
   out_5996324224975557063[176] = 0;
   out_5996324224975557063[177] = 0;
   out_5996324224975557063[178] = 0;
   out_5996324224975557063[179] = 0;
   out_5996324224975557063[180] = 0;
   out_5996324224975557063[181] = 0;
   out_5996324224975557063[182] = 0;
   out_5996324224975557063[183] = 0;
   out_5996324224975557063[184] = 0;
   out_5996324224975557063[185] = 0;
   out_5996324224975557063[186] = 0;
   out_5996324224975557063[187] = 0;
   out_5996324224975557063[188] = 0;
   out_5996324224975557063[189] = 0;
   out_5996324224975557063[190] = 1;
   out_5996324224975557063[191] = 0;
   out_5996324224975557063[192] = 0;
   out_5996324224975557063[193] = 0;
   out_5996324224975557063[194] = 0;
   out_5996324224975557063[195] = 0;
   out_5996324224975557063[196] = 0;
   out_5996324224975557063[197] = 0;
   out_5996324224975557063[198] = 0;
   out_5996324224975557063[199] = 0;
   out_5996324224975557063[200] = 0;
   out_5996324224975557063[201] = 0;
   out_5996324224975557063[202] = 0;
   out_5996324224975557063[203] = 0;
   out_5996324224975557063[204] = 0;
   out_5996324224975557063[205] = 0;
   out_5996324224975557063[206] = 0;
   out_5996324224975557063[207] = 0;
   out_5996324224975557063[208] = 0;
   out_5996324224975557063[209] = 1;
   out_5996324224975557063[210] = 0;
   out_5996324224975557063[211] = 0;
   out_5996324224975557063[212] = 0;
   out_5996324224975557063[213] = 0;
   out_5996324224975557063[214] = 0;
   out_5996324224975557063[215] = 0;
   out_5996324224975557063[216] = 0;
   out_5996324224975557063[217] = 0;
   out_5996324224975557063[218] = 0;
   out_5996324224975557063[219] = 0;
   out_5996324224975557063[220] = 0;
   out_5996324224975557063[221] = 0;
   out_5996324224975557063[222] = 0;
   out_5996324224975557063[223] = 0;
   out_5996324224975557063[224] = 0;
   out_5996324224975557063[225] = 0;
   out_5996324224975557063[226] = 0;
   out_5996324224975557063[227] = 0;
   out_5996324224975557063[228] = 1;
   out_5996324224975557063[229] = 0;
   out_5996324224975557063[230] = 0;
   out_5996324224975557063[231] = 0;
   out_5996324224975557063[232] = 0;
   out_5996324224975557063[233] = 0;
   out_5996324224975557063[234] = 0;
   out_5996324224975557063[235] = 0;
   out_5996324224975557063[236] = 0;
   out_5996324224975557063[237] = 0;
   out_5996324224975557063[238] = 0;
   out_5996324224975557063[239] = 0;
   out_5996324224975557063[240] = 0;
   out_5996324224975557063[241] = 0;
   out_5996324224975557063[242] = 0;
   out_5996324224975557063[243] = 0;
   out_5996324224975557063[244] = 0;
   out_5996324224975557063[245] = 0;
   out_5996324224975557063[246] = 0;
   out_5996324224975557063[247] = 1;
   out_5996324224975557063[248] = 0;
   out_5996324224975557063[249] = 0;
   out_5996324224975557063[250] = 0;
   out_5996324224975557063[251] = 0;
   out_5996324224975557063[252] = 0;
   out_5996324224975557063[253] = 0;
   out_5996324224975557063[254] = 0;
   out_5996324224975557063[255] = 0;
   out_5996324224975557063[256] = 0;
   out_5996324224975557063[257] = 0;
   out_5996324224975557063[258] = 0;
   out_5996324224975557063[259] = 0;
   out_5996324224975557063[260] = 0;
   out_5996324224975557063[261] = 0;
   out_5996324224975557063[262] = 0;
   out_5996324224975557063[263] = 0;
   out_5996324224975557063[264] = 0;
   out_5996324224975557063[265] = 0;
   out_5996324224975557063[266] = 1;
   out_5996324224975557063[267] = 0;
   out_5996324224975557063[268] = 0;
   out_5996324224975557063[269] = 0;
   out_5996324224975557063[270] = 0;
   out_5996324224975557063[271] = 0;
   out_5996324224975557063[272] = 0;
   out_5996324224975557063[273] = 0;
   out_5996324224975557063[274] = 0;
   out_5996324224975557063[275] = 0;
   out_5996324224975557063[276] = 0;
   out_5996324224975557063[277] = 0;
   out_5996324224975557063[278] = 0;
   out_5996324224975557063[279] = 0;
   out_5996324224975557063[280] = 0;
   out_5996324224975557063[281] = 0;
   out_5996324224975557063[282] = 0;
   out_5996324224975557063[283] = 0;
   out_5996324224975557063[284] = 0;
   out_5996324224975557063[285] = 1;
   out_5996324224975557063[286] = 0;
   out_5996324224975557063[287] = 0;
   out_5996324224975557063[288] = 0;
   out_5996324224975557063[289] = 0;
   out_5996324224975557063[290] = 0;
   out_5996324224975557063[291] = 0;
   out_5996324224975557063[292] = 0;
   out_5996324224975557063[293] = 0;
   out_5996324224975557063[294] = 0;
   out_5996324224975557063[295] = 0;
   out_5996324224975557063[296] = 0;
   out_5996324224975557063[297] = 0;
   out_5996324224975557063[298] = 0;
   out_5996324224975557063[299] = 0;
   out_5996324224975557063[300] = 0;
   out_5996324224975557063[301] = 0;
   out_5996324224975557063[302] = 0;
   out_5996324224975557063[303] = 0;
   out_5996324224975557063[304] = 1;
   out_5996324224975557063[305] = 0;
   out_5996324224975557063[306] = 0;
   out_5996324224975557063[307] = 0;
   out_5996324224975557063[308] = 0;
   out_5996324224975557063[309] = 0;
   out_5996324224975557063[310] = 0;
   out_5996324224975557063[311] = 0;
   out_5996324224975557063[312] = 0;
   out_5996324224975557063[313] = 0;
   out_5996324224975557063[314] = 0;
   out_5996324224975557063[315] = 0;
   out_5996324224975557063[316] = 0;
   out_5996324224975557063[317] = 0;
   out_5996324224975557063[318] = 0;
   out_5996324224975557063[319] = 0;
   out_5996324224975557063[320] = 0;
   out_5996324224975557063[321] = 0;
   out_5996324224975557063[322] = 0;
   out_5996324224975557063[323] = 1;
}
void h_4(double *state, double *unused, double *out_2660879566845649022) {
   out_2660879566845649022[0] = state[6] + state[9];
   out_2660879566845649022[1] = state[7] + state[10];
   out_2660879566845649022[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_6786733476361746073) {
   out_6786733476361746073[0] = 0;
   out_6786733476361746073[1] = 0;
   out_6786733476361746073[2] = 0;
   out_6786733476361746073[3] = 0;
   out_6786733476361746073[4] = 0;
   out_6786733476361746073[5] = 0;
   out_6786733476361746073[6] = 1;
   out_6786733476361746073[7] = 0;
   out_6786733476361746073[8] = 0;
   out_6786733476361746073[9] = 1;
   out_6786733476361746073[10] = 0;
   out_6786733476361746073[11] = 0;
   out_6786733476361746073[12] = 0;
   out_6786733476361746073[13] = 0;
   out_6786733476361746073[14] = 0;
   out_6786733476361746073[15] = 0;
   out_6786733476361746073[16] = 0;
   out_6786733476361746073[17] = 0;
   out_6786733476361746073[18] = 0;
   out_6786733476361746073[19] = 0;
   out_6786733476361746073[20] = 0;
   out_6786733476361746073[21] = 0;
   out_6786733476361746073[22] = 0;
   out_6786733476361746073[23] = 0;
   out_6786733476361746073[24] = 0;
   out_6786733476361746073[25] = 1;
   out_6786733476361746073[26] = 0;
   out_6786733476361746073[27] = 0;
   out_6786733476361746073[28] = 1;
   out_6786733476361746073[29] = 0;
   out_6786733476361746073[30] = 0;
   out_6786733476361746073[31] = 0;
   out_6786733476361746073[32] = 0;
   out_6786733476361746073[33] = 0;
   out_6786733476361746073[34] = 0;
   out_6786733476361746073[35] = 0;
   out_6786733476361746073[36] = 0;
   out_6786733476361746073[37] = 0;
   out_6786733476361746073[38] = 0;
   out_6786733476361746073[39] = 0;
   out_6786733476361746073[40] = 0;
   out_6786733476361746073[41] = 0;
   out_6786733476361746073[42] = 0;
   out_6786733476361746073[43] = 0;
   out_6786733476361746073[44] = 1;
   out_6786733476361746073[45] = 0;
   out_6786733476361746073[46] = 0;
   out_6786733476361746073[47] = 1;
   out_6786733476361746073[48] = 0;
   out_6786733476361746073[49] = 0;
   out_6786733476361746073[50] = 0;
   out_6786733476361746073[51] = 0;
   out_6786733476361746073[52] = 0;
   out_6786733476361746073[53] = 0;
}
void h_10(double *state, double *unused, double *out_323021619404120390) {
   out_323021619404120390[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_323021619404120390[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_323021619404120390[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_4140593631986057126) {
   out_4140593631986057126[0] = 0;
   out_4140593631986057126[1] = 9.8100000000000005*cos(state[1]);
   out_4140593631986057126[2] = 0;
   out_4140593631986057126[3] = 0;
   out_4140593631986057126[4] = -state[8];
   out_4140593631986057126[5] = state[7];
   out_4140593631986057126[6] = 0;
   out_4140593631986057126[7] = state[5];
   out_4140593631986057126[8] = -state[4];
   out_4140593631986057126[9] = 0;
   out_4140593631986057126[10] = 0;
   out_4140593631986057126[11] = 0;
   out_4140593631986057126[12] = 1;
   out_4140593631986057126[13] = 0;
   out_4140593631986057126[14] = 0;
   out_4140593631986057126[15] = 1;
   out_4140593631986057126[16] = 0;
   out_4140593631986057126[17] = 0;
   out_4140593631986057126[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_4140593631986057126[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_4140593631986057126[20] = 0;
   out_4140593631986057126[21] = state[8];
   out_4140593631986057126[22] = 0;
   out_4140593631986057126[23] = -state[6];
   out_4140593631986057126[24] = -state[5];
   out_4140593631986057126[25] = 0;
   out_4140593631986057126[26] = state[3];
   out_4140593631986057126[27] = 0;
   out_4140593631986057126[28] = 0;
   out_4140593631986057126[29] = 0;
   out_4140593631986057126[30] = 0;
   out_4140593631986057126[31] = 1;
   out_4140593631986057126[32] = 0;
   out_4140593631986057126[33] = 0;
   out_4140593631986057126[34] = 1;
   out_4140593631986057126[35] = 0;
   out_4140593631986057126[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_4140593631986057126[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_4140593631986057126[38] = 0;
   out_4140593631986057126[39] = -state[7];
   out_4140593631986057126[40] = state[6];
   out_4140593631986057126[41] = 0;
   out_4140593631986057126[42] = state[4];
   out_4140593631986057126[43] = -state[3];
   out_4140593631986057126[44] = 0;
   out_4140593631986057126[45] = 0;
   out_4140593631986057126[46] = 0;
   out_4140593631986057126[47] = 0;
   out_4140593631986057126[48] = 0;
   out_4140593631986057126[49] = 0;
   out_4140593631986057126[50] = 1;
   out_4140593631986057126[51] = 0;
   out_4140593631986057126[52] = 0;
   out_4140593631986057126[53] = 1;
}
void h_13(double *state, double *unused, double *out_3801171022701447110) {
   out_3801171022701447110[0] = state[3];
   out_3801171022701447110[1] = state[4];
   out_3801171022701447110[2] = state[5];
}
void H_13(double *state, double *unused, double *out_3574459651029413272) {
   out_3574459651029413272[0] = 0;
   out_3574459651029413272[1] = 0;
   out_3574459651029413272[2] = 0;
   out_3574459651029413272[3] = 1;
   out_3574459651029413272[4] = 0;
   out_3574459651029413272[5] = 0;
   out_3574459651029413272[6] = 0;
   out_3574459651029413272[7] = 0;
   out_3574459651029413272[8] = 0;
   out_3574459651029413272[9] = 0;
   out_3574459651029413272[10] = 0;
   out_3574459651029413272[11] = 0;
   out_3574459651029413272[12] = 0;
   out_3574459651029413272[13] = 0;
   out_3574459651029413272[14] = 0;
   out_3574459651029413272[15] = 0;
   out_3574459651029413272[16] = 0;
   out_3574459651029413272[17] = 0;
   out_3574459651029413272[18] = 0;
   out_3574459651029413272[19] = 0;
   out_3574459651029413272[20] = 0;
   out_3574459651029413272[21] = 0;
   out_3574459651029413272[22] = 1;
   out_3574459651029413272[23] = 0;
   out_3574459651029413272[24] = 0;
   out_3574459651029413272[25] = 0;
   out_3574459651029413272[26] = 0;
   out_3574459651029413272[27] = 0;
   out_3574459651029413272[28] = 0;
   out_3574459651029413272[29] = 0;
   out_3574459651029413272[30] = 0;
   out_3574459651029413272[31] = 0;
   out_3574459651029413272[32] = 0;
   out_3574459651029413272[33] = 0;
   out_3574459651029413272[34] = 0;
   out_3574459651029413272[35] = 0;
   out_3574459651029413272[36] = 0;
   out_3574459651029413272[37] = 0;
   out_3574459651029413272[38] = 0;
   out_3574459651029413272[39] = 0;
   out_3574459651029413272[40] = 0;
   out_3574459651029413272[41] = 1;
   out_3574459651029413272[42] = 0;
   out_3574459651029413272[43] = 0;
   out_3574459651029413272[44] = 0;
   out_3574459651029413272[45] = 0;
   out_3574459651029413272[46] = 0;
   out_3574459651029413272[47] = 0;
   out_3574459651029413272[48] = 0;
   out_3574459651029413272[49] = 0;
   out_3574459651029413272[50] = 0;
   out_3574459651029413272[51] = 0;
   out_3574459651029413272[52] = 0;
   out_3574459651029413272[53] = 0;
}
void h_14(double *state, double *unused, double *out_7703097660121677315) {
   out_7703097660121677315[0] = state[6];
   out_7703097660121677315[1] = state[7];
   out_7703097660121677315[2] = state[8];
}
void H_14(double *state, double *unused, double *out_2823492620022261544) {
   out_2823492620022261544[0] = 0;
   out_2823492620022261544[1] = 0;
   out_2823492620022261544[2] = 0;
   out_2823492620022261544[3] = 0;
   out_2823492620022261544[4] = 0;
   out_2823492620022261544[5] = 0;
   out_2823492620022261544[6] = 1;
   out_2823492620022261544[7] = 0;
   out_2823492620022261544[8] = 0;
   out_2823492620022261544[9] = 0;
   out_2823492620022261544[10] = 0;
   out_2823492620022261544[11] = 0;
   out_2823492620022261544[12] = 0;
   out_2823492620022261544[13] = 0;
   out_2823492620022261544[14] = 0;
   out_2823492620022261544[15] = 0;
   out_2823492620022261544[16] = 0;
   out_2823492620022261544[17] = 0;
   out_2823492620022261544[18] = 0;
   out_2823492620022261544[19] = 0;
   out_2823492620022261544[20] = 0;
   out_2823492620022261544[21] = 0;
   out_2823492620022261544[22] = 0;
   out_2823492620022261544[23] = 0;
   out_2823492620022261544[24] = 0;
   out_2823492620022261544[25] = 1;
   out_2823492620022261544[26] = 0;
   out_2823492620022261544[27] = 0;
   out_2823492620022261544[28] = 0;
   out_2823492620022261544[29] = 0;
   out_2823492620022261544[30] = 0;
   out_2823492620022261544[31] = 0;
   out_2823492620022261544[32] = 0;
   out_2823492620022261544[33] = 0;
   out_2823492620022261544[34] = 0;
   out_2823492620022261544[35] = 0;
   out_2823492620022261544[36] = 0;
   out_2823492620022261544[37] = 0;
   out_2823492620022261544[38] = 0;
   out_2823492620022261544[39] = 0;
   out_2823492620022261544[40] = 0;
   out_2823492620022261544[41] = 0;
   out_2823492620022261544[42] = 0;
   out_2823492620022261544[43] = 0;
   out_2823492620022261544[44] = 1;
   out_2823492620022261544[45] = 0;
   out_2823492620022261544[46] = 0;
   out_2823492620022261544[47] = 0;
   out_2823492620022261544[48] = 0;
   out_2823492620022261544[49] = 0;
   out_2823492620022261544[50] = 0;
   out_2823492620022261544[51] = 0;
   out_2823492620022261544[52] = 0;
   out_2823492620022261544[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_5984861409348236479) {
  err_fun(nom_x, delta_x, out_5984861409348236479);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1707565234864444406) {
  inv_err_fun(nom_x, true_x, out_1707565234864444406);
}
void pose_H_mod_fun(double *state, double *out_938651669609787352) {
  H_mod_fun(state, out_938651669609787352);
}
void pose_f_fun(double *state, double dt, double *out_336566407888546136) {
  f_fun(state,  dt, out_336566407888546136);
}
void pose_F_fun(double *state, double dt, double *out_5996324224975557063) {
  F_fun(state,  dt, out_5996324224975557063);
}
void pose_h_4(double *state, double *unused, double *out_2660879566845649022) {
  h_4(state, unused, out_2660879566845649022);
}
void pose_H_4(double *state, double *unused, double *out_6786733476361746073) {
  H_4(state, unused, out_6786733476361746073);
}
void pose_h_10(double *state, double *unused, double *out_323021619404120390) {
  h_10(state, unused, out_323021619404120390);
}
void pose_H_10(double *state, double *unused, double *out_4140593631986057126) {
  H_10(state, unused, out_4140593631986057126);
}
void pose_h_13(double *state, double *unused, double *out_3801171022701447110) {
  h_13(state, unused, out_3801171022701447110);
}
void pose_H_13(double *state, double *unused, double *out_3574459651029413272) {
  H_13(state, unused, out_3574459651029413272);
}
void pose_h_14(double *state, double *unused, double *out_7703097660121677315) {
  h_14(state, unused, out_7703097660121677315);
}
void pose_H_14(double *state, double *unused, double *out_2823492620022261544) {
  H_14(state, unused, out_2823492620022261544);
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

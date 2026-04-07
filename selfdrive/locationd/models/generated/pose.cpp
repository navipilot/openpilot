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
void err_fun(double *nom_x, double *delta_x, double *out_9191542510053319957) {
   out_9191542510053319957[0] = delta_x[0] + nom_x[0];
   out_9191542510053319957[1] = delta_x[1] + nom_x[1];
   out_9191542510053319957[2] = delta_x[2] + nom_x[2];
   out_9191542510053319957[3] = delta_x[3] + nom_x[3];
   out_9191542510053319957[4] = delta_x[4] + nom_x[4];
   out_9191542510053319957[5] = delta_x[5] + nom_x[5];
   out_9191542510053319957[6] = delta_x[6] + nom_x[6];
   out_9191542510053319957[7] = delta_x[7] + nom_x[7];
   out_9191542510053319957[8] = delta_x[8] + nom_x[8];
   out_9191542510053319957[9] = delta_x[9] + nom_x[9];
   out_9191542510053319957[10] = delta_x[10] + nom_x[10];
   out_9191542510053319957[11] = delta_x[11] + nom_x[11];
   out_9191542510053319957[12] = delta_x[12] + nom_x[12];
   out_9191542510053319957[13] = delta_x[13] + nom_x[13];
   out_9191542510053319957[14] = delta_x[14] + nom_x[14];
   out_9191542510053319957[15] = delta_x[15] + nom_x[15];
   out_9191542510053319957[16] = delta_x[16] + nom_x[16];
   out_9191542510053319957[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3004969772891744590) {
   out_3004969772891744590[0] = -nom_x[0] + true_x[0];
   out_3004969772891744590[1] = -nom_x[1] + true_x[1];
   out_3004969772891744590[2] = -nom_x[2] + true_x[2];
   out_3004969772891744590[3] = -nom_x[3] + true_x[3];
   out_3004969772891744590[4] = -nom_x[4] + true_x[4];
   out_3004969772891744590[5] = -nom_x[5] + true_x[5];
   out_3004969772891744590[6] = -nom_x[6] + true_x[6];
   out_3004969772891744590[7] = -nom_x[7] + true_x[7];
   out_3004969772891744590[8] = -nom_x[8] + true_x[8];
   out_3004969772891744590[9] = -nom_x[9] + true_x[9];
   out_3004969772891744590[10] = -nom_x[10] + true_x[10];
   out_3004969772891744590[11] = -nom_x[11] + true_x[11];
   out_3004969772891744590[12] = -nom_x[12] + true_x[12];
   out_3004969772891744590[13] = -nom_x[13] + true_x[13];
   out_3004969772891744590[14] = -nom_x[14] + true_x[14];
   out_3004969772891744590[15] = -nom_x[15] + true_x[15];
   out_3004969772891744590[16] = -nom_x[16] + true_x[16];
   out_3004969772891744590[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_7359357434108742810) {
   out_7359357434108742810[0] = 1.0;
   out_7359357434108742810[1] = 0.0;
   out_7359357434108742810[2] = 0.0;
   out_7359357434108742810[3] = 0.0;
   out_7359357434108742810[4] = 0.0;
   out_7359357434108742810[5] = 0.0;
   out_7359357434108742810[6] = 0.0;
   out_7359357434108742810[7] = 0.0;
   out_7359357434108742810[8] = 0.0;
   out_7359357434108742810[9] = 0.0;
   out_7359357434108742810[10] = 0.0;
   out_7359357434108742810[11] = 0.0;
   out_7359357434108742810[12] = 0.0;
   out_7359357434108742810[13] = 0.0;
   out_7359357434108742810[14] = 0.0;
   out_7359357434108742810[15] = 0.0;
   out_7359357434108742810[16] = 0.0;
   out_7359357434108742810[17] = 0.0;
   out_7359357434108742810[18] = 0.0;
   out_7359357434108742810[19] = 1.0;
   out_7359357434108742810[20] = 0.0;
   out_7359357434108742810[21] = 0.0;
   out_7359357434108742810[22] = 0.0;
   out_7359357434108742810[23] = 0.0;
   out_7359357434108742810[24] = 0.0;
   out_7359357434108742810[25] = 0.0;
   out_7359357434108742810[26] = 0.0;
   out_7359357434108742810[27] = 0.0;
   out_7359357434108742810[28] = 0.0;
   out_7359357434108742810[29] = 0.0;
   out_7359357434108742810[30] = 0.0;
   out_7359357434108742810[31] = 0.0;
   out_7359357434108742810[32] = 0.0;
   out_7359357434108742810[33] = 0.0;
   out_7359357434108742810[34] = 0.0;
   out_7359357434108742810[35] = 0.0;
   out_7359357434108742810[36] = 0.0;
   out_7359357434108742810[37] = 0.0;
   out_7359357434108742810[38] = 1.0;
   out_7359357434108742810[39] = 0.0;
   out_7359357434108742810[40] = 0.0;
   out_7359357434108742810[41] = 0.0;
   out_7359357434108742810[42] = 0.0;
   out_7359357434108742810[43] = 0.0;
   out_7359357434108742810[44] = 0.0;
   out_7359357434108742810[45] = 0.0;
   out_7359357434108742810[46] = 0.0;
   out_7359357434108742810[47] = 0.0;
   out_7359357434108742810[48] = 0.0;
   out_7359357434108742810[49] = 0.0;
   out_7359357434108742810[50] = 0.0;
   out_7359357434108742810[51] = 0.0;
   out_7359357434108742810[52] = 0.0;
   out_7359357434108742810[53] = 0.0;
   out_7359357434108742810[54] = 0.0;
   out_7359357434108742810[55] = 0.0;
   out_7359357434108742810[56] = 0.0;
   out_7359357434108742810[57] = 1.0;
   out_7359357434108742810[58] = 0.0;
   out_7359357434108742810[59] = 0.0;
   out_7359357434108742810[60] = 0.0;
   out_7359357434108742810[61] = 0.0;
   out_7359357434108742810[62] = 0.0;
   out_7359357434108742810[63] = 0.0;
   out_7359357434108742810[64] = 0.0;
   out_7359357434108742810[65] = 0.0;
   out_7359357434108742810[66] = 0.0;
   out_7359357434108742810[67] = 0.0;
   out_7359357434108742810[68] = 0.0;
   out_7359357434108742810[69] = 0.0;
   out_7359357434108742810[70] = 0.0;
   out_7359357434108742810[71] = 0.0;
   out_7359357434108742810[72] = 0.0;
   out_7359357434108742810[73] = 0.0;
   out_7359357434108742810[74] = 0.0;
   out_7359357434108742810[75] = 0.0;
   out_7359357434108742810[76] = 1.0;
   out_7359357434108742810[77] = 0.0;
   out_7359357434108742810[78] = 0.0;
   out_7359357434108742810[79] = 0.0;
   out_7359357434108742810[80] = 0.0;
   out_7359357434108742810[81] = 0.0;
   out_7359357434108742810[82] = 0.0;
   out_7359357434108742810[83] = 0.0;
   out_7359357434108742810[84] = 0.0;
   out_7359357434108742810[85] = 0.0;
   out_7359357434108742810[86] = 0.0;
   out_7359357434108742810[87] = 0.0;
   out_7359357434108742810[88] = 0.0;
   out_7359357434108742810[89] = 0.0;
   out_7359357434108742810[90] = 0.0;
   out_7359357434108742810[91] = 0.0;
   out_7359357434108742810[92] = 0.0;
   out_7359357434108742810[93] = 0.0;
   out_7359357434108742810[94] = 0.0;
   out_7359357434108742810[95] = 1.0;
   out_7359357434108742810[96] = 0.0;
   out_7359357434108742810[97] = 0.0;
   out_7359357434108742810[98] = 0.0;
   out_7359357434108742810[99] = 0.0;
   out_7359357434108742810[100] = 0.0;
   out_7359357434108742810[101] = 0.0;
   out_7359357434108742810[102] = 0.0;
   out_7359357434108742810[103] = 0.0;
   out_7359357434108742810[104] = 0.0;
   out_7359357434108742810[105] = 0.0;
   out_7359357434108742810[106] = 0.0;
   out_7359357434108742810[107] = 0.0;
   out_7359357434108742810[108] = 0.0;
   out_7359357434108742810[109] = 0.0;
   out_7359357434108742810[110] = 0.0;
   out_7359357434108742810[111] = 0.0;
   out_7359357434108742810[112] = 0.0;
   out_7359357434108742810[113] = 0.0;
   out_7359357434108742810[114] = 1.0;
   out_7359357434108742810[115] = 0.0;
   out_7359357434108742810[116] = 0.0;
   out_7359357434108742810[117] = 0.0;
   out_7359357434108742810[118] = 0.0;
   out_7359357434108742810[119] = 0.0;
   out_7359357434108742810[120] = 0.0;
   out_7359357434108742810[121] = 0.0;
   out_7359357434108742810[122] = 0.0;
   out_7359357434108742810[123] = 0.0;
   out_7359357434108742810[124] = 0.0;
   out_7359357434108742810[125] = 0.0;
   out_7359357434108742810[126] = 0.0;
   out_7359357434108742810[127] = 0.0;
   out_7359357434108742810[128] = 0.0;
   out_7359357434108742810[129] = 0.0;
   out_7359357434108742810[130] = 0.0;
   out_7359357434108742810[131] = 0.0;
   out_7359357434108742810[132] = 0.0;
   out_7359357434108742810[133] = 1.0;
   out_7359357434108742810[134] = 0.0;
   out_7359357434108742810[135] = 0.0;
   out_7359357434108742810[136] = 0.0;
   out_7359357434108742810[137] = 0.0;
   out_7359357434108742810[138] = 0.0;
   out_7359357434108742810[139] = 0.0;
   out_7359357434108742810[140] = 0.0;
   out_7359357434108742810[141] = 0.0;
   out_7359357434108742810[142] = 0.0;
   out_7359357434108742810[143] = 0.0;
   out_7359357434108742810[144] = 0.0;
   out_7359357434108742810[145] = 0.0;
   out_7359357434108742810[146] = 0.0;
   out_7359357434108742810[147] = 0.0;
   out_7359357434108742810[148] = 0.0;
   out_7359357434108742810[149] = 0.0;
   out_7359357434108742810[150] = 0.0;
   out_7359357434108742810[151] = 0.0;
   out_7359357434108742810[152] = 1.0;
   out_7359357434108742810[153] = 0.0;
   out_7359357434108742810[154] = 0.0;
   out_7359357434108742810[155] = 0.0;
   out_7359357434108742810[156] = 0.0;
   out_7359357434108742810[157] = 0.0;
   out_7359357434108742810[158] = 0.0;
   out_7359357434108742810[159] = 0.0;
   out_7359357434108742810[160] = 0.0;
   out_7359357434108742810[161] = 0.0;
   out_7359357434108742810[162] = 0.0;
   out_7359357434108742810[163] = 0.0;
   out_7359357434108742810[164] = 0.0;
   out_7359357434108742810[165] = 0.0;
   out_7359357434108742810[166] = 0.0;
   out_7359357434108742810[167] = 0.0;
   out_7359357434108742810[168] = 0.0;
   out_7359357434108742810[169] = 0.0;
   out_7359357434108742810[170] = 0.0;
   out_7359357434108742810[171] = 1.0;
   out_7359357434108742810[172] = 0.0;
   out_7359357434108742810[173] = 0.0;
   out_7359357434108742810[174] = 0.0;
   out_7359357434108742810[175] = 0.0;
   out_7359357434108742810[176] = 0.0;
   out_7359357434108742810[177] = 0.0;
   out_7359357434108742810[178] = 0.0;
   out_7359357434108742810[179] = 0.0;
   out_7359357434108742810[180] = 0.0;
   out_7359357434108742810[181] = 0.0;
   out_7359357434108742810[182] = 0.0;
   out_7359357434108742810[183] = 0.0;
   out_7359357434108742810[184] = 0.0;
   out_7359357434108742810[185] = 0.0;
   out_7359357434108742810[186] = 0.0;
   out_7359357434108742810[187] = 0.0;
   out_7359357434108742810[188] = 0.0;
   out_7359357434108742810[189] = 0.0;
   out_7359357434108742810[190] = 1.0;
   out_7359357434108742810[191] = 0.0;
   out_7359357434108742810[192] = 0.0;
   out_7359357434108742810[193] = 0.0;
   out_7359357434108742810[194] = 0.0;
   out_7359357434108742810[195] = 0.0;
   out_7359357434108742810[196] = 0.0;
   out_7359357434108742810[197] = 0.0;
   out_7359357434108742810[198] = 0.0;
   out_7359357434108742810[199] = 0.0;
   out_7359357434108742810[200] = 0.0;
   out_7359357434108742810[201] = 0.0;
   out_7359357434108742810[202] = 0.0;
   out_7359357434108742810[203] = 0.0;
   out_7359357434108742810[204] = 0.0;
   out_7359357434108742810[205] = 0.0;
   out_7359357434108742810[206] = 0.0;
   out_7359357434108742810[207] = 0.0;
   out_7359357434108742810[208] = 0.0;
   out_7359357434108742810[209] = 1.0;
   out_7359357434108742810[210] = 0.0;
   out_7359357434108742810[211] = 0.0;
   out_7359357434108742810[212] = 0.0;
   out_7359357434108742810[213] = 0.0;
   out_7359357434108742810[214] = 0.0;
   out_7359357434108742810[215] = 0.0;
   out_7359357434108742810[216] = 0.0;
   out_7359357434108742810[217] = 0.0;
   out_7359357434108742810[218] = 0.0;
   out_7359357434108742810[219] = 0.0;
   out_7359357434108742810[220] = 0.0;
   out_7359357434108742810[221] = 0.0;
   out_7359357434108742810[222] = 0.0;
   out_7359357434108742810[223] = 0.0;
   out_7359357434108742810[224] = 0.0;
   out_7359357434108742810[225] = 0.0;
   out_7359357434108742810[226] = 0.0;
   out_7359357434108742810[227] = 0.0;
   out_7359357434108742810[228] = 1.0;
   out_7359357434108742810[229] = 0.0;
   out_7359357434108742810[230] = 0.0;
   out_7359357434108742810[231] = 0.0;
   out_7359357434108742810[232] = 0.0;
   out_7359357434108742810[233] = 0.0;
   out_7359357434108742810[234] = 0.0;
   out_7359357434108742810[235] = 0.0;
   out_7359357434108742810[236] = 0.0;
   out_7359357434108742810[237] = 0.0;
   out_7359357434108742810[238] = 0.0;
   out_7359357434108742810[239] = 0.0;
   out_7359357434108742810[240] = 0.0;
   out_7359357434108742810[241] = 0.0;
   out_7359357434108742810[242] = 0.0;
   out_7359357434108742810[243] = 0.0;
   out_7359357434108742810[244] = 0.0;
   out_7359357434108742810[245] = 0.0;
   out_7359357434108742810[246] = 0.0;
   out_7359357434108742810[247] = 1.0;
   out_7359357434108742810[248] = 0.0;
   out_7359357434108742810[249] = 0.0;
   out_7359357434108742810[250] = 0.0;
   out_7359357434108742810[251] = 0.0;
   out_7359357434108742810[252] = 0.0;
   out_7359357434108742810[253] = 0.0;
   out_7359357434108742810[254] = 0.0;
   out_7359357434108742810[255] = 0.0;
   out_7359357434108742810[256] = 0.0;
   out_7359357434108742810[257] = 0.0;
   out_7359357434108742810[258] = 0.0;
   out_7359357434108742810[259] = 0.0;
   out_7359357434108742810[260] = 0.0;
   out_7359357434108742810[261] = 0.0;
   out_7359357434108742810[262] = 0.0;
   out_7359357434108742810[263] = 0.0;
   out_7359357434108742810[264] = 0.0;
   out_7359357434108742810[265] = 0.0;
   out_7359357434108742810[266] = 1.0;
   out_7359357434108742810[267] = 0.0;
   out_7359357434108742810[268] = 0.0;
   out_7359357434108742810[269] = 0.0;
   out_7359357434108742810[270] = 0.0;
   out_7359357434108742810[271] = 0.0;
   out_7359357434108742810[272] = 0.0;
   out_7359357434108742810[273] = 0.0;
   out_7359357434108742810[274] = 0.0;
   out_7359357434108742810[275] = 0.0;
   out_7359357434108742810[276] = 0.0;
   out_7359357434108742810[277] = 0.0;
   out_7359357434108742810[278] = 0.0;
   out_7359357434108742810[279] = 0.0;
   out_7359357434108742810[280] = 0.0;
   out_7359357434108742810[281] = 0.0;
   out_7359357434108742810[282] = 0.0;
   out_7359357434108742810[283] = 0.0;
   out_7359357434108742810[284] = 0.0;
   out_7359357434108742810[285] = 1.0;
   out_7359357434108742810[286] = 0.0;
   out_7359357434108742810[287] = 0.0;
   out_7359357434108742810[288] = 0.0;
   out_7359357434108742810[289] = 0.0;
   out_7359357434108742810[290] = 0.0;
   out_7359357434108742810[291] = 0.0;
   out_7359357434108742810[292] = 0.0;
   out_7359357434108742810[293] = 0.0;
   out_7359357434108742810[294] = 0.0;
   out_7359357434108742810[295] = 0.0;
   out_7359357434108742810[296] = 0.0;
   out_7359357434108742810[297] = 0.0;
   out_7359357434108742810[298] = 0.0;
   out_7359357434108742810[299] = 0.0;
   out_7359357434108742810[300] = 0.0;
   out_7359357434108742810[301] = 0.0;
   out_7359357434108742810[302] = 0.0;
   out_7359357434108742810[303] = 0.0;
   out_7359357434108742810[304] = 1.0;
   out_7359357434108742810[305] = 0.0;
   out_7359357434108742810[306] = 0.0;
   out_7359357434108742810[307] = 0.0;
   out_7359357434108742810[308] = 0.0;
   out_7359357434108742810[309] = 0.0;
   out_7359357434108742810[310] = 0.0;
   out_7359357434108742810[311] = 0.0;
   out_7359357434108742810[312] = 0.0;
   out_7359357434108742810[313] = 0.0;
   out_7359357434108742810[314] = 0.0;
   out_7359357434108742810[315] = 0.0;
   out_7359357434108742810[316] = 0.0;
   out_7359357434108742810[317] = 0.0;
   out_7359357434108742810[318] = 0.0;
   out_7359357434108742810[319] = 0.0;
   out_7359357434108742810[320] = 0.0;
   out_7359357434108742810[321] = 0.0;
   out_7359357434108742810[322] = 0.0;
   out_7359357434108742810[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_729710836783527414) {
   out_729710836783527414[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_729710836783527414[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_729710836783527414[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_729710836783527414[3] = dt*state[12] + state[3];
   out_729710836783527414[4] = dt*state[13] + state[4];
   out_729710836783527414[5] = dt*state[14] + state[5];
   out_729710836783527414[6] = state[6];
   out_729710836783527414[7] = state[7];
   out_729710836783527414[8] = state[8];
   out_729710836783527414[9] = state[9];
   out_729710836783527414[10] = state[10];
   out_729710836783527414[11] = state[11];
   out_729710836783527414[12] = state[12];
   out_729710836783527414[13] = state[13];
   out_729710836783527414[14] = state[14];
   out_729710836783527414[15] = state[15];
   out_729710836783527414[16] = state[16];
   out_729710836783527414[17] = state[17];
}
void F_fun(double *state, double dt, double *out_8499021045100499940) {
   out_8499021045100499940[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8499021045100499940[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8499021045100499940[2] = 0;
   out_8499021045100499940[3] = 0;
   out_8499021045100499940[4] = 0;
   out_8499021045100499940[5] = 0;
   out_8499021045100499940[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8499021045100499940[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8499021045100499940[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8499021045100499940[9] = 0;
   out_8499021045100499940[10] = 0;
   out_8499021045100499940[11] = 0;
   out_8499021045100499940[12] = 0;
   out_8499021045100499940[13] = 0;
   out_8499021045100499940[14] = 0;
   out_8499021045100499940[15] = 0;
   out_8499021045100499940[16] = 0;
   out_8499021045100499940[17] = 0;
   out_8499021045100499940[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8499021045100499940[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8499021045100499940[20] = 0;
   out_8499021045100499940[21] = 0;
   out_8499021045100499940[22] = 0;
   out_8499021045100499940[23] = 0;
   out_8499021045100499940[24] = 0;
   out_8499021045100499940[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8499021045100499940[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8499021045100499940[27] = 0;
   out_8499021045100499940[28] = 0;
   out_8499021045100499940[29] = 0;
   out_8499021045100499940[30] = 0;
   out_8499021045100499940[31] = 0;
   out_8499021045100499940[32] = 0;
   out_8499021045100499940[33] = 0;
   out_8499021045100499940[34] = 0;
   out_8499021045100499940[35] = 0;
   out_8499021045100499940[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8499021045100499940[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8499021045100499940[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8499021045100499940[39] = 0;
   out_8499021045100499940[40] = 0;
   out_8499021045100499940[41] = 0;
   out_8499021045100499940[42] = 0;
   out_8499021045100499940[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8499021045100499940[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8499021045100499940[45] = 0;
   out_8499021045100499940[46] = 0;
   out_8499021045100499940[47] = 0;
   out_8499021045100499940[48] = 0;
   out_8499021045100499940[49] = 0;
   out_8499021045100499940[50] = 0;
   out_8499021045100499940[51] = 0;
   out_8499021045100499940[52] = 0;
   out_8499021045100499940[53] = 0;
   out_8499021045100499940[54] = 0;
   out_8499021045100499940[55] = 0;
   out_8499021045100499940[56] = 0;
   out_8499021045100499940[57] = 1;
   out_8499021045100499940[58] = 0;
   out_8499021045100499940[59] = 0;
   out_8499021045100499940[60] = 0;
   out_8499021045100499940[61] = 0;
   out_8499021045100499940[62] = 0;
   out_8499021045100499940[63] = 0;
   out_8499021045100499940[64] = 0;
   out_8499021045100499940[65] = 0;
   out_8499021045100499940[66] = dt;
   out_8499021045100499940[67] = 0;
   out_8499021045100499940[68] = 0;
   out_8499021045100499940[69] = 0;
   out_8499021045100499940[70] = 0;
   out_8499021045100499940[71] = 0;
   out_8499021045100499940[72] = 0;
   out_8499021045100499940[73] = 0;
   out_8499021045100499940[74] = 0;
   out_8499021045100499940[75] = 0;
   out_8499021045100499940[76] = 1;
   out_8499021045100499940[77] = 0;
   out_8499021045100499940[78] = 0;
   out_8499021045100499940[79] = 0;
   out_8499021045100499940[80] = 0;
   out_8499021045100499940[81] = 0;
   out_8499021045100499940[82] = 0;
   out_8499021045100499940[83] = 0;
   out_8499021045100499940[84] = 0;
   out_8499021045100499940[85] = dt;
   out_8499021045100499940[86] = 0;
   out_8499021045100499940[87] = 0;
   out_8499021045100499940[88] = 0;
   out_8499021045100499940[89] = 0;
   out_8499021045100499940[90] = 0;
   out_8499021045100499940[91] = 0;
   out_8499021045100499940[92] = 0;
   out_8499021045100499940[93] = 0;
   out_8499021045100499940[94] = 0;
   out_8499021045100499940[95] = 1;
   out_8499021045100499940[96] = 0;
   out_8499021045100499940[97] = 0;
   out_8499021045100499940[98] = 0;
   out_8499021045100499940[99] = 0;
   out_8499021045100499940[100] = 0;
   out_8499021045100499940[101] = 0;
   out_8499021045100499940[102] = 0;
   out_8499021045100499940[103] = 0;
   out_8499021045100499940[104] = dt;
   out_8499021045100499940[105] = 0;
   out_8499021045100499940[106] = 0;
   out_8499021045100499940[107] = 0;
   out_8499021045100499940[108] = 0;
   out_8499021045100499940[109] = 0;
   out_8499021045100499940[110] = 0;
   out_8499021045100499940[111] = 0;
   out_8499021045100499940[112] = 0;
   out_8499021045100499940[113] = 0;
   out_8499021045100499940[114] = 1;
   out_8499021045100499940[115] = 0;
   out_8499021045100499940[116] = 0;
   out_8499021045100499940[117] = 0;
   out_8499021045100499940[118] = 0;
   out_8499021045100499940[119] = 0;
   out_8499021045100499940[120] = 0;
   out_8499021045100499940[121] = 0;
   out_8499021045100499940[122] = 0;
   out_8499021045100499940[123] = 0;
   out_8499021045100499940[124] = 0;
   out_8499021045100499940[125] = 0;
   out_8499021045100499940[126] = 0;
   out_8499021045100499940[127] = 0;
   out_8499021045100499940[128] = 0;
   out_8499021045100499940[129] = 0;
   out_8499021045100499940[130] = 0;
   out_8499021045100499940[131] = 0;
   out_8499021045100499940[132] = 0;
   out_8499021045100499940[133] = 1;
   out_8499021045100499940[134] = 0;
   out_8499021045100499940[135] = 0;
   out_8499021045100499940[136] = 0;
   out_8499021045100499940[137] = 0;
   out_8499021045100499940[138] = 0;
   out_8499021045100499940[139] = 0;
   out_8499021045100499940[140] = 0;
   out_8499021045100499940[141] = 0;
   out_8499021045100499940[142] = 0;
   out_8499021045100499940[143] = 0;
   out_8499021045100499940[144] = 0;
   out_8499021045100499940[145] = 0;
   out_8499021045100499940[146] = 0;
   out_8499021045100499940[147] = 0;
   out_8499021045100499940[148] = 0;
   out_8499021045100499940[149] = 0;
   out_8499021045100499940[150] = 0;
   out_8499021045100499940[151] = 0;
   out_8499021045100499940[152] = 1;
   out_8499021045100499940[153] = 0;
   out_8499021045100499940[154] = 0;
   out_8499021045100499940[155] = 0;
   out_8499021045100499940[156] = 0;
   out_8499021045100499940[157] = 0;
   out_8499021045100499940[158] = 0;
   out_8499021045100499940[159] = 0;
   out_8499021045100499940[160] = 0;
   out_8499021045100499940[161] = 0;
   out_8499021045100499940[162] = 0;
   out_8499021045100499940[163] = 0;
   out_8499021045100499940[164] = 0;
   out_8499021045100499940[165] = 0;
   out_8499021045100499940[166] = 0;
   out_8499021045100499940[167] = 0;
   out_8499021045100499940[168] = 0;
   out_8499021045100499940[169] = 0;
   out_8499021045100499940[170] = 0;
   out_8499021045100499940[171] = 1;
   out_8499021045100499940[172] = 0;
   out_8499021045100499940[173] = 0;
   out_8499021045100499940[174] = 0;
   out_8499021045100499940[175] = 0;
   out_8499021045100499940[176] = 0;
   out_8499021045100499940[177] = 0;
   out_8499021045100499940[178] = 0;
   out_8499021045100499940[179] = 0;
   out_8499021045100499940[180] = 0;
   out_8499021045100499940[181] = 0;
   out_8499021045100499940[182] = 0;
   out_8499021045100499940[183] = 0;
   out_8499021045100499940[184] = 0;
   out_8499021045100499940[185] = 0;
   out_8499021045100499940[186] = 0;
   out_8499021045100499940[187] = 0;
   out_8499021045100499940[188] = 0;
   out_8499021045100499940[189] = 0;
   out_8499021045100499940[190] = 1;
   out_8499021045100499940[191] = 0;
   out_8499021045100499940[192] = 0;
   out_8499021045100499940[193] = 0;
   out_8499021045100499940[194] = 0;
   out_8499021045100499940[195] = 0;
   out_8499021045100499940[196] = 0;
   out_8499021045100499940[197] = 0;
   out_8499021045100499940[198] = 0;
   out_8499021045100499940[199] = 0;
   out_8499021045100499940[200] = 0;
   out_8499021045100499940[201] = 0;
   out_8499021045100499940[202] = 0;
   out_8499021045100499940[203] = 0;
   out_8499021045100499940[204] = 0;
   out_8499021045100499940[205] = 0;
   out_8499021045100499940[206] = 0;
   out_8499021045100499940[207] = 0;
   out_8499021045100499940[208] = 0;
   out_8499021045100499940[209] = 1;
   out_8499021045100499940[210] = 0;
   out_8499021045100499940[211] = 0;
   out_8499021045100499940[212] = 0;
   out_8499021045100499940[213] = 0;
   out_8499021045100499940[214] = 0;
   out_8499021045100499940[215] = 0;
   out_8499021045100499940[216] = 0;
   out_8499021045100499940[217] = 0;
   out_8499021045100499940[218] = 0;
   out_8499021045100499940[219] = 0;
   out_8499021045100499940[220] = 0;
   out_8499021045100499940[221] = 0;
   out_8499021045100499940[222] = 0;
   out_8499021045100499940[223] = 0;
   out_8499021045100499940[224] = 0;
   out_8499021045100499940[225] = 0;
   out_8499021045100499940[226] = 0;
   out_8499021045100499940[227] = 0;
   out_8499021045100499940[228] = 1;
   out_8499021045100499940[229] = 0;
   out_8499021045100499940[230] = 0;
   out_8499021045100499940[231] = 0;
   out_8499021045100499940[232] = 0;
   out_8499021045100499940[233] = 0;
   out_8499021045100499940[234] = 0;
   out_8499021045100499940[235] = 0;
   out_8499021045100499940[236] = 0;
   out_8499021045100499940[237] = 0;
   out_8499021045100499940[238] = 0;
   out_8499021045100499940[239] = 0;
   out_8499021045100499940[240] = 0;
   out_8499021045100499940[241] = 0;
   out_8499021045100499940[242] = 0;
   out_8499021045100499940[243] = 0;
   out_8499021045100499940[244] = 0;
   out_8499021045100499940[245] = 0;
   out_8499021045100499940[246] = 0;
   out_8499021045100499940[247] = 1;
   out_8499021045100499940[248] = 0;
   out_8499021045100499940[249] = 0;
   out_8499021045100499940[250] = 0;
   out_8499021045100499940[251] = 0;
   out_8499021045100499940[252] = 0;
   out_8499021045100499940[253] = 0;
   out_8499021045100499940[254] = 0;
   out_8499021045100499940[255] = 0;
   out_8499021045100499940[256] = 0;
   out_8499021045100499940[257] = 0;
   out_8499021045100499940[258] = 0;
   out_8499021045100499940[259] = 0;
   out_8499021045100499940[260] = 0;
   out_8499021045100499940[261] = 0;
   out_8499021045100499940[262] = 0;
   out_8499021045100499940[263] = 0;
   out_8499021045100499940[264] = 0;
   out_8499021045100499940[265] = 0;
   out_8499021045100499940[266] = 1;
   out_8499021045100499940[267] = 0;
   out_8499021045100499940[268] = 0;
   out_8499021045100499940[269] = 0;
   out_8499021045100499940[270] = 0;
   out_8499021045100499940[271] = 0;
   out_8499021045100499940[272] = 0;
   out_8499021045100499940[273] = 0;
   out_8499021045100499940[274] = 0;
   out_8499021045100499940[275] = 0;
   out_8499021045100499940[276] = 0;
   out_8499021045100499940[277] = 0;
   out_8499021045100499940[278] = 0;
   out_8499021045100499940[279] = 0;
   out_8499021045100499940[280] = 0;
   out_8499021045100499940[281] = 0;
   out_8499021045100499940[282] = 0;
   out_8499021045100499940[283] = 0;
   out_8499021045100499940[284] = 0;
   out_8499021045100499940[285] = 1;
   out_8499021045100499940[286] = 0;
   out_8499021045100499940[287] = 0;
   out_8499021045100499940[288] = 0;
   out_8499021045100499940[289] = 0;
   out_8499021045100499940[290] = 0;
   out_8499021045100499940[291] = 0;
   out_8499021045100499940[292] = 0;
   out_8499021045100499940[293] = 0;
   out_8499021045100499940[294] = 0;
   out_8499021045100499940[295] = 0;
   out_8499021045100499940[296] = 0;
   out_8499021045100499940[297] = 0;
   out_8499021045100499940[298] = 0;
   out_8499021045100499940[299] = 0;
   out_8499021045100499940[300] = 0;
   out_8499021045100499940[301] = 0;
   out_8499021045100499940[302] = 0;
   out_8499021045100499940[303] = 0;
   out_8499021045100499940[304] = 1;
   out_8499021045100499940[305] = 0;
   out_8499021045100499940[306] = 0;
   out_8499021045100499940[307] = 0;
   out_8499021045100499940[308] = 0;
   out_8499021045100499940[309] = 0;
   out_8499021045100499940[310] = 0;
   out_8499021045100499940[311] = 0;
   out_8499021045100499940[312] = 0;
   out_8499021045100499940[313] = 0;
   out_8499021045100499940[314] = 0;
   out_8499021045100499940[315] = 0;
   out_8499021045100499940[316] = 0;
   out_8499021045100499940[317] = 0;
   out_8499021045100499940[318] = 0;
   out_8499021045100499940[319] = 0;
   out_8499021045100499940[320] = 0;
   out_8499021045100499940[321] = 0;
   out_8499021045100499940[322] = 0;
   out_8499021045100499940[323] = 1;
}
void h_4(double *state, double *unused, double *out_1075969755128808186) {
   out_1075969755128808186[0] = state[6] + state[9];
   out_1075969755128808186[1] = state[7] + state[10];
   out_1075969755128808186[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_5239304832848850085) {
   out_5239304832848850085[0] = 0;
   out_5239304832848850085[1] = 0;
   out_5239304832848850085[2] = 0;
   out_5239304832848850085[3] = 0;
   out_5239304832848850085[4] = 0;
   out_5239304832848850085[5] = 0;
   out_5239304832848850085[6] = 1;
   out_5239304832848850085[7] = 0;
   out_5239304832848850085[8] = 0;
   out_5239304832848850085[9] = 1;
   out_5239304832848850085[10] = 0;
   out_5239304832848850085[11] = 0;
   out_5239304832848850085[12] = 0;
   out_5239304832848850085[13] = 0;
   out_5239304832848850085[14] = 0;
   out_5239304832848850085[15] = 0;
   out_5239304832848850085[16] = 0;
   out_5239304832848850085[17] = 0;
   out_5239304832848850085[18] = 0;
   out_5239304832848850085[19] = 0;
   out_5239304832848850085[20] = 0;
   out_5239304832848850085[21] = 0;
   out_5239304832848850085[22] = 0;
   out_5239304832848850085[23] = 0;
   out_5239304832848850085[24] = 0;
   out_5239304832848850085[25] = 1;
   out_5239304832848850085[26] = 0;
   out_5239304832848850085[27] = 0;
   out_5239304832848850085[28] = 1;
   out_5239304832848850085[29] = 0;
   out_5239304832848850085[30] = 0;
   out_5239304832848850085[31] = 0;
   out_5239304832848850085[32] = 0;
   out_5239304832848850085[33] = 0;
   out_5239304832848850085[34] = 0;
   out_5239304832848850085[35] = 0;
   out_5239304832848850085[36] = 0;
   out_5239304832848850085[37] = 0;
   out_5239304832848850085[38] = 0;
   out_5239304832848850085[39] = 0;
   out_5239304832848850085[40] = 0;
   out_5239304832848850085[41] = 0;
   out_5239304832848850085[42] = 0;
   out_5239304832848850085[43] = 0;
   out_5239304832848850085[44] = 1;
   out_5239304832848850085[45] = 0;
   out_5239304832848850085[46] = 0;
   out_5239304832848850085[47] = 1;
   out_5239304832848850085[48] = 0;
   out_5239304832848850085[49] = 0;
   out_5239304832848850085[50] = 0;
   out_5239304832848850085[51] = 0;
   out_5239304832848850085[52] = 0;
   out_5239304832848850085[53] = 0;
}
void h_10(double *state, double *unused, double *out_4050058798394118968) {
   out_4050058798394118968[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_4050058798394118968[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_4050058798394118968[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_3614267989921698938) {
   out_3614267989921698938[0] = 0;
   out_3614267989921698938[1] = 9.8100000000000005*cos(state[1]);
   out_3614267989921698938[2] = 0;
   out_3614267989921698938[3] = 0;
   out_3614267989921698938[4] = -state[8];
   out_3614267989921698938[5] = state[7];
   out_3614267989921698938[6] = 0;
   out_3614267989921698938[7] = state[5];
   out_3614267989921698938[8] = -state[4];
   out_3614267989921698938[9] = 0;
   out_3614267989921698938[10] = 0;
   out_3614267989921698938[11] = 0;
   out_3614267989921698938[12] = 1;
   out_3614267989921698938[13] = 0;
   out_3614267989921698938[14] = 0;
   out_3614267989921698938[15] = 1;
   out_3614267989921698938[16] = 0;
   out_3614267989921698938[17] = 0;
   out_3614267989921698938[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_3614267989921698938[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_3614267989921698938[20] = 0;
   out_3614267989921698938[21] = state[8];
   out_3614267989921698938[22] = 0;
   out_3614267989921698938[23] = -state[6];
   out_3614267989921698938[24] = -state[5];
   out_3614267989921698938[25] = 0;
   out_3614267989921698938[26] = state[3];
   out_3614267989921698938[27] = 0;
   out_3614267989921698938[28] = 0;
   out_3614267989921698938[29] = 0;
   out_3614267989921698938[30] = 0;
   out_3614267989921698938[31] = 1;
   out_3614267989921698938[32] = 0;
   out_3614267989921698938[33] = 0;
   out_3614267989921698938[34] = 1;
   out_3614267989921698938[35] = 0;
   out_3614267989921698938[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_3614267989921698938[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_3614267989921698938[38] = 0;
   out_3614267989921698938[39] = -state[7];
   out_3614267989921698938[40] = state[6];
   out_3614267989921698938[41] = 0;
   out_3614267989921698938[42] = state[4];
   out_3614267989921698938[43] = -state[3];
   out_3614267989921698938[44] = 0;
   out_3614267989921698938[45] = 0;
   out_3614267989921698938[46] = 0;
   out_3614267989921698938[47] = 0;
   out_3614267989921698938[48] = 0;
   out_3614267989921698938[49] = 0;
   out_3614267989921698938[50] = 1;
   out_3614267989921698938[51] = 0;
   out_3614267989921698938[52] = 0;
   out_3614267989921698938[53] = 1;
}
void h_13(double *state, double *unused, double *out_9106371473023855959) {
   out_9106371473023855959[0] = state[3];
   out_9106371473023855959[1] = state[4];
   out_9106371473023855959[2] = state[5];
}
void H_13(double *state, double *unused, double *out_8451578658181182886) {
   out_8451578658181182886[0] = 0;
   out_8451578658181182886[1] = 0;
   out_8451578658181182886[2] = 0;
   out_8451578658181182886[3] = 1;
   out_8451578658181182886[4] = 0;
   out_8451578658181182886[5] = 0;
   out_8451578658181182886[6] = 0;
   out_8451578658181182886[7] = 0;
   out_8451578658181182886[8] = 0;
   out_8451578658181182886[9] = 0;
   out_8451578658181182886[10] = 0;
   out_8451578658181182886[11] = 0;
   out_8451578658181182886[12] = 0;
   out_8451578658181182886[13] = 0;
   out_8451578658181182886[14] = 0;
   out_8451578658181182886[15] = 0;
   out_8451578658181182886[16] = 0;
   out_8451578658181182886[17] = 0;
   out_8451578658181182886[18] = 0;
   out_8451578658181182886[19] = 0;
   out_8451578658181182886[20] = 0;
   out_8451578658181182886[21] = 0;
   out_8451578658181182886[22] = 1;
   out_8451578658181182886[23] = 0;
   out_8451578658181182886[24] = 0;
   out_8451578658181182886[25] = 0;
   out_8451578658181182886[26] = 0;
   out_8451578658181182886[27] = 0;
   out_8451578658181182886[28] = 0;
   out_8451578658181182886[29] = 0;
   out_8451578658181182886[30] = 0;
   out_8451578658181182886[31] = 0;
   out_8451578658181182886[32] = 0;
   out_8451578658181182886[33] = 0;
   out_8451578658181182886[34] = 0;
   out_8451578658181182886[35] = 0;
   out_8451578658181182886[36] = 0;
   out_8451578658181182886[37] = 0;
   out_8451578658181182886[38] = 0;
   out_8451578658181182886[39] = 0;
   out_8451578658181182886[40] = 0;
   out_8451578658181182886[41] = 1;
   out_8451578658181182886[42] = 0;
   out_8451578658181182886[43] = 0;
   out_8451578658181182886[44] = 0;
   out_8451578658181182886[45] = 0;
   out_8451578658181182886[46] = 0;
   out_8451578658181182886[47] = 0;
   out_8451578658181182886[48] = 0;
   out_8451578658181182886[49] = 0;
   out_8451578658181182886[50] = 0;
   out_8451578658181182886[51] = 0;
   out_8451578658181182886[52] = 0;
   out_8451578658181182886[53] = 0;
}
void h_14(double *state, double *unused, double *out_4347596682095509082) {
   out_4347596682095509082[0] = state[6];
   out_4347596682095509082[1] = state[7];
   out_4347596682095509082[2] = state[8];
}
void H_14(double *state, double *unused, double *out_9202545689188334614) {
   out_9202545689188334614[0] = 0;
   out_9202545689188334614[1] = 0;
   out_9202545689188334614[2] = 0;
   out_9202545689188334614[3] = 0;
   out_9202545689188334614[4] = 0;
   out_9202545689188334614[5] = 0;
   out_9202545689188334614[6] = 1;
   out_9202545689188334614[7] = 0;
   out_9202545689188334614[8] = 0;
   out_9202545689188334614[9] = 0;
   out_9202545689188334614[10] = 0;
   out_9202545689188334614[11] = 0;
   out_9202545689188334614[12] = 0;
   out_9202545689188334614[13] = 0;
   out_9202545689188334614[14] = 0;
   out_9202545689188334614[15] = 0;
   out_9202545689188334614[16] = 0;
   out_9202545689188334614[17] = 0;
   out_9202545689188334614[18] = 0;
   out_9202545689188334614[19] = 0;
   out_9202545689188334614[20] = 0;
   out_9202545689188334614[21] = 0;
   out_9202545689188334614[22] = 0;
   out_9202545689188334614[23] = 0;
   out_9202545689188334614[24] = 0;
   out_9202545689188334614[25] = 1;
   out_9202545689188334614[26] = 0;
   out_9202545689188334614[27] = 0;
   out_9202545689188334614[28] = 0;
   out_9202545689188334614[29] = 0;
   out_9202545689188334614[30] = 0;
   out_9202545689188334614[31] = 0;
   out_9202545689188334614[32] = 0;
   out_9202545689188334614[33] = 0;
   out_9202545689188334614[34] = 0;
   out_9202545689188334614[35] = 0;
   out_9202545689188334614[36] = 0;
   out_9202545689188334614[37] = 0;
   out_9202545689188334614[38] = 0;
   out_9202545689188334614[39] = 0;
   out_9202545689188334614[40] = 0;
   out_9202545689188334614[41] = 0;
   out_9202545689188334614[42] = 0;
   out_9202545689188334614[43] = 0;
   out_9202545689188334614[44] = 1;
   out_9202545689188334614[45] = 0;
   out_9202545689188334614[46] = 0;
   out_9202545689188334614[47] = 0;
   out_9202545689188334614[48] = 0;
   out_9202545689188334614[49] = 0;
   out_9202545689188334614[50] = 0;
   out_9202545689188334614[51] = 0;
   out_9202545689188334614[52] = 0;
   out_9202545689188334614[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_9191542510053319957) {
  err_fun(nom_x, delta_x, out_9191542510053319957);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3004969772891744590) {
  inv_err_fun(nom_x, true_x, out_3004969772891744590);
}
void pose_H_mod_fun(double *state, double *out_7359357434108742810) {
  H_mod_fun(state, out_7359357434108742810);
}
void pose_f_fun(double *state, double dt, double *out_729710836783527414) {
  f_fun(state,  dt, out_729710836783527414);
}
void pose_F_fun(double *state, double dt, double *out_8499021045100499940) {
  F_fun(state,  dt, out_8499021045100499940);
}
void pose_h_4(double *state, double *unused, double *out_1075969755128808186) {
  h_4(state, unused, out_1075969755128808186);
}
void pose_H_4(double *state, double *unused, double *out_5239304832848850085) {
  H_4(state, unused, out_5239304832848850085);
}
void pose_h_10(double *state, double *unused, double *out_4050058798394118968) {
  h_10(state, unused, out_4050058798394118968);
}
void pose_H_10(double *state, double *unused, double *out_3614267989921698938) {
  H_10(state, unused, out_3614267989921698938);
}
void pose_h_13(double *state, double *unused, double *out_9106371473023855959) {
  h_13(state, unused, out_9106371473023855959);
}
void pose_H_13(double *state, double *unused, double *out_8451578658181182886) {
  H_13(state, unused, out_8451578658181182886);
}
void pose_h_14(double *state, double *unused, double *out_4347596682095509082) {
  h_14(state, unused, out_4347596682095509082);
}
void pose_H_14(double *state, double *unused, double *out_9202545689188334614) {
  H_14(state, unused, out_9202545689188334614);
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

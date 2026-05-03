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
void err_fun(double *nom_x, double *delta_x, double *out_5188089899736494345) {
   out_5188089899736494345[0] = delta_x[0] + nom_x[0];
   out_5188089899736494345[1] = delta_x[1] + nom_x[1];
   out_5188089899736494345[2] = delta_x[2] + nom_x[2];
   out_5188089899736494345[3] = delta_x[3] + nom_x[3];
   out_5188089899736494345[4] = delta_x[4] + nom_x[4];
   out_5188089899736494345[5] = delta_x[5] + nom_x[5];
   out_5188089899736494345[6] = delta_x[6] + nom_x[6];
   out_5188089899736494345[7] = delta_x[7] + nom_x[7];
   out_5188089899736494345[8] = delta_x[8] + nom_x[8];
   out_5188089899736494345[9] = delta_x[9] + nom_x[9];
   out_5188089899736494345[10] = delta_x[10] + nom_x[10];
   out_5188089899736494345[11] = delta_x[11] + nom_x[11];
   out_5188089899736494345[12] = delta_x[12] + nom_x[12];
   out_5188089899736494345[13] = delta_x[13] + nom_x[13];
   out_5188089899736494345[14] = delta_x[14] + nom_x[14];
   out_5188089899736494345[15] = delta_x[15] + nom_x[15];
   out_5188089899736494345[16] = delta_x[16] + nom_x[16];
   out_5188089899736494345[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_516319331577825853) {
   out_516319331577825853[0] = -nom_x[0] + true_x[0];
   out_516319331577825853[1] = -nom_x[1] + true_x[1];
   out_516319331577825853[2] = -nom_x[2] + true_x[2];
   out_516319331577825853[3] = -nom_x[3] + true_x[3];
   out_516319331577825853[4] = -nom_x[4] + true_x[4];
   out_516319331577825853[5] = -nom_x[5] + true_x[5];
   out_516319331577825853[6] = -nom_x[6] + true_x[6];
   out_516319331577825853[7] = -nom_x[7] + true_x[7];
   out_516319331577825853[8] = -nom_x[8] + true_x[8];
   out_516319331577825853[9] = -nom_x[9] + true_x[9];
   out_516319331577825853[10] = -nom_x[10] + true_x[10];
   out_516319331577825853[11] = -nom_x[11] + true_x[11];
   out_516319331577825853[12] = -nom_x[12] + true_x[12];
   out_516319331577825853[13] = -nom_x[13] + true_x[13];
   out_516319331577825853[14] = -nom_x[14] + true_x[14];
   out_516319331577825853[15] = -nom_x[15] + true_x[15];
   out_516319331577825853[16] = -nom_x[16] + true_x[16];
   out_516319331577825853[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_4951567132479851062) {
   out_4951567132479851062[0] = 1.0;
   out_4951567132479851062[1] = 0.0;
   out_4951567132479851062[2] = 0.0;
   out_4951567132479851062[3] = 0.0;
   out_4951567132479851062[4] = 0.0;
   out_4951567132479851062[5] = 0.0;
   out_4951567132479851062[6] = 0.0;
   out_4951567132479851062[7] = 0.0;
   out_4951567132479851062[8] = 0.0;
   out_4951567132479851062[9] = 0.0;
   out_4951567132479851062[10] = 0.0;
   out_4951567132479851062[11] = 0.0;
   out_4951567132479851062[12] = 0.0;
   out_4951567132479851062[13] = 0.0;
   out_4951567132479851062[14] = 0.0;
   out_4951567132479851062[15] = 0.0;
   out_4951567132479851062[16] = 0.0;
   out_4951567132479851062[17] = 0.0;
   out_4951567132479851062[18] = 0.0;
   out_4951567132479851062[19] = 1.0;
   out_4951567132479851062[20] = 0.0;
   out_4951567132479851062[21] = 0.0;
   out_4951567132479851062[22] = 0.0;
   out_4951567132479851062[23] = 0.0;
   out_4951567132479851062[24] = 0.0;
   out_4951567132479851062[25] = 0.0;
   out_4951567132479851062[26] = 0.0;
   out_4951567132479851062[27] = 0.0;
   out_4951567132479851062[28] = 0.0;
   out_4951567132479851062[29] = 0.0;
   out_4951567132479851062[30] = 0.0;
   out_4951567132479851062[31] = 0.0;
   out_4951567132479851062[32] = 0.0;
   out_4951567132479851062[33] = 0.0;
   out_4951567132479851062[34] = 0.0;
   out_4951567132479851062[35] = 0.0;
   out_4951567132479851062[36] = 0.0;
   out_4951567132479851062[37] = 0.0;
   out_4951567132479851062[38] = 1.0;
   out_4951567132479851062[39] = 0.0;
   out_4951567132479851062[40] = 0.0;
   out_4951567132479851062[41] = 0.0;
   out_4951567132479851062[42] = 0.0;
   out_4951567132479851062[43] = 0.0;
   out_4951567132479851062[44] = 0.0;
   out_4951567132479851062[45] = 0.0;
   out_4951567132479851062[46] = 0.0;
   out_4951567132479851062[47] = 0.0;
   out_4951567132479851062[48] = 0.0;
   out_4951567132479851062[49] = 0.0;
   out_4951567132479851062[50] = 0.0;
   out_4951567132479851062[51] = 0.0;
   out_4951567132479851062[52] = 0.0;
   out_4951567132479851062[53] = 0.0;
   out_4951567132479851062[54] = 0.0;
   out_4951567132479851062[55] = 0.0;
   out_4951567132479851062[56] = 0.0;
   out_4951567132479851062[57] = 1.0;
   out_4951567132479851062[58] = 0.0;
   out_4951567132479851062[59] = 0.0;
   out_4951567132479851062[60] = 0.0;
   out_4951567132479851062[61] = 0.0;
   out_4951567132479851062[62] = 0.0;
   out_4951567132479851062[63] = 0.0;
   out_4951567132479851062[64] = 0.0;
   out_4951567132479851062[65] = 0.0;
   out_4951567132479851062[66] = 0.0;
   out_4951567132479851062[67] = 0.0;
   out_4951567132479851062[68] = 0.0;
   out_4951567132479851062[69] = 0.0;
   out_4951567132479851062[70] = 0.0;
   out_4951567132479851062[71] = 0.0;
   out_4951567132479851062[72] = 0.0;
   out_4951567132479851062[73] = 0.0;
   out_4951567132479851062[74] = 0.0;
   out_4951567132479851062[75] = 0.0;
   out_4951567132479851062[76] = 1.0;
   out_4951567132479851062[77] = 0.0;
   out_4951567132479851062[78] = 0.0;
   out_4951567132479851062[79] = 0.0;
   out_4951567132479851062[80] = 0.0;
   out_4951567132479851062[81] = 0.0;
   out_4951567132479851062[82] = 0.0;
   out_4951567132479851062[83] = 0.0;
   out_4951567132479851062[84] = 0.0;
   out_4951567132479851062[85] = 0.0;
   out_4951567132479851062[86] = 0.0;
   out_4951567132479851062[87] = 0.0;
   out_4951567132479851062[88] = 0.0;
   out_4951567132479851062[89] = 0.0;
   out_4951567132479851062[90] = 0.0;
   out_4951567132479851062[91] = 0.0;
   out_4951567132479851062[92] = 0.0;
   out_4951567132479851062[93] = 0.0;
   out_4951567132479851062[94] = 0.0;
   out_4951567132479851062[95] = 1.0;
   out_4951567132479851062[96] = 0.0;
   out_4951567132479851062[97] = 0.0;
   out_4951567132479851062[98] = 0.0;
   out_4951567132479851062[99] = 0.0;
   out_4951567132479851062[100] = 0.0;
   out_4951567132479851062[101] = 0.0;
   out_4951567132479851062[102] = 0.0;
   out_4951567132479851062[103] = 0.0;
   out_4951567132479851062[104] = 0.0;
   out_4951567132479851062[105] = 0.0;
   out_4951567132479851062[106] = 0.0;
   out_4951567132479851062[107] = 0.0;
   out_4951567132479851062[108] = 0.0;
   out_4951567132479851062[109] = 0.0;
   out_4951567132479851062[110] = 0.0;
   out_4951567132479851062[111] = 0.0;
   out_4951567132479851062[112] = 0.0;
   out_4951567132479851062[113] = 0.0;
   out_4951567132479851062[114] = 1.0;
   out_4951567132479851062[115] = 0.0;
   out_4951567132479851062[116] = 0.0;
   out_4951567132479851062[117] = 0.0;
   out_4951567132479851062[118] = 0.0;
   out_4951567132479851062[119] = 0.0;
   out_4951567132479851062[120] = 0.0;
   out_4951567132479851062[121] = 0.0;
   out_4951567132479851062[122] = 0.0;
   out_4951567132479851062[123] = 0.0;
   out_4951567132479851062[124] = 0.0;
   out_4951567132479851062[125] = 0.0;
   out_4951567132479851062[126] = 0.0;
   out_4951567132479851062[127] = 0.0;
   out_4951567132479851062[128] = 0.0;
   out_4951567132479851062[129] = 0.0;
   out_4951567132479851062[130] = 0.0;
   out_4951567132479851062[131] = 0.0;
   out_4951567132479851062[132] = 0.0;
   out_4951567132479851062[133] = 1.0;
   out_4951567132479851062[134] = 0.0;
   out_4951567132479851062[135] = 0.0;
   out_4951567132479851062[136] = 0.0;
   out_4951567132479851062[137] = 0.0;
   out_4951567132479851062[138] = 0.0;
   out_4951567132479851062[139] = 0.0;
   out_4951567132479851062[140] = 0.0;
   out_4951567132479851062[141] = 0.0;
   out_4951567132479851062[142] = 0.0;
   out_4951567132479851062[143] = 0.0;
   out_4951567132479851062[144] = 0.0;
   out_4951567132479851062[145] = 0.0;
   out_4951567132479851062[146] = 0.0;
   out_4951567132479851062[147] = 0.0;
   out_4951567132479851062[148] = 0.0;
   out_4951567132479851062[149] = 0.0;
   out_4951567132479851062[150] = 0.0;
   out_4951567132479851062[151] = 0.0;
   out_4951567132479851062[152] = 1.0;
   out_4951567132479851062[153] = 0.0;
   out_4951567132479851062[154] = 0.0;
   out_4951567132479851062[155] = 0.0;
   out_4951567132479851062[156] = 0.0;
   out_4951567132479851062[157] = 0.0;
   out_4951567132479851062[158] = 0.0;
   out_4951567132479851062[159] = 0.0;
   out_4951567132479851062[160] = 0.0;
   out_4951567132479851062[161] = 0.0;
   out_4951567132479851062[162] = 0.0;
   out_4951567132479851062[163] = 0.0;
   out_4951567132479851062[164] = 0.0;
   out_4951567132479851062[165] = 0.0;
   out_4951567132479851062[166] = 0.0;
   out_4951567132479851062[167] = 0.0;
   out_4951567132479851062[168] = 0.0;
   out_4951567132479851062[169] = 0.0;
   out_4951567132479851062[170] = 0.0;
   out_4951567132479851062[171] = 1.0;
   out_4951567132479851062[172] = 0.0;
   out_4951567132479851062[173] = 0.0;
   out_4951567132479851062[174] = 0.0;
   out_4951567132479851062[175] = 0.0;
   out_4951567132479851062[176] = 0.0;
   out_4951567132479851062[177] = 0.0;
   out_4951567132479851062[178] = 0.0;
   out_4951567132479851062[179] = 0.0;
   out_4951567132479851062[180] = 0.0;
   out_4951567132479851062[181] = 0.0;
   out_4951567132479851062[182] = 0.0;
   out_4951567132479851062[183] = 0.0;
   out_4951567132479851062[184] = 0.0;
   out_4951567132479851062[185] = 0.0;
   out_4951567132479851062[186] = 0.0;
   out_4951567132479851062[187] = 0.0;
   out_4951567132479851062[188] = 0.0;
   out_4951567132479851062[189] = 0.0;
   out_4951567132479851062[190] = 1.0;
   out_4951567132479851062[191] = 0.0;
   out_4951567132479851062[192] = 0.0;
   out_4951567132479851062[193] = 0.0;
   out_4951567132479851062[194] = 0.0;
   out_4951567132479851062[195] = 0.0;
   out_4951567132479851062[196] = 0.0;
   out_4951567132479851062[197] = 0.0;
   out_4951567132479851062[198] = 0.0;
   out_4951567132479851062[199] = 0.0;
   out_4951567132479851062[200] = 0.0;
   out_4951567132479851062[201] = 0.0;
   out_4951567132479851062[202] = 0.0;
   out_4951567132479851062[203] = 0.0;
   out_4951567132479851062[204] = 0.0;
   out_4951567132479851062[205] = 0.0;
   out_4951567132479851062[206] = 0.0;
   out_4951567132479851062[207] = 0.0;
   out_4951567132479851062[208] = 0.0;
   out_4951567132479851062[209] = 1.0;
   out_4951567132479851062[210] = 0.0;
   out_4951567132479851062[211] = 0.0;
   out_4951567132479851062[212] = 0.0;
   out_4951567132479851062[213] = 0.0;
   out_4951567132479851062[214] = 0.0;
   out_4951567132479851062[215] = 0.0;
   out_4951567132479851062[216] = 0.0;
   out_4951567132479851062[217] = 0.0;
   out_4951567132479851062[218] = 0.0;
   out_4951567132479851062[219] = 0.0;
   out_4951567132479851062[220] = 0.0;
   out_4951567132479851062[221] = 0.0;
   out_4951567132479851062[222] = 0.0;
   out_4951567132479851062[223] = 0.0;
   out_4951567132479851062[224] = 0.0;
   out_4951567132479851062[225] = 0.0;
   out_4951567132479851062[226] = 0.0;
   out_4951567132479851062[227] = 0.0;
   out_4951567132479851062[228] = 1.0;
   out_4951567132479851062[229] = 0.0;
   out_4951567132479851062[230] = 0.0;
   out_4951567132479851062[231] = 0.0;
   out_4951567132479851062[232] = 0.0;
   out_4951567132479851062[233] = 0.0;
   out_4951567132479851062[234] = 0.0;
   out_4951567132479851062[235] = 0.0;
   out_4951567132479851062[236] = 0.0;
   out_4951567132479851062[237] = 0.0;
   out_4951567132479851062[238] = 0.0;
   out_4951567132479851062[239] = 0.0;
   out_4951567132479851062[240] = 0.0;
   out_4951567132479851062[241] = 0.0;
   out_4951567132479851062[242] = 0.0;
   out_4951567132479851062[243] = 0.0;
   out_4951567132479851062[244] = 0.0;
   out_4951567132479851062[245] = 0.0;
   out_4951567132479851062[246] = 0.0;
   out_4951567132479851062[247] = 1.0;
   out_4951567132479851062[248] = 0.0;
   out_4951567132479851062[249] = 0.0;
   out_4951567132479851062[250] = 0.0;
   out_4951567132479851062[251] = 0.0;
   out_4951567132479851062[252] = 0.0;
   out_4951567132479851062[253] = 0.0;
   out_4951567132479851062[254] = 0.0;
   out_4951567132479851062[255] = 0.0;
   out_4951567132479851062[256] = 0.0;
   out_4951567132479851062[257] = 0.0;
   out_4951567132479851062[258] = 0.0;
   out_4951567132479851062[259] = 0.0;
   out_4951567132479851062[260] = 0.0;
   out_4951567132479851062[261] = 0.0;
   out_4951567132479851062[262] = 0.0;
   out_4951567132479851062[263] = 0.0;
   out_4951567132479851062[264] = 0.0;
   out_4951567132479851062[265] = 0.0;
   out_4951567132479851062[266] = 1.0;
   out_4951567132479851062[267] = 0.0;
   out_4951567132479851062[268] = 0.0;
   out_4951567132479851062[269] = 0.0;
   out_4951567132479851062[270] = 0.0;
   out_4951567132479851062[271] = 0.0;
   out_4951567132479851062[272] = 0.0;
   out_4951567132479851062[273] = 0.0;
   out_4951567132479851062[274] = 0.0;
   out_4951567132479851062[275] = 0.0;
   out_4951567132479851062[276] = 0.0;
   out_4951567132479851062[277] = 0.0;
   out_4951567132479851062[278] = 0.0;
   out_4951567132479851062[279] = 0.0;
   out_4951567132479851062[280] = 0.0;
   out_4951567132479851062[281] = 0.0;
   out_4951567132479851062[282] = 0.0;
   out_4951567132479851062[283] = 0.0;
   out_4951567132479851062[284] = 0.0;
   out_4951567132479851062[285] = 1.0;
   out_4951567132479851062[286] = 0.0;
   out_4951567132479851062[287] = 0.0;
   out_4951567132479851062[288] = 0.0;
   out_4951567132479851062[289] = 0.0;
   out_4951567132479851062[290] = 0.0;
   out_4951567132479851062[291] = 0.0;
   out_4951567132479851062[292] = 0.0;
   out_4951567132479851062[293] = 0.0;
   out_4951567132479851062[294] = 0.0;
   out_4951567132479851062[295] = 0.0;
   out_4951567132479851062[296] = 0.0;
   out_4951567132479851062[297] = 0.0;
   out_4951567132479851062[298] = 0.0;
   out_4951567132479851062[299] = 0.0;
   out_4951567132479851062[300] = 0.0;
   out_4951567132479851062[301] = 0.0;
   out_4951567132479851062[302] = 0.0;
   out_4951567132479851062[303] = 0.0;
   out_4951567132479851062[304] = 1.0;
   out_4951567132479851062[305] = 0.0;
   out_4951567132479851062[306] = 0.0;
   out_4951567132479851062[307] = 0.0;
   out_4951567132479851062[308] = 0.0;
   out_4951567132479851062[309] = 0.0;
   out_4951567132479851062[310] = 0.0;
   out_4951567132479851062[311] = 0.0;
   out_4951567132479851062[312] = 0.0;
   out_4951567132479851062[313] = 0.0;
   out_4951567132479851062[314] = 0.0;
   out_4951567132479851062[315] = 0.0;
   out_4951567132479851062[316] = 0.0;
   out_4951567132479851062[317] = 0.0;
   out_4951567132479851062[318] = 0.0;
   out_4951567132479851062[319] = 0.0;
   out_4951567132479851062[320] = 0.0;
   out_4951567132479851062[321] = 0.0;
   out_4951567132479851062[322] = 0.0;
   out_4951567132479851062[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_898454825503606476) {
   out_898454825503606476[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_898454825503606476[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_898454825503606476[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_898454825503606476[3] = dt*state[12] + state[3];
   out_898454825503606476[4] = dt*state[13] + state[4];
   out_898454825503606476[5] = dt*state[14] + state[5];
   out_898454825503606476[6] = state[6];
   out_898454825503606476[7] = state[7];
   out_898454825503606476[8] = state[8];
   out_898454825503606476[9] = state[9];
   out_898454825503606476[10] = state[10];
   out_898454825503606476[11] = state[11];
   out_898454825503606476[12] = state[12];
   out_898454825503606476[13] = state[13];
   out_898454825503606476[14] = state[14];
   out_898454825503606476[15] = state[15];
   out_898454825503606476[16] = state[16];
   out_898454825503606476[17] = state[17];
}
void F_fun(double *state, double dt, double *out_906698228237069812) {
   out_906698228237069812[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_906698228237069812[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_906698228237069812[2] = 0;
   out_906698228237069812[3] = 0;
   out_906698228237069812[4] = 0;
   out_906698228237069812[5] = 0;
   out_906698228237069812[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_906698228237069812[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_906698228237069812[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_906698228237069812[9] = 0;
   out_906698228237069812[10] = 0;
   out_906698228237069812[11] = 0;
   out_906698228237069812[12] = 0;
   out_906698228237069812[13] = 0;
   out_906698228237069812[14] = 0;
   out_906698228237069812[15] = 0;
   out_906698228237069812[16] = 0;
   out_906698228237069812[17] = 0;
   out_906698228237069812[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_906698228237069812[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_906698228237069812[20] = 0;
   out_906698228237069812[21] = 0;
   out_906698228237069812[22] = 0;
   out_906698228237069812[23] = 0;
   out_906698228237069812[24] = 0;
   out_906698228237069812[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_906698228237069812[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_906698228237069812[27] = 0;
   out_906698228237069812[28] = 0;
   out_906698228237069812[29] = 0;
   out_906698228237069812[30] = 0;
   out_906698228237069812[31] = 0;
   out_906698228237069812[32] = 0;
   out_906698228237069812[33] = 0;
   out_906698228237069812[34] = 0;
   out_906698228237069812[35] = 0;
   out_906698228237069812[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_906698228237069812[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_906698228237069812[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_906698228237069812[39] = 0;
   out_906698228237069812[40] = 0;
   out_906698228237069812[41] = 0;
   out_906698228237069812[42] = 0;
   out_906698228237069812[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_906698228237069812[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_906698228237069812[45] = 0;
   out_906698228237069812[46] = 0;
   out_906698228237069812[47] = 0;
   out_906698228237069812[48] = 0;
   out_906698228237069812[49] = 0;
   out_906698228237069812[50] = 0;
   out_906698228237069812[51] = 0;
   out_906698228237069812[52] = 0;
   out_906698228237069812[53] = 0;
   out_906698228237069812[54] = 0;
   out_906698228237069812[55] = 0;
   out_906698228237069812[56] = 0;
   out_906698228237069812[57] = 1;
   out_906698228237069812[58] = 0;
   out_906698228237069812[59] = 0;
   out_906698228237069812[60] = 0;
   out_906698228237069812[61] = 0;
   out_906698228237069812[62] = 0;
   out_906698228237069812[63] = 0;
   out_906698228237069812[64] = 0;
   out_906698228237069812[65] = 0;
   out_906698228237069812[66] = dt;
   out_906698228237069812[67] = 0;
   out_906698228237069812[68] = 0;
   out_906698228237069812[69] = 0;
   out_906698228237069812[70] = 0;
   out_906698228237069812[71] = 0;
   out_906698228237069812[72] = 0;
   out_906698228237069812[73] = 0;
   out_906698228237069812[74] = 0;
   out_906698228237069812[75] = 0;
   out_906698228237069812[76] = 1;
   out_906698228237069812[77] = 0;
   out_906698228237069812[78] = 0;
   out_906698228237069812[79] = 0;
   out_906698228237069812[80] = 0;
   out_906698228237069812[81] = 0;
   out_906698228237069812[82] = 0;
   out_906698228237069812[83] = 0;
   out_906698228237069812[84] = 0;
   out_906698228237069812[85] = dt;
   out_906698228237069812[86] = 0;
   out_906698228237069812[87] = 0;
   out_906698228237069812[88] = 0;
   out_906698228237069812[89] = 0;
   out_906698228237069812[90] = 0;
   out_906698228237069812[91] = 0;
   out_906698228237069812[92] = 0;
   out_906698228237069812[93] = 0;
   out_906698228237069812[94] = 0;
   out_906698228237069812[95] = 1;
   out_906698228237069812[96] = 0;
   out_906698228237069812[97] = 0;
   out_906698228237069812[98] = 0;
   out_906698228237069812[99] = 0;
   out_906698228237069812[100] = 0;
   out_906698228237069812[101] = 0;
   out_906698228237069812[102] = 0;
   out_906698228237069812[103] = 0;
   out_906698228237069812[104] = dt;
   out_906698228237069812[105] = 0;
   out_906698228237069812[106] = 0;
   out_906698228237069812[107] = 0;
   out_906698228237069812[108] = 0;
   out_906698228237069812[109] = 0;
   out_906698228237069812[110] = 0;
   out_906698228237069812[111] = 0;
   out_906698228237069812[112] = 0;
   out_906698228237069812[113] = 0;
   out_906698228237069812[114] = 1;
   out_906698228237069812[115] = 0;
   out_906698228237069812[116] = 0;
   out_906698228237069812[117] = 0;
   out_906698228237069812[118] = 0;
   out_906698228237069812[119] = 0;
   out_906698228237069812[120] = 0;
   out_906698228237069812[121] = 0;
   out_906698228237069812[122] = 0;
   out_906698228237069812[123] = 0;
   out_906698228237069812[124] = 0;
   out_906698228237069812[125] = 0;
   out_906698228237069812[126] = 0;
   out_906698228237069812[127] = 0;
   out_906698228237069812[128] = 0;
   out_906698228237069812[129] = 0;
   out_906698228237069812[130] = 0;
   out_906698228237069812[131] = 0;
   out_906698228237069812[132] = 0;
   out_906698228237069812[133] = 1;
   out_906698228237069812[134] = 0;
   out_906698228237069812[135] = 0;
   out_906698228237069812[136] = 0;
   out_906698228237069812[137] = 0;
   out_906698228237069812[138] = 0;
   out_906698228237069812[139] = 0;
   out_906698228237069812[140] = 0;
   out_906698228237069812[141] = 0;
   out_906698228237069812[142] = 0;
   out_906698228237069812[143] = 0;
   out_906698228237069812[144] = 0;
   out_906698228237069812[145] = 0;
   out_906698228237069812[146] = 0;
   out_906698228237069812[147] = 0;
   out_906698228237069812[148] = 0;
   out_906698228237069812[149] = 0;
   out_906698228237069812[150] = 0;
   out_906698228237069812[151] = 0;
   out_906698228237069812[152] = 1;
   out_906698228237069812[153] = 0;
   out_906698228237069812[154] = 0;
   out_906698228237069812[155] = 0;
   out_906698228237069812[156] = 0;
   out_906698228237069812[157] = 0;
   out_906698228237069812[158] = 0;
   out_906698228237069812[159] = 0;
   out_906698228237069812[160] = 0;
   out_906698228237069812[161] = 0;
   out_906698228237069812[162] = 0;
   out_906698228237069812[163] = 0;
   out_906698228237069812[164] = 0;
   out_906698228237069812[165] = 0;
   out_906698228237069812[166] = 0;
   out_906698228237069812[167] = 0;
   out_906698228237069812[168] = 0;
   out_906698228237069812[169] = 0;
   out_906698228237069812[170] = 0;
   out_906698228237069812[171] = 1;
   out_906698228237069812[172] = 0;
   out_906698228237069812[173] = 0;
   out_906698228237069812[174] = 0;
   out_906698228237069812[175] = 0;
   out_906698228237069812[176] = 0;
   out_906698228237069812[177] = 0;
   out_906698228237069812[178] = 0;
   out_906698228237069812[179] = 0;
   out_906698228237069812[180] = 0;
   out_906698228237069812[181] = 0;
   out_906698228237069812[182] = 0;
   out_906698228237069812[183] = 0;
   out_906698228237069812[184] = 0;
   out_906698228237069812[185] = 0;
   out_906698228237069812[186] = 0;
   out_906698228237069812[187] = 0;
   out_906698228237069812[188] = 0;
   out_906698228237069812[189] = 0;
   out_906698228237069812[190] = 1;
   out_906698228237069812[191] = 0;
   out_906698228237069812[192] = 0;
   out_906698228237069812[193] = 0;
   out_906698228237069812[194] = 0;
   out_906698228237069812[195] = 0;
   out_906698228237069812[196] = 0;
   out_906698228237069812[197] = 0;
   out_906698228237069812[198] = 0;
   out_906698228237069812[199] = 0;
   out_906698228237069812[200] = 0;
   out_906698228237069812[201] = 0;
   out_906698228237069812[202] = 0;
   out_906698228237069812[203] = 0;
   out_906698228237069812[204] = 0;
   out_906698228237069812[205] = 0;
   out_906698228237069812[206] = 0;
   out_906698228237069812[207] = 0;
   out_906698228237069812[208] = 0;
   out_906698228237069812[209] = 1;
   out_906698228237069812[210] = 0;
   out_906698228237069812[211] = 0;
   out_906698228237069812[212] = 0;
   out_906698228237069812[213] = 0;
   out_906698228237069812[214] = 0;
   out_906698228237069812[215] = 0;
   out_906698228237069812[216] = 0;
   out_906698228237069812[217] = 0;
   out_906698228237069812[218] = 0;
   out_906698228237069812[219] = 0;
   out_906698228237069812[220] = 0;
   out_906698228237069812[221] = 0;
   out_906698228237069812[222] = 0;
   out_906698228237069812[223] = 0;
   out_906698228237069812[224] = 0;
   out_906698228237069812[225] = 0;
   out_906698228237069812[226] = 0;
   out_906698228237069812[227] = 0;
   out_906698228237069812[228] = 1;
   out_906698228237069812[229] = 0;
   out_906698228237069812[230] = 0;
   out_906698228237069812[231] = 0;
   out_906698228237069812[232] = 0;
   out_906698228237069812[233] = 0;
   out_906698228237069812[234] = 0;
   out_906698228237069812[235] = 0;
   out_906698228237069812[236] = 0;
   out_906698228237069812[237] = 0;
   out_906698228237069812[238] = 0;
   out_906698228237069812[239] = 0;
   out_906698228237069812[240] = 0;
   out_906698228237069812[241] = 0;
   out_906698228237069812[242] = 0;
   out_906698228237069812[243] = 0;
   out_906698228237069812[244] = 0;
   out_906698228237069812[245] = 0;
   out_906698228237069812[246] = 0;
   out_906698228237069812[247] = 1;
   out_906698228237069812[248] = 0;
   out_906698228237069812[249] = 0;
   out_906698228237069812[250] = 0;
   out_906698228237069812[251] = 0;
   out_906698228237069812[252] = 0;
   out_906698228237069812[253] = 0;
   out_906698228237069812[254] = 0;
   out_906698228237069812[255] = 0;
   out_906698228237069812[256] = 0;
   out_906698228237069812[257] = 0;
   out_906698228237069812[258] = 0;
   out_906698228237069812[259] = 0;
   out_906698228237069812[260] = 0;
   out_906698228237069812[261] = 0;
   out_906698228237069812[262] = 0;
   out_906698228237069812[263] = 0;
   out_906698228237069812[264] = 0;
   out_906698228237069812[265] = 0;
   out_906698228237069812[266] = 1;
   out_906698228237069812[267] = 0;
   out_906698228237069812[268] = 0;
   out_906698228237069812[269] = 0;
   out_906698228237069812[270] = 0;
   out_906698228237069812[271] = 0;
   out_906698228237069812[272] = 0;
   out_906698228237069812[273] = 0;
   out_906698228237069812[274] = 0;
   out_906698228237069812[275] = 0;
   out_906698228237069812[276] = 0;
   out_906698228237069812[277] = 0;
   out_906698228237069812[278] = 0;
   out_906698228237069812[279] = 0;
   out_906698228237069812[280] = 0;
   out_906698228237069812[281] = 0;
   out_906698228237069812[282] = 0;
   out_906698228237069812[283] = 0;
   out_906698228237069812[284] = 0;
   out_906698228237069812[285] = 1;
   out_906698228237069812[286] = 0;
   out_906698228237069812[287] = 0;
   out_906698228237069812[288] = 0;
   out_906698228237069812[289] = 0;
   out_906698228237069812[290] = 0;
   out_906698228237069812[291] = 0;
   out_906698228237069812[292] = 0;
   out_906698228237069812[293] = 0;
   out_906698228237069812[294] = 0;
   out_906698228237069812[295] = 0;
   out_906698228237069812[296] = 0;
   out_906698228237069812[297] = 0;
   out_906698228237069812[298] = 0;
   out_906698228237069812[299] = 0;
   out_906698228237069812[300] = 0;
   out_906698228237069812[301] = 0;
   out_906698228237069812[302] = 0;
   out_906698228237069812[303] = 0;
   out_906698228237069812[304] = 1;
   out_906698228237069812[305] = 0;
   out_906698228237069812[306] = 0;
   out_906698228237069812[307] = 0;
   out_906698228237069812[308] = 0;
   out_906698228237069812[309] = 0;
   out_906698228237069812[310] = 0;
   out_906698228237069812[311] = 0;
   out_906698228237069812[312] = 0;
   out_906698228237069812[313] = 0;
   out_906698228237069812[314] = 0;
   out_906698228237069812[315] = 0;
   out_906698228237069812[316] = 0;
   out_906698228237069812[317] = 0;
   out_906698228237069812[318] = 0;
   out_906698228237069812[319] = 0;
   out_906698228237069812[320] = 0;
   out_906698228237069812[321] = 0;
   out_906698228237069812[322] = 0;
   out_906698228237069812[323] = 1;
}
void h_4(double *state, double *unused, double *out_6577940567349030778) {
   out_6577940567349030778[0] = state[6] + state[9];
   out_6577940567349030778[1] = state[7] + state[10];
   out_6577940567349030778[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_913158878120160673) {
   out_913158878120160673[0] = 0;
   out_913158878120160673[1] = 0;
   out_913158878120160673[2] = 0;
   out_913158878120160673[3] = 0;
   out_913158878120160673[4] = 0;
   out_913158878120160673[5] = 0;
   out_913158878120160673[6] = 1;
   out_913158878120160673[7] = 0;
   out_913158878120160673[8] = 0;
   out_913158878120160673[9] = 1;
   out_913158878120160673[10] = 0;
   out_913158878120160673[11] = 0;
   out_913158878120160673[12] = 0;
   out_913158878120160673[13] = 0;
   out_913158878120160673[14] = 0;
   out_913158878120160673[15] = 0;
   out_913158878120160673[16] = 0;
   out_913158878120160673[17] = 0;
   out_913158878120160673[18] = 0;
   out_913158878120160673[19] = 0;
   out_913158878120160673[20] = 0;
   out_913158878120160673[21] = 0;
   out_913158878120160673[22] = 0;
   out_913158878120160673[23] = 0;
   out_913158878120160673[24] = 0;
   out_913158878120160673[25] = 1;
   out_913158878120160673[26] = 0;
   out_913158878120160673[27] = 0;
   out_913158878120160673[28] = 1;
   out_913158878120160673[29] = 0;
   out_913158878120160673[30] = 0;
   out_913158878120160673[31] = 0;
   out_913158878120160673[32] = 0;
   out_913158878120160673[33] = 0;
   out_913158878120160673[34] = 0;
   out_913158878120160673[35] = 0;
   out_913158878120160673[36] = 0;
   out_913158878120160673[37] = 0;
   out_913158878120160673[38] = 0;
   out_913158878120160673[39] = 0;
   out_913158878120160673[40] = 0;
   out_913158878120160673[41] = 0;
   out_913158878120160673[42] = 0;
   out_913158878120160673[43] = 0;
   out_913158878120160673[44] = 1;
   out_913158878120160673[45] = 0;
   out_913158878120160673[46] = 0;
   out_913158878120160673[47] = 1;
   out_913158878120160673[48] = 0;
   out_913158878120160673[49] = 0;
   out_913158878120160673[50] = 0;
   out_913158878120160673[51] = 0;
   out_913158878120160673[52] = 0;
   out_913158878120160673[53] = 0;
}
void h_10(double *state, double *unused, double *out_3373051515203641771) {
   out_3373051515203641771[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_3373051515203641771[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_3373051515203641771[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_7288760212554836769) {
   out_7288760212554836769[0] = 0;
   out_7288760212554836769[1] = 9.8100000000000005*cos(state[1]);
   out_7288760212554836769[2] = 0;
   out_7288760212554836769[3] = 0;
   out_7288760212554836769[4] = -state[8];
   out_7288760212554836769[5] = state[7];
   out_7288760212554836769[6] = 0;
   out_7288760212554836769[7] = state[5];
   out_7288760212554836769[8] = -state[4];
   out_7288760212554836769[9] = 0;
   out_7288760212554836769[10] = 0;
   out_7288760212554836769[11] = 0;
   out_7288760212554836769[12] = 1;
   out_7288760212554836769[13] = 0;
   out_7288760212554836769[14] = 0;
   out_7288760212554836769[15] = 1;
   out_7288760212554836769[16] = 0;
   out_7288760212554836769[17] = 0;
   out_7288760212554836769[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_7288760212554836769[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_7288760212554836769[20] = 0;
   out_7288760212554836769[21] = state[8];
   out_7288760212554836769[22] = 0;
   out_7288760212554836769[23] = -state[6];
   out_7288760212554836769[24] = -state[5];
   out_7288760212554836769[25] = 0;
   out_7288760212554836769[26] = state[3];
   out_7288760212554836769[27] = 0;
   out_7288760212554836769[28] = 0;
   out_7288760212554836769[29] = 0;
   out_7288760212554836769[30] = 0;
   out_7288760212554836769[31] = 1;
   out_7288760212554836769[32] = 0;
   out_7288760212554836769[33] = 0;
   out_7288760212554836769[34] = 1;
   out_7288760212554836769[35] = 0;
   out_7288760212554836769[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_7288760212554836769[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_7288760212554836769[38] = 0;
   out_7288760212554836769[39] = -state[7];
   out_7288760212554836769[40] = state[6];
   out_7288760212554836769[41] = 0;
   out_7288760212554836769[42] = state[4];
   out_7288760212554836769[43] = -state[3];
   out_7288760212554836769[44] = 0;
   out_7288760212554836769[45] = 0;
   out_7288760212554836769[46] = 0;
   out_7288760212554836769[47] = 0;
   out_7288760212554836769[48] = 0;
   out_7288760212554836769[49] = 0;
   out_7288760212554836769[50] = 1;
   out_7288760212554836769[51] = 0;
   out_7288760212554836769[52] = 0;
   out_7288760212554836769[53] = 1;
}
void h_13(double *state, double *unused, double *out_413669578140119338) {
   out_413669578140119338[0] = state[3];
   out_413669578140119338[1] = state[4];
   out_413669578140119338[2] = state[5];
}
void H_13(double *state, double *unused, double *out_8523790086436861602) {
   out_8523790086436861602[0] = 0;
   out_8523790086436861602[1] = 0;
   out_8523790086436861602[2] = 0;
   out_8523790086436861602[3] = 1;
   out_8523790086436861602[4] = 0;
   out_8523790086436861602[5] = 0;
   out_8523790086436861602[6] = 0;
   out_8523790086436861602[7] = 0;
   out_8523790086436861602[8] = 0;
   out_8523790086436861602[9] = 0;
   out_8523790086436861602[10] = 0;
   out_8523790086436861602[11] = 0;
   out_8523790086436861602[12] = 0;
   out_8523790086436861602[13] = 0;
   out_8523790086436861602[14] = 0;
   out_8523790086436861602[15] = 0;
   out_8523790086436861602[16] = 0;
   out_8523790086436861602[17] = 0;
   out_8523790086436861602[18] = 0;
   out_8523790086436861602[19] = 0;
   out_8523790086436861602[20] = 0;
   out_8523790086436861602[21] = 0;
   out_8523790086436861602[22] = 1;
   out_8523790086436861602[23] = 0;
   out_8523790086436861602[24] = 0;
   out_8523790086436861602[25] = 0;
   out_8523790086436861602[26] = 0;
   out_8523790086436861602[27] = 0;
   out_8523790086436861602[28] = 0;
   out_8523790086436861602[29] = 0;
   out_8523790086436861602[30] = 0;
   out_8523790086436861602[31] = 0;
   out_8523790086436861602[32] = 0;
   out_8523790086436861602[33] = 0;
   out_8523790086436861602[34] = 0;
   out_8523790086436861602[35] = 0;
   out_8523790086436861602[36] = 0;
   out_8523790086436861602[37] = 0;
   out_8523790086436861602[38] = 0;
   out_8523790086436861602[39] = 0;
   out_8523790086436861602[40] = 0;
   out_8523790086436861602[41] = 1;
   out_8523790086436861602[42] = 0;
   out_8523790086436861602[43] = 0;
   out_8523790086436861602[44] = 0;
   out_8523790086436861602[45] = 0;
   out_8523790086436861602[46] = 0;
   out_8523790086436861602[47] = 0;
   out_8523790086436861602[48] = 0;
   out_8523790086436861602[49] = 0;
   out_8523790086436861602[50] = 0;
   out_8523790086436861602[51] = 0;
   out_8523790086436861602[52] = 0;
   out_8523790086436861602[53] = 0;
}
void h_14(double *state, double *unused, double *out_8770101800505378255) {
   out_8770101800505378255[0] = state[6];
   out_8770101800505378255[1] = state[7];
   out_8770101800505378255[2] = state[8];
}
void H_14(double *state, double *unused, double *out_4876399734459645202) {
   out_4876399734459645202[0] = 0;
   out_4876399734459645202[1] = 0;
   out_4876399734459645202[2] = 0;
   out_4876399734459645202[3] = 0;
   out_4876399734459645202[4] = 0;
   out_4876399734459645202[5] = 0;
   out_4876399734459645202[6] = 1;
   out_4876399734459645202[7] = 0;
   out_4876399734459645202[8] = 0;
   out_4876399734459645202[9] = 0;
   out_4876399734459645202[10] = 0;
   out_4876399734459645202[11] = 0;
   out_4876399734459645202[12] = 0;
   out_4876399734459645202[13] = 0;
   out_4876399734459645202[14] = 0;
   out_4876399734459645202[15] = 0;
   out_4876399734459645202[16] = 0;
   out_4876399734459645202[17] = 0;
   out_4876399734459645202[18] = 0;
   out_4876399734459645202[19] = 0;
   out_4876399734459645202[20] = 0;
   out_4876399734459645202[21] = 0;
   out_4876399734459645202[22] = 0;
   out_4876399734459645202[23] = 0;
   out_4876399734459645202[24] = 0;
   out_4876399734459645202[25] = 1;
   out_4876399734459645202[26] = 0;
   out_4876399734459645202[27] = 0;
   out_4876399734459645202[28] = 0;
   out_4876399734459645202[29] = 0;
   out_4876399734459645202[30] = 0;
   out_4876399734459645202[31] = 0;
   out_4876399734459645202[32] = 0;
   out_4876399734459645202[33] = 0;
   out_4876399734459645202[34] = 0;
   out_4876399734459645202[35] = 0;
   out_4876399734459645202[36] = 0;
   out_4876399734459645202[37] = 0;
   out_4876399734459645202[38] = 0;
   out_4876399734459645202[39] = 0;
   out_4876399734459645202[40] = 0;
   out_4876399734459645202[41] = 0;
   out_4876399734459645202[42] = 0;
   out_4876399734459645202[43] = 0;
   out_4876399734459645202[44] = 1;
   out_4876399734459645202[45] = 0;
   out_4876399734459645202[46] = 0;
   out_4876399734459645202[47] = 0;
   out_4876399734459645202[48] = 0;
   out_4876399734459645202[49] = 0;
   out_4876399734459645202[50] = 0;
   out_4876399734459645202[51] = 0;
   out_4876399734459645202[52] = 0;
   out_4876399734459645202[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_5188089899736494345) {
  err_fun(nom_x, delta_x, out_5188089899736494345);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_516319331577825853) {
  inv_err_fun(nom_x, true_x, out_516319331577825853);
}
void pose_H_mod_fun(double *state, double *out_4951567132479851062) {
  H_mod_fun(state, out_4951567132479851062);
}
void pose_f_fun(double *state, double dt, double *out_898454825503606476) {
  f_fun(state,  dt, out_898454825503606476);
}
void pose_F_fun(double *state, double dt, double *out_906698228237069812) {
  F_fun(state,  dt, out_906698228237069812);
}
void pose_h_4(double *state, double *unused, double *out_6577940567349030778) {
  h_4(state, unused, out_6577940567349030778);
}
void pose_H_4(double *state, double *unused, double *out_913158878120160673) {
  H_4(state, unused, out_913158878120160673);
}
void pose_h_10(double *state, double *unused, double *out_3373051515203641771) {
  h_10(state, unused, out_3373051515203641771);
}
void pose_H_10(double *state, double *unused, double *out_7288760212554836769) {
  H_10(state, unused, out_7288760212554836769);
}
void pose_h_13(double *state, double *unused, double *out_413669578140119338) {
  h_13(state, unused, out_413669578140119338);
}
void pose_H_13(double *state, double *unused, double *out_8523790086436861602) {
  H_13(state, unused, out_8523790086436861602);
}
void pose_h_14(double *state, double *unused, double *out_8770101800505378255) {
  h_14(state, unused, out_8770101800505378255);
}
void pose_H_14(double *state, double *unused, double *out_4876399734459645202) {
  H_14(state, unused, out_4876399734459645202);
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

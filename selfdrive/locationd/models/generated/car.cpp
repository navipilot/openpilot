#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_1461273597009064329) {
   out_1461273597009064329[0] = delta_x[0] + nom_x[0];
   out_1461273597009064329[1] = delta_x[1] + nom_x[1];
   out_1461273597009064329[2] = delta_x[2] + nom_x[2];
   out_1461273597009064329[3] = delta_x[3] + nom_x[3];
   out_1461273597009064329[4] = delta_x[4] + nom_x[4];
   out_1461273597009064329[5] = delta_x[5] + nom_x[5];
   out_1461273597009064329[6] = delta_x[6] + nom_x[6];
   out_1461273597009064329[7] = delta_x[7] + nom_x[7];
   out_1461273597009064329[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_7289765730580897738) {
   out_7289765730580897738[0] = -nom_x[0] + true_x[0];
   out_7289765730580897738[1] = -nom_x[1] + true_x[1];
   out_7289765730580897738[2] = -nom_x[2] + true_x[2];
   out_7289765730580897738[3] = -nom_x[3] + true_x[3];
   out_7289765730580897738[4] = -nom_x[4] + true_x[4];
   out_7289765730580897738[5] = -nom_x[5] + true_x[5];
   out_7289765730580897738[6] = -nom_x[6] + true_x[6];
   out_7289765730580897738[7] = -nom_x[7] + true_x[7];
   out_7289765730580897738[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8304527178040913025) {
   out_8304527178040913025[0] = 1.0;
   out_8304527178040913025[1] = 0.0;
   out_8304527178040913025[2] = 0.0;
   out_8304527178040913025[3] = 0.0;
   out_8304527178040913025[4] = 0.0;
   out_8304527178040913025[5] = 0.0;
   out_8304527178040913025[6] = 0.0;
   out_8304527178040913025[7] = 0.0;
   out_8304527178040913025[8] = 0.0;
   out_8304527178040913025[9] = 0.0;
   out_8304527178040913025[10] = 1.0;
   out_8304527178040913025[11] = 0.0;
   out_8304527178040913025[12] = 0.0;
   out_8304527178040913025[13] = 0.0;
   out_8304527178040913025[14] = 0.0;
   out_8304527178040913025[15] = 0.0;
   out_8304527178040913025[16] = 0.0;
   out_8304527178040913025[17] = 0.0;
   out_8304527178040913025[18] = 0.0;
   out_8304527178040913025[19] = 0.0;
   out_8304527178040913025[20] = 1.0;
   out_8304527178040913025[21] = 0.0;
   out_8304527178040913025[22] = 0.0;
   out_8304527178040913025[23] = 0.0;
   out_8304527178040913025[24] = 0.0;
   out_8304527178040913025[25] = 0.0;
   out_8304527178040913025[26] = 0.0;
   out_8304527178040913025[27] = 0.0;
   out_8304527178040913025[28] = 0.0;
   out_8304527178040913025[29] = 0.0;
   out_8304527178040913025[30] = 1.0;
   out_8304527178040913025[31] = 0.0;
   out_8304527178040913025[32] = 0.0;
   out_8304527178040913025[33] = 0.0;
   out_8304527178040913025[34] = 0.0;
   out_8304527178040913025[35] = 0.0;
   out_8304527178040913025[36] = 0.0;
   out_8304527178040913025[37] = 0.0;
   out_8304527178040913025[38] = 0.0;
   out_8304527178040913025[39] = 0.0;
   out_8304527178040913025[40] = 1.0;
   out_8304527178040913025[41] = 0.0;
   out_8304527178040913025[42] = 0.0;
   out_8304527178040913025[43] = 0.0;
   out_8304527178040913025[44] = 0.0;
   out_8304527178040913025[45] = 0.0;
   out_8304527178040913025[46] = 0.0;
   out_8304527178040913025[47] = 0.0;
   out_8304527178040913025[48] = 0.0;
   out_8304527178040913025[49] = 0.0;
   out_8304527178040913025[50] = 1.0;
   out_8304527178040913025[51] = 0.0;
   out_8304527178040913025[52] = 0.0;
   out_8304527178040913025[53] = 0.0;
   out_8304527178040913025[54] = 0.0;
   out_8304527178040913025[55] = 0.0;
   out_8304527178040913025[56] = 0.0;
   out_8304527178040913025[57] = 0.0;
   out_8304527178040913025[58] = 0.0;
   out_8304527178040913025[59] = 0.0;
   out_8304527178040913025[60] = 1.0;
   out_8304527178040913025[61] = 0.0;
   out_8304527178040913025[62] = 0.0;
   out_8304527178040913025[63] = 0.0;
   out_8304527178040913025[64] = 0.0;
   out_8304527178040913025[65] = 0.0;
   out_8304527178040913025[66] = 0.0;
   out_8304527178040913025[67] = 0.0;
   out_8304527178040913025[68] = 0.0;
   out_8304527178040913025[69] = 0.0;
   out_8304527178040913025[70] = 1.0;
   out_8304527178040913025[71] = 0.0;
   out_8304527178040913025[72] = 0.0;
   out_8304527178040913025[73] = 0.0;
   out_8304527178040913025[74] = 0.0;
   out_8304527178040913025[75] = 0.0;
   out_8304527178040913025[76] = 0.0;
   out_8304527178040913025[77] = 0.0;
   out_8304527178040913025[78] = 0.0;
   out_8304527178040913025[79] = 0.0;
   out_8304527178040913025[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_1355173870151422192) {
   out_1355173870151422192[0] = state[0];
   out_1355173870151422192[1] = state[1];
   out_1355173870151422192[2] = state[2];
   out_1355173870151422192[3] = state[3];
   out_1355173870151422192[4] = state[4];
   out_1355173870151422192[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_1355173870151422192[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_1355173870151422192[7] = state[7];
   out_1355173870151422192[8] = state[8];
}
void F_fun(double *state, double dt, double *out_3219998669912464347) {
   out_3219998669912464347[0] = 1;
   out_3219998669912464347[1] = 0;
   out_3219998669912464347[2] = 0;
   out_3219998669912464347[3] = 0;
   out_3219998669912464347[4] = 0;
   out_3219998669912464347[5] = 0;
   out_3219998669912464347[6] = 0;
   out_3219998669912464347[7] = 0;
   out_3219998669912464347[8] = 0;
   out_3219998669912464347[9] = 0;
   out_3219998669912464347[10] = 1;
   out_3219998669912464347[11] = 0;
   out_3219998669912464347[12] = 0;
   out_3219998669912464347[13] = 0;
   out_3219998669912464347[14] = 0;
   out_3219998669912464347[15] = 0;
   out_3219998669912464347[16] = 0;
   out_3219998669912464347[17] = 0;
   out_3219998669912464347[18] = 0;
   out_3219998669912464347[19] = 0;
   out_3219998669912464347[20] = 1;
   out_3219998669912464347[21] = 0;
   out_3219998669912464347[22] = 0;
   out_3219998669912464347[23] = 0;
   out_3219998669912464347[24] = 0;
   out_3219998669912464347[25] = 0;
   out_3219998669912464347[26] = 0;
   out_3219998669912464347[27] = 0;
   out_3219998669912464347[28] = 0;
   out_3219998669912464347[29] = 0;
   out_3219998669912464347[30] = 1;
   out_3219998669912464347[31] = 0;
   out_3219998669912464347[32] = 0;
   out_3219998669912464347[33] = 0;
   out_3219998669912464347[34] = 0;
   out_3219998669912464347[35] = 0;
   out_3219998669912464347[36] = 0;
   out_3219998669912464347[37] = 0;
   out_3219998669912464347[38] = 0;
   out_3219998669912464347[39] = 0;
   out_3219998669912464347[40] = 1;
   out_3219998669912464347[41] = 0;
   out_3219998669912464347[42] = 0;
   out_3219998669912464347[43] = 0;
   out_3219998669912464347[44] = 0;
   out_3219998669912464347[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_3219998669912464347[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_3219998669912464347[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3219998669912464347[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3219998669912464347[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_3219998669912464347[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_3219998669912464347[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_3219998669912464347[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_3219998669912464347[53] = -9.8100000000000005*dt;
   out_3219998669912464347[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_3219998669912464347[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_3219998669912464347[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3219998669912464347[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3219998669912464347[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_3219998669912464347[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_3219998669912464347[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_3219998669912464347[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3219998669912464347[62] = 0;
   out_3219998669912464347[63] = 0;
   out_3219998669912464347[64] = 0;
   out_3219998669912464347[65] = 0;
   out_3219998669912464347[66] = 0;
   out_3219998669912464347[67] = 0;
   out_3219998669912464347[68] = 0;
   out_3219998669912464347[69] = 0;
   out_3219998669912464347[70] = 1;
   out_3219998669912464347[71] = 0;
   out_3219998669912464347[72] = 0;
   out_3219998669912464347[73] = 0;
   out_3219998669912464347[74] = 0;
   out_3219998669912464347[75] = 0;
   out_3219998669912464347[76] = 0;
   out_3219998669912464347[77] = 0;
   out_3219998669912464347[78] = 0;
   out_3219998669912464347[79] = 0;
   out_3219998669912464347[80] = 1;
}
void h_25(double *state, double *unused, double *out_7686598041594365016) {
   out_7686598041594365016[0] = state[6];
}
void H_25(double *state, double *unused, double *out_2220878862694778786) {
   out_2220878862694778786[0] = 0;
   out_2220878862694778786[1] = 0;
   out_2220878862694778786[2] = 0;
   out_2220878862694778786[3] = 0;
   out_2220878862694778786[4] = 0;
   out_2220878862694778786[5] = 0;
   out_2220878862694778786[6] = 1;
   out_2220878862694778786[7] = 0;
   out_2220878862694778786[8] = 0;
}
void h_24(double *state, double *unused, double *out_3813839643785118531) {
   out_3813839643785118531[0] = state[4];
   out_3813839643785118531[1] = state[5];
}
void H_24(double *state, double *unused, double *out_1337046736107544350) {
   out_1337046736107544350[0] = 0;
   out_1337046736107544350[1] = 0;
   out_1337046736107544350[2] = 0;
   out_1337046736107544350[3] = 0;
   out_1337046736107544350[4] = 1;
   out_1337046736107544350[5] = 0;
   out_1337046736107544350[6] = 0;
   out_1337046736107544350[7] = 0;
   out_1337046736107544350[8] = 0;
   out_1337046736107544350[9] = 0;
   out_1337046736107544350[10] = 0;
   out_1337046736107544350[11] = 0;
   out_1337046736107544350[12] = 0;
   out_1337046736107544350[13] = 0;
   out_1337046736107544350[14] = 1;
   out_1337046736107544350[15] = 0;
   out_1337046736107544350[16] = 0;
   out_1337046736107544350[17] = 0;
}
void h_30(double *state, double *unused, double *out_2120268555396644716) {
   out_2120268555396644716[0] = state[4];
}
void H_30(double *state, double *unused, double *out_297454095812469841) {
   out_297454095812469841[0] = 0;
   out_297454095812469841[1] = 0;
   out_297454095812469841[2] = 0;
   out_297454095812469841[3] = 0;
   out_297454095812469841[4] = 1;
   out_297454095812469841[5] = 0;
   out_297454095812469841[6] = 0;
   out_297454095812469841[7] = 0;
   out_297454095812469841[8] = 0;
}
void h_26(double *state, double *unused, double *out_2881404712264200929) {
   out_2881404712264200929[0] = state[7];
}
void H_26(double *state, double *unused, double *out_5962382181568835010) {
   out_5962382181568835010[0] = 0;
   out_5962382181568835010[1] = 0;
   out_5962382181568835010[2] = 0;
   out_5962382181568835010[3] = 0;
   out_5962382181568835010[4] = 0;
   out_5962382181568835010[5] = 0;
   out_5962382181568835010[6] = 0;
   out_5962382181568835010[7] = 1;
   out_5962382181568835010[8] = 0;
}
void h_27(double *state, double *unused, double *out_901564640210476392) {
   out_901564640210476392[0] = state[3];
}
void H_27(double *state, double *unused, double *out_2521048166996413058) {
   out_2521048166996413058[0] = 0;
   out_2521048166996413058[1] = 0;
   out_2521048166996413058[2] = 0;
   out_2521048166996413058[3] = 1;
   out_2521048166996413058[4] = 0;
   out_2521048166996413058[5] = 0;
   out_2521048166996413058[6] = 0;
   out_2521048166996413058[7] = 0;
   out_2521048166996413058[8] = 0;
}
void h_29(double *state, double *unused, double *out_952524754320502979) {
   out_952524754320502979[0] = state[1];
}
void H_29(double *state, double *unused, double *out_807685440126862025) {
   out_807685440126862025[0] = 0;
   out_807685440126862025[1] = 1;
   out_807685440126862025[2] = 0;
   out_807685440126862025[3] = 0;
   out_807685440126862025[4] = 0;
   out_807685440126862025[5] = 0;
   out_807685440126862025[6] = 0;
   out_807685440126862025[7] = 0;
   out_807685440126862025[8] = 0;
}
void h_28(double *state, double *unused, double *out_8550909813521810734) {
   out_8550909813521810734[0] = state[0];
}
void H_28(double *state, double *unused, double *out_4274713576942668549) {
   out_4274713576942668549[0] = 1;
   out_4274713576942668549[1] = 0;
   out_4274713576942668549[2] = 0;
   out_4274713576942668549[3] = 0;
   out_4274713576942668549[4] = 0;
   out_4274713576942668549[5] = 0;
   out_4274713576942668549[6] = 0;
   out_4274713576942668549[7] = 0;
   out_4274713576942668549[8] = 0;
}
void h_31(double *state, double *unused, double *out_7961792103878870905) {
   out_7961792103878870905[0] = state[8];
}
void H_31(double *state, double *unused, double *out_6588590283802186486) {
   out_6588590283802186486[0] = 0;
   out_6588590283802186486[1] = 0;
   out_6588590283802186486[2] = 0;
   out_6588590283802186486[3] = 0;
   out_6588590283802186486[4] = 0;
   out_6588590283802186486[5] = 0;
   out_6588590283802186486[6] = 0;
   out_6588590283802186486[7] = 0;
   out_6588590283802186486[8] = 1;
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

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_1461273597009064329) {
  err_fun(nom_x, delta_x, out_1461273597009064329);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7289765730580897738) {
  inv_err_fun(nom_x, true_x, out_7289765730580897738);
}
void car_H_mod_fun(double *state, double *out_8304527178040913025) {
  H_mod_fun(state, out_8304527178040913025);
}
void car_f_fun(double *state, double dt, double *out_1355173870151422192) {
  f_fun(state,  dt, out_1355173870151422192);
}
void car_F_fun(double *state, double dt, double *out_3219998669912464347) {
  F_fun(state,  dt, out_3219998669912464347);
}
void car_h_25(double *state, double *unused, double *out_7686598041594365016) {
  h_25(state, unused, out_7686598041594365016);
}
void car_H_25(double *state, double *unused, double *out_2220878862694778786) {
  H_25(state, unused, out_2220878862694778786);
}
void car_h_24(double *state, double *unused, double *out_3813839643785118531) {
  h_24(state, unused, out_3813839643785118531);
}
void car_H_24(double *state, double *unused, double *out_1337046736107544350) {
  H_24(state, unused, out_1337046736107544350);
}
void car_h_30(double *state, double *unused, double *out_2120268555396644716) {
  h_30(state, unused, out_2120268555396644716);
}
void car_H_30(double *state, double *unused, double *out_297454095812469841) {
  H_30(state, unused, out_297454095812469841);
}
void car_h_26(double *state, double *unused, double *out_2881404712264200929) {
  h_26(state, unused, out_2881404712264200929);
}
void car_H_26(double *state, double *unused, double *out_5962382181568835010) {
  H_26(state, unused, out_5962382181568835010);
}
void car_h_27(double *state, double *unused, double *out_901564640210476392) {
  h_27(state, unused, out_901564640210476392);
}
void car_H_27(double *state, double *unused, double *out_2521048166996413058) {
  H_27(state, unused, out_2521048166996413058);
}
void car_h_29(double *state, double *unused, double *out_952524754320502979) {
  h_29(state, unused, out_952524754320502979);
}
void car_H_29(double *state, double *unused, double *out_807685440126862025) {
  H_29(state, unused, out_807685440126862025);
}
void car_h_28(double *state, double *unused, double *out_8550909813521810734) {
  h_28(state, unused, out_8550909813521810734);
}
void car_H_28(double *state, double *unused, double *out_4274713576942668549) {
  H_28(state, unused, out_4274713576942668549);
}
void car_h_31(double *state, double *unused, double *out_7961792103878870905) {
  h_31(state, unused, out_7961792103878870905);
}
void car_H_31(double *state, double *unused, double *out_6588590283802186486) {
  H_31(state, unused, out_6588590283802186486);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)

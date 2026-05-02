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
void err_fun(double *nom_x, double *delta_x, double *out_2165520225426293203) {
   out_2165520225426293203[0] = delta_x[0] + nom_x[0];
   out_2165520225426293203[1] = delta_x[1] + nom_x[1];
   out_2165520225426293203[2] = delta_x[2] + nom_x[2];
   out_2165520225426293203[3] = delta_x[3] + nom_x[3];
   out_2165520225426293203[4] = delta_x[4] + nom_x[4];
   out_2165520225426293203[5] = delta_x[5] + nom_x[5];
   out_2165520225426293203[6] = delta_x[6] + nom_x[6];
   out_2165520225426293203[7] = delta_x[7] + nom_x[7];
   out_2165520225426293203[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_48394968560250279) {
   out_48394968560250279[0] = -nom_x[0] + true_x[0];
   out_48394968560250279[1] = -nom_x[1] + true_x[1];
   out_48394968560250279[2] = -nom_x[2] + true_x[2];
   out_48394968560250279[3] = -nom_x[3] + true_x[3];
   out_48394968560250279[4] = -nom_x[4] + true_x[4];
   out_48394968560250279[5] = -nom_x[5] + true_x[5];
   out_48394968560250279[6] = -nom_x[6] + true_x[6];
   out_48394968560250279[7] = -nom_x[7] + true_x[7];
   out_48394968560250279[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8198135828365669542) {
   out_8198135828365669542[0] = 1.0;
   out_8198135828365669542[1] = 0.0;
   out_8198135828365669542[2] = 0.0;
   out_8198135828365669542[3] = 0.0;
   out_8198135828365669542[4] = 0.0;
   out_8198135828365669542[5] = 0.0;
   out_8198135828365669542[6] = 0.0;
   out_8198135828365669542[7] = 0.0;
   out_8198135828365669542[8] = 0.0;
   out_8198135828365669542[9] = 0.0;
   out_8198135828365669542[10] = 1.0;
   out_8198135828365669542[11] = 0.0;
   out_8198135828365669542[12] = 0.0;
   out_8198135828365669542[13] = 0.0;
   out_8198135828365669542[14] = 0.0;
   out_8198135828365669542[15] = 0.0;
   out_8198135828365669542[16] = 0.0;
   out_8198135828365669542[17] = 0.0;
   out_8198135828365669542[18] = 0.0;
   out_8198135828365669542[19] = 0.0;
   out_8198135828365669542[20] = 1.0;
   out_8198135828365669542[21] = 0.0;
   out_8198135828365669542[22] = 0.0;
   out_8198135828365669542[23] = 0.0;
   out_8198135828365669542[24] = 0.0;
   out_8198135828365669542[25] = 0.0;
   out_8198135828365669542[26] = 0.0;
   out_8198135828365669542[27] = 0.0;
   out_8198135828365669542[28] = 0.0;
   out_8198135828365669542[29] = 0.0;
   out_8198135828365669542[30] = 1.0;
   out_8198135828365669542[31] = 0.0;
   out_8198135828365669542[32] = 0.0;
   out_8198135828365669542[33] = 0.0;
   out_8198135828365669542[34] = 0.0;
   out_8198135828365669542[35] = 0.0;
   out_8198135828365669542[36] = 0.0;
   out_8198135828365669542[37] = 0.0;
   out_8198135828365669542[38] = 0.0;
   out_8198135828365669542[39] = 0.0;
   out_8198135828365669542[40] = 1.0;
   out_8198135828365669542[41] = 0.0;
   out_8198135828365669542[42] = 0.0;
   out_8198135828365669542[43] = 0.0;
   out_8198135828365669542[44] = 0.0;
   out_8198135828365669542[45] = 0.0;
   out_8198135828365669542[46] = 0.0;
   out_8198135828365669542[47] = 0.0;
   out_8198135828365669542[48] = 0.0;
   out_8198135828365669542[49] = 0.0;
   out_8198135828365669542[50] = 1.0;
   out_8198135828365669542[51] = 0.0;
   out_8198135828365669542[52] = 0.0;
   out_8198135828365669542[53] = 0.0;
   out_8198135828365669542[54] = 0.0;
   out_8198135828365669542[55] = 0.0;
   out_8198135828365669542[56] = 0.0;
   out_8198135828365669542[57] = 0.0;
   out_8198135828365669542[58] = 0.0;
   out_8198135828365669542[59] = 0.0;
   out_8198135828365669542[60] = 1.0;
   out_8198135828365669542[61] = 0.0;
   out_8198135828365669542[62] = 0.0;
   out_8198135828365669542[63] = 0.0;
   out_8198135828365669542[64] = 0.0;
   out_8198135828365669542[65] = 0.0;
   out_8198135828365669542[66] = 0.0;
   out_8198135828365669542[67] = 0.0;
   out_8198135828365669542[68] = 0.0;
   out_8198135828365669542[69] = 0.0;
   out_8198135828365669542[70] = 1.0;
   out_8198135828365669542[71] = 0.0;
   out_8198135828365669542[72] = 0.0;
   out_8198135828365669542[73] = 0.0;
   out_8198135828365669542[74] = 0.0;
   out_8198135828365669542[75] = 0.0;
   out_8198135828365669542[76] = 0.0;
   out_8198135828365669542[77] = 0.0;
   out_8198135828365669542[78] = 0.0;
   out_8198135828365669542[79] = 0.0;
   out_8198135828365669542[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_5413666688161130858) {
   out_5413666688161130858[0] = state[0];
   out_5413666688161130858[1] = state[1];
   out_5413666688161130858[2] = state[2];
   out_5413666688161130858[3] = state[3];
   out_5413666688161130858[4] = state[4];
   out_5413666688161130858[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_5413666688161130858[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_5413666688161130858[7] = state[7];
   out_5413666688161130858[8] = state[8];
}
void F_fun(double *state, double dt, double *out_7314745635201977822) {
   out_7314745635201977822[0] = 1;
   out_7314745635201977822[1] = 0;
   out_7314745635201977822[2] = 0;
   out_7314745635201977822[3] = 0;
   out_7314745635201977822[4] = 0;
   out_7314745635201977822[5] = 0;
   out_7314745635201977822[6] = 0;
   out_7314745635201977822[7] = 0;
   out_7314745635201977822[8] = 0;
   out_7314745635201977822[9] = 0;
   out_7314745635201977822[10] = 1;
   out_7314745635201977822[11] = 0;
   out_7314745635201977822[12] = 0;
   out_7314745635201977822[13] = 0;
   out_7314745635201977822[14] = 0;
   out_7314745635201977822[15] = 0;
   out_7314745635201977822[16] = 0;
   out_7314745635201977822[17] = 0;
   out_7314745635201977822[18] = 0;
   out_7314745635201977822[19] = 0;
   out_7314745635201977822[20] = 1;
   out_7314745635201977822[21] = 0;
   out_7314745635201977822[22] = 0;
   out_7314745635201977822[23] = 0;
   out_7314745635201977822[24] = 0;
   out_7314745635201977822[25] = 0;
   out_7314745635201977822[26] = 0;
   out_7314745635201977822[27] = 0;
   out_7314745635201977822[28] = 0;
   out_7314745635201977822[29] = 0;
   out_7314745635201977822[30] = 1;
   out_7314745635201977822[31] = 0;
   out_7314745635201977822[32] = 0;
   out_7314745635201977822[33] = 0;
   out_7314745635201977822[34] = 0;
   out_7314745635201977822[35] = 0;
   out_7314745635201977822[36] = 0;
   out_7314745635201977822[37] = 0;
   out_7314745635201977822[38] = 0;
   out_7314745635201977822[39] = 0;
   out_7314745635201977822[40] = 1;
   out_7314745635201977822[41] = 0;
   out_7314745635201977822[42] = 0;
   out_7314745635201977822[43] = 0;
   out_7314745635201977822[44] = 0;
   out_7314745635201977822[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_7314745635201977822[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_7314745635201977822[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7314745635201977822[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7314745635201977822[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_7314745635201977822[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_7314745635201977822[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_7314745635201977822[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_7314745635201977822[53] = -9.8100000000000005*dt;
   out_7314745635201977822[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_7314745635201977822[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_7314745635201977822[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7314745635201977822[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7314745635201977822[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_7314745635201977822[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_7314745635201977822[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_7314745635201977822[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7314745635201977822[62] = 0;
   out_7314745635201977822[63] = 0;
   out_7314745635201977822[64] = 0;
   out_7314745635201977822[65] = 0;
   out_7314745635201977822[66] = 0;
   out_7314745635201977822[67] = 0;
   out_7314745635201977822[68] = 0;
   out_7314745635201977822[69] = 0;
   out_7314745635201977822[70] = 1;
   out_7314745635201977822[71] = 0;
   out_7314745635201977822[72] = 0;
   out_7314745635201977822[73] = 0;
   out_7314745635201977822[74] = 0;
   out_7314745635201977822[75] = 0;
   out_7314745635201977822[76] = 0;
   out_7314745635201977822[77] = 0;
   out_7314745635201977822[78] = 0;
   out_7314745635201977822[79] = 0;
   out_7314745635201977822[80] = 1;
}
void h_25(double *state, double *unused, double *out_1723156805424676224) {
   out_1723156805424676224[0] = state[6];
}
void H_25(double *state, double *unused, double *out_6725559606698516926) {
   out_6725559606698516926[0] = 0;
   out_6725559606698516926[1] = 0;
   out_6725559606698516926[2] = 0;
   out_6725559606698516926[3] = 0;
   out_6725559606698516926[4] = 0;
   out_6725559606698516926[5] = 0;
   out_6725559606698516926[6] = 1;
   out_6725559606698516926[7] = 0;
   out_6725559606698516926[8] = 0;
}
void h_24(double *state, double *unused, double *out_6301088908339500348) {
   out_6301088908339500348[0] = state[4];
   out_6301088908339500348[1] = state[5];
}
void H_24(double *state, double *unused, double *out_8163258868208711554) {
   out_8163258868208711554[0] = 0;
   out_8163258868208711554[1] = 0;
   out_8163258868208711554[2] = 0;
   out_8163258868208711554[3] = 0;
   out_8163258868208711554[4] = 1;
   out_8163258868208711554[5] = 0;
   out_8163258868208711554[6] = 0;
   out_8163258868208711554[7] = 0;
   out_8163258868208711554[8] = 0;
   out_8163258868208711554[9] = 0;
   out_8163258868208711554[10] = 0;
   out_8163258868208711554[11] = 0;
   out_8163258868208711554[12] = 0;
   out_8163258868208711554[13] = 0;
   out_8163258868208711554[14] = 1;
   out_8163258868208711554[15] = 0;
   out_8163258868208711554[16] = 0;
   out_8163258868208711554[17] = 0;
}
void h_30(double *state, double *unused, double *out_5821460103704048088) {
   out_5821460103704048088[0] = state[4];
}
void H_30(double *state, double *unused, double *out_4804494125519417935) {
   out_4804494125519417935[0] = 0;
   out_4804494125519417935[1] = 0;
   out_4804494125519417935[2] = 0;
   out_4804494125519417935[3] = 0;
   out_4804494125519417935[4] = 1;
   out_4804494125519417935[5] = 0;
   out_4804494125519417935[6] = 0;
   out_4804494125519417935[7] = 0;
   out_4804494125519417935[8] = 0;
}
void h_26(double *state, double *unused, double *out_3604740557565027086) {
   out_3604740557565027086[0] = state[7];
}
void H_26(double *state, double *unused, double *out_8416658497250234089) {
   out_8416658497250234089[0] = 0;
   out_8416658497250234089[1] = 0;
   out_8416658497250234089[2] = 0;
   out_8416658497250234089[3] = 0;
   out_8416658497250234089[4] = 0;
   out_8416658497250234089[5] = 0;
   out_8416658497250234089[6] = 0;
   out_8416658497250234089[7] = 1;
   out_8416658497250234089[8] = 0;
}
void h_27(double *state, double *unused, double *out_4759942856869008909) {
   out_4759942856869008909[0] = state[3];
}
void H_27(double *state, double *unused, double *out_6979257437319842846) {
   out_6979257437319842846[0] = 0;
   out_6979257437319842846[1] = 0;
   out_6979257437319842846[2] = 0;
   out_6979257437319842846[3] = 1;
   out_6979257437319842846[4] = 0;
   out_6979257437319842846[5] = 0;
   out_6979257437319842846[6] = 0;
   out_6979257437319842846[7] = 0;
   out_6979257437319842846[8] = 0;
}
void h_29(double *state, double *unused, double *out_5218144219027570571) {
   out_5218144219027570571[0] = state[1];
}
void H_29(double *state, double *unused, double *out_4294262781205025751) {
   out_4294262781205025751[0] = 0;
   out_4294262781205025751[1] = 1;
   out_4294262781205025751[2] = 0;
   out_4294262781205025751[3] = 0;
   out_4294262781205025751[4] = 0;
   out_4294262781205025751[5] = 0;
   out_4294262781205025751[6] = 0;
   out_4294262781205025751[7] = 0;
   out_4294262781205025751[8] = 0;
}
void h_28(double *state, double *unused, double *out_5703452709770671344) {
   out_5703452709770671344[0] = state[0];
}
void H_28(double *state, double *unused, double *out_4671724892450627163) {
   out_4671724892450627163[0] = 1;
   out_4671724892450627163[1] = 0;
   out_4671724892450627163[2] = 0;
   out_4671724892450627163[3] = 0;
   out_4671724892450627163[4] = 0;
   out_4671724892450627163[5] = 0;
   out_4671724892450627163[6] = 0;
   out_4671724892450627163[7] = 0;
   out_4671724892450627163[8] = 0;
}
void h_31(double *state, double *unused, double *out_1998350867709182113) {
   out_1998350867709182113[0] = state[8];
}
void H_31(double *state, double *unused, double *out_4644509216499217437) {
   out_4644509216499217437[0] = 0;
   out_4644509216499217437[1] = 0;
   out_4644509216499217437[2] = 0;
   out_4644509216499217437[3] = 0;
   out_4644509216499217437[4] = 0;
   out_4644509216499217437[5] = 0;
   out_4644509216499217437[6] = 0;
   out_4644509216499217437[7] = 0;
   out_4644509216499217437[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_2165520225426293203) {
  err_fun(nom_x, delta_x, out_2165520225426293203);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_48394968560250279) {
  inv_err_fun(nom_x, true_x, out_48394968560250279);
}
void car_H_mod_fun(double *state, double *out_8198135828365669542) {
  H_mod_fun(state, out_8198135828365669542);
}
void car_f_fun(double *state, double dt, double *out_5413666688161130858) {
  f_fun(state,  dt, out_5413666688161130858);
}
void car_F_fun(double *state, double dt, double *out_7314745635201977822) {
  F_fun(state,  dt, out_7314745635201977822);
}
void car_h_25(double *state, double *unused, double *out_1723156805424676224) {
  h_25(state, unused, out_1723156805424676224);
}
void car_H_25(double *state, double *unused, double *out_6725559606698516926) {
  H_25(state, unused, out_6725559606698516926);
}
void car_h_24(double *state, double *unused, double *out_6301088908339500348) {
  h_24(state, unused, out_6301088908339500348);
}
void car_H_24(double *state, double *unused, double *out_8163258868208711554) {
  H_24(state, unused, out_8163258868208711554);
}
void car_h_30(double *state, double *unused, double *out_5821460103704048088) {
  h_30(state, unused, out_5821460103704048088);
}
void car_H_30(double *state, double *unused, double *out_4804494125519417935) {
  H_30(state, unused, out_4804494125519417935);
}
void car_h_26(double *state, double *unused, double *out_3604740557565027086) {
  h_26(state, unused, out_3604740557565027086);
}
void car_H_26(double *state, double *unused, double *out_8416658497250234089) {
  H_26(state, unused, out_8416658497250234089);
}
void car_h_27(double *state, double *unused, double *out_4759942856869008909) {
  h_27(state, unused, out_4759942856869008909);
}
void car_H_27(double *state, double *unused, double *out_6979257437319842846) {
  H_27(state, unused, out_6979257437319842846);
}
void car_h_29(double *state, double *unused, double *out_5218144219027570571) {
  h_29(state, unused, out_5218144219027570571);
}
void car_H_29(double *state, double *unused, double *out_4294262781205025751) {
  H_29(state, unused, out_4294262781205025751);
}
void car_h_28(double *state, double *unused, double *out_5703452709770671344) {
  h_28(state, unused, out_5703452709770671344);
}
void car_H_28(double *state, double *unused, double *out_4671724892450627163) {
  H_28(state, unused, out_4671724892450627163);
}
void car_h_31(double *state, double *unused, double *out_1998350867709182113) {
  h_31(state, unused, out_1998350867709182113);
}
void car_H_31(double *state, double *unused, double *out_4644509216499217437) {
  H_31(state, unused, out_4644509216499217437);
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
